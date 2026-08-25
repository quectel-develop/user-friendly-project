#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__
#include <stdarg.h>
#include "qosa_log.h"
#include "qosa_time.h"
#include "ql_module_compat.h"
#include "ql_net.h"

static bool s_global_net_init = false;
static osa_sem_t s_net_sem = NULL;
static void ql_net_ntp(struct at_client *client, const char *data, size_t size, void *arg)
{
    struct tm tm = {0};
    int tz_offset = 32;
    char tz_sign; // Time zone symbol (+/-)
    int tz_sign_factor = 1;
    int result = sscanf(data, "+QNTP: %*d,\"%d/%d/%d,%d:%d:%d%c%d\"",
            &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &tz_sign, &tz_offset);
    if (result >= 8)
    {
        tz_sign_factor = (tz_sign == '+') ? 1 : -1;
        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
        tm.tm_hour += tz_sign_factor * tz_offset / 4;
        tm.tm_isdst = -1;  // Automatically judge DST
        update_ntp_time(mktime(&tm));
    }
    else 
        LOG_W("NTP failed.");
    qosa_sem_release(s_net_sem);
}

static const struct at_urc s_net_urc_table[] =
{
	{"+QNTP:", "\r\n", ql_net_ntp}
};


static int ql_net_query_sim_state(ql_net_t handle)
{
    at_response_t resp;
    const char *line;

    // Creating a response object for AT command
    resp = at_create_resp(1024, 0, (3000));
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
                    LOG_I("SIM card detected successfully");
                    at_delete_resp(resp);
                    return 0; // SIM is ready
                }
                else
                {
                    LOG_W("SIM card detected failed, Try %d", 3 - 1 - attempt);
                    break;
                }
            }
        }

        qosa_task_sleep_ms(2000);
    }
    at_delete_resp(resp);
    return -1;
}

static void ql_net_set_cfun_mode(ql_net_t handle, u8_t mode)
{
    at_response_t resp;

    if(mode < 0 || mode > 4)
    {
        LOG_W("Invalid CFUN mode!\n");
        return;
    }

    resp = at_create_resp(128, 0, (15 * 1000));
    if (NULL == resp)
    {
        LOG_E("No memory for response object!\n");
        return;
    }

    LOG_I("Set CFUN mode %d", mode);
    at_obj_exec_cmd(handle->client, resp, "AT+CFUN=%d", mode);

    at_delete_resp(resp);
}

static int ql_net_check_creg(ql_net_t handle)
{
    int status = -1;
    at_response_t query_resp = NULL;
    query_resp = at_create_resp_new(128, 0, (3000), NULL);
    if (at_obj_exec_cmd(handle->client, query_resp, "AT+CREG?") < 0)
    {
        at_delete_resp(query_resp);
        return -1;
    }

    for (int i = 0; i < query_resp->line_counts; i++)
    {
        const char *line = at_resp_get_line(query_resp, i + 1);

        if (sscanf(line, "+CREG: %*d,%d", &status) == 1)
        {
            if (status == 1 || status == 5)
            {
                at_delete_resp(query_resp);
                return 0;
            }
        }
    }

    at_delete_resp(query_resp);
    return -1;
}

static int ql_net_check_cereg(ql_net_t handle)
{
    int status = -1;
    at_response_t query_resp = NULL;
    query_resp = at_create_resp_new(128, 0, (3000), NULL);
    if (at_obj_exec_cmd(handle->client, query_resp, "AT+CEREG?") < 0)
    {
        at_delete_resp(query_resp);
        return -1;
    }

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

    at_delete_resp(query_resp);
    return -1;
}

static int ql_net_query_register_status(ql_net_t handle, u32_t timeout)
{
    u64_t start_time;
    start_time = time(NULL);
    while (1)
    {
        if (time(NULL) - start_time > timeout)
        {
            LOG_E("Register status query failed.");
            break;
        }
        int ret1 = ql_net_check_creg(handle);
        int ret2 = ql_net_check_cereg(handle);
        if (0 == ret1 || 0 == ret2)
        {
            LOG_I("Network registration successful");
            return 0;
        }
        qosa_task_sleep_ms(2000);
    }

    return -1;
}

static int ql_net_query_actice_state(ql_net_t handle)
{
    at_response_t resp;
    int ret = -1;
    const char *line;

    resp = at_create_resp(512, 0, (4000));
    if (at_obj_exec_cmd(handle->client, resp, "AT+QIACT?") < 0)
    {
        LOG_E("network query active state failed.");
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

static int ql_net_active(ql_net_t handle)
{
    at_response_t resp;
    const char *line;

    resp = at_create_resp(256, 0, (150*1000));
    if (at_obj_exec_cmd(handle->client, resp, "AT+QIACT=%d", handle->contextid) < 0)
    {
         LOG_E("network active failed.");

        for (int i = 0; i < resp->line_counts; i++)
        {
            line = at_resp_get_line(resp, i + 1);
            if (strstr(line, "ERROR"))
            {
                at_delete_resp(resp);
                return -1;
            }
        }
         at_delete_resp(resp);
        return -2;
    }

    at_delete_resp(resp);
    return 0;
}

static int ql_net_deactive(ql_net_t handle)
{
    at_response_t resp;
    const char *line;

    resp = at_create_resp(256, 0, (40*1000));
    if (at_obj_exec_cmd(handle->client, resp, "AT+QIDEACT=%d", handle->contextid) < 0)
    {
         LOG_E("network deactive failed.");
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

static void net_request_ntp_time(ql_net_t handle, const char *ntp_server)
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
    qosa_sem_wait(s_net_sem, 30 * 1000);
    at_delete_resp(query_resp);
}

ql_net_t ql_net_init(at_client_t client)
{
    ql_net_t handle = (ql_net_t)malloc(sizeof(ql_net_s));
    if (NULL == handle)
    {
        LOG_E("no memory for AT client response object.");
        return NULL;
    }
    handle->client = client;
    handle->contextid = 1;
    memcpy(handle->ip,  "127.0.0.1", strlen("127.0.0.1"));
    handle->ip[strlen("127.0.0.1")] = '\0';
    if (!s_global_net_init)
    {
        at_set_urc_table(s_net_urc_table, sizeof(s_net_urc_table) / sizeof(s_net_urc_table[0]));
        s_global_net_init = true;
    }
    qosa_sem_create(&s_net_sem, 0);
    return handle;
}

QL_NET_ERR_CODE_E ql_usim_get(ql_net_t handle)
{
    if (NULL == handle)
        return QL_NET_ERR_NOINIT;
    if (ql_net_query_sim_state(handle) == 0)
    {
        return QL_NET_OK;
    }
    return QL_NET_ERR_SIM;
}

bool ql_net_set_opt(ql_net_t handle, QL_NET_OPTION_E option, ...)
{
    if (NULL == handle)
        return false;
    char cmd[128];
    va_list args;
    va_start(args, option);
    switch (option)
    {
    case QL_NET_OPTINON_CONTENT:
        ql_net_content_s *content = va_arg(args, ql_net_content_s*);
        handle->contextid = content->contentid;
        snprintf(cmd, sizeof(cmd), "AT+QICSGP=%d,1,\"%s\",\"%s\",\"%s\"", content->contentid,
                        content->apn, content->username, content->password);
        break;
    case QL_NET_OPTINON_SCANMODE:
        if (!ql_support_net_scanmode())
            return false;
        snprintf(cmd, sizeof(cmd), "AT+QCFG=\"nwscanmode\",%d", va_arg(args, int));
        break;
    case QL_NET_OPTINON_SCANSEQ:
        snprintf(cmd, sizeof(cmd), "AT+QCFG=\"nwscanseq\",%s,1", va_arg(args, const char *));
        break;
    case QL_NET_OPTINON_IOTOPMODE:
        if (!ql_support_net_iotopmode())
            return false;
        snprintf(cmd, sizeof(cmd), "AT+QCFG=\"iotopmode\",%d,1", va_arg(args, int));
        break;
    case QL_NET_OPTINON_BAND:
        ql_net_band_s *band = va_arg(args, ql_net_band_s*);
        snprintf(cmd, sizeof(cmd), "AT+QCFG=\"band\",%s,%s,%s", band->band1, band->band2, band->band3);
        break;
    default:
        LOG_W("Unsupported network config option\n");
        break;
    }
    va_end(args);

    at_response_t resp = at_create_resp_new(256, 0, 5000, NULL);

    if (at_exec_cmd(resp, cmd) < 0)
    {
        LOG_E("network config failed.");
        at_delete_resp(resp);
        return false;
    }

    at_delete_resp(resp);
    return true;
}

QL_NET_ERR_CODE_E ql_net_attach(ql_net_t handle)
{
    int ret = 0;

    if (NULL == handle)
        return QL_NET_ERR_NOINIT;
    if (ql_net_query_register_status(handle, 120) != 0)
    {
        ql_net_set_cfun_mode(handle, 0);
        ql_net_set_cfun_mode(handle, 1);
        if (ql_net_query_register_status(handle, 120) != 0)
            return QL_NET_ERR_REGISTER;
    }

    if (ql_net_query_actice_state(handle) != 0)
    {
        for (int i = 0; i < 3; i++)
        {
            ret = ql_net_active(handle);
            if (-2 == ret) // active timeout, reset the module
                return QL_NET_ERR_ACTIVE;
            else if (-1 == ret)
            {
                // three deactivation failures, reset the module
                for (i = 0; i < 3; i++)
                {
                    ret = ql_net_deactive(handle);
                    if (-2 == ret) // deactive, timeout, reset the module
                        return QL_NET_ERR_DEACTIVE;
                    else if (0 == ret)
                        break;
                }
                if (ret != 0)
                    return QL_NET_ERR_DEACTIVE;

                if (ql_net_active(handle) != 0)
                    return QL_NET_ERR_ACTIVE;
            }
            else
                break;
        }
        ql_net_query_actice_state(handle);
    }

    net_request_ntp_time(handle, "pool.ntp.org");
    return QL_NET_OK;
}

int ql_net_get_signal_info(ql_net_t handle, int *strength, int *quality)
{
    if (NULL == handle)
        return -1;
    int result = -1;
    at_response_t query_resp = NULL;
    query_resp = at_create_resp_new(128, 0, (500), NULL);
    if (at_obj_exec_cmd(handle->client, query_resp, "AT+QENG=\"servingcell\"") < 0)
    {
        LOG_E("get network singal strength failed");
        return -1;
    }
    for (int i = 0; i < query_resp->line_counts; i++)
    {
        const char *line = at_resp_get_line(query_resp, i + 1);
        if (strstr(line, "+QENG:") == NULL)
            continue;
        if (strstr(line, "GSM") != NULL)
        {
            result = sscanf(line, "+QENG: \"servingcell\",\"%*[^\"]\",\"%*[^\"]\",%*d,%*d,%*[^,],%*[^,],"
                "%*d,%*d,%*d,%d", strength);
            result += 1;
            *quality = 0;
            LOG_W("GSM does not support signal quality");
        }
        else if (strstr(line, "WCDMA") != NULL)
        {
            result = sscanf(line, "+QENG: \"servingcell\",\"%*[^\"]\",\"%*[^\"]\",%*d,%*d,%*[^,],%*[^,],"
                "%*d,%*d,%*d,%d, %d", strength, quality);
        }
        else
        {
            result = sscanf(line, "+QENG: \"servingcell\",\"%*[^\"]\",\"%*[^\"]\",\"%*[^\"]\",%*d,"
                "%*d,%*[^,],%*d,%*d,%*d,%*d,%*d,%*[^,],%d,%*d,%*d,%d", strength, quality);
        }
    }
    at_delete_resp(query_resp);
    return result == 2 ? 0 : -1;
}

const char* ql_net_get_ip(ql_net_t handle)
{
    if (NULL == handle ||  strlen(handle->ip) == 0)
        return NULL;
    return handle->ip;
}

bool ql_net_is_ok(ql_net_t handle)
{
    int ret1 = ql_net_check_creg(handle);
    int ret2 = ql_net_check_cereg(handle);
    if (0 == ret1 || 0 == ret2)
    {
        return true;
    }
    return false;
}

QL_NET_ERR_CODE_E ql_net_reconnect(ql_net_t handle)
{
    LOG_E("Reset module");
    ql_net_set_cfun_mode(handle, 0);
    ql_net_set_cfun_mode(handle, 1);
    return ql_net_attach(handle);
}

void ql_module_reboot(ql_net_t handle)
{
    at_response_t resp = NULL;
    resp = at_create_resp(128, 0, (2000));

    LOG_E("Restart module");
    at_obj_exec_cmd(handle->client, resp, "AT+CFUN=1,1");
    at_delete_resp(resp);
}

void ql_net_detach(ql_net_t handle)
{

}

void ql_net_deinit(ql_net_t handle)
{
    qosa_sem_delete(s_net_sem);
    if (handle != NULL)
    {
        free(handle);
    }
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__ */
