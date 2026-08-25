#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#include "qosa_log.h"
#include "ql_fota.h"
#include "ql_dev.h"
#include "cli_fota.h"
void cli_fota_get_help(void)
{
    LOG_I("  0: fota by http ");
    LOG_I("        fota download_type async contextid url location save_path");
    LOG_I("             sslenble sslctxid ciphersuite seclevel sslversion");
    LOG_I("              download_type  : 0-http(s)");
    LOG_I("              async          : 0-false, 1-true");
    LOG_I("              contextid      : PDP context ID, range: 1-16");
    LOG_I("              url            : HTTP(S) server URL");
    LOG_I("              location       : 0-scard, 1-flash, now only support sdcard");
    LOG_I("              save_path      : FotaFile.bin(File path on the storage)");
    LOG_I("              sslenble       : Whether ssl is enabled, 0-disabled, 1-enabled");
    LOG_I("              sslctxid       : SSL context ID used for https, range: 0-5");
    LOG_I("              ciphersuite    : Numeric type in HEX format. SSL cipher suites");
    LOG_I("              seclevel       : Authentication mode");
    LOG_I("                               0: No authentication");
    LOG_I("                               1: Perform server authentication");
    LOG_I("                               2: Perform server and client authentication");
    LOG_I("              sslversion     : SSL Version");
    LOG_I("                               0: SSL3.0");
    LOG_I("                               1: TLS1.0");
    LOG_I("                               2: TLS1.1");
    LOG_I("                               3: TLS1.2");
    LOG_I("                               4: ALL");
    LOG_I("  1: fota by ftp ");
    LOG_I("        fota download_type async contextid address port username password wrok_dir rem_name location save_path");
    LOG_I("              sslenble ssltype sslctxid ciphersuite seclevel sslversion");
    LOG_I("              download_type  : 1-ftp(s)");
    LOG_I("              async          : 0-false, 1-true");
    LOG_I("              contextid      : PDP context ID, range: 1-16");
    LOG_I("              address        : FTP(S) server address");
    LOG_I("              port           : FTP(S) server port");
    LOG_I("              username       : Username for logging in to the ftp(s) server");
    LOG_I("              password       : Password for logging in to the Ftp(S) server");
    LOG_I("              wrok_dir       : Working directory on the FTP(S) server");
    LOG_I("              rem_name       : The file name of the server");
    LOG_I("              location       : 0-scard, 1-flash, now only support sdcard");
    LOG_I("              save_path      : FotaFile.bin(File path on the storage)");
    LOG_I("              sslenble       : Whether ssl is enabled, 0-disabled, 1-enabled");
    LOG_I("              ssltype        : Module used as FTP client or FTPS client");
    LOG_I("                               0 FTP clients");
    LOG_I("                               1 FTPS implicit encryption");
    LOG_I("                               2 FTPS explicit encryption");
    LOG_I("              sslctxid       : SSL context ID used for HTTPS, range: 0-5");
    LOG_I("              ciphersuite    : Numeric type in HEX format. SSL cipher suites");
    LOG_I("              seclevel       : Authentication mode");
    LOG_I("                               0: No authentication");
    LOG_I("                               1: Perform server authentication");
    LOG_I("                               2: Perform server and client authentication");
    LOG_I("              sslversion     : SSL Version");
    LOG_I("                               0: SSL3.0");
    LOG_I("                               1: TLS1.0");
    LOG_I("                               2: TLS1.1");
    LOG_I("                               3: TLS1.2");
    LOG_I("                               4: ALL");
}

static void ota_progress_callback(QL_FOTA_STATUS_E status, int progress, void *userData)
{
    switch (status)
    {
        case QL_FOTA_INPROGRESS:
            LOG_I("FOTA download: %d", progress);
            break;
        case QL_FOTA_SUCCEED:
            LOG_I("FOTA download successfully");
            cli_reboot(0, NULL);
            break;
        case QL_FOTA_FAIL:
            LOG_I("FOTA download failed");
            break;
        case QL_FOTA_SETFLAG:
            LOG_I("FOTA set flag");
            break;
        default:
            break;
    }
}

static int cli_fota_test_http(int argc, char *argv[])
{ 
    LOG_I("args: %d", argc);
    if (argc < 8 || (atoi(argv[7]) == 1 && argc < 12))
    {
        LOG_E("Invalid parameter");
        cli_fota_get_help();
        return -1;
    }

    ql_fota_config_u config;
    memset(&config, 0, sizeof(config));
    config.http_config.async = atoi(argv[2]);
    config.http_config.content_id = atoi(argv[3]);
    strcpy(config.http_config.url, argv[4]);
    config.http_config.location = atoi(argv[5]);
    strcpy(config.http_config.save_path, argv[6]);
    config.http_config.cb = ota_progress_callback;
    config.http_config.user_data = NULL;
    config.http_config.ssl_cfg.sslenble = atoi(argv[7]);
    LOG_I("     download_type : %s", "http(s)");
    LOG_I("     async         : %d", config.http_config.async);
    LOG_I("     contextid     : %d", config.http_config.content_id);
    LOG_I("     url           : %s", config.http_config.url);
    LOG_I("     location      : %d", config.http_config.location);
    LOG_I("     save_path     : %s", config.http_config.save_path);
    LOG_I("     sslenble      : %d", config.http_config.ssl_cfg.sslenble);

    if (config.http_config.ssl_cfg.sslenble)
    {
        config.http_config.ssl_cfg.sslctxid = atoi(argv[8]);
        config.http_config.ssl_cfg.ciphersuite = strtol(argv[9], NULL, 16);
        config.http_config.ssl_cfg.seclevel = atoi(argv[10]);
        config.http_config.ssl_cfg.sslversion = atoi(argv[11]);
        config.http_config.ssl_cfg.src_is_path = true;
        config.http_config.ssl_cfg.cacert_src = "http_ca.pem";
        config.http_config.ssl_cfg.clientcert_src = "http_user.pem";
        config.http_config.ssl_cfg.clientkey_src = "http_user_key.pem";
        config.http_config.ssl_cfg.cacert_dst_path = "http_ca.pem";
        config.http_config.ssl_cfg.clientcert_dst_path = "http_user.pem";
        config.http_config.ssl_cfg.clientkey_dst_path = "http_user_key.pem";
        LOG_I("     sslctxid      : %d", config.http_config.ssl_cfg.sslctxid);
        LOG_I("     ciphersuite   : 0x%x", config.http_config.ssl_cfg.ciphersuite);
        LOG_I("     seclevel      : %d", config.http_config.ssl_cfg.seclevel);
        LOG_I("     sslversion    : %d", config.http_config.ssl_cfg.sslversion);
    }
    int ret = ql_fota_firmware_download(QL_FOTA_DWNLD_MOD_HTTP, config);
    if (!config.http_config.async)
        LOG_I("FOTA test end");
    else
        LOG_I("FOTA test start");
    return ret;
}

static int cli_fota_test_ftp(int argc, char *argv[])
{
    LOG_I("args: %d", argc);
    if (argc < 13 || (atoi(argv[12]) == 1 && argc < 18))
    {
        LOG_E("Invalid parameter");
        cli_fota_get_help();
        return -1;
    }

    ql_fota_config_u config;
    memset(&config, 0, sizeof(config));
    config.ftp_config.async = atoi(argv[2]);
    config.ftp_config.content_id = atoi(argv[3]);
    strcpy(config.ftp_config.address, argv[4]);
    config.ftp_config.port = atoi(argv[5]);
    strcpy(config.ftp_config.username, argv[6]);
    strcpy(config.ftp_config.password, argv[7]);
    strcpy(config.ftp_config.work_dir, argv[8]);
    strcpy(config.ftp_config.file_name, argv[9]);
    config.ftp_config.location = atoi(argv[10]);
    strcpy(config.ftp_config.save_path, argv[11]);
    config.ftp_config.ssl_cfg.sslenble = atoi(argv[12]);
    config.ftp_config.cb = ota_progress_callback;
    config.ftp_config.user_data = NULL;
    LOG_I("     download_type : %s", "ftp(s)");
    LOG_I("     async         : %s", config.ftp_config.async == 0 ? "false" : "true");
    LOG_I("     contextid     : %d", config.ftp_config.content_id);
    LOG_I("     address       : %s", config.ftp_config.address);
    LOG_I("     port          : %d", config.ftp_config.port);
    LOG_I("     username      : %s", config.ftp_config.username);
    LOG_I("     password      : %s", config.ftp_config.password);
    LOG_I("     work_dir      : %s", config.ftp_config.work_dir);
    LOG_I("     rem_name      : %s", config.ftp_config.file_name);
    LOG_I("     location      : %d", config.ftp_config.location);
    LOG_I("     save_path     : %s", config.ftp_config.save_path);
    LOG_I("     sslenble      : %d", config.ftp_config.ssl_cfg.sslenble);

    if (config.ftp_config.ssl_cfg.sslenble)
    {
        config.ftp_config.ssl_cfg.ssltype = atoi(argv[13]);
        config.ftp_config.ssl_cfg.sslctxid = atoi(argv[14]);
        config.ftp_config.ssl_cfg.ciphersuite = strtol(argv[15], NULL, 16);
        config.ftp_config.ssl_cfg.seclevel = atoi(argv[16]);
        config.ftp_config.ssl_cfg.sslversion = atoi(argv[17]);
        config.ftp_config.ssl_cfg.src_is_path = true;
        config.ftp_config.ssl_cfg.cacert_src = "ftp_ca.pem";
        config.ftp_config.ssl_cfg.cacert_dst_path = "ftp_ca.pem";
        LOG_I("     ssltype       : %d", config.ftp_config.ssl_cfg.ssltype);
        LOG_I("     sslctxid      : %d", config.ftp_config.ssl_cfg.sslctxid);
        LOG_I("     ciphersuite   : 0x%x", config.ftp_config.ssl_cfg.ciphersuite);
        LOG_I("     seclevel      : %d", config.ftp_config.ssl_cfg.seclevel);
        LOG_I("     sslversion    : %d", config.ftp_config.ssl_cfg.sslversion);
    }
    int ret = ql_fota_firmware_download(QL_FOTA_DWNLD_MOD_FTP, config);
    if (!config.ftp_config.async)
        LOG_I("FOTA test end");
    else
        LOG_I("FOTA test start");
    return ret;
}

int cli_fota_test(s32_t argc, char *argv[])
{
    if (argc < 2)
    {
        LOG_E("Invalid parameter");
        cli_fota_get_help();
        return -1;
    }
    if (atoi(argv[1])== 0)
        return cli_fota_test_http(argc, argv);
    else if (atoi(argv[1])== 1)
        return cli_fota_test_ftp(argc, argv);
    else
    {
        LOG_E("Invalid parameter");
        cli_fota_get_help();
        return -1;
    }
}

#endif