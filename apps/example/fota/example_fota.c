#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#include "qosa_log.h"
#include "ql_dev.h"
#include "ql_fota.h"

static void ota_progress_callback(QL_FOTA_STATUS_E status, int progress, void *userData)
{
    switch (status)
    {
        case QL_FOTA_INPROGRESS:
            LOG_I("FOTA progress: %d", progress);
            break;
        case QL_FOTA_SUCCEED:
            LOG_I("FOTA succeed");
            cli_reboot(0, NULL);
            break;
        case QL_FOTA_FAIL:
            LOG_E("FOTA failed");
            break;
        case QL_FOTA_SETFLAG:
            LOG_I("FOTA set flag");
            break;
        default:
            break;
    }
}
void example_fota_http(void)
{
    ql_fota_config_u config;
    memset(&config, 0, sizeof(config));
    config.http_config.async = false;
    config.http_config.content_id = 1;
    strcpy(config.http_config.url, "http://112.31.84.164:8300/Wells/Quectel_UFP_STM32F413RGT6_20251022.bin");
    config.http_config.location = QL_FOTA_DOWNLOAD_SD_CARD;
    strcpy(config.http_config.save_path, "FotaFile.bin");
    config.http_config.cb = ota_progress_callback;
    config.http_config.user_data = NULL;

    // https configure, if need.
    config.http_config.ssl_cfg.sslenble = 0;
    config.http_config.ssl_cfg.sslctxid = 0;
    config.http_config.ssl_cfg.ciphersuite = 0xFFFF;
    config.http_config.ssl_cfg.seclevel = SEC_LEVEL_SERVER_AUTHENTICATION;
    config.http_config.ssl_cfg.sslversion = SSL_VERSION_ALL;
    config.http_config.ssl_cfg.src_is_path = true;
    config.http_config.ssl_cfg.cacert_src = "http_ca.pem";
    config.http_config.ssl_cfg.cacert_dst_path = "http_ca.pem";

    ql_fota_firmware_download(QL_FOTA_DWNLD_MOD_HTTP, config);
}

void example_fota_ftp(void)
{
    ql_fota_config_u config;
    memset(&config, 0, sizeof(config));
    config.ftp_config.async = true;
    config.ftp_config.content_id = 1;
    strcpy(config.ftp_config.address, "112.31.84.164");
    config.ftp_config.port = 8311;
    strcpy(config.ftp_config.username, "test");
    strcpy(config.ftp_config.password, "test");
    strcpy(config.ftp_config.work_dir, "/FTP-TEST");
    strcpy(config.ftp_config.file_name, "Quectel_UFP_STM32F413RGT6_20251022.bin");
    config.ftp_config.location = QL_FOTA_DOWNLOAD_SD_CARD;
    strcpy(config.ftp_config.save_path, "FotaFile.bin");
    config.ftp_config.cb = ota_progress_callback;
    config.ftp_config.user_data = NULL;

    // ftps configure, if need.
    config.ftp_config.ssl_cfg.sslenble = 1;
    config.ftp_config.ssl_cfg.ssltype = 1;
    config.ftp_config.ssl_cfg.sslctxid = 0;
    config.ftp_config.ssl_cfg.ciphersuite = 0xFFFF;
    config.ftp_config.ssl_cfg.seclevel = SEC_LEVEL_SERVER_AUTHENTICATION;
    config.ftp_config.ssl_cfg.sslversion = SSL_VERSION_ALL;
    config.ftp_config.ssl_cfg.src_is_path = true;
    config.ftp_config.ssl_cfg.cacert_src = "ftp_ca.pem";
    config.ftp_config.ssl_cfg.cacert_dst_path = "ftp_ca.pem";

    
    ql_fota_firmware_download(QL_FOTA_DWNLD_MOD_FTP, config);
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */