#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__
#include "ql_ftp.h"
#include "ql_ssl.h"
#include "cli_ftp.h"
#include "qosa_log.h"


void cli_ftp_get_help(void)
{
    LOG_I("ftp contextid username password filetype transmode rsptimeout hostname port ftp_type directoryToSet local_name rem_name  sslenble ssltype sslctxid ciphersuite seclevel sslversion");
    LOG_I("     contextid     : PDP context ID");
    LOG_I("     username      : Username for logging in to the Ftp(S) server");
    LOG_I("     password      : Password for logging in to the Ftp(S) server");
    LOG_I("     file_type     : The type of transferred data.");
    LOG_I("     0: Binary 1: ASCII");
    LOG_I("     transmode     : Whether the FTP(S) server or client listens on a port for data connection.");
    LOG_I("     0: Active mode, the module will listen on a port for data connection");
    LOG_I("     1: Passive mode, the FTP(S) server will listen on a port for data connection");
    LOG_I("     rsptimeout       : Range: 20-180. Default value: 90. Unit: second.");
    LOG_I("     hostname       : FTP(S) server URL");
    LOG_I("     port           : FTP(S) server port");
    LOG_I("     ftp_type       : FTP fun mode");
    LOG_I("     1: file     list ");
    LOG_I("     2: file     download");
    LOG_I("     3: file     upload");
    LOG_I("     directoryToSet       : The directory of the server");
    LOG_I("     local_name  : Data path in SD card");
    LOG_I("     rem_name  : The file name of the server");
    LOG_I("     sslenble      : Whether ssl is enabled");
    LOG_I("                     0: Disable SSL");
    LOG_I("                     1: Enable SSL");
    LOG_I("            ssltype      : Module used as FTP client or FTPS client");
    LOG_I("                            0 FTP clients");
    LOG_I("                            1 FTPS implicit encryption");
    LOG_I("                            2 FTPS explicit encryption");
    LOG_I("            sslctxid      : SSL context ID used for FTPS(S)");
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


// print all list
static void print_ftp_list(const ql_ftp_file_info_s *head)
{
    LOG_I("file list:");
    const ql_ftp_file_info_s *current = head;
    while (current != NULL)
    {
        LOG_I("\tType: %c %s %d  %s %s %ld %s %s", current->type, current->permissions, current->links,
          current->owner, current->group, current->size, current->date, current->name);
        current = current->next;
    }
    LOG_I("\n");
}

static void progress_callback(int progress, int total, void* cur_size)
{
    LOG_I("ftp exec progress: %d/%d", progress, total);
}
int cli_ftp_test(s32_t argc, char *argv[])
{
    if (argc < 14)
    {
        LOG_E("Invalid parameter");
        cli_ftp_get_help();
        return -1;
    }
    LOG_I("     contextid         : %d", atoi(argv[1]));
    LOG_I("     username          : %s", argv[2]);
    LOG_I("     password          : %s", argv[3]);
    LOG_I("     filetype          : %d", atoi(argv[4]));
    LOG_I("     transmode         : %d", atoi(argv[5]));
    LOG_I("     rsptimeout        : %d", atoi(argv[6]));
    LOG_I("     request_url       : %s", argv[7]);
    LOG_I("     port              : %d", atoi(argv[8]));
    LOG_I("     ftp_type          : %d", atoi(argv[9]));
    LOG_I("     directoryToSet    : %s", argv[10]);
    LOG_I("     local_name        : %s", argv[11]);
    LOG_I("     rem_name          : %s", argv[12]);
    LOG_I("     sslenble          : %d", atoi(argv[13]));
    ql_SSL_Config ssl_config;
    memset(&ssl_config, 0, sizeof(ssl_config));
    ssl_config.sslenble = atoi(argv[13]);
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SSL__
    if (ssl_config.sslenble == 1)
    {
        ssl_config.ssltype = atoi(argv[14]);
        ssl_config.sslctxid = atoi(argv[15]);
        ssl_config.ciphersuite = strtol(argv[16], NULL, 16);
        ssl_config.seclevel = atoi(argv[17]);
        ssl_config.sslversion = atoi(argv[18]);
        ssl_config.src_is_path = true;
        ssl_config.cacert_src = "ftp_ca.pem";
        ssl_config.cacert_dst_path = "ftp_ca.pem";
        LOG_I("     ssltype           : %d", ssl_config.ssltype);
        LOG_I("     sslctxid          : %d", ssl_config.sslctxid);
        LOG_I("     ciphersuite       : 0x%x", ssl_config.ciphersuite);
        LOG_I("     seclevel          : %d", ssl_config.seclevel);
        LOG_I("     sslversion        : %d", ssl_config.sslversion);
    }
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SSL__ */
    ql_ftp_t ftp_handle = ql_ftp_init(argv[7], atoi(argv[8]), at_client_get_first());
    ql_ftp_setopt(ftp_handle, QL_FTP_OPT_CONTEXT_ID, atoi(argv[1]));
    ql_ftp_setopt(ftp_handle, QL_FTP_OPT_FILE_TYPE, atoi(argv[4]));
    ql_ftp_setopt(ftp_handle, QL_FTP_OPT_RSP_TIMEOUT, atoi(argv[6]));
    ql_ftp_setopt(ftp_handle, QL_FTP_OPT_PROGRESS_FUNCTION, progress_callback);
    ql_ftp_setopt(ftp_handle, QL_FTP_OPT_PROGRESS_DATA, ftp_handle);
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SSL__
    if (!ql_ftp_set_ssl(ftp_handle, ssl_config))
        return -1;
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SSL__ */
    if (ql_ftp_login(ftp_handle, argv[2], argv[3]) != QL_FTP_OK )
    {
        LOG_E("Login failed");
        ql_ftp_uninit(ftp_handle);
        return -1;
    }
    QL_FTP_ERR_CODE_E err = ql_ftp_cwd(ftp_handle, argv[10]);
    if (err != QL_FTP_OK)
    {
        ql_ftp_logout(ftp_handle);
        ql_ftp_uninit(ftp_handle);
        return -1;
    }
    if (atoi(argv[9])== 1)
    {
        ql_ftp_file_info_s *file_head = NULL;
        err = ql_ftp_list(ftp_handle, argv[10], &file_head);
        if (err != QL_FTP_OK)
        {
            LOG_E("ftp list failed %s", argv[10]);
        }
        else
        {
            print_ftp_list(file_head);
            ql_ftp_list_free(file_head);
            LOG_I("ftp get file list success");
        }
    }
    else if (atoi(argv[9]) == 2)
    {
        err = ql_ftp_download(ftp_handle, argv[12], argv[11]);
        if (err != QL_FTP_OK)
        {
            LOG_E("ftp download file failed %s %d", argv[12], err);
        }
        else
            LOG_I("ftp download success");
    }
    else if (atoi(argv[9]) == 3)
    {
        err = ql_ftp_upload(ftp_handle, argv[11], argv[12]);
        if (err != QL_FTP_OK)
        {
            LOG_E("ftp upload file failed %s %d", argv[11], err);
        }
        else
            LOG_I("ftp upload success");
    }
    ql_ftp_logout(ftp_handle);
    ql_ftp_uninit(ftp_handle);
    LOG_I("ftp close");
    return err == QL_FTP_OK ? 0 : -1;
}

#else
void cli_ftp_get_help(void)
{
    LOG_W("This function is not supported");
}
int cli_ftp_test(s32_t argc, char *argv[])
{
    LOG_W("This function is not supported");
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
