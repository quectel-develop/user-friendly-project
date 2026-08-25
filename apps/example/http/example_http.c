#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_HTTP_S__
#include "qosa_log.h"
#include "ql_http.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#elif __linux__
#else
#include "cmsis_os2.h"
#endif

#include "ff.h"

static void http_rsp_callback(QL_HTTP_USR_EVENT_E event, const char *data, size_t len, void *arg)
{
    static FIL fil;
    size_t size = 0;
    static int total_len = 0;
    switch (event)
    {
    case QL_HTTP_USR_EVENT_START:
    {
        total_len = 0;
        FRESULT res = f_open(&fil, (char*)arg, FA_CREATE_ALWAYS | FA_WRITE);
        if (res != FR_OK)
        {
            LOG_W("open file failed");
        }
    }
        break;
    case QL_HTTP_USR_EVENT_HEADER_DATA:
        LOG_I("header data: %s", (char*)data);
        break;
    case QL_HTTP_USR_EVENT_BODY_DATA:
        total_len += len;
        if ((fil.flag & FA_WRITE))
            f_write(&fil, data, len, &size);
        break;
    case QL_HTTP_USR_EVENT_END:
        LOG_I("recv total len = %d", total_len);
        f_close(&fil);
        break;
    default:
        break;
    }
}
void example_test_get_extend()
{
    ql_http_t handle = NULL;
    handle = ql_http_init(at_client_get_first());
    // ql_http_setopt(handle, QL_HTTP_OPT_CONTEXT_ID, 1);
    // ql_http_setopt(handle, QL_HTTP_OPT_REQUEST_HEADER, true);
    // ql_http_setopt(handle, QL_HTTP_OPT_RESPONSE_HEADER, true);
    // ql_http_setopt(handle, QL_HTTP_OPT_SSL_CONTEXT_ID, 0);
    // ql_http_setopt(handle, QL_HTTP_OPT_CONTENT_TYPE, app_urlencoded);
    // char headers[1024] = {0};
    // snprintf(headers, sizeof(headers), "%s", "Accept: application/json");
    // ql_http_setopt(handle, QL_HTTP_OPT_CUSTOM_HEADER, headers);
    ql_http_setopt(handle, QL_HTTP_OPT_TIMEOUT, 60);
    ql_http_setopt(handle, QL_HTTP_OPT_WRITE_FUNCTION, http_rsp_callback);
    ql_http_setopt(handle, QL_HTTP_OPT_WRITE_DATA, "0:test_cb.txt");
    const char *url = "https://httpbin.org/bytes/1024";
    QL_HTTP_ERR_CODE_E err = ql_http_request(handle, url, QL_HTTP_METHORD_GET, NULL, 0);
    LOG_D("ql_http_request %d", err);

    ql_http_deinit(handle);
}

void example_test_get()
{
    ql_http_t handle = NULL;
    handle = ql_http_init(at_client_get_first());
    // ql_http_setopt(handle, QL_HTTP_OPT_CONTEXT_ID, 1);
    // ql_http_setopt(handle, QL_HTTP_OPT_REQUEST_HEADER, true);
    // ql_http_setopt(handle, QL_HTTP_OPT_RESPONSE_HEADER, true);
    // ql_http_setopt(handle, QL_HTTP_OPT_SSL_CONTEXT_ID, 0);
    // ql_http_setopt(handle, QL_HTTP_OPT_CONTENT_TYPE, app_urlencoded);
    // char headers[1024] = {0};
    // snprintf(headers, sizeof(headers), "%s", "Accept: application/json");
    // ql_http_setopt(handle, QL_HTTP_OPT_CUSTOM_HEADER, headers);
    const char *url = "https://httpbin.org/get";
    QL_HTTP_ERR_CODE_E err = ql_http_request(handle, url, QL_HTTP_METHORD_GET, NULL, 0);
    if (err != QL_HTTP_OK)
    {
        ql_http_deinit(handle);
        return;
    }
    char data[1024];
    UINT size = 100;
    int read = 0;
    int total = 0;
    FIL fil;
    FRESULT res = f_open(&fil, "0:test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
        LOG_W("open file failed");
    }
    while (1)
    {
        read = ql_http_recv(handle, data, size);
        if (read <= 0)
        {
            break;
        }

        if ((fil.flag & FA_WRITE))
            f_write(&fil, data, (UINT)read, &size);
        total += read;
        LOG_D("read %d bytes size %d bytes total %d", read, size, total);
    }
    f_close(&fil);
    ql_http_deinit(handle);
}

static size_t http_read_callback(char *buffer, size_t size, void *arg)
{
    FIL *fil = (FIL*)arg;
    UINT br = 0;
    f_read(fil, buffer, size, &br);
    return br;
}
void example_test_post_ex()
{
    ql_http_t handle = NULL;
    FIL fil;
    if (f_open(&fil, "0:get_1k.txt", FA_OPEN_EXISTING | FA_READ) != FR_OK)
    {
        LOG_E("open file error");
        return;
    }
    handle = ql_http_init(at_client_get_first());
    const char* url = "http://postman-echo.com/post";
    ql_http_setopt(handle, QL_HTTP_OPT_TIMEOUT, 60);
    ql_http_setopt(handle, QL_HTTP_OPT_WRITE_FUNCTION, http_rsp_callback);
    ql_http_setopt(handle, QL_HTTP_OPT_WRITE_DATA, "0:test_post.txt");
    ql_http_setopt(handle, QL_HTTP_OPT_READ_FUNCTION, http_read_callback);
    FSIZE_t file_size = f_size(&fil);
    LOG_D("file_size %d", file_size);
    ql_http_setopt(handle, QL_HTTP_OPT_READ_DATA, (void*)&fil);
    QL_HTTP_ERR_CODE_E err = ql_http_request(handle, url, QL_HTTP_METHORD_POST, NULL, file_size);
    f_close(&fil);
    LOG_D("ql_http_request %d", err);
    ql_http_deinit(handle);
}

void example_test_post()
{
    ql_http_t handle = NULL;
    handle = ql_http_init(at_client_get_first());
    const char* url = "https://postman-echo.com/post";
    ql_http_setopt(handle, QL_HTTP_OPT_TIMEOUT, 10);
    ql_http_setopt(handle, QL_HTTP_OPT_WRITE_FUNCTION, http_rsp_callback);
    ql_http_setopt(handle, QL_HTTP_OPT_WRITE_DATA, "0:test_post.txt");
    ql_http_setopt(handle, QL_HTTP_OPT_CONTENT_TYPE, QL_HTTP_CONTENT_JSON);
    const char* data = R"({"email": "eve.holt@reqres.in","password": "cityslicka"})";
    QL_HTTP_ERR_CODE_E err = ql_http_request(handle, url, QL_HTTP_METHORD_POST, data, strlen(data));
    LOG_D("ql_http_request %d", err);
    ql_http_deinit(handle);
}

void example_test_put()
{
    ql_http_t handle = NULL;
    handle = ql_http_init(at_client_get_first());
    // const char* url = "http://112.31.84.164:8300/upload.php";
    // const char *url = "https://httpbin.org/post";
    const char* url = "http://112.31.84.164:8300/put.php?ext=txt";
    ql_http_setopt(handle, QL_HTTP_OPT_TIMEOUT, 10);
    ql_http_setopt(handle, QL_HTTP_OPT_WRITE_FUNCTION, http_rsp_callback);
    ql_http_setopt(handle, QL_HTTP_OPT_WRITE_DATA, "0:test_put.txt");
    ql_http_setopt(handle, QL_HTTP_OPT_CONTENT_TYPE, QL_HTTP_CONTENT_JSON);
    const char* data = R"({"email": "eve.holt@reqres.in","password": "cityslicka"})";
    QL_HTTP_ERR_CODE_E err = ql_http_request(handle, url, QL_HTTP_METHORD_PUT, data, strlen(data));
    LOG_D("ql_http_request %d", err);
    ql_http_deinit(handle);
}

void example_test_https()
{
    ql_http_t handle = NULL;
    handle = ql_http_init(at_client_get_first());
    const char* url = "https://112.31.84.164:8303/upload.php";
    ql_http_setopt(handle, QL_HTTP_OPT_TIMEOUT, 10);
    ql_http_setopt(handle, QL_HTTP_OPT_WRITE_FUNCTION, http_rsp_callback);
    ql_http_setopt(handle, QL_HTTP_OPT_WRITE_DATA, "0:test_post.txt");
    ql_http_setopt(handle, QL_HTTP_OPT_CONTENT_TYPE, QL_HTTP_CONTENT_JSON);

    ql_SSL_Config ssl_config;
    memset(&ssl_config, 0, sizeof(ssl_config));
    ssl_config.sslenble = 1;
    ssl_config.sslctxid = 0;
    ssl_config.ciphersuite = 0xFFFF;
    ssl_config.seclevel = SEC_LEVEL_SERVER_AND_CLIENT_AUTHENTICATION;
    ssl_config.sslversion = SSL_VERSION_ALL;
    ssl_config.src_is_path = true; // if false cacert_src/clientkey_src/clientcert_dst_path is file content
    ssl_config.cacert_src = "http_ca.pem";
    ssl_config.clientcert_src = "http_user.pem";
    ssl_config.clientkey_src = "http_user_key.pem";
    ssl_config.cacert_dst_path = "http_ca.pem";
    ssl_config.clientcert_dst_path = "http_user.pem";
    ssl_config.clientkey_dst_path = "http_user_key.pem";
    ql_http_set_ssl(handle, ssl_config);
    const char* data = R"({"email": "eve.holt@reqres.in","password": "cityslicka"})";
    QL_HTTP_ERR_CODE_E err = ql_http_request(handle, url, QL_HTTP_METHORD_POST, data, strlen(data));
    LOG_D("ql_http_request %d", err);
    ql_http_deinit(handle);
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_HTTP_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
