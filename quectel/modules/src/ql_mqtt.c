#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__
#include <stdarg.h>
#include "qosa_log.h"
#include "module_info.h"
#include "ql_mqtt.h"

#define MQTT_MAX_CLIENT_IDX 6
static bool s_global_mqtt_init = false;
static const char* s_mqtt_client_option_strings[] =
{
    [QT_MQTT_OPT_PDPCID]         = "pdpcid",
    [QT_MQTT_OPT_WILL]           = "will",
    [QT_MQTT_OPT_TIMEOUT]        = "timeout",
    [QT_MQTT_OPT_CLEAN_SESSION]  = "session",
    [QT_MQTT_OPT_KEEP_ALIVETIME] = "keepalive",
    [QT_MQTT_OPT_SSL_ENABLE]     = "ssl",
    [QT_MQTT_OPT_ALI_AUTH]       = "aliauth",
    [QT_MQTT_OPT_RECV_MODE]       = "recv/mode"
};

typedef struct quectel_mqtt_client_instance
{
    quectel_mqtt_client_t handle;
    bool used;
} quectel_mqtt_client_instance_s;

static quectel_mqtt_client_instance_s s_mqtt_client_indices[MQTT_MAX_CLIENT_IDX] = {false};
static osa_mutex_t s_idx_lock = NULL; // never delete

static int quectel_mqtt_take_vaild_idx(quectel_mqtt_client_t handle)
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
static void quectel_mqtt_release_idx(int idx)
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
static quectel_mqtt_client_t quectel_mqtt_find_handle_by_idx(int idx)
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

static void quectel_mqtt_urc_open(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    int err = 0;
	sscanf(data, "+QMTOPEN: %d,%d", &client_idx, &err);
    quectel_mqtt_client_t handle = quectel_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    if (err != 0)
        handle->status = QT_MQTT_STATUS_CLOSED;
    else
        handle->status = QT_MQTT_STATUS_OPEN;
    qosa_sem_release(handle->sem);
}

static void quectel_mqtt_urc_connect(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    int err = 0;
	sscanf(data, "+QMTCONN: %d,%d", &client_idx, &err);
    quectel_mqtt_client_t handle = quectel_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    if (0 == err)
        handle->status = QT_MQTT_STATUS_CONNECTED;
    qosa_sem_release(handle->sem);
}

static void quectel_mqtt_urc_disconnect(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    int err = 0;
	sscanf(data, "+QMTDISC: %d,%d", &client_idx, &err);
    quectel_mqtt_client_t handle = quectel_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    if (0 == err)
        handle->status = QT_MQTT_STATUS_DISCONNECTED;
    // qosa_sem_release(handle->sem);
}

static void quectel_mqtt_urc_close(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    int err = 0;
	sscanf(data, "+QMTCLOSE: %d,%d", &client_idx, &err);
    quectel_mqtt_client_t handle = quectel_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    handle->status = QT_MQTT_STATUS_CLOSED;
}

static void quectel_mqtt_urc_stat(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    int err = 0;
	sscanf(data, "+QMTSTAT: %d,%d", &client_idx, &err);
    quectel_mqtt_client_t handle = quectel_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    if (5 == err || 1 == err)
        qosa_sem_release(handle->sem);
}

static void quectel_mqtt_urc_pub(struct at_client *client, const char *data, size_t size, void *arg)
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
    quectel_mqtt_client_t handle = quectel_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    qosa_sem_release(handle->sem);
}

static void quectel_mqtt_urc_sub(struct at_client *client, const char *data, size_t size, void *arg)
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
    quectel_mqtt_client_t handle = quectel_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    qosa_sem_release(handle->sem);
}

static void quectel_mqtt_urc_recv(struct at_client *client, const char *data, size_t size, void *arg)
{
    int client_idx = -1;
    // int msg_id = -1;
	char *topic;
	size_t msg_len;
	char tmp[4];
	char *msg;
    quectel_mqtt_client_t handle = NULL;
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

    handle = quectel_mqtt_find_handle_by_idx(client_idx);
    if (NULL == handle)
        return;
    if (handle->sub_cb != NULL)
        handle->sub_cb(topic, msg_len, msg, handle->user_data);
}

static const struct at_urc s_mqtt_urc_table[] =
{
	{"+QMTOPEN:", "\r\n", quectel_mqtt_urc_open},
	{"+QMTCONN:", "\r\n", quectel_mqtt_urc_connect},
	{"+QMTDISC:", "\r\n", quectel_mqtt_urc_disconnect},
    {"+QMTCLOSE:", "\r\n", quectel_mqtt_urc_close},
	{"+QMTSTAT:", "\r\n", quectel_mqtt_urc_stat},
	{"+QMTPUB:", "\r\n", quectel_mqtt_urc_pub},
	{"+QMTSUB:", "\r\n", quectel_mqtt_urc_sub},
	{"+QMTRECV:", "\r\n", quectel_mqtt_urc_recv},
};

quectel_mqtt_client_t quectel_mqtt_client_create(at_client_t client)
{
    quectel_mqtt_client_t handle = (quectel_mqtt_client_t)malloc(sizeof(quectel_mqtt_client_s));
    if (NULL == handle)
    {
        LOG_E("no memory for AT client response object.");
        return NULL;
    }
    handle->client = client;
    handle->client_idx = quectel_mqtt_take_vaild_idx(handle);
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
    handle->status = QT_MQTT_STATUS_CLOSED;
    handle->ssl.sslenble = 0;
    handle->ssl.ssltype = 0;
    handle->ssl.sslctxid = 0;
    handle->ssl.ciphersuite = 0xFF;
    handle->ssl.seclevel = SEC_LEVEL_SERVER_AUTHENTICATION;
    handle->ssl.sslversion = SSL_VERSION_ALL;
    qosa_sem_create(&handle->sem, 0);
    qosa_mutex_create(&handle->lock);
    quectel_mqtt_setopt(handle, QT_MQTT_OPT_RECV_MODE, true);
    qt_mqtt_ssl_options_s options;
    options.ssl_ctx_id = 0;
    options.ssl_enable = false;
    quectel_mqtt_setopt(handle, QT_MQTT_OPT_SSL_ENABLE, &options);
    return handle;
}

bool quectel_mqtt_setopt(quectel_mqtt_client_t handle, QtMqttClientOption option, ...)
{
    va_list arg;
    at_response_t resp = NULL;
    bool ret = false;
    char *option_content = NULL;
    if (NULL == handle)
        return QT_MQTT_ERR_NOINIT;
    resp = at_create_resp_new(256, 0, 200, handle);
    va_start(arg, option);

    switch (option)
    {
        case QT_MQTT_OPT_PDPCID:
        {
            uint8_t value = (uint8_t)va_arg(arg, int);
            if (value < 1 || value > 16)
                break;
            option_content = (char*)malloc(12);
            snprintf(option_content, 12, "%d", value);
            ret = true;
            break;
        }
        case QT_MQTT_OPT_WILL:
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
        case QT_MQTT_OPT_TIMEOUT:
        {
            qt_mqtt_timeout_options_s *timeout = va_arg(arg, qt_mqtt_timeout_options_s*);
            if (NULL == timeout || timeout->pkt_timeout < 1|| timeout->pkt_timeout > 60
                                || timeout->retry_times < 0 || timeout->retry_times > 10)
                break;
            option_content = (char*)malloc(16);
            snprintf(option_content, 16, "%d,%d,%d", timeout->pkt_timeout, timeout->retry_times, timeout->timeout_notice);
            ret = true;
            break;
        }
        case QT_MQTT_OPT_CLEAN_SESSION:
        {
            bool value = (bool)va_arg(arg, int);
            option_content = (char*)malloc(4);
            snprintf(option_content, 4, "%d", value);
            ret = true;
            break;
        }
        case QT_MQTT_OPT_KEEP_ALIVETIME:
        {
            int value = va_arg(arg, int);
            if (value < 0 || value > 3600)
                break;
            option_content = (char*)malloc(12);
            snprintf(option_content, 122, "%d", value);
            ret = true;
            break;
        }
        case QT_MQTT_OPT_SSL_ENABLE:
        {
            qt_mqtt_ssl_options_s *ssl = va_arg(arg, qt_mqtt_ssl_options_s*);
            if (NULL == ssl || (ssl->ssl_enable && (ssl->ssl_ctx_id > 5 || ssl->ssl_ctx_id < 0)))
                break;
            option_content = (char*)malloc(8);
            snprintf(option_content, 8, "%d,%d", ssl->ssl_enable, ssl->ssl_ctx_id);
            ret = true;
            break;
        }
        case QT_MQTT_OPT_ALI_AUTH:
        {
            qt_mqtt_ali_auth_options_s *auth = va_arg(arg, qt_mqtt_ali_auth_options_s*);
            if (NULL == auth || NULL == auth->product_key || NULL == auth->device_name || NULL == auth->device_secret)
                break;
            option_content = (char*)malloc(1024);
            snprintf(option_content, 1024, "\"%s\",\"%s\",\"%s\"", auth->product_key, auth->device_name, auth->device_secret);
            ret = true;
            break;
        }
        case QT_MQTT_OPT_RECV_MODE:
        {
            bool value = (bool)va_arg(arg, int);
            option_content = (char*)malloc(4);
            snprintf(option_content, 4, "%d,%d", 0, value);
            ret = true;
            break;
        }
        case QT_MQTT_OPT_SUB_CALLBACK:
            handle->sub_cb = va_arg(arg, quectel_sub_cb_func);
            at_delete_resp(resp);
            va_end(arg);
            return true;
        case QT_MQTT_OPT_USER_DATA:
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

void quectel_mqtt_set_ssl(quectel_mqtt_client_t handle, ql_SSL_Config config)
{
    handle->ssl = config;
    qt_mqtt_ssl_options_s options;
    options.ssl_ctx_id = config.sslctxid;
    options.ssl_enable = config.sslenble;
    handle->ssl.cacert_path = "mqtt_ca.pem";
    handle->ssl.clientcert_path = "mqtt_user.pem";
    handle->ssl.clientkey_path = "mqtt_user_key.pem";
    quectel_mqtt_setopt(handle, QT_MQTT_OPT_SSL_ENABLE, &options);
    configure_ssl(&handle->ssl);
}

QtMqttErrCode quectel_mqtt_connect(quectel_mqtt_client_t handle, const char* server, int port, const char* username, const char* password)
{
    if (NULL == handle)
        return QT_MQTT_ERR_NOINIT;
    int ret = -1;

    at_response_t resp = at_create_resp_new(128, 0, 3000, handle);
	if (at_obj_exec_cmd(handle->client, resp, "AT+QMTOPEN=%d,\"%s\",%d", handle->client_idx, server, port) < 0)
    {
        at_delete_resp(resp);
        return QT_MQTT_ERR_OPEN;
    }
    qosa_sem_wait(handle->sem, 20000);
    at_delete_resp(resp);
    if (handle->status != QT_MQTT_STATUS_OPEN)
        return QT_MQTT_ERR_OPEN;

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
        return QT_MQTT_ERR_CONNECT;
    }
    qosa_sem_wait(handle->sem, 20000);
    at_delete_resp(resp);
    if (handle->status != QT_MQTT_STATUS_CONNECTED)
        return QT_MQTT_ERR_CONNECT;
    return QT_MQTT_OK;
}

void quectel_mqtt_disconnect(quectel_mqtt_client_t handle)
{
    if (NULL == handle)
        return;
    at_response_t resp = at_create_resp_new(128, 0, 3000, handle);
    at_obj_exec_cmd(handle->client, resp, "AT+QMTDISC=%d", handle->client_idx);
    if (qosa_sem_wait(handle->sem, 5 * 1000) != QOSA_OK)
    {
        resp = at_resp_set_info_new(resp, 128, 0, 3000, handle);
        at_obj_exec_cmd(handle->client, resp, "AT+QMTCLOSE=%d", handle->client_idx);
    }
    at_delete_resp(resp);
}

QtMqttErrCode quectel_mqtt_pub(quectel_mqtt_client_t handle, const char *topic, const char *message, QtMqttQoSLevel qos, bool retain)
{
    if (NULL == handle)
        return QT_MQTT_ERR_NOINIT;
    at_response_t resp = at_create_resp_new(128, 0, 3000, handle);
    const char *cmd = NULL;
    uint16_t mid = 0;
    if (strstr(get_module_type_name(), "BG95") != NULL)
    {
        cmd = "AT+QMTPUBEX";
    }
    else
    {
        cmd = "AT+QMTPUB";
    }
    if (qos != QT_MQTT_QOS0)
    {
        qosa_mutex_lock(handle->lock, QOSA_WAIT_FOREVER);
        mid = handle->mid++;
        handle->mid = handle->mid == 0 ? 1 : handle->mid;
        qosa_mutex_unlock(handle->lock);
    }
    if (at_obj_exec_cmd(handle->client, resp, "%s=%d,%d,%d,%d,\"%s\",\"%s\"", cmd, handle->client_idx, mid, qos, retain, topic, message) < 0)
        return QT_MQTT_ERR_PUB;
    qosa_sem_wait(handle->sem, 2000);
    at_delete_resp(resp);

    return QT_MQTT_OK;
}

QtMqttErrCode quectel_mqtt_sub(quectel_mqtt_client_t handle, const char *topic, QtMqttQoSLevel qos)
{
    if (NULL == handle)
        return QT_MQTT_ERR_NOINIT;
    at_response_t resp = at_create_resp_new(128, 0, 3000, handle);
    uint16_t mid = 1;
    qosa_mutex_lock(handle->lock, QOSA_WAIT_FOREVER);
    mid = handle->mid++;
    handle->mid = handle->mid == 0 ? 1 : handle->mid;
    qosa_mutex_unlock(handle->lock);
    at_obj_exec_cmd(handle->client, resp, "AT+QMTSUB=%d,%d,\"%s\",%d", handle->client_idx, mid, topic, qos);
    qosa_sem_wait(handle->sem, 2000);
    at_delete_resp(resp);

    return QT_MQTT_OK;
}

QtMqttErrCode quectel_mqtt_unsub(quectel_mqtt_client_t handle, const char *topic)
{
    if (NULL == handle)
        return QT_MQTT_ERR_NOINIT;
    at_response_t resp = at_create_resp_new(128, 0, 3000, handle);
    uint16_t mid = 1;
    qosa_mutex_lock(handle->lock, QOSA_WAIT_FOREVER);
    mid = handle->mid++;
    handle->mid = handle->mid == 0 ? 1 : handle->mid;
    qosa_mutex_unlock(handle->lock);
    at_obj_exec_cmd(handle->client, resp, "AT+QMTUNS=%d,%d,\"%s\"", handle->client_idx, mid, topic);
    qosa_sem_wait(handle->sem, 2000);
    at_delete_resp(resp);
    return QT_MQTT_OK;
}

void qutecel_mqtt_destroy(quectel_mqtt_client_t handle)
{
    if (NULL == handle)
        return;
    quectel_mqtt_release_idx(handle->client_idx);
    qosa_sem_delete(handle->sem);
    qosa_mutex_delete(handle->lock);
    free(handle);
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__ */
