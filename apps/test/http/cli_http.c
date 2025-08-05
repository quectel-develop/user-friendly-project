#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_HTTP_S__
#include "qosa_system.h"
#include "qosa_log.h"
#include "ql_http.h"
#include "ql_ssl.h"
#include "cli_http.h"


#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#elif __linux__
#else
#include "cmsis_os2.h"
#endif

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_TFCARD__
#include "ff.h"
#endif


void cli_http_get_help(void)
{
    LOG_I("| http contextid requestheader responseheader contenttype custom_header timeout rsptime            |");
    LOG_I("|      wait_time request_url method request_mode username password sd_card_path sslenble           |");
    LOG_I("|      sslctxid ciphersuite seclevel sslversion                                                    |");
    LOG_I("|      contextid     : PDP context ID                                                              |");
    LOG_I("|                      Range: 1-16                                                                 |");
    LOG_I("|      requestheader : Disable or enable customization of HTTP(S) request header                   |");
    LOG_I("|                      0: Disable                                                                  |");
    LOG_I("|                      1: Enable                                                                   |");
    LOG_I("|      responseheader: Disable or enable the outputting of HTTP(S) response header                 |");
    LOG_I("|                      0: Disable                                                                  |");
    LOG_I("|                      1: Enable                                                                   |");
    LOG_I("|      contenttype   : Data type of HTTP(S) body                                                   |");
    LOG_I("|                      0: application/x-www-form-urlencoded                                        |");
    LOG_I("|                      1: text/plain                                                               |");
    LOG_I("|                      2: application/octet-stream                                                 |");
    LOG_I("|                      3: multipart/form-data                                                      |");
    LOG_I("|                      4: application/json                                                         |");
    LOG_I("|                      5: image/jpeg                                                               |");
    LOG_I("|      custom_header : User-defined HTTP(S) request header                                         |");
    LOG_I("|      timeout       : The maximum time for inputting URL. Range: 1-2038. Unit: second             |");
    LOG_I("|      rsptime       : Timeout value for the HTTP(S) GET response,Range: 1-65535. Unit: second     |");
    LOG_I("|      wait_time     : Maximum time between receive two packets of data.Range: 1-65535.Unit: second|");
    LOG_I("|      request_url   : HTTP(S) server URL                                                          |");
    LOG_I("|      method        : Request type                                                                |");
    LOG_I("|                      0: Get                                                                      |");
    LOG_I("|                      1: Post                                                                     |");
    LOG_I("|      request_mode  : Request mode                                                                |");
    LOG_I("|                      0: Async                                                                    |");
    LOG_I("|                      1: Sync                                                                     |");
    LOG_I("|      username      : Username for logging in to the HTTP(S) server                               |");
    LOG_I("|      password      : Password for logging in to the HTTP(S) server                               |");
    LOG_I("|      sd_card_path  : Data path in SD card                                                        |");
    LOG_I("|      sslenble      : Whether ssl is enabled                                                      |");
    LOG_I("|                      0: Disable SSL                                                              |");
    LOG_I("|                      1: Enable SSL                                                               |");
    LOG_I("|             sslctxid      : SSL context ID used for HTTP(S)                                      |");
    LOG_I("|                             Range: 0-5                                                           |");
    LOG_I("|             ciphersuite   : Numeric type in HEX format. SSL cipher suites                        |");
    LOG_I("|                             0x0035:TLS_RSA_WITH_AES_256_CBC_SHA                                  |");
    LOG_I("|                             0x002F:TLS_RSA_WITH_AES_128_CBC_SHA                                  |");
    LOG_I("|                             0x0005:TLS_RSA_WITH_RC4_128_SHA                                      |");
    LOG_I("|                             0x0004:TLS_RSA_WITH_RC4_128_MD5                                      |");
    LOG_I("|                             0x000A:TLS_RSA_WITH_3DES_EDE_CBC_SHA                                 |");
    LOG_I("|                             0x003D:TLS_RSA_WITH_AES_256_CBC_SHA256                               |");
    LOG_I("|                             0xC002:TLS_ECDH_ECDSA_WITH_RC4_128_SHA                               |");
    LOG_I("|                             0xC003:TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA                          |");
    LOG_I("|                             0xC004:TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA                           |");
    LOG_I("|                             0xC005:TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA                           |");
    LOG_I("|                             0xC007:TLS_ECDHE_ECDSA_WITH_RC4_128_SHA                              |");
    LOG_I("|                             0xC008:TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA                         |");
    LOG_I("|                             0xC009:TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA                          |");
    LOG_I("|                             0xC00A:TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA                          |");
    LOG_I("|                             0xC011:TLS_ECDHE_RSA_WITH_RC4_128_SHA                                |");
    LOG_I("|                             0xC012:TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA                           |");
    LOG_I("|                             0xC013:TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA                            |");
    LOG_I("|                             0xC014:TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA                            |");
    LOG_I("|                             0xC00C:TLS_ECDH_RSA_WITH_RC4_128_SHA                                 |");
    LOG_I("|                             0xC00D:TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA                            |");
    LOG_I("|                             0xC00E:TLS_ECDH_RSA_WITH_AES_128_CBC_SHA                             |");
    LOG_I("|                             0xC00F:TLS_ECDH_RSA_WITH_AES_256_CBC_SHA                             |");
    LOG_I("|                             0xC023:TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256                       |");
    LOG_I("|                             0xC024:TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384                       |");
    LOG_I("|                             0xC025:TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256                        |");
    LOG_I("|                             0xC026:TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384                        |");
    LOG_I("|                             0xC027:TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256                         |");
    LOG_I("|                             0xC028:TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384                         |");
    LOG_I("|                             0xC029:TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256                          |");
    LOG_I("|                             0xC02A:TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384                          |");
    LOG_I("|                             0xC02B:TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256                       |");
    LOG_I("|                             0xC02F:TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256                         |");
    LOG_I("|                             0xC0A8:TLS_PSK_WITH_AES_128_CCM_8                                    |");
    LOG_I("|                             0x00AE:TLS_PSK_WITH_AES_128_CBC_SHA256                               |");
    LOG_I("|                             0xC0AE:TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8                            |");
    LOG_I("|             seclevel      : Authentication mode                                                  |");
    LOG_I("|                             0: No authentication                                                 |");
    LOG_I("|                             1: Perform server authentication                                     |");
    LOG_I("|                             2: Perform server and client authentication                          |");
    LOG_I("|             sslversion    : SSL Version                                                          |");
    LOG_I("|                             0: SSL3.0                                                            |");
    LOG_I("|                             1: TLS1.0                                                            |");
    LOG_I("|                             2: TLS1.1                                                            |");
    LOG_I("|                             3: TLS1.2                                                            |");
    LOG_I("|                             4: ALL                                                               |");
    LOG_I("----------------------------------------------------------------------------------------------------");
}


static int total_len = 0;
static void user_http_callback(QtHttpUsrEvent event, const char *data, size_t len, void *arg)
{
    static FIL fil;
    size_t size = 0;
    total_len += len;
    switch (event)
    {
    case QT_HTTP_USR_EVENT_START:
    {
        LOG_I("open file: %s", (char*)arg);
        FRESULT res = f_open(&fil, (char*)arg, FA_CREATE_ALWAYS | FA_WRITE);
        if (res != FR_OK)
        {
            LOG_W("open file failed");
        }
    }
        break;
    case QT_HTTP_USR_EVENT_DATA:
        if(size < 128)
            LOG_I("http recv data: %s", data);
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

int cli_http_test(s32_t argc, char *argv[])
{
    if (argc < 16)
    {
        cli_http_get_help();
        return -1;
    }
    quectel_http_t handle = NULL;
    handle = quectel_http_init(at_client_get_first());

    quectel_http_setopt(handle, QT_HTTP_OPT_CONTEXT_ID, atoi(argv[1]));
    quectel_http_setopt(handle, QT_HTTP_OPT_REQUEST_HEADER, (bool)(atoi(argv[2])));

    quectel_http_setopt(handle, QT_HTTP_OPT_RESPONSE_HEADER, (bool)(atoi(argv[3])));
    quectel_http_setopt(handle, QT_HTTP_OPT_CONTENT_TYPE, atoi(argv[4]));
    if (strlen(argv[5]) >= 5)
        quectel_http_setopt(handle, QT_HTTP_OPT_CUSTOM_HEADER, atoi(argv[5]));

    quectel_http_setopt(handle, QT_HTTP_OPT_TIMEOUT, atoi(argv[6]));
    // http_config.param.rsptime = atoi(argv[7]);
    // http_config.param.wait_time = atoi(argv[8]);

    ql_SSL_Config http_ssl;
    http_ssl.sslenble = atoi(argv[15]);

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SSL__
    if (http_ssl.sslenble == 1)
    {
        http_ssl.sslctxid = atoi(argv[16]);
        quectel_http_setopt(handle, QT_HTTP_OPT_SSL_CONTEXT_ID, atoi(argv[16]));
        http_ssl.ciphersuite = strtol(argv[17], NULL, 16);
        http_ssl.seclevel = atoi(argv[18]);
        http_ssl.sslversion = atoi(argv[19]);
        LOG_I("            sslctxid      : %d", http_ssl.sslctxid);
        LOG_I("            ciphersuite   : 0x%x", http_ssl.ciphersuite);
        LOG_I("            seclevel      : %d", http_ssl.seclevel);
        LOG_I("            sslversion    : %d", http_ssl.sslversion);
        quectel_http_set_ssl(handle, http_ssl);
    }
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SSL__ */
    char *data = NULL;
    char path[128] = {0};
    snprintf(path, 128, "0:%s", argv[14]);
    total_len = 0;
    quectel_http_setopt(handle, QT_HTTP_OPT_WRITE_FUNCTION, user_http_callback);
    if (QT_HTTP_METHORD_GET == atoi(argv[10]))
    {
        quectel_http_setopt(handle, QT_HTTP_OPT_WRITE_DATA, path);
    }
    else
    {
        quectel_http_setopt(handle, QT_HTTP_OPT_WRITE_DATA, "0:post_info.txt");
        FIL SDFile;
        if (f_open(&SDFile, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
        {
            LOG_E("open file error");
            return -1;
        }
        FSIZE_t file_size = f_size(&SDFile);
        LOG_I("path = %s, file size = %d", path, file_size);
        data = (char*)malloc(file_size+1);
        if (data == NULL)
        {
            LOG_E("malloc error");
            f_close(&SDFile);
            return -1;
        }
        UINT br = 0;
        f_read(&SDFile, data, file_size, &br);
        data[file_size]='\0';
        f_close(&SDFile);
    }
    QtHttpErrCode err = quectel_http_request(handle, argv[9], atoi(argv[10]), data);
    free(data);
    LOG_D("quectel_http_request %d", err);
    quectel_http_deinit(handle);
    return 0;
}

#else
void cli_http_get_help(void)
{
    LOG_W("This function is not supported");
}
int cli_http_test(int argc, char *argv[])
{
    LOG_W("This function is not supported");
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_HTTP_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
