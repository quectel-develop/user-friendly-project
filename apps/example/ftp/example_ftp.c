#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__
#include "ql_ftp.h"
#include "qosa_log.h"

void example_ftp()
{
    quectel_ftp_t handle = quectel_ftp_init("112.31.84.164", 8309, at_client_get_first());
    quectel_ftp_setopt(handle, QT_FTP_OPT_FILE_TYPE, QT_FTP_FILE_TYPE_BINARY);
    quectel_ftp_setopt(handle, QT_FTP_OPT_CONTEXT_ID, 1);
    quectel_ftp_setopt(handle, QT_FTP_OPT_RSP_TIMEOUT, 80);
    quectel_ftp_setopt(handle, QT_FTP_OPT_SSL_TYPE, QT_FTP_SSL_TYPE_CLIENT);
    quectel_ftp_setopt(handle, QT_FTP_OPT_SSL_CTXID, 0);
    quectel_ftp_setopt(handle, QT_FTP_OPT_SSL_DATA_ADDR, QT_FTP_DATA_ADDR_PORT);
    quectel_ftp_setopt(handle, QT_FTP_OPT_REST_ENABLE, false);
    QtFtpErrCode err = quectel_ftp_login(handle, "test", "test");
    LOG_D("err = %d", err);
    // quectel_ftp_rmdir(handle, "/Wells");
    // quectel_ftp_mkdir(handle, "/Wells");
    quectel_ftp_cwd(handle, "/Wells");
    // char pwd_path[256];
    // quectel_ftp_pwd(handle, pwd_path);
    // quectel_ftp_upload(handle, "0:no_file.txt", "111.txt");
    // quectel_ftp_upload(handle, "0:111.txt", "111.txt");
    // quectel_ftp_upload(handle, "UFS:ca.pem", "ca.pem");
    // quectel_ftp_download(handle, "no_file.txt", "0:222.txt");
    // quectel_ftp_download(handle, "111.txt", "0:222.txt");
    // quectel_ftp_download(handle, "ca.pem", "UFS:ca1.pem");
    quectel_ftp_file_delete(handle, "ca.pem");
    // quectel_ftp_list(handle, "/");
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
