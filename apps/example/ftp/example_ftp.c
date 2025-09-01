#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__
#include "ql_ftp.h"
#include "qosa_log.h"

void example_ftp()
{
    ql_ftp_t handle = ql_ftp_init("112.31.84.164", 8309, at_client_get_first());
    ql_ftp_setopt(handle, QL_FTP_OPT_CONTEXT_ID, 1);
    ql_ftp_setopt(handle, QL_FTP_OPT_FILE_TYPE, QL_FTP_FILEL_TYPE_ASCII);
    ql_ftp_setopt(handle, QL_FTP_OPT_RSP_TIMEOUT, 80);
    QL_FTP_ERR_CODE_E err = ql_ftp_login(handle, "test", "test");
    LOG_D("err = %d", err);
    // ql_ftp_mkdir(handle, "/Test");
    ql_ftp_cwd(handle, "/Test");
    ql_ftp_upload(handle, "0:111.txt", "dst_3k.txt");
    ql_ftp_file_info_s *file_head = NULL;
    ql_ftp_list(handle, "/Test", &file_head);
    ql_ftp_list_free(file_head);
    ql_ftp_download(handle, "dst_3k.txt", "0:dst_3k.txt");
    ql_ftp_file_delete(handle, "dst_3k.txt");
    ql_ftp_uninit(handle);
}

void example_ftps()
{
    ql_ftp_t handle = ql_ftp_init("112.31.84.164", 8311, at_client_get_first());
    ql_ftp_setopt(handle, QL_FTP_OPT_FILE_TYPE, QL_FTP_FILE_TYPE_BINARY);
    ql_ftp_setopt(handle, QL_FTP_OPT_CONTEXT_ID, 1);
    ql_ftp_setopt(handle, QL_FTP_OPT_RSP_TIMEOUT, 80);


    ql_SSL_Config ssl_config;
    memset(&ssl_config, 0, sizeof(ssl_config));
    ssl_config.sslenble = 1;
    ssl_config.ssltype = 1;
    ssl_config.sslctxid = 0;
    ssl_config.ciphersuite = 0xFFFF;
    ssl_config.seclevel = SEC_LEVEL_SERVER_AUTHENTICATION;
    ssl_config.sslversion = SSL_VERSION_ALL;
    ssl_config.src_is_path = true;   // if false cacert_src is file content
    ssl_config.cacert_src = "ftp_ca.pem";
    ssl_config.cacert_dst_path = "ftp_ca.pem";
    ql_ftp_set_ssl(handle, ssl_config);

    QL_FTP_ERR_CODE_E err = ql_ftp_login(handle, "test", "test");
    LOG_D("err = %d", err);
    ql_ftp_mkdir(handle, "/FTP-TEST");
    ql_ftp_cwd(handle, "/TEST");
    ql_ftp_upload(handle, "0:111.txt", "111.txt");
    ql_ftp_file_info_s *file_head = NULL;
    ql_ftp_list(handle, "/FTP-TEST", &file_head);
    ql_ftp_list_free(file_head);
    ql_ftp_download(handle, "111.txt", "0:222.txt");
    ql_ftp_file_delete(handle, "111.txt"); 
    ql_ftp_uninit(handle);
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
