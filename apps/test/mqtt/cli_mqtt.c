#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__
#include "cli_mqtt.h"
#include "qosa_log.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#elif __linux__
#else
#include "cmsis_os.h"
#endif
 
static ql_mqtt_t s_handle[6] = {NULL};
void cli_mqtt_get_help(void)
{
    LOG_I("  1.   open mqtt ");
    LOG_I("              mqtt test_type Server_type 0:ALP 1:other  Client_ID server port ProductKey/username DeviceName/password DeviceSecret sslenble ciphersuite seclevel sslversion");
    LOG_I("     example: mqtt 1 0 Test a1vvrmkn43t.iot-as-mqtt.cn-shanghai.aliyuncs.com 1883 a1vvrmkn43t NiFtKoHMcu6j0VIXtC6e 3115a9a768482d98a28d7390e7b9376b 0");
    LOG_I("  2.  sub topic");
    LOG_I("              mqtt test_type  mqtt_fd topic_name ");
    LOG_I("     example: mqtt 2 0 /a1vvrmkn43t/NiFtKoHMcu6j0VIXtC6e/user/tre1");
    LOG_I("  3.   public topic");
    LOG_I("              mqtt test_type  mqtt_fd topic_name mssagec");
    LOG_I("     example: mqtt 3 1 /a1vvrmkn43t/p1U1UtVAPjZhkOEZnlUt/user/get 111");
    LOG_I(" 4.   dis mqtt");
    LOG_I("              mqtt test_type  mqtt_fd ");
    LOG_I("     example: mqtt 4 1 ");
    LOG_I("     sslenble      : Whether ssl is enabled");
    LOG_I("                     0: Disable SSL");
    LOG_I("                     1: Enable SSL");
    LOG_I("            ciphersuite   : Numeric type in HEX format. SSL cipher suites");
    LOG_I("                            0x0035:TLS_RSA_WITH_AES_256_CBC_SHA");
    LOG_I("                            0x002F:TLS_RSA_WITH_AES_128_CBC_SHA");
    LOG_I("                            0x0005:TLS_RSA_WITH_RC4_128_SHA");
    LOG_I("                            0x0004:TLS_RSA_WITH_RC4_128_MD5");
    LOG_I("                            0x000A:TLS_RSA_WITH_3DES_EDE_CBC_SHA");
    LOG_I("                            0x003D:TLS_RSA_WITH_AES_256_CBC_SHA256");
    LOG_I("                            0xC002:TLS_ECDH_ECDSA_WITH_RC4_128_SHA");
    LOG_I("                            0xC003:TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA");
    LOG_I("                            0xC004:TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA");
    LOG_I("                            0xC005:TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA");
    LOG_I("                            0xC007:TLS_ECDHE_ECDSA_WITH_RC4_128_SHA");
    LOG_I("                            0xC008:TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA");
    LOG_I("                            0xC009:TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA");
    LOG_I("                            0xC00A:TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA");
    LOG_I("                            0xC011:TLS_ECDHE_RSA_WITH_RC4_128_SHA");
    LOG_I("                            0xC012:TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA");
    LOG_I("                            0xC013:TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA");
    LOG_I("                            0xC014:TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA");
    LOG_I("                            0xC00C:TLS_ECDH_RSA_WITH_RC4_128_SHA");
    LOG_I("                            0xC00D:TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA");
    LOG_I("                            0xC00E:TLS_ECDH_RSA_WITH_AES_128_CBC_SHA");
    LOG_I("                            0xC00F:TLS_ECDH_RSA_WITH_AES_256_CBC_SHA");
    LOG_I("                            0xC023:TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256");
    LOG_I("                            0xC024:TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384");
    LOG_I("                            0xC025:TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256");
    LOG_I("                            0xC026:TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384");
    LOG_I("                            0xC027:TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256");
    LOG_I("                            0xC028:TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384");
    LOG_I("                            0xC029:TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256");
    LOG_I("                            0xC02A:TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384");
    LOG_I("                            0xC02B:TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256");
    LOG_I("                            0xC02F:TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256");
    LOG_I("                            0xC0A8:TLS_PSK_WITH_AES_128_CCM_8");
    LOG_I("                            0x00AE:TLS_PSK_WITH_AES_128_CBC_SHA256");
    LOG_I("                            0xC0AE:TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8");
    LOG_I("                            0xFFFF:ALL");
    LOG_I("            seclevel      : Authentication mode");
    LOG_I("                            0: No authentication");
    LOG_I("                            1: Perform server authentication");
    LOG_I("                            2: Perform server and client authentication if requested by the remote server");
    LOG_I("            sslversion    : SSL Version");
    LOG_I("                            0: SSL3.0");
    LOG_I("                            1: TLS1.0");
    LOG_I("                            2: TLS1.1");
    LOG_I("                            3: TLS1.2");
    LOG_I("                            4: ALL");
}

static void sub_func(const char* topic, size_t size, const char* msg, void* arg)
{
    LOG_I("topic = %s, len = %d, msg = %s %p", topic, size, msg, arg);
}

static int cli_mqtt_connect(s32_t argc, char *argv[])
{
    if (argc < 10)
    {
        LOG_E("Invalid parameter");
        cli_mqtt_get_help();
        return -1;
    }
    LOG_I("     test_type              : %d", atoi(argv[1]));
    LOG_I("     Server_type            : %d", atoi(argv[2]));
    LOG_I("     Client_ID              : %s", argv[3]);
    LOG_I("     server                 : %s", argv[4]);
    LOG_I("     port                   : %d", atoi(argv[5]));
    LOG_I("     ProductKey/username    : %s", argv[6]);
    LOG_I("     DeviceName/password    : %s", argv[7]);
    LOG_I("     DeviceSecret           : %s", argv[8]);
    LOG_I("     sslenble               : %d", atoi(argv[9]));
    ql_SSL_Config ssl_config;
    memset(&ssl_config, 0, sizeof(ssl_config));
    ssl_config.sslenble = atoi(argv[9]);
    if (ssl_config.sslenble)
    {
        ssl_config.sslctxid = 0;
        ssl_config.ssltype = 0;
        ssl_config.ciphersuite = strtol(argv[10], NULL, 16);
        ssl_config.seclevel = atoi(argv[11]);
        ssl_config.sslversion = atoi(argv[12]);
        ssl_config.src_is_path = true;
        ssl_config.cacert_src = "mqtt_ca.pem";
        ssl_config.clientcert_src = "mqtt_user.pem";
        ssl_config.clientkey_src = "mqtt_user_key.pem";
        ssl_config.cacert_dst_path = "mqtt_ca.pem";
        ssl_config.clientcert_dst_path = "mqtt_user.pem";
        ssl_config.clientkey_dst_path = "mqtt_user_key.pem";
        LOG_I("     ciphersuite            : 0x%x", ssl_config.ciphersuite);
        LOG_I("     seclevel               : %d", ssl_config.seclevel);
        LOG_I("     sslversion             : %d", ssl_config.sslversion);
    }
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        if (NULL == s_handle[i])
            break;
    }
    if (i >= 6)
        return -1;
    s_handle[i] = ql_mqtt_create(at_client_get_first());
    if (NULL == s_handle[i])
        return -1;

    const char *username = (strcmp(argv[6], "0") == 0 || strcmp(argv[7], "0") == 0) ? NULL : argv[6];
    const char *password = (username == NULL) ? NULL : argv[7];
    if (atoi(argv[2]) == 0)
    {
        qt_mqtt_ali_auth_options_s auth = {.product_key = argv[6], .device_name = argv[7], .device_secret = argv[8]};
        ql_mqtt_setopt(s_handle[i], QL_MQTT_OPT_ALI_AUTH, &auth);
        username = NULL;
        password = NULL;
    }
    if (ssl_config.sslenble)
        if (!ql_mqtt_set_ssl(s_handle[i], ssl_config))
        {
            LOG_E("mqtt set ssl failed");
            ql_mqtt_destroy(s_handle[i]);
            s_handle[i] = NULL;
            return -1;
        }
    ql_mqtt_setopt(s_handle[i], QL_MQTT_OPT_SUB_CALLBACK, sub_func);
    ql_mqtt_setopt(s_handle[i], QL_MQTT_OPT_USER_DATA, s_handle[i]);
    QL_MQTT_ERR_CODE_E err = ql_mqtt_connect(s_handle[i], argv[4], atoi(argv[5]), username, password);
    if (err != QL_MQTT_OK)
    {
        LOG_E("mqtt connect failed");
        ql_mqtt_destroy(s_handle[i]);
        s_handle[i] = NULL;
        return -1;
    }
    LOG_I("mqtt connect success");
    return 0;
}

static int cli_mqtt_sub(s32_t argc, char *argv[])
{
    if (argc < 4)
    {
        LOG_E("Invalid parameter");
        cli_mqtt_get_help();
        return -1;
    }
    LOG_I("     test_type         : %d", atoi(argv[1]));
    LOG_I("     mqtt_fd           : %d", atoi(argv[2]));
    LOG_I("     topic_name        : %s", argv[3]);
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        if (atoi(argv[2]) == s_handle[i]->client_idx)
            break;
    }
    if (i >= 6)
    {
        LOG_E("current id not used");
        return -1;
    }
    if (ql_mqtt_sub(s_handle[i], argv[3], QL_MQTT_QOS0) != QL_MQTT_OK)
    {
        LOG_E("mqtt subscribe failed");
        return -1;
    }
    else
        LOG_I("mqtt subscribe success");
    return 0;
}

static int cli_mqtt_pub(s32_t argc, char *argv[])
{
    if (argc < 5)
    {
        LOG_E("Invalid parameter");
        cli_mqtt_get_help();
        return -1;
    }
    LOG_I("     test_type         : %d", atoi(argv[1]));
    LOG_I("     mqtt_fd           : %d", atoi(argv[2]));
    LOG_I("     topic_name        : %s", argv[3]);
    LOG_I("     message           : %s", argv[4]);
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        if (atoi(argv[2]) == s_handle[i]->client_idx)
            break;
    }
    if (i >= 6)
    {
        LOG_E("current id not used");
        return -1;
    }
    if (ql_mqtt_pub(s_handle[i], argv[3], argv[4], QL_MQTT_QOS0, false) != QL_MQTT_OK)
    {
        LOG_E("mqtt publish failed");
        return -1;
    }
    else
        LOG_I("mqtt publish success");
    return 0; 
}

static int cli_mqtt_disconnect(s32_t argc, char *argv[])
{
    if (argc < 3)
    {
        LOG_E("Invalid parameter");
        cli_mqtt_get_help();
        return -1;
    }
    LOG_I("     test_type         : %d", atoi(argv[1]));
    LOG_I("     mqtt_fd           : %d", atoi(argv[2]));
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        if (atoi(argv[2]) == s_handle[i]->client_idx)
            break;
    }
    if (i >= 6)
    {
        LOG_E("current id not used");
        return -1;
    }
    ql_mqtt_disconnect(s_handle[i]);
    ql_mqtt_destroy(s_handle[i]);
    LOG_I("mqtt close");
    s_handle[i] = NULL;
    return 0;
}
int cli_mqtt_test(s32_t argc, char *argv[])
{
    if (argc < 2) 
    {
        LOG_E("Invalid parameter");
        cli_mqtt_get_help();
        return -1;
    }

    if (atoi(argv[1]) == 1)
    {
        return cli_mqtt_connect(argc, argv);
    }
    else if (atoi(argv[1]) == 2)
    {
        return cli_mqtt_sub(argc, argv);
    }
    else if (atoi(argv[1]) == 3)
    {
        return cli_mqtt_pub(argc, argv);
    }
    else if (atoi(argv[1]) == 4)
    {
        return cli_mqtt_disconnect(argc, argv);
    }
    return 0;
}

#else
void cli_mqtt_get_help(void)
{
    LOG_W("This function is not supported");
}
int cli_mqtt_test(s32_t argc, char *argv[])
{
    LOG_W("This function is not supported");
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
