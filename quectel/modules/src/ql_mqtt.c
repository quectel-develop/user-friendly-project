#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__
#include <stdarg.h>
#include "qosa_log.h"
#include "ql_module_compat.h"
#include "ql_mqtt.h"

#define MQTT_MAX_CLIENT_IDX 6
static bool s_global_mqtt_init = false;
static const char* s_mqtt_client_option_strings[] =
{
    [QL_MQTT_OPT_CONTEXT_ID]         = "pdpcid",
    [QL_MQTT_OPT_WILL]           = "will",
    [QL_MQTT_OPT_TIMEOUT]        = "timeout",
    [QL_MQTT_OPT_CLEAN_SESSION]  = "session",
    [QL_MQTT_OPT_KEEP_ALIVETIME] = "keepalive",
    [QL_MQTT_OPT_SSL_ENABLE]     = "ssl",
    [QL_MQTT_OPT_ALI_AUTH]       = "aliauth",
    [QL_MQTT_OPT_RECV_MODE]       = "recv/mode"
};

typedef struct ql_mqtt_instance
{
    ql_mqtt_t handle;
    bool used;
} ql_mqtt_instance_s;

static ql_mqtt_instance_s s_mqtt_client_indices[MQTT_MAX_CLIENT_IDX] = {0};
static osa_mutex_t s_idx_lock = NULL; // never delete

static int ql_mqtt_take_vaild_idx(ql_mqtt_t handle)
{
    qosa_mutex_lock(s_idx_lock, QOSA_WAIT_FOREVER);
    for (int i = 0; i < MQTT_MAX_CLIENT_IDX; i++)
    {
        if (!s_mqtt_client_indices[i].used)
        {
            s_mqtt_client_indices[i].used = true; // Mark as used
            s_mqtt_client_indices[i].handle = handle;
            qosa_mutex_unlock(s_idx_lock);
            return i; // Return first available index
        }
    }
    qosa_mutex_unlock(s_idx_lock);
    return -1;
}
static void ql_mqtt_release_idx(int idx)
{
    qosa_mutex_lock(s_idx_lock, QOSA_WAIT_FOREVER);
    if (idx >= 0 && idx < MQTT_MAX_CLIENT_IDX)
    {
        s_mqtt_client_indices[idx].used = false;
    }
    qosa_mutex_unlock(s_idx_lock);
}
/**
 * @brief find handle by index
 * @param idx index
 * @return success handle，otherwise NULL
 */
static ql_mqtt_t ql_mqtt_find_handle_by_idx(int idx)
{
    qosa_mutex_lock(s_idx_lock, QOSA_WAIT_FOREVER);
    if (idx >= 0 && idx < MQTT_MAX_CLIENT_IDX &&
        s_mqtt_client_indices[idx].used)
    {
        qosa_mutex_unlock(s_idx_lock);
        return s_mqtt_client_indices[idx].handle;
    }
    qosa_mutex_unlock(s_idx_lock);
    return NULL;
}

static void ql_mqtt_urc_open(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    int err = 0;
	sscanf(data, "+QMTOPEN: %d,%d", &client_idx, &err);
    ql_mqtt_t handle = ql_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    if (err != 0)
        handle->status = QL_MQTT_STATUS_CLOSED;
    else
        handle->status = QL_MQTT_STATUS_OPEN;
    qosa_sem_release(handle->sem);
}

static void ql_mqtt_urc_connect(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    int err = 0;
	sscanf(data, "+QMTCONN: %d,%d", &client_idx, &err);
    ql_mqtt_t handle = ql_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    if (0 == err)
        handle->status = QL_MQTT_STATUS_CONNECTED;
    qosa_sem_release(handle->sem);
}

static void ql_mqtt_urc_disconnect(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    int err = 0;
	sscanf(data, "+QMTDISC: %d,%d", &client_idx, &err);
    ql_mqtt_t handle = ql_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    if (0 == err)
        handle->status = QL_MQTT_STATUS_DISCONNECTED;
    qosa_sem_release(handle->sem);
}

static void ql_mqtt_urc_close(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    int err = 0;
	sscanf(data, "+QMTCLOSE: %d,%d", &client_idx, &err);
    ql_mqtt_t handle = ql_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    handle->status = QL_MQTT_STATUS_CLOSED;
}

static void ql_mqtt_urc_stat(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    int err = 0;
	sscanf(data, "+QMTSTAT: %d,%d", &client_idx, &err);
    ql_mqtt_t handle = ql_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    if (5 == err || 1 == err)
        handle->status = QL_MQTT_STATUS_CLOSED;
    qosa_sem_release(handle->sem);
}

static void ql_mqtt_urc_pub(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    int msg_id = -1;
    int err = -1;
    int value = -1;
    int pos = -1;
    if (sscanf(data, "+QMTPUB: %d,%d,%d%n", &client_idx, &msg_id, &err, &pos) >= 3)
    {
        if (data[pos] == ',' && 1 == err)
        {
            sscanf(data + pos + 1, "%d", &value);
        }
    }
    ql_mqtt_t handle = ql_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    qosa_sem_release(handle->sem);
}

static void ql_mqtt_urc_sub(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    int msg_id = -1;
    int err = -1;
    int value = -1;
    int pos = -1;
    if (sscanf(data, "+QMTSUB: %d,%d,%d%n", &client_idx, &msg_id, &err, &pos) >= 3)
    {
        if (data[pos] == ',' && err != 2)
        {
            sscanf(data + pos + 1, "%d", &value);
        }
    }
    ql_mqtt_t handle = ql_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    qosa_sem_release(handle->sem);
}

static void ql_mqtt_urc_recv(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    // int msg_id = -1;
	char *topic;
	size_t msg_len;
	char tmp[4];
	char *msg;
    ql_mqtt_t handle = NULL;
	char *data_begin = strstr(data, ": ") + 2;
	char *data_end = strstr(data_begin, ",");
	memset(tmp, 0, 4);
	strncpy(tmp, data_begin, data_end - data_begin);
	client_idx = atoi(tmp);

	data_begin = data_end + 1;
	data_end = strstr(data_begin, ",");
	memset(tmp, 0, 4);
	strncpy(tmp, data_begin, data_end - data_begin);
	// msg_id = atoi(tmp);

	data_begin = data_end + 1 + 1;
	data_end = strstr(data_begin, "\"");
	*data_end = 0;
	topic = data_begin;
	data_end += 2;

	data_begin = data_end;
	data_end = strstr(data_begin, ",");
	memset(tmp, 0, 4);
	strncpy(tmp, data_begin, data_end - data_begin);
	msg_len = atoi(tmp);

	data_begin = data_end + 2;
	data_end = strstr(data_begin, "\"");
	*data_end = 0;
	msg = data_begin;

    handle = ql_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    if (handle->sub_cb != NULL)
        handle->sub_cb(topic, msg_len, msg, handle->user_data);
}

static const struct at_urc s_mqtt_urc_table[] =
{
	{"+QMTOPEN:", "\r\n", ql_mqtt_urc_open},
	{"+QMTCONN:", "\r\n", ql_mqtt_urc_connect},
	{"+QMTDISC:", "\r\n", ql_mqtt_urc_disconnect},
    {"+QMTCLOSE:", "\r\n", ql_mqtt_urc_close},
	{"+QMTSTAT:", "\r\n", ql_mqtt_urc_stat},
	{"+QMTPUB:", "\r\n", ql_mqtt_urc_pub},
	{"+QMTSUB:", "\r\n", ql_mqtt_urc_sub},
	{"+QMTRECV:", "\r\n", ql_mqtt_urc_recv},
};

ql_mqtt_t ql_mqtt_create(at_client_t client)
{
    ql_mqtt_t handle = (ql_mqtt_t)malloc(sizeof(ql_mqtt_s));
    if (NULL == handle)
    {
        LOG_E("no memory for mqtt client.");
        return NULL;
    }
    handle->client = client;
    handle->client_idx = ql_mqtt_take_vaild_idx(handle);
    if (handle->client_idx < 0 || handle->client_idx > MQTT_MAX_CLIENT_IDX)
    {
        LOG_E("no available client index.");
        free(handle);
        return NULL;
    }
    if (!s_global_mqtt_init)
    {
        qosa_mutex_create(&s_idx_lock);
        at_set_urc_table(s_mqtt_urc_table, sizeof(s_mqtt_urc_table) / sizeof(s_mqtt_urc_table[0]));
        s_global_mqtt_init = true;
    }
    handle->mid = 1;
    handle->pkt_timeout = 5;
    handle->retry_times = 3;
    handle->status = QL_MQTT_STATUS_CLOSED;
    handle->ssl.sslenble = 0;
    handle->ssl.ssltype = 0;
    handle->ssl.sslctxid = 0;
    handle->ssl.ciphersuite = 0xFF;
    handle->ssl.seclevel = SEC_LEVEL_SERVER_AUTHENTICATION;
    handle->ssl.sslversion = SSL_VERSION_ALL;
    qosa_sem_create(&handle->sem, 0);
    qosa_mutex_create(&handle->lock);
    ql_mqtt_setopt(handle, QL_MQTT_OPT_RECV_MODE, true);
    return handle;
}

bool ql_mqtt_setopt(ql_mqtt_t handle, QL_MQTT_OPTION_E option, ...)
{
    va_list arg;
    at_response_t resp = NULL;
    bool ret = false;
    char *option_content = NULL;
    if (NULL == handle)
        return QL_MQTT_ERR_NOINIT;
    resp = at_create_resp_new(256, 0, 200, handle);
    va_start(arg, option);

    switch (option)
    {
        case QL_MQTT_OPT_CONTEXT_ID:
        {
            uint8_t value = (uint8_t)va_arg(arg, int);
            if (value < 1 || value > 16)
                break;
            option_content = (char*)malloc(12);
            snprintf(option_content, 12, "%d", value);
            ret = true;
            break;
        }
        case QL_MQTT_OPT_WILL:
        {
            qt_mqtt_will_options_s *will = va_arg(arg, qt_mqtt_will_options_s*);
            if (NULL == will)
                break;
            option_content = (char*)malloc(1024);
            if (will->flag)
                snprintf(option_content, 1024, "%d,%d,%d,\"%s\",\"%s\"", will->flag, will->qos, will->retain, will->topic, will->msg);
            else
                snprintf(option_content, 1024, "0");
            ret = true;
            break;
        }
        case QL_MQTT_OPT_TIMEOUT:
        {
            qt_mqtt_timeout_options_s *timeout = va_arg(arg, qt_mqtt_timeout_options_s*);
            if (NULL == timeout || timeout->pkt_timeout < 1|| timeout->pkt_timeout > 60
                                || timeout->retry_times < 0 || timeout->retry_times > 10)
                break;
            handle->pkt_timeout = timeout->pkt_timeout;
            handle->retry_times = timeout->retry_times;
            option_content = (char*)malloc(16);
            snprintf(option_content, 16, "%d,%d,%d", timeout->pkt_timeout, timeout->retry_times, timeout->timeout_notice);
            ret = true;
            break;
        }
        case QL_MQTT_OPT_CLEAN_SESSION:
        {
            bool value = (bool)va_arg(arg, int);
            option_content = (char*)malloc(4);
            snprintf(option_content, 4, "%d", value);
            ret = true;
            break;
        }
        case QL_MQTT_OPT_KEEP_ALIVETIME:
        {
            int value = va_arg(arg, int);
            if (value < 0 || value > 3600)
                break;
            option_content = (char*)malloc(12);
            snprintf(option_content, 122, "%d", value);
            ret = true;
            break;
        }
        case QL_MQTT_OPT_SSL_ENABLE:
        {
            qt_mqtt_ssl_options_s *ssl = va_arg(arg, qt_mqtt_ssl_options_s*);
            if (NULL == ssl || (ssl->ssl_enable && (ssl->ssl_ctx_id > 5 || ssl->ssl_ctx_id < 0)))
                break;
            option_content = (char*)malloc(8);
            snprintf(option_content, 8, "%d,%d", ssl->ssl_enable, ssl->ssl_ctx_id);
            ret = true;
            break;
        }
        case QL_MQTT_OPT_ALI_AUTH:
        {
            qt_mqtt_ali_auth_options_s *auth = va_arg(arg, qt_mqtt_ali_auth_options_s*);
            if (NULL == auth || NULL == auth->product_key || NULL == auth->device_name || NULL == auth->device_secret)
                break;
            option_content = (char*)malloc(1024);
            snprintf(option_content, 1024, "\"%s\",\"%s\",\"%s\"", auth->product_key, auth->device_name, auth->device_secret);
            ret = true;
            break;
        }
        case QL_MQTT_OPT_RECV_MODE:
        {
            bool value = (bool)va_arg(arg, int);
            option_content = (char*)malloc(4);
            snprintf(option_content, 4, "%d,%d", 0, value);
            ret = true;
            break;
        }
        case QL_MQTT_OPT_SUB_CALLBACK:
            handle->sub_cb = va_arg(arg, ql_sub_cb_func);
            at_delete_resp(resp);
            va_end(arg);
            return true;
        case QL_MQTT_OPT_USER_DATA:
            handle->user_data = va_arg(arg, void*);
            at_delete_resp(resp);
            va_end(arg);
            return true;
        default:
            ret = false;
            LOG_W("unknown option %d", option);
            break;
    }
    if (ret)
    {
        if (at_obj_exec_cmd(handle->client, resp, "AT+QMTCFG=\"%s\",%d,%s", s_mqtt_client_option_strings[option], handle->client_idx, option_content) < 0)
            ret = false;
    }
    at_delete_resp(resp);
    if (option_content)
        free(option_content);
    va_end(arg);
    return ret;
}

bool ql_mqtt_set_ssl(ql_mqtt_t handle, ql_SSL_Config config)
{
    handle->ssl = config;
    qt_mqtt_ssl_options_s options;
    options.ssl_ctx_id = config.sslctxid;
    options.ssl_enable = config.sslenble;
    if (NULL == handle->ssl.cacert_dst_path)
        handle->ssl.cacert_dst_path = "mqtt_ca.pem";
    if (NULL == handle->ssl.clientcert_dst_path)
        handle->ssl.clientcert_dst_path = "mqtt_user.pem";
    if (NULL == handle->ssl.clientkey_dst_path)
        handle->ssl.clientkey_dst_path = "mqtt_user_key.pem";
    handle->ssl.client = handle->client;
    ql_mqtt_setopt(handle, QL_MQTT_OPT_SSL_ENABLE, &options);
    return (configure_ssl(&handle->ssl) == 0) ? true : false;
}

QL_MQTT_ERR_CODE_E ql_mqtt_connect(ql_mqtt_t handle, const char* server, int port, const char* username, const char* password)
{
    if (NULL == handle)
        return QL_MQTT_ERR_NOINIT;
    int ret = -1;

    if (!handle->ssl.sslenble)
    {
        qt_mqtt_ssl_options_s options;
        options.ssl_ctx_id = 0;
        options.ssl_enable = false;
        ql_mqtt_setopt(handle, QL_MQTT_OPT_SSL_ENABLE, &options);
    }
    at_response_t resp = at_create_resp_new(128, 0, 3000, handle);
	if (at_obj_exec_cmd(handle->client, resp, "AT+QMTOPEN=%d,\"%s\",%d", handle->client_idx, server, port) < 0)
    {
        at_delete_resp(resp);
        return QL_MQTT_ERR_OPEN;
    }
    qosa_sem_wait(handle->sem, 121 * 1000);
    at_delete_resp(resp);
    if (handle->status != QL_MQTT_STATUS_OPEN)
        return QL_MQTT_ERR_OPEN;

    resp = at_create_resp_new(128, 0, 3000, handle);
    if (NULL == username || NULL == password)
    {
        ret = at_obj_exec_cmd(handle->client, resp, "AT+QMTCONN=%d,\"client_%d\"", handle->client_idx, handle->client_idx);
    }
    else
    {
        ret = at_obj_exec_cmd(handle->client, resp, "AT+QMTCONN=%d,\"client_%d\",\"%s\",\"%s\"", handle->client_idx, handle->client_idx, username, password);
    }
    if (ret < 0)
    {
        at_delete_resp(resp);
        return QL_MQTT_ERR_CONNECT;
    }
    qosa_sem_wait(handle->sem, (handle->pkt_timeout + 1) * 1000);
    at_delete_resp(resp);
    if (handle->status != QL_MQTT_STATUS_CONNECTED)
        return QL_MQTT_ERR_CONNECT;
    return QL_MQTT_OK;
}

void ql_mqtt_disconnect(ql_mqtt_t handle)
{
    if (NULL == handle)
        return;
    if (handle->status == QL_MQTT_STATUS_CLOSED)
        return;
    at_response_t resp = at_create_resp_new(128, 0, 3000, handle);
    handle->status = QL_MQTT_STATUS_DISCONNECTING;
    at_obj_exec_cmd(handle->client, resp, "AT+QMTDISC=%d", handle->client_idx);
    qosa_sem_wait(handle->sem, 31 * 1000);// wait disconnect
    if (ql_use_mqtt_close())
    {
        if (handle->status == QL_MQTT_STATUS_DISCONNECTED)
        {
            qosa_sem_wait(handle->sem, 180 * 1000);// wait stat
        }
        if (handle->status != QL_MQTT_STATUS_CLOSED) 
        {
            resp = at_resp_set_info_new(resp, 128, 0, 3000, handle);
            at_obj_exec_cmd(handle->client, resp, "AT+QMTCLOSE=%d", handle->client_idx);
        }
    }
    else
    {
        handle->status = QL_MQTT_STATUS_CLOSED;
    }
    at_delete_resp(resp);
}

QL_MQTT_ERR_CODE_E ql_mqtt_pub(ql_mqtt_t handle, const char *topic, const char *message, QL_MQTT_QOS_LEVEL_E qos, bool retain)
{
    if (NULL == handle)
        return QL_MQTT_ERR_NOINIT;
    if (handle->status == QL_MQTT_STATUS_CLOSED)
    {
        LOG_E("mqtt closed");
        return QL_MQTT_ERR_PUB;
    }
    at_response_t resp = at_create_resp_new(128, 0, 3000, handle);
    const char *cmd = NULL;
    uint16_t mid = 0;
    if (ql_use_mqtt_pubex())
    {
        cmd = "AT+QMTPUBEX";
    }
    else
    {
        cmd = "AT+QMTPUB";
    }
    if (qos != QL_MQTT_QOS0)
    {
        qosa_mutex_lock(handle->lock, QOSA_WAIT_FOREVER);
        mid = handle->mid++;
        handle->mid = handle->mid == 0 ? 1 : handle->mid;
        qosa_mutex_unlock(handle->lock);
    }
    if (at_obj_exec_cmd(handle->client, resp, "%s=%d,%d,%d,%d,\"%s\",\"%s\"", cmd, handle->client_idx, mid, qos, retain, topic, message) < 0)
        return QL_MQTT_ERR_PUB;
    qosa_sem_wait(handle->sem, (handle->pkt_timeout * handle->retry_times+1) * 1000);
    at_delete_resp(resp);

    return QL_MQTT_OK;
}

QL_MQTT_ERR_CODE_E ql_mqtt_sub(ql_mqtt_t handle, const char *topic, QL_MQTT_QOS_LEVEL_E qos)
{
    if (NULL == handle)
        return QL_MQTT_ERR_NOINIT;
    if (handle->status == QL_MQTT_STATUS_CLOSED)
    {
        LOG_E("mqtt closed");
        return QL_MQTT_ERR_SUB;
    }
    at_response_t resp = at_create_resp_new(128, 0, 3000, handle);
    uint16_t mid = 1;
    qosa_mutex_lock(handle->lock, QOSA_WAIT_FOREVER);
    mid = handle->mid++;
    handle->mid = handle->mid == 0 ? 1 : handle->mid;
    qosa_mutex_unlock(handle->lock);
    at_obj_exec_cmd(handle->client, resp, "AT+QMTSUB=%d,%d,\"%s\",%d", handle->client_idx, mid, topic, qos);
    qosa_sem_wait(handle->sem, (handle->pkt_timeout * handle->retry_times+1) * 1000);
    at_delete_resp(resp);

    return QL_MQTT_OK;
}

QL_MQTT_ERR_CODE_E ql_mqtt_unsub(ql_mqtt_t handle, const char *topic)
{
    if (NULL == handle)
        return QL_MQTT_ERR_NOINIT;
    at_response_t resp = at_create_resp_new(128, 0, 3000, handle);
    uint16_t mid = 1;
    qosa_mutex_lock(handle->lock, QOSA_WAIT_FOREVER);
    mid = handle->mid++;
    handle->mid = handle->mid == 0 ? 1 : handle->mid;
    qosa_mutex_unlock(handle->lock);
    at_obj_exec_cmd(handle->client, resp, "AT+QMTUNS=%d,%d,\"%s\"", handle->client_idx, mid, topic);
    qosa_sem_wait(handle->sem, (handle->pkt_timeout * handle->retry_times+1) * 1000);
    at_delete_resp(resp);
    return QL_MQTT_OK;
}

void ql_mqtt_destroy(ql_mqtt_t handle)
{
    if (NULL == handle)
        return;
    ql_mqtt_release_idx(handle->client_idx);
    qosa_sem_delete(handle->sem);
    qosa_mutex_delete(handle->lock);
    free(handle);
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__ */
