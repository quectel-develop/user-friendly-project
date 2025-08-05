#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__
#include "qosa_log.h"
#include "qosa_time.h"
#include "module_info.h"
#include "ql_net.h"

static bool s_global_network_init = false;
static osa_sem_t s_network_sem = NULL;
static void quectel_network_ntp(struct at_client *client, const char *data, size_t size, void *arg)
{
    struct tm tm = {0};
    int tz_offset = 32;
    char tz_sign; // 时区符号（+/-）
    int tz_sign_factor = 1;
    int result = sscanf(data, "+QNTP: %*d,\"%d/%d/%d,%d:%d:%d%c%d\"",
            &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &tz_sign, &tz_offset);
    if (result >= 8)
    {
        tz_sign_factor = (tz_sign == '+') ? 1 : -1;
        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
        tm.tm_hour += tz_sign_factor * tz_offset / 4;
        tm.tm_isdst = -1;  // 自动判断夏令时
        update_ntp_time(mktime(&tm));
        qosa_sem_release(s_network_sem);
    }
}

static const struct at_urc s_network_urc_table[] =
{
	{"+QNTP:", "\r\n", quectel_network_ntp}
};

static int network_connect_module(quectel_network_t handle, u32_t timeout_ms)
{
    int result = 0;
    at_response_t resp = NULL;
    u64_t start_time = 0;

    resp = at_create_resp(128, 0, 2000);
    if (NULL == resp)
    {
        LOG_E("no memory for AT client response object.");
        return -1;
    }
    start_time = time(NULL);

    while (1)
    {
        /* Check whether it is timeout */
        if (time(NULL) - start_time > timeout_ms)
        {
            LOG_E("wait AT client connect timeout(%d tick).", timeout_ms);
            result = -1;
            break;
        }

        at_obj_exec_cmd(handle->client, resp, "AT");
        if (handle->client->resp_status == AT_RESP_OK)
        {
            LOG_I("AT command successful");
            result = 0;
            break;
        }
    }

    at_delete_resp(resp);
    return result;
}

static void network_setting_echo(quectel_network_t handle, int value)
{
    at_response_t resp = NULL;
    resp = at_create_resp(1024, 0, (300));
    if (NULL == resp)
    {
        LOG_E("No memory for AT response.\n");
        return;
    }
    at_obj_exec_cmd(handle->client, resp, "ATE%d", value);
    if (handle->client->resp_status == AT_RESP_OK)
    {
        LOG_I("ATE0 command successful");
    }
    else
    {
        LOG_E("ATE0 command failed");
    }
    at_delete_resp(resp);
}

static void network_query_moudle_type(quectel_network_t handle)
{
    at_response_t query_resp = NULL;
    // Creating a response object for AT command
    query_resp = at_create_resp(256, 0, (500));
    if (NULL == query_resp)
    {
        LOG_E("No memory for response object.");
        return;
    }

    // Sending the AT command to query EMM reject cause value
    if (at_obj_exec_cmd(handle->client, query_resp, "ATI") < 0)
    {
        LOG_E("ATI query failed.");
        at_delete_resp(query_resp);
        return;
    }

    // Parsing the response to extract EMM reject cause value
    for (int i = 0; i < query_resp->line_counts; i++)
    {
        const char *line = at_resp_get_line(query_resp, i + 1);
        if (set_module_type(line))
            break;
    }

    // If the expected format is not found in the response
    at_delete_resp(query_resp);
}

static int network_query_sim_state(quectel_network_t handle)
{
    at_response_t resp;
    const char *line;

    // Creating a response object for AT command
    resp = at_create_resp(1024, 0, (200));
    if (resp == NULL)
    {
        LOG_E("No memory for AT response.\n");
        return -1;
    }

    // Trying up to 3 times to check SIM state
    for (u8_t attempt = 0; attempt < 3; attempt++)
    {
        at_obj_exec_cmd(handle->client, resp, "AT+CPIN?");

        // Scanning each line of the AT command response
        for (int i = 0; i < resp->line_counts; i++)
        {
            line = at_resp_get_line(resp, i + 1);

            // Checking if the line contains the SIM state information
            if (strstr(line, "+CPIN:"))
            {
                if (strstr(line, "READY"))
                {
                    LOG_I("SIM1 Test OK");
                    at_delete_resp(resp);
                    return 0; // SIM is ready
                }
                else
                {
                    LOG_W("SIM1 Test Fail, Try %d", 3 - 1 - attempt);
                    break;
                }
            }
        }

        qosa_task_sleep_ms(2000);
    }
    at_delete_resp(resp);
    return -1;
}

static void network_set_cfun_mode(quectel_network_t handle, u8_t mode)
{
    at_response_t resp;

    if(mode < 0 || mode > 4)
    {
        LOG_W("Invalid CFUN mode!\n");
        return;
    }

    resp = at_create_resp(128, 0, (300));
    if (NULL == resp)
    {
        LOG_E("No memory for response object!\n");
        return;
    }

    LOG_I("Set CFUN mode %d", mode);
    at_obj_exec_cmd(handle->client, resp, "AT+CFUN=%d", mode);

    at_delete_resp(resp);
}

/*
    in LPWA Module:
        1. The command is invalid on BG95-M1 module.
        2. GSM RAT is valid only on BG95-M3, BG95-M5 and BG600L-M3 modules.
    RATs searching sequence, e.g.: 020301 stands for eMTC → NB-IoT →
    GSM.
    00 Automatic (eMTC → NB-IoT → GSM)
    01 GSM
    02 eMTC
    03 NB-IoT

    in LTE Standard Module, Integer type. RATs searching sequence.
    E.g.0102030405 stands for GSM → TDS→ WCDMA → LTE → CDMA.
    Default value: 0403010502.
    00 Automatic
    01 GSM
    02 TDS
    03 WCDMA
    04 LTE
    05 CDMA
    The value of <effect> parameter can only be set to 0
*/
static void network_set_scan_seq(quectel_network_t handle)
{
    if (get_module_type() != MOD_TYPE_BG95)
        return;
    at_response_t query_resp = NULL;
    query_resp = at_create_resp(256, 0, (300));
    int effect = 1;
    if (at_obj_exec_cmd(handle->client, query_resp, "AT+QCFG=\"nwscanseq\",%s,1", handle->scan_seq, effect) < 0)
    {
        LOG_W("set scan seq failed");
    }

    at_delete_resp(query_resp);
}

/*
    in LPWA Module, This command is valid only on BG95-M3, BG95-M5 and BG600L-M3 modules.
    mode type:
    0 Automatic (GSM and LTE)
    1 GSM only
    3 LTE only

    in LTE Standard Module,
    mode type:
    0 AUTO
    1 GSM only
    2 WCDMA only
    3 LTE only
    4 TD-SCDMA only
    5 UMTS only
    6 CDMA only
    7 HDR only
    8 CDMA and HDR only
*/
static void network_set_scan_mode(quectel_network_t handle)
{
    if (get_module_type() == MOD_TYPE_BG95)
        return;
    at_response_t query_resp = NULL;
    query_resp = at_create_resp(256, 0, (300));

    if (at_obj_exec_cmd(handle->client, query_resp, "AT+QCFG=\"nwscanmode\",%d,1", handle->scanmode) < 0)
    {
        //LOG_W("set scan mode failed");
    }

    at_delete_resp(query_resp);
}

/*
    in LPWA Module This command is invalid on BG95-M1 module.
    mode type :
    0 eMTC
    1 NB-IoT
    2 eMTC and NB-IoT

    in LTE Standard Module,  not support
*/
static void network_set_iotop_mode(quectel_network_t handle)
{
    if (get_module_type() != MOD_TYPE_BG95)
        return;
    at_response_t query_resp = NULL;
    query_resp = at_create_resp(256, 0, (300));

    if (at_obj_exec_cmd(handle->client, query_resp, "AT+QCFG=\"iotopmode\",%d,1", handle->iotopmode) < 0)
    {
        LOG_W("set iotop mode failed");
    }

    at_delete_resp(query_resp);
}

static int network_query_register_status(quectel_network_t handle, u32_t timeout_ms)
{
    int status = -1;
    at_response_t query_resp = NULL;
    u64_t start_time;
    query_resp = at_create_resp(128, 0, (500));
    if (NULL == query_resp)
    {
        LOG_E("no memory for AT client response object.");
        return -1;
    }
    start_time = time(NULL);
    while (1)
    {
        /* Check whether it is timeout */
        if (time(NULL) - start_time > timeout_ms)
        {
            LOG_E("query register status failed.");
            break;
        }

        if (at_obj_exec_cmd(handle->client, query_resp, "AT+CEREG?") < 0)
            continue;

        for (int i = 0; i < query_resp->line_counts; i++)
        {
            const char *line = at_resp_get_line(query_resp, i + 1);

            if (sscanf(line, "+CEREG: %*d,%d", &status) == 1)
            {
                if (status == 1 || status == 5)
                {
                    at_delete_resp(query_resp);
                    return 0;
                }
            }
        }
        qosa_task_sleep_ms(1000);
    }

    at_delete_resp(query_resp);
    return -1;
}

static void network_config_pdp(quectel_network_t handle, int contentid)
{
    at_response_t resp;

    resp = at_create_resp(128, 0, (300));
    at_obj_exec_cmd(handle->client, resp, "AT+QICSGP=%d,1,\"%s\",\"%s\",\"%s\"", "", contentid, "", "", "");
    at_delete_resp(resp);

}

static int network_query_actice_state(quectel_network_t handle)
{
    at_response_t resp;
    int ret = -1;
    const char *line;

    resp = at_create_resp(512, 0, (4000));
    if (at_obj_exec_cmd(handle->client, resp, "AT+QIACT?") < 0)
    {
        LOG_E("networkt query active state failed.");
        at_delete_resp(resp);
        return -1;
    }

    for (int i = 0; i < resp->line_counts; i++)
    {
        line = at_resp_get_line(resp, i + 1);

        if (strstr(line, "+QIACT:"))
        {
            if (sscanf(line, "+QIACT: %*d,%*d,%*d,\"%15[^\"]\"", handle->ip) == 1)
            {
                LOG_I("device IP address: %s", handle->ip);
            }
            at_delete_resp(resp);
            ret = 0;
            break;
        }
        else if (strstr(line, "OK"))
        {
            at_delete_resp(resp);
            ret = 1;
            break;
        }

    }

    at_delete_resp(resp);
    return ret;
}

static int networkt_active(quectel_network_t handle)
{
    at_response_t resp;
    const char *line;

    resp = at_create_resp(256, 0, (150*1000));
    if (at_obj_exec_cmd(handle->client, resp, "AT+QIACT=%d", handle->contextid) < 0)
    {
         LOG_E("networkt active failed.");
         at_delete_resp(resp);
        return -2;
    }

    for (int i = 0; i < resp->line_counts; i++)
    {
        line = at_resp_get_line(resp, i + 1);

        if (strstr(line, "OK"))
        {
            break;
        }
        else if (strstr(line, "ERROR"))
        {
            at_delete_resp(resp);
            return -1;
        }
    }

    at_delete_resp(resp);
    return 0;
}

static int networkt_deactive(quectel_network_t handle)
{
    at_response_t resp;
    const char *line;

    resp = at_create_resp(256, 0, (40*1000));
    if (at_obj_exec_cmd(handle->client, resp, "AT+QIDEACT=%d", handle->contextid) < 0)
    {
         LOG_E("networkt deactive failed.");
         at_delete_resp(resp);
        return -2;
    }

    for (int i = 0; i < resp->line_counts; i++)
    {
        line = at_resp_get_line(resp, i + 1);

        if (strstr(line, "OK"))
        {
            break;
        }
        else if (strstr(line, "ERROR"))
        {
            at_delete_resp(resp);
            return -1;
        }
    }

    at_delete_resp(resp);
    return 0;
}

static void network_request_ntp_time(quectel_network_t handle, const char *ntp_server)
{
    at_response_t query_resp = NULL;
    int try;
    query_resp = at_create_resp(128, 0, (125000));
    for (try = 0; try < 3; try++)
    {
        if (at_obj_exec_cmd(handle->client, query_resp, "AT+QNTP=1,\"%s\"", ntp_server) < 0)
        {
            qosa_task_sleep_sec(1);
            continue;
        }
        break;
    }
    qosa_sem_wait(s_network_sem, 30 * 1000);
    at_delete_resp(query_resp);
}

quectel_network_t quectel_network_init(at_client_t client)
{
    quectel_network_t handle = (quectel_network_t)malloc(sizeof(quectel_network_s));
    if (NULL == handle)
    {
        LOG_E("no memory for AT client response object.");
        return NULL;
    }
    handle->client = client;
    handle->echo = 0;
    handle->contextid = 1;
    memcpy(handle->scan_seq, "0203", 4);
    handle->scan_seq[4] = '\0';
    handle->iotopmode = 2;
    memcpy(handle->ip,  "127.0.0.1", strlen("127.0.0.1"));
    handle->ip[strlen("127.0.0.1")] = '\0';
    if (!s_global_network_init)
    {
        at_set_urc_table(s_network_urc_table, sizeof(s_network_urc_table) / sizeof(s_network_urc_table[0]));
        s_global_network_init = true;
    }
    qosa_sem_create(&s_network_sem, 0);
    return handle;
}

void quectel_network_set_param(quectel_network_t handle, QtNetworkParams type, void* value)
{
    if (NULL == handle || NULL == value)
        return;
    switch (type)
    {
    case QT_NETWORK_PARAM_ECHO:
        handle->echo = *(int*)value;
        break;
    case QT_NETWORK_PARAM_SCANMODE:
        handle->scanmode = *(int*)value;
        break;
    case QT_NETWORK_PARAM_SCANSEQ:
        memcpy(handle->scan_seq, value, strlen((char*)value));
        handle->scan_seq[strlen((char*)value)] = '\0';
        break;
    case QT_NETWORK_PARAM_IOTOPMODE:
        handle->iotopmode = *(int*)value;
        break;
    case QT_NETWORK_PARAM_SAVE:
        handle->save = *(int*)value;
        break;
    default:
        break;
    }
}

QtNetworkErrCode quectel_network_attach(quectel_network_t handle)
{
    QtNetworkErrCode status = QT_NETWORK_OK;
    int ret = 0;
    int i = 0;

    if (NULL == handle)
        return QT_NETWORK_ERR_NOINIT;
    if (network_connect_module(handle, 2000) != 0)
        return QT_NETWORK_ERR_CONNECT;
    network_setting_echo(handle, handle->echo);
    network_query_moudle_type(handle);
    status = QT_NETWORK_ERR_CPIN;
    for (i = 0; i < 3; i++)
    {
        if (network_query_sim_state(handle) == 0)
        {
            status = QT_NETWORK_OK;
            break;
        }
        network_set_cfun_mode(handle, 4);
        network_set_cfun_mode(handle, 1);
    }
    if (status != QT_NETWORK_OK)
    {
        LOG_E("quectel_network_attach cpin error");
        return status;
    }
    network_set_scan_seq(handle);
    // network_set_scan_mode(handle);
    network_set_iotop_mode(handle);
    if (network_query_register_status(handle, 60 * 1000) != 0)
    {
        network_config_pdp(handle, handle->contextid);
        network_set_cfun_mode(handle, 4);
        network_set_cfun_mode(handle, 1);
    }

    if (network_query_actice_state(handle) != 0)
    {
        ret = networkt_active(handle);
        if (-2 == ret) // active timeout, reset the module
            return QT_NETWORK_ERR_ACTIVE;
        else if (-1 == ret)
        {
            // three deactivation failures, reset the module
            for (i = 0; i < 3; i++)
            {
                ret = networkt_deactive(handle);
                if (-2 == ret) // deactive, timeout, reset the module
                    return QT_NETWORK_ERR_DEACTIVE;
                else if (0 == ret)
                    break;
            }
            if (ret != 0)
                return QT_NETWORK_ERR_DEACTIVE;

            if (networkt_active(handle) != 0)
                return QT_NETWORK_ERR_ACTIVE;
        }
    }

    network_query_actice_state(handle);
    network_request_ntp_time(handle, "ntp.aliyun.com");
    return QT_NETWORK_OK;
}

int quectel_network_get_rssi(quectel_network_t handle)
{
    int rssi = -1;
    at_response_t query_resp = NULL;
    query_resp = at_create_resp(128, 0, (500));
    if (at_obj_exec_cmd(handle->client, query_resp, "AT+CSQ") < 0)
    {
        LOG_E("get network rssi failed");
        return -1;
    }
    for (int i = 0; i < query_resp->line_counts; i++)
    {
        const char *line = at_resp_get_line(query_resp, i + 1);

        if (sscanf(line, "+CSQ: %d,*d", &rssi) == 1)
        {
            break;
        }
    }

    at_delete_resp(query_resp);
    return rssi;
}

char* quectel_network_get_ip(quectel_network_t handle)
{
    if (NULL == handle ||  strlen(handle->ip) == 0)
        return NULL;
    return handle->ip;
}

void quectel_module_reboot(quectel_network_t handle)
{
    at_response_t resp = NULL;
    resp = at_create_resp(128, 0, (2000));

    LOG_E("Restart module");
    at_obj_exec_cmd(handle->client, resp, "AT+CFUN=1,1");
    at_delete_resp(resp);
}

void quectel_network_detach(quectel_network_t handle)
{

}

void quectel_network_deinit(quectel_network_t handle)
{
    qosa_sem_delete(s_network_sem);
    if (handle != NULL)
    {
        free(handle);
    }
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__ */
