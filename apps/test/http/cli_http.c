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

#if 0
const char *http_ca_cert = "-----BEGIN CERTIFICATE-----\n"
"MIIE4zCCA0ugAwIBAgIJAIaV+KPqwKDTMA0GCSqGSIb3DQEBCwUAMIGGMQswCQYD\n"
"VQQGEwJDTjELMAkGA1UECAwCQUgxCzAJBgNVBAcMAkhGMRAwDgYDVQQKDAdRVUVD\n"
"VEVMMQswCQYDVQQLDAJTVDEWMBQGA1UEAwwNMTEyLjMxLjg0LjE2NDEmMCQGCSqG\n"
"SIb3DQEJARYXZWRkaWUuemhhbmdAcXVlY3RlbC5jb20wIBcNMjIwMTI1MDYyMTA2\n"
"WhgPMjEyMjAxMDEwNjIxMDZaMIGGMQswCQYDVQQGEwJDTjELMAkGA1UECAwCQUgx\n"
"CzAJBgNVBAcMAkhGMRAwDgYDVQQKDAdRVUVDVEVMMQswCQYDVQQLDAJTVDEWMBQG\n"
"A1UEAwwNMTEyLjMxLjg0LjE2NDEmMCQGCSqGSIb3DQEJARYXZWRkaWUuemhhbmdA\n"
"cXVlY3RlbC5jb20wggGiMA0GCSqGSIb3DQEBAQUAA4IBjwAwggGKAoIBgQDLd3I2\n"
"NHixA6r/GOxaJgPuzRuc7QG0e/j/1sYz5aHakQ2eR0DkZHwK+OIAGXFrCqKOAx0y\n"
"J8K/SQWwZxEwTJy/Ptd4dT0aeQn8Fyhb46aVmLwWX3k8IVkGG0jCiDaEvndWZddY\n"
"4P/mW0nexYtG2KvIqR27d9B/jQDUGT9V0HbAn+f5L5/r14j+BICXOeiUPS+gCyBd\n"
"kOhdxvz02mCLYOOtzZb/0vnkYtVTA0rPEHPg05VcJYGWSXUpQvGI6+aVaqMl5vBD\n"
"Sn0k63Yay4ikliU8/SVK9OBO0ifgqAQJy2DTx0dstAhUv9eWgmtSWytu2Sf9HLwb\n"
"86VnZHIKw7SiHMTWDPUBT6XoW9TeKYKotDT4QpHPwWrH5l1Gk8DOIIYYuLUEabSj\n"
"HUzf5HcgvleWvB8oWzb8OaniZSySqYGfV9x0M7irLSjJMSzbgyMyvU7zu8y6MK/t\n"
"AXrVHZZavnw410DNDPvT9179lZg8BkZJmdCI2KQ45vqqhkxVS881cBKlQ+sCAwEA\n"
"AaNQME4wHQYDVR0OBBYEFNUvIPGNPgsC8f1SRtlwaRCLj1eYMB8GA1UdIwQYMBaA\n"
"FNUvIPGNPgsC8f1SRtlwaRCLj1eYMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQEL\n"
"BQADggGBAGEiAVP/Z2odReW2NmwvZ9vtccTe7eQX4XpQqrpgnnRu59bldPi9IXBc\n"
"4VDtmJeVnGEYSIAgkG5u8kItKdeYqHpU4ujXGxbqV2fR8o8+7ywrDWVAn/01guUF\n"
"eZ4CtHnXse1i1Jr73CRlskYYajJrE6wwe1GdcBolzXa/2I//C9hdonfiUNz/kJsL\n"
"4Wn2/tacgXWbBlVqZvoKpqap2halu9zC/vZNTWj8/AV1pXBZGAcTeffdF35O/Aeo\n"
"+5QIhUPJgezuNunyIHyk7wOyDVC6BmgcF+633zVJcz7WUPPXOZNOQg0zijCkwYOg\n"
"uv9L7syz2k82TW0GaYz7YGAp4Ocfza9ltj73QDIQpa70HVFgLTr0VAvN5xLY0nDB\n"
"Ff+2jSOx087lVK++QfgDuRtQ2iWljCSab5Tdfp7TPR4LrljaXE4C2IXDi9FJ+vuL\n"
"P/+3qn1h1ZMzpLVTLPt8iqLJv0soAKeHXB96TETAZDa0pk+4VhadOo8WoKuAm3k9\n"
"EdJNFsgC7A==\n"
"-----END CERTIFICATE-----\n";

const char *http_user = "-----BEGIN CERTIFICATE-----\n"
"MIIEjDCCAvQCCQDUEg/AtXowWjANBgkqhkiG9w0BAQsFADCBhjELMAkGA1UEBhMC\n"
"Q04xCzAJBgNVBAgMAkFIMQswCQYDVQQHDAJIRjEQMA4GA1UECgwHUVVFQ1RFTDEL\n"
"MAkGA1UECwwCU1QxFjAUBgNVBAMMDTExMi4zMS44NC4xNjQxJjAkBgkqhkiG9w0B\n"
"CQEWF2VkZGllLnpoYW5nQHF1ZWN0ZWwuY29tMCAXDTIyMDEyNTA2MjI0MloYDzIx\n"
"MjIwMTAxMDYyMjQyWjCBhjELMAkGA1UEBhMCQ04xCzAJBgNVBAgMAkFIMQswCQYD\n"
"VQQHDAJIRjEQMA4GA1UECgwHUVVFQ1RFSzELMAkGA1UECwwCU1QxFjAUBgNVBAMM\n"
"DTExMi4zMS44NC4xNjQxJjAkBgkqhkiG9w0BCQEWF2VkZGllLnpoYW5nQHF1ZWN0\n"
"ZWwuY29tMIIBojANBgkqhkiG9w0BAQEFAAOCAY8AMIIBigKCAYEA46qNt9Xs+qis\n"
"XaMaE3shUIYQQq1V0cbkwK3eMlK10B3hsx+/i8sH98T07b0tdDoBpzpwlBFx6LhO\n"
"zypJRcPgIiDIEwfB+0oj8BG2/b3BDBDjbMNnFG/vcR7al8HFeqcThtXFTRlaIHay\n"
"bRrxOQ/477VIBPpsw3G3YGpFFyKaIW2yw9GlGBAc0AETGKACkEXf8mgtLhTq0XGS\n"
"9bNMT8H1o40cyP6AxXdzF/jCKOy1YxtJ1CLBHXfNObnMhV0LwGcf2xkv2i5XHlnL\n"
"KHImNDeZmpFuYboHOg1EORRx0Vz27IzX/OC370j4y3FAhPBgx7NPV0/glNejM8VZ\n"
"sxE62OezRenOGMxTvvG5uGNoiyRUWTQfzM2Jv+Ym5RCNhVnW5RLiwDi2OuCt1xU2\n"
"x5d+ncETcltznhiUY7e01LP9nXQllFW/h6jSOcdxEvBRNdlpIbdY5ZBI4XuH1riN\n"
"6Rq3VKSdDJhoXdmhA2471lF3mYyhdy/3Isjez0nElrHQFVRtTYatAgMBAAEwDQYJ\n"
"KoZIhvcNAQELBQADggGBAEEKY4568flzjFCYjjD5CEzIuLpFUyAtmIx6J8dLaEY1\n"
"vyl3+JcYZqERJTfJu4Au0CjeAN3oNis0uKnjONszLU8uBFybqvcG2C45RsmkymEM\n"
"TwiElLLk7yiVhjscjK91AIxRMLgBd6WraNSM7l88LI8SAOjGx/p3GIhHZzSMF1TN\n"
"TKGGNM2lBg/MKwulvvAuqRIHyuElwakcW7z3AF9cUtRe6yeyMp8keJgtCXb71VHz\n"
"LoSRx7yBwxTpownquy2zWMf1LqKQk1fddth7+ZSj2ft4k7BferFEnBALV2ws3pBX\n"
"g3THUydNZyOIoB/iCXza2+daroizGDBiRY4zoLE9azlzLttfPCpISRQEdaX87K2i\n"
"J6OhjDsaobOPoYSu8d11fI7vOuoIjk5VS2UKBfnaGWlF3xHxXJYHyhE6/rijtvyi\n"
"AhWLjWtIIftg0Njmt0YVTWen1Iczd4oanYwPzVFs9AmHhyBJDsFRm5STcDyDbjzy\n"
"Q+Ep7YHxMYXseaUKVeZ8+Q==\n"
"-----END CERTIFICATE-----\n";

const char *http_user_key = "-----BEGIN RSA PRIVATE KEY-----\n"
"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\n"
"-----END RSA PRIVATE KEY-----\n";
#endif

void cli_http_get_help(void)
{
    LOG_I("| http contextid requestheader responseheader contenttype custom_header rsptime                    |");
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
    LOG_I("|                             0xFFFF:ALL above cipher suites                                       |");
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


extern void at_print_raw_cmd(const char *type, const char *cmd, size_t size);

static int total_len = 0;
static void user_http_callback(QL_HTTP_USR_EVENT_E event, const char *data, size_t len, void *arg)
{
    static FIL fil;
    size_t size = 0;
    switch (event)
    {
    case QL_HTTP_USR_EVENT_START:
    {
        total_len = 0;
        LOG_I("open file: %s", (char*)arg);
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
        at_print_raw_cmd("http recv data", data, len);
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

int cli_http_test(s32_t argc, char *argv[])
{
    if (argc < 15)
    {
        LOG_E("Invalid parameter");
        cli_http_get_help();
        return -1;
    }
    LOG_I("     contextid     : %d", atoi(argv[1]));
    LOG_I("     requestheader : %d", atoi(argv[2]));
    LOG_I("     responseheader: %d", atoi(argv[3]));
    LOG_I("     contenttype   : %d", atoi(argv[4]));
    LOG_I("     custom_header : %s", argv[5]);
    LOG_I("     rsptime       : %d", atoi(argv[6]));
    LOG_I("     wait_time     : %d", atoi(argv[7]));
    LOG_I("     request_url   : %s", argv[8]);
    LOG_I("     method        : %d", atoi(argv[9]));
    LOG_I("     request_mode  : %d", atoi(argv[10]));
    LOG_I("     username      : %s", argv[11]);
    LOG_I("     password      : %s", argv[12]);
    LOG_I("     sslenble      : %d", atoi(argv[14]));
    ql_http_t handle = NULL;
    handle = ql_http_init(at_client_get_first());
    ql_SSL_Config http_ssl;
    memset(&http_ssl, 0, sizeof(http_ssl));
    http_ssl.sslenble = atoi(argv[14]);
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SSL__
    if (http_ssl.sslenble == 1)
    {
        http_ssl.sslctxid = atoi(argv[15]);
        http_ssl.ciphersuite = strtol(argv[16], NULL, 16);
        http_ssl.seclevel = atoi(argv[17]);
        http_ssl.sslversion = atoi(argv[18]);
        http_ssl.src_is_path = true;
        http_ssl.cacert_src = "http_ca.pem";
        http_ssl.clientcert_src = "http_user.pem";
        http_ssl.clientkey_src = "http_user_key.pem";
        http_ssl.cacert_dst_path = "http_ca.pem";
        http_ssl.clientcert_dst_path = "http_user.pem";
        http_ssl.clientkey_dst_path = "http_user_key.pem";
        LOG_I("     sslctxid      : %d", http_ssl.sslctxid);
        LOG_I("     ciphersuite   : 0x%x", http_ssl.ciphersuite);
        LOG_I("     seclevel      : %d", http_ssl.seclevel);
        LOG_I("     sslversion    : %d", http_ssl.sslversion);
        ql_http_set_ssl(handle, http_ssl);
    }
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SSL__ */

    ql_http_setopt(handle, QL_HTTP_OPT_CONTEXT_ID, atoi(argv[1]));
    ql_http_setopt(handle, QL_HTTP_OPT_REQUEST_HEADER, (bool)(atoi(argv[2])));

    ql_http_setopt(handle, QL_HTTP_OPT_RESPONSE_HEADER, (bool)(atoi(argv[3])));
    ql_http_setopt(handle, QL_HTTP_OPT_CONTENT_TYPE, atoi(argv[4]));
    if (strlen(argv[5]) >= 5)
        ql_http_setopt(handle, QL_HTTP_OPT_CUSTOM_HEADER, argv[5]);

    ql_http_setopt(handle, QL_HTTP_OPT_TIMEOUT, atoi(argv[6]));
    ql_http_setopt(handle, QL_HTTP_OPT_WAIT_TIME, atoi(argv[7]));
    char *data = NULL;
    char path[128] = {0};
    snprintf(path, 128, "0:%s", argv[13]);
    total_len = 0;
    ql_http_setopt(handle, QL_HTTP_OPT_WRITE_FUNCTION, user_http_callback);
    if (QL_HTTP_METHORD_GET == atoi(argv[9]))
    {
        ql_http_setopt(handle, QL_HTTP_OPT_WRITE_DATA, path);
    }
    else
    {
        ql_http_setopt(handle, QL_HTTP_OPT_WRITE_DATA, "0:post_info.txt");
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
    QL_HTTP_ERR_CODE_E err = ql_http_request(handle, argv[8], atoi(argv[9]), data, (data == NULL ? 0 : strlen(data)));
    free(data);
    if (err != QL_HTTP_OK)
        LOG_E("http request failed");
    else
        LOG_I("http request success");
    ql_http_deinit(handle);
    LOG_I("http request end");
    return 0;
}

#else
void cli_http_get_help(void)
{
    LOG_W("This function is not supported");
}
int cli_http_test(s32_t argc, char *argv[])
{
    LOG_W("This function is not supported");
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_HTTP_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
