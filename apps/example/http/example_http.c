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

static void user_http_callback(QtHttpUsrEvent event, const char *data, size_t len, void *arg)
{
    static FIL fil;
    size_t size = 0;
    static int total_len = 0;
    total_len += len;
    switch (event)
    {
    case QT_HTTP_USR_EVENT_START:
    {
        FRESULT res = f_open(&fil, (char*)arg, FA_CREATE_ALWAYS | FA_WRITE);
        if (res != FR_OK)
        {
            LOG_W("open file failed");
        }
    }
        break;
    case QT_HTTP_USR_EVENT_DATA:
        if ((fil.flag & FA_WRITE))
            f_write(&fil, data, len, &size);
        break;
    case QT_HTTP_USR_EVENT_END:
        LOG_I("recv total len = %d", total_len);
        f_close(&fil);
        break;
    default:
        break;
    }
}
void example_test_get_extend()
{
    quectel_http_t handle = NULL;
    handle = quectel_http_init(at_client_get_first());
    // quectel_http_setopt(handle, QT_HTTP_OPT_CONTEXT_ID, 1);
    // quectel_http_setopt(handle, QT_HTTP_OPT_REQUEST_HEADER, true);
    // quectel_http_setopt(handle, QT_HTTP_OPT_RESPONSE_HEADER, true);
    // quectel_http_setopt(handle, QT_HTTP_OPT_SSL_CONTEXT_ID, 0);
    // quectel_http_setopt(handle, QT_HTTP_OPT_CONTENT_TYPE, app_urlencoded);
    // quectel_http_setopt(handle, QT_HTTP_OPT_AUTO_RESPONSE, true);
    // quectel_http_setopt(handle, QT_HTTP_OPT_CLOSE_INDICATION, true);
    // quectel_http_setopt(handle, QT_HTTP_OPT_WINDOW_SIZE, 1024);
    // quectel_http_setopt(handle, QT_HTTP_OPT_CLOSE_WAIT_TIME, 5000);
    // quectel_http_setopt(handle, QT_HTTP_OPT_AUTH, "username:password");
    // header_list = quectel_http_header_append(header_list, "Content-Type: application/json");
    // header_list = quectel_http_header_append(header_list, "Accept: application/json");
    // char headers[1024] = {0};
    // snprintf(headers, sizeof(headers), "%s", "Accept: application/json");
    // quectel_http_setopt(handle, QT_HTTP_OPT_CUSTOM_HEADER, headers);
    quectel_http_setopt(handle, QT_HTTP_OPT_TIMEOUT, 10);
    quectel_http_setopt(handle, QT_HTTP_OPT_WRITE_FUNCTION, user_http_callback);
    quectel_http_setopt(handle, QT_HTTP_OPT_WRITE_DATA, "0:test_cb.txt");
    const char *url = "https://httpbin.org/bytes/1024";
    QtHttpErrCode err = quectel_http_request(handle, url, QT_HTTP_METHORD_GET, NULL);
    LOG_D("quectel_http_request %d", err);

    quectel_http_deinit(handle);
}

void example_test_get()
{
    quectel_http_t handle = NULL;
    handle = quectel_http_init(at_client_get_first());
    // quectel_http_setopt(handle, QT_HTTP_OPT_CONTEXT_ID, 1);
    // quectel_http_setopt(handle, QT_HTTP_OPT_REQUEST_HEADER, true);
    // quectel_http_setopt(handle, QT_HTTP_OPT_RESPONSE_HEADER, true);
    // quectel_http_setopt(handle, QT_HTTP_OPT_SSL_CONTEXT_ID, 0);
    // quectel_http_setopt(handle, QT_HTTP_OPT_CONTENT_TYPE, app_urlencoded);
    // quectel_http_setopt(handle, QT_HTTP_OPT_AUTO_RESPONSE, true);
    // quectel_http_setopt(handle, QT_HTTP_OPT_CLOSE_INDICATION, true);
    // quectel_http_setopt(handle, QT_HTTP_OPT_WINDOW_SIZE, 1024);
    // quectel_http_setopt(handle, QT_HTTP_OPT_CLOSE_WAIT_TIME, 5000);
    // quectel_http_setopt(handle, QT_HTTP_OPT_AUTH, "username:password");
    // header_list = quectel_http_header_append(header_list, "Content-Type: application/json");
    // header_list = quectel_http_header_append(header_list, "Accept: application/json");
    // char headers[1024] = {0};
    // snprintf(headers, sizeof(headers), "%s", "Accept: application/json");
    // quectel_http_setopt(handle, QT_HTTP_OPT_CUSTOM_HEADER, headers);
    const char *url = "https://httpbin.org/get";
    QtHttpErrCode err = quectel_http_request(handle, url, QT_HTTP_METHORD_GET, NULL) != QT_HTTP_OK;
    if (err != QT_HTTP_OK)
    {
        quectel_http_deinit(handle);
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
        read = quectel_http_recv(handle, data, size);
        if (read <= 0)
        {
            break;
        }

        if ((fil.flag & FA_WRITE))
            f_write(&fil, data, (UINT)read, &size);
        total += read;
        LOG_D("read %d bytes size %d bytes total %d", read, size, total);
        if (read < size)
        {
            LOG_D("break");
            break;
        }
    }
    f_close(&fil);
    quectel_http_deinit(handle);
}

void example_test_post()
{
    quectel_http_t handle = NULL;
    handle = quectel_http_init(at_client_get_first());
    // const char* url = "http://112.31.84.164:8300/upload.php";
    // const char *url = "https://httpbin.org/post";
    const char* url = "https://postman-echo.com/post";
    quectel_http_setopt(handle, QT_HTTP_OPT_TIMEOUT, 10);
    quectel_http_setopt(handle, QT_HTTP_OPT_WRITE_FUNCTION, user_http_callback);
    quectel_http_setopt(handle, QT_HTTP_OPT_WRITE_DATA, "0:test_post.txt");
    quectel_http_setopt(handle, QT_HTTP_OPT_CONTENT_TYPE, QT_HTTP_CONTENT_JSON);
    const char* data = R"({"email": "eve.holt@reqres.in","password": "cityslicka"})";
    QtHttpErrCode err = quectel_http_request(handle, url, QT_HTTP_METHORD_POST, data);
    LOG_D("quectel_http_request %d", err);
    quectel_http_deinit(handle);
#if 0
    char data[1024];
    UINT size = 100;
    int read = 0;
    int total = 0;
    FIL fil;
    FRESULT res = f_open(&fil, "0:test_post.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
        LOG_W("open file failed");
    }
    while (1)
    {
        read = quectel_http_recv(handle, data, size);
        if (read <= 0)
        {
            break;
        }

        if ((fil.flag & FA_WRITE))
            f_write(&fil, data, (UINT)read, &size);
        total += read;
        LOG_D("read %d bytes size %d bytes total %d", read, size, total);
        if (read < size)
        {
            LOG_D("break");
            break;
        }
    }
    f_close(&fil);
#endif
}

void example_test_put()
{
    quectel_http_t handle = NULL;
    handle = quectel_http_init(at_client_get_first());
    // const char* url = "http://112.31.84.164:8300/upload.php";
    // const char *url = "https://httpbin.org/post";
    const char* url = "http://112.31.84.164:8300/put.php?ext=txt";
    quectel_http_setopt(handle, QT_HTTP_OPT_TIMEOUT, 10);
    quectel_http_setopt(handle, QT_HTTP_OPT_WRITE_FUNCTION, user_http_callback);
    quectel_http_setopt(handle, QT_HTTP_OPT_WRITE_DATA, "0:test_put.txt");
    quectel_http_setopt(handle, QT_HTTP_OPT_CONTENT_TYPE, QT_HTTP_CONTENT_JSON);
    const char* data = R"({"email": "eve.holt@reqres.in","password": "cityslicka"})";
    QtHttpErrCode err = quectel_http_request(handle, url, QT_HTTP_METHORD_PUT, data);
    LOG_D("quectel_http_request %d", err);
    quectel_http_deinit(handle);
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_HTTP_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
