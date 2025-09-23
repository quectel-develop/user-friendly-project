#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_HTTP_S__
#include <stdarg.h>
#include "qosa_log.h"
#include "module_info.h"
#include "ql_http.h"

/**
 * Initialize an HTTP client instance.
 *
 * @param client AT client handle used for HTTP communication
 * @return Returns a handle to the initialized HTTP client instance;
 *         returns NULL if memory allocation fails
 *
 * This function creates and initializes an HTTP client object by allocating
 * the necessary memory resources. If memory allocation fails, it logs an error
 * message and returns NULL to indicate initialization failure.
 */

#define HTTP_POST_MAX_LEN          (1024)

static bool s_global_http_init = false;
static const char* s_body_start_description = "CONNECT\r\n";
static const char* s_body_end_description = "\r\nOK\r\n";
static const char* s_qt_http_option_strings[] =
{
    [QL_HTTP_OPT_CONTEXT_ID]       = "contextid",
    [QL_HTTP_OPT_REQUEST_HEADER]   = "requestheader",
    [QL_HTTP_OPT_RESPONSE_HEADER]  = "responseheader",
    [QL_HTTP_OPT_SSL_CONTEXT_ID]   = "sslctxid",
    [QL_HTTP_OPT_CONTENT_TYPE]     = "contenttype",
    [QL_HTTP_OPT_CUSTOM_HEADER]    = "custom_header",
    [QL_HTTP_OPT_UNKNOWN]          = "unknown"
};

static void ql_http_rsp_info(int result)
{
    switch(result)
    {
        case 0   : break;
        case 200 : break;
        case 400 : LOG_E("%d : Bad Request",                  result); break;
        case 403 : LOG_E("%d : Forbidden",                    result); break;
        case 404 : LOG_E("%d : Not found",                    result); break;
        case 409 : LOG_E("%d : Conflict",                     result); break;
        case 411 : LOG_E("%d : Length required",              result); break;
        case 500 : LOG_E("%d : Internal server error",        result); break;
        default  : LOG_E("%d : Unknown err code",             result); break;
    }
}

static void ql_http_err_info(int result)
{
    switch(result)
    {
        case 0   : break;
        case 701 : LOG_E("%d : HTTP(S) unknown error",                   result); break;
        case 702 : LOG_E("%d : HTTP(S) timeout",                         result); break;
        case 703 : LOG_E("%d : HTTP(S) busy",                            result); break;
        case 704 : LOG_E("%d : HTTP(S) UART busy",                       result); break;
        case 705 : LOG_E("%d : HTTP(S) no GET/POST/PUT requests",        result); break;
        case 706 : LOG_E("%d : HTTP(S) network busy",                    result); break;
        case 707 : LOG_E("%d : HTTP(S) network open failed",             result); break;
        case 708 : LOG_E("%d : HTTP(S) network no configuration",        result); break;
        case 709 : LOG_E("%d : HTTP(S) network deactivated",             result); break;
        case 710 : LOG_E("%d : HTTP(S) network error",                   result); break;
        case 711 : LOG_E("%d : HTTP(S) URL error",                       result); break;
        case 712 : LOG_E("%d : HTTP(S) empty URL",                       result); break;
        case 713 : LOG_E("%d : HTTP(S) IP address error",                result); break;
        case 714 : LOG_E("%d : HTTP(S) DNS error",                       result); break;
        case 715 : LOG_E("%d : HTTP(S) socket create error",             result); break;
        case 716 : LOG_E("%d : HTTP(S) socket connect error",            result); break;
        case 717 : LOG_E("%d : HTTP(S) socket read error",               result); break;
        case 718 : LOG_E("%d : HTTP(S) socket write error",              result); break;
        case 719 : LOG_D("%d : HTTP(S) socket closed",                   result); break;
        case 720 : LOG_E("%d : HTTP(S) data encode error",               result); break;
        case 721 : LOG_E("%d : HTTP(S) data decode error",               result); break;
        case 722 : LOG_E("%d : HTTP(S) read timeout",                    result); break;
        case 723 : LOG_E("%d : HTTP(S) response failed",                 result); break;
        case 724 : LOG_E("%d : Incoming call busy",                      result); break;
        case 725 : LOG_E("%d : Voice call busy",                         result); break;
        case 726 : LOG_E("%d : Input timeout",                           result); break;
        case 727 : LOG_E("%d : Wait data timeout",                       result); break;
        case 728 : LOG_E("%d : Wait HTTP(S) response timeout",           result); break;
        case 729 : LOG_E("%d : Memory allocation failed",                result); break;
        case 730 : LOG_E("%d : Invalid parameter",                       result); break;
        default  : LOG_E("%d : Unknown err code",                        result); break;
    }
}

// static void ql_http_urc_error(struct at_client *client, const char *data, size_t size, void* arg)
// {
//     if (NULL == data || size <= 0)
//         return;
//     ql_http_t handle = (ql_http_t)arg;
//     sscanf(data, "+CME ERROR:%d", &handle->err_code);
//     ql_http_err_info(handle->err_code);
//     qosa_sem_release(handle->sem);
// }

static void ql_http_urc_get(struct at_client *client, const char *data, size_t size, void* arg)
{
    if (NULL == data || size <= 0)
        return;
    ql_http_t handle = (ql_http_t)arg;
    sscanf(data, "+QHTTPGET: %d,%d,%d", &handle->err_code, &handle->rsp_code, &handle->content_length);
    handle->content_left = handle->content_length;
    ql_http_rsp_info(handle->rsp_code);
    ql_http_err_info(handle->err_code);
    qosa_sem_release(handle->sem);
}

static void ql_http_urc_read(struct at_client *client, const char *data, size_t size, void* arg)
{
    if (NULL == data || size <= 0)
        return;
    ql_http_t handle = (ql_http_t)arg;
    sscanf(data, "+QHTTPREAD:%d", &handle->err_code);
    ql_http_err_info(handle->err_code);
    qosa_sem_release(handle->sem);

}

static void ql_http_urc_post(struct at_client *client, const char *data, size_t size, void* arg)
{
    if (NULL == data || size <= 0)
        return;
    ql_http_t handle = (ql_http_t)arg;
    sscanf(data, "+QHTTPPOST: %d,%d,%d", &handle->err_code, &handle->rsp_code, &handle->content_length);
    handle->content_left = handle->content_length;
    ql_http_rsp_info(handle->rsp_code);
    ql_http_err_info(handle->err_code);
    qosa_sem_release(handle->sem);
}

static void ql_http_urc_put(struct at_client *client, const char *data, size_t size, void* arg)
{
    if (NULL == data || size <= 0)
        return;
    ql_http_t handle = (ql_http_t)arg;
    sscanf(data, "+QHTTPPUT: %d,%d,%d", &handle->err_code, &handle->rsp_code, &handle->content_length);
    handle->content_left = handle->content_length;
    ql_http_rsp_info(handle->rsp_code);
    ql_http_err_info(handle->err_code);
    qosa_sem_release(handle->sem);
}

static const struct at_urc s_http_urc_table[] =
{
    // {"+CME ERROR:",    "\r\n",                 ql_http_urc_error},
    {"+QHTTPGET:",     "\r\n",                 ql_http_urc_get},
    {"+QHTTPREAD:",    "\r\n",                 ql_http_urc_read},
    {"+QHTTPPOST:",    "\r\n",                 ql_http_urc_post},
    {"+QHTTPPUT:",     "\r\n",                 ql_http_urc_put}
};

static void ql_http_cb(const char *data, size_t len, void* arg)
{
    ql_http_t handle = (ql_http_t)arg;
    char body_desc[16] = {0};
    char body_data[HTTP_POST_MAX_LEN] = {0};
    int read_len = 0;
    at_client_obj_recv(handle->client, body_desc, strlen(s_body_start_description), 2 * RT_TICK_PER_SECOND, true);
    if (memcmp(body_desc, s_body_start_description, strlen(s_body_start_description)) != 0)
    {
        LOG_E("GET CONNECT FAILED %s", body_desc);
        return;
    }
    if (handle->usr_write_cb != NULL)
    {
        handle->usr_write_cb(QL_HTTP_USR_EVENT_START, NULL, 0, handle->user_write_data);
    }
    bool has_header = handle->response_header;
    while (handle->content_left > 0)
    {
        // need process header
        if (has_header)
        {
            memset(body_data, 0, HTTP_POST_MAX_LEN);
            read_len = at_client_self_recv(handle->client, body_data, HTTP_POST_MAX_LEN, (handle->timeout - 1) * RT_TICK_PER_SECOND, 1, false);
            handle->usr_write_cb(QL_HTTP_USR_EVENT_DATA, body_data, read_len, handle->user_write_data);
            if (strcmp(body_data, "\r\n") == 0)
            {
                LOG_I("http header end");
                has_header = false;
            }
            else
                continue;
        }
        read_len = handle->content_left > HTTP_POST_MAX_LEN ? HTTP_POST_MAX_LEN : handle->content_left;
        read_len = at_client_obj_recv(handle->client, body_data, read_len, (handle->timeout - 1) * RT_TICK_PER_SECOND, false);
        if (handle->usr_write_cb != NULL)
        {
            handle->usr_write_cb(QL_HTTP_USR_EVENT_DATA, body_data, read_len, handle->user_write_data);
        }
        else
        {
            memcpy(handle->data + handle->content_length - handle->content_left, body_data, read_len);
        }
        handle->content_left -= read_len;
    }
    handle->content_left = handle->content_length; // reset
    memset(body_desc, 0, strlen(body_desc));
    at_client_obj_recv(handle->client, body_desc, strlen(s_body_end_description), 2 * RT_TICK_PER_SECOND, true);
    if (memcmp(body_desc, s_body_end_description, strlen(s_body_end_description)) != 0)
    {
        LOG_W("GET OK FAILED %s", body_desc);
        qosa_sem_release(handle->sem);
    }
    if (handle->usr_write_cb != NULL)
    {
        handle->usr_write_cb(QL_HTTP_USR_EVENT_END, NULL, 0, handle->user_write_data);
    }
    // else
    // {
    //     qosa_sem_release(handle->sem);
    // }
}

static QL_HTTP_ERR_CODE_E ql_http_set_url(ql_http_t handle, at_response_t resp, const char *url)
{
    if (NULL == url)
         return QL_HTTP_ERR_URL_INVALID;
    ql_http_setopt(handle, QL_HTTP_OPT_SSL_CONTEXT_ID, handle->ssl.sslctxid);
    if (strstr((char *)url, "https://") != NULL)
    {
        handle->ssl.sslenble = 1;
        if (NULL == handle->ssl.cacert_dst_path)
            handle->ssl.cacert_dst_path = "http_ca.pem";
        if (NULL == handle->ssl.clientcert_dst_path)
            handle->ssl.clientcert_dst_path = "http_user.pem";
        if (NULL == handle->ssl.clientkey_dst_path)
            handle->ssl.clientkey_dst_path = "http_user_key.pem";
        handle->ssl.client = handle->client;
        if (configure_ssl(&handle->ssl) != 0)
            return QL_HTTP_ERR_SSL_CONFIG;
    }
    else if (strstr((char *)url, "http://") != NULL)
    {
    }
    else
        return QL_HTTP_ERR_URL_INVALID;

    if (at_obj_exec_cmd(handle->client, resp, "AT+QHTTPURL=%d,%d", strlen(url), handle->timeout) < 0)
    {
        return QL_HTTP_ERR_SET_URL;
    }
    if ((at_resp_get_line_by_kw(resp, "CONNECT") == NULL))
    {
        return QL_HTTP_ERR_CONNECT;
    }

    at_resp_set_info_new(resp, 128, 0, handle->timeout * RT_TICK_PER_SECOND, handle);
    if (at_obj_exec_cmd(handle->client, resp, "%s", url) < 0)
    {
        return QL_HTTP_ERR_SET_URL;
    }
    return QL_HTTP_OK;
}

static QL_HTTP_ERR_CODE_E ql_http_get(ql_http_t handle, at_response_t resp, const char* data, size_t data_len)
{
    if (!handle->request_header)
    {
        if (at_obj_exec_cmd(handle->client, resp, "AT+QHTTPGET=%d", handle->timeout) < 0)
            return QL_HTTP_ERR_GET;
    }
    else
    {
        if (NULL == data)
            return QL_HTTP_ERR_PARAM_INVALID;
        at_resp_set_info_new(resp, 128, 2, (300), handle);
        if (at_obj_exec_cmd(handle->client, resp, "AT+QHTTPGET=%d,%d,%d", handle->timeout, data_len, handle->timeout) < 0)
        {
            return QL_HTTP_ERR_GET;
        }   
        if ((at_resp_get_line_by_kw(resp, "CONNECT") == NULL))
        {
            return QL_HTTP_ERR_GET;
        }
        at_resp_set_info_new(resp, 64, 0, handle->timeout * RT_TICK_PER_SECOND, handle);
        if (at_obj_exec_cmd(handle->client, resp, "%s", data) < 0)
        {
            return QL_HTTP_ERR_INPUT_BODY;
        }
    }
    return QL_HTTP_OK;
}

static QL_HTTP_ERR_CODE_E ql_http_post(ql_http_t handle, at_response_t resp, const char* data, size_t data_len)
{
    if (data_len <= 0)
        return QL_HTTP_ERR_PARAM_INVALID;
    at_resp_set_info_new(resp, 128, 2, handle->timeout * RT_TICK_PER_SECOND * 2, handle);
    if (at_obj_exec_cmd(handle->client, resp, "AT+QHTTPPOST=%d,%d,%d", data_len, handle->timeout, handle->timeout) < 0)
    {
        return QL_HTTP_ERR_POST;
    }
    if ((at_resp_get_line_by_kw(resp, "CONNECT") == NULL))
    {
        return QL_HTTP_ERR_POST;
    }
    int sent = 0;
    char *buffer = (char*)malloc(HTTP_POST_MAX_LEN);
    while (sent < data_len)
    {
        if (handle->usr_read_cb != NULL)
        {
            size_t ret = handle->usr_read_cb(buffer, HTTP_POST_MAX_LEN, handle->user_read_data);
            if (ret <= 0)
                break;
            at_client_obj_send(handle->client, buffer, ret, false);
            sent += ret;
        }
        else
        {
            int chunk_size = (data_len - sent > HTTP_POST_MAX_LEN) ? HTTP_POST_MAX_LEN : data_len - sent;
            at_client_obj_send(handle->client, data + sent, chunk_size, false);
            sent += chunk_size;
        }
        LOG_I("send size %d", sent);
    }
    free(buffer);
    return QL_HTTP_OK;
}

static QL_HTTP_ERR_CODE_E ql_http_post_file(ql_http_t handle, at_response_t resp, const char* data, size_t data_len)
{
    if (NULL == data)
        return QL_HTTP_ERR_PARAM_INVALID;
    if (at_obj_exec_cmd(handle->client, resp, "AT+QHTTPPOSTFILE=%s,%d", data, handle->timeout) < 0)
        return QL_HTTP_ERR_POST_FILE;
    return QL_HTTP_OK;
}

static QL_HTTP_ERR_CODE_E ql_http_put(ql_http_t handle, at_response_t resp, const char* data, size_t data_len)
{
    if (NULL == data)
        return QL_HTTP_ERR_PARAM_INVALID;
    at_resp_set_info_new(resp, 128, 2, handle->timeout * RT_TICK_PER_SECOND * 2, handle);
    if (at_obj_exec_cmd(handle->client, resp, "AT+QHTTPPUT=%d,%d,%d", data_len, handle->timeout, handle->timeout) < 0)
    {
        return QL_HTTP_ERR_PUT;
    }
    if ((at_resp_get_line_by_kw(resp, "CONNECT") == NULL))
    {
        return QL_HTTP_ERR_PUT;
    }
    int sent = 0;
    char *buffer = (char*)malloc(HTTP_POST_MAX_LEN);
    while (sent < data_len)
    {
        if (handle->usr_read_cb != NULL)
        {
            size_t ret = handle->usr_read_cb(buffer, HTTP_POST_MAX_LEN, handle->user_read_data);
            if (ret <= 0)
                break;
            at_client_obj_send(handle->client, buffer, ret, false);
            sent += ret;
        }
        else
        {
            int chunk_size = (data_len - sent > HTTP_POST_MAX_LEN) ? HTTP_POST_MAX_LEN : data_len - sent;
            at_client_obj_send(handle->client, data + sent, chunk_size, false);
            sent += chunk_size;
        }
        LOG_I("send size %d", sent);
    }
    free(buffer);
    return QL_HTTP_OK;
}

static QL_HTTP_ERR_CODE_E ql_http_put_file(ql_http_t handle, at_response_t resp, const char* data, QL_HTTP_FILE_TYPE_E type)
{
    if (NULL == data)
        return QL_HTTP_ERR_PARAM_INVALID;
    if (at_obj_exec_cmd(handle->client, resp, "AT+QHTTPPUTFILE=%s,%d,%d", data, handle->timeout, type) < 0)
        return QL_HTTP_ERR_PUT_FILE;
    return QL_HTTP_OK;
}
static QL_HTTP_ERR_CODE_E ql_http_read(ql_http_t handle)
{
    at_response_t query_resp = NULL;
   if (NULL == handle)
        return QL_HTTP_ERR_NOINIT;
    query_resp = at_create_resp_by_selffunc_new(1024, 0, handle->wait_time * RT_TICK_PER_SECOND, ql_http_cb, handle);
    if (at_obj_exec_cmd(handle->client, query_resp, "AT+QHTTPREAD=%d", handle->wait_time) < 0)
    {
        at_delete_resp(query_resp);
        return QL_HTTP_ERR_READ;
    }
    at_delete_resp(query_resp);
    qosa_sem_wait(handle->sem, (handle->wait_time +1) * RT_TICK_PER_SECOND);
    if (handle->err_code != QL_HTTP_OK)
        return handle->err_code;
    return QL_HTTP_OK;
}

ql_http_t ql_http_init(at_client_t client)
{
    ql_http_t handle = (ql_http_t)malloc(sizeof(ql_http_s));
    if (NULL == handle)
    {
        LOG_E("no memory for AT client response object.");
        return NULL;
    }
    handle->client = client;
    handle->timeout = 60;
    handle->wait_time = 60;
    handle->request_header = false;
    handle->response_header = false;
    handle->file_type = QL_HTTP_FILE_BOTH_HEADERS_BODY;
    handle->content_length = 0;
    handle->content_left = 0;
    handle->usr_write_cb = NULL;
    handle->usr_read_cb  = NULL;
    handle->user_write_data = NULL;
    handle->user_read_data = NULL;
    handle->rsp_code = 0;
    handle->err_code = 0;
    handle->data = NULL;
    handle->ssl.sslenble = 0;
    handle->ssl.ssltype = 0;
    handle->ssl.sslctxid = 0;
    handle->ssl.ciphersuite = 0xFFFF;
    handle->ssl.seclevel = SEC_LEVEL_NO_AUTHENTICATION;
    handle->ssl.sslversion = SSL_VERSION_ALL;
    qosa_sem_create(&handle->sem, 0);
    if (!s_global_http_init)
    {
        at_set_urc_table(s_http_urc_table, sizeof(s_http_urc_table) / sizeof(s_http_urc_table[0]));
        s_global_http_init = true;
    }
    return handle;
}

void ql_http_set_ssl(ql_http_t handle, ql_SSL_Config config)
{
    handle->ssl = config;
}

bool ql_http_setopt(ql_http_t handle, QL_HTTP_OPTION_E option, ...)
{
    va_list arg;
    at_response_t resp = NULL;
    bool ret = false;
    char *option_content = NULL;
    if (NULL == handle)
        return QL_HTTP_ERR_NOINIT;
    resp = at_create_resp_new(256, 0, 1000, handle);
    va_start(arg, option);

    switch (option)
    {
        case QL_HTTP_OPT_CONTEXT_ID:
        {
            uint8_t value = (uint8_t)va_arg(arg, int);
            if (value < 1 || value > 16)
                break;
            option_content = (char*)malloc(4);
            snprintf(option_content, 4, "%d", value);
            ret = true;
            break;
        }
        case QL_HTTP_OPT_REQUEST_HEADER:
        {
            handle->request_header = (bool)va_arg(arg, int);
            option_content = (char*)malloc(4);
            snprintf(option_content, 4, "%d", handle->request_header);
            ret = true;
            break;
        }
        case QL_HTTP_OPT_RESPONSE_HEADER:
        {
            handle->response_header = (bool)va_arg(arg, int);
            option_content = (char*)malloc(4);
            snprintf(option_content, 4, "%d", handle->response_header);
            ret = true;
            break;
        }
        case QL_HTTP_OPT_SSL_CONTEXT_ID:
        {
            uint8_t value = (uint8_t)va_arg(arg, int);
            if (value > 5)
                break;
            option_content = (char*)malloc(4);
            snprintf(option_content, 4, "%d", value);
            handle->ssl.sslctxid = value;
            ret = true;
        }
            break;
        case QL_HTTP_OPT_CONTENT_TYPE:
        {

            QL_HTTP_CONTENT_TYPE_E value = (QL_HTTP_CONTENT_TYPE_E)va_arg(arg, int);
            if (value > QL_HTTP_CONTENT_JPEG)
                break;
            option_content = (char*)malloc(4);
            snprintf(option_content, 4, "%d", value);
            ret = true;
            break;
        }
        case QL_HTTP_OPT_CUSTOM_HEADER:
        {
            const char* value = va_arg(arg, const char*);
            option_content = (char*)malloc(strlen(value) + 4); // contain \"\"  and '\0'
            snprintf(option_content, strlen(value) + 4, "\"%s\"", value);
            LOG_D("option_content = %s", option_content);
            ret = true;
            break;
        }
        case QL_HTTP_OPT_TIMEOUT:
            handle->timeout = va_arg(arg, uint32_t);
            at_delete_resp(resp);
            va_end(arg);
            return true;
        case QL_HTTP_OPT_WAIT_TIME:
            handle->wait_time = va_arg(arg, uint32_t);
            at_delete_resp(resp);
            va_end(arg);
            return true;
        case QL_HTTP_OPT_FILE_TYPE:
            handle->file_type = va_arg(arg, uint32_t);
            at_delete_resp(resp);
            va_end(arg);
            return true;

        case QL_HTTP_OPT_WRITE_FUNCTION:
            handle->usr_write_cb = va_arg(arg, user_write_callback);
            at_delete_resp(resp);
            va_end(arg);
            return true;

        case QL_HTTP_OPT_READ_FUNCTION:
            handle->usr_read_cb = va_arg(arg, user_read_callback);
            at_delete_resp(resp);
            va_end(arg);
            return true;

        case QL_HTTP_OPT_WRITE_DATA:
            handle->user_write_data = va_arg(arg, void*);
            at_delete_resp(resp);
            va_end(arg);
            return true;
       case QL_HTTP_OPT_READ_DATA:
            handle->user_read_data = va_arg(arg, void*);
            at_delete_resp(resp);
            va_end(arg);
            return true;
        default:
            ret = 0;
            break;
    }
    if (ret)
    {
        if (at_obj_exec_cmd(handle->client, resp, "AT+QHTTPCFG=\"%s\",%s", s_qt_http_option_strings[option], option_content) < 0)
            ret = false;
    }
    at_delete_resp(resp);
    if (option_content)
        free(option_content);
    va_end(arg);
    return ret;
}

QL_HTTP_ERR_CODE_E ql_http_request(ql_http_t handle, const char* url, QL_HTTP_METHOD_E method, const char* data, size_t data_len)
{
    QL_HTTP_ERR_CODE_E err = QL_HTTP_OK;
    at_response_t resp = NULL;
    if (NULL == handle)
        return QL_HTTP_ERR_NOINIT;
    resp = at_create_resp_new(128, 2, handle->timeout * RT_TICK_PER_SECOND * 2, handle);
    err = ql_http_set_url(handle, resp, url);
    if (err != QL_HTTP_OK)
    {
        at_delete_resp(resp);
        return err;
    }
    at_resp_set_info_new(resp, 128, 4, handle->timeout * RT_TICK_PER_SECOND * 2, handle);
    qosa_mutex_lock(handle->client->lock, QOSA_WAIT_FOREVER);
    switch (method)
    {
    case QL_HTTP_METHORD_GET:
        err = ql_http_get(handle, resp, data, data_len);
        break;
    case QL_HTTP_METHORD_POST:
        err = ql_http_post(handle, resp, data, data_len);
        break;
    case QL_HTTP_METHORD_POST_FILE:
        err = ql_http_post_file(handle, resp, data, data_len);
        break;
    case QL_HTTP_METHORD_PUT:
        err = ql_http_put(handle, resp, data, data_len);
        /* code */
        break;
    case QL_HTTP_METHORD_PUT_FILE:
        err = ql_http_put_file(handle, resp, data, handle->file_type);
        break;
    default:
        break;
    }
    if (err != QL_HTTP_OK)
    {
        at_delete_resp(resp);
        qosa_mutex_unlock(handle->client->lock);
        return err;
    }
    qosa_sem_wait(handle->sem, (handle->timeout+1) * RT_TICK_PER_SECOND);
    at_delete_resp(resp);
    if (handle->err_code != QL_HTTP_OK)
    {
        qosa_mutex_unlock(handle->client->lock);
        return handle->err_code;
    }
    if (NULL == handle->usr_write_cb)
        handle->data = (char*)malloc(handle->content_length);
    err = ql_http_read(handle);
    qosa_mutex_unlock(handle->client->lock);
    return err;
}

int ql_http_recv(ql_http_t handle, char* buf, size_t size)
{
    if (NULL == handle || handle->usr_write_cb != NULL)
        return 0;
    if (handle->content_left <= 0)
        return 0;
    if (handle->err_code != QOSA_OK)
        return 0;
    int need_read =  handle->content_left > size ? size : handle->content_left;
    memcpy(buf, handle->data + handle->content_length - handle->content_left, need_read);
    handle->content_left -= need_read;

    return need_read;
}

void ql_http_deinit(ql_http_t handle)
{
    if(NULL == handle)
        return;
    if (strstr(get_module_type_name(), "BG95") == NULL && strstr(get_module_type_name(), "BG96") == NULL)
    {
        at_response_t resp = at_create_resp_new(128, 2, handle->timeout * RT_TICK_PER_SECOND, handle);
        at_obj_exec_cmd(handle->client, resp, "AT+QHTTPSTOP", handle->timeout);
        at_delete_resp(resp);
    }

    if (handle->sem)
    {
        qosa_sem_delete(handle->sem);
    }
    if (handle->data != NULL)
    {
        free(handle->data);
    }
    free(handle);
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_HTTP_S__ */
