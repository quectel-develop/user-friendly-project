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
    ql_ftp_cwd(handle, "/FTP-TEST");
    ql_ftp_upload(handle, "0:111.txt", "dst_3k.txt");
    ql_ftp_file_info_s *file_head = NULL;
    ql_ftp_list(handle, "/FTP-TEST", &file_head);
    ql_ftp_list_free(file_head);
    ql_ftp_download(handle, "dst_3k.txt", "0:dst_3k.txt");
    ql_ftp_file_delete(handle, "dst_3k.txt");
    ql_ftp_logout(handle);
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
    // ql_ftp_mkdir(handle, "/FTP-TEST");
    ql_ftp_cwd(handle, "/FTP-TEST");
    ql_ftp_upload(handle, "0:111.txt", "111.txt");
    ql_ftp_file_info_s *file_head = NULL;
    ql_ftp_list(handle, "/FTP-TEST", &file_head);
    ql_ftp_list_free(file_head);
    ql_ftp_download(handle, "111.txt", "0:222.txt");
    ql_ftp_file_delete(handle, "111.txt");
    ql_ftp_logout(handle);
    ql_ftp_uninit(handle);
}


 static void user_write_cb(QL_FTP_USR_DOWNLOAD_EVENT_E event, const char *data, size_t len, void *user_data)
{
    static FIL fil;
    static size_t total_len = 0;
    size_t size = 0;

    switch (event)
    {
    case QL_FTP_USR_DOWNLOAD_START:
    {
        LOG_I("open file: %s", (char*)user_data);
        FRESULT res = f_open(&fil, (char*)user_data, FA_CREATE_ALWAYS | FA_WRITE);
        if (res != FR_OK)
        {
            LOG_W("open file failed");
        }
    }
        break;
    case QL_FTP_USR_DOWNLOAD_DATA:
        // at_print_raw_cmd("ftp recv data", data, len);
        total_len += len;
        if ((fil.flag & FA_WRITE))
            f_write(&fil, data, len, &size);
        break;
    case QL_FTP_USR_DOWNLOAD_END:
        LOG_I("ftp total len = %u", total_len);
        f_close(&fil);
        break;
    default:
        break;
    }
}

void example_ftp_download_by_callback()
{
    ql_ftp_t handle = ql_ftp_init("112.31.84.164", 8309, at_client_get_first());
    ql_ftp_setopt(handle, QL_FTP_OPT_RSP_TIMEOUT, 80);
    ql_ftp_setopt(handle, QL_FTP_OPT_WRITE_FUNCTION, user_write_cb);
    ql_ftp_setopt(handle, QL_FTP_OPT_WRITE_DATA, "0:111.txt");
    ql_ftp_login(handle, "test", "test");
    ql_ftp_cwd(handle, "/FTP-TEST");
    ql_ftp_download(handle, "src_9k.txt", NULL);
    ql_ftp_logout(handle);
    ql_ftp_uninit(handle);
    LOG_I("ftp download done");
}

static size_t user_read_cb(char *buf, size_t len, void *user_data)
{
    FIL *fil = (FIL*)user_data;
    UINT br = 0;
    f_read(fil, buf, len, &br);
    return br;
}
void example_ftp_upload_by_callback()
{
    FIL fil;
    if (f_open(&fil, "0:111.txt", FA_OPEN_EXISTING | FA_READ) != FR_OK)
    {
        LOG_E("open file error");
        return;
    }
    ql_ftp_t handle = ql_ftp_init("112.31.84.164", 8309, at_client_get_first());
    ql_ftp_setopt(handle, QL_FTP_OPT_RSP_TIMEOUT, 80);
    ql_ftp_setopt(handle, QL_FTP_OPT_READ_FUNCTION, user_read_cb);
    ql_ftp_setopt(handle, QL_FTP_OPT_READ_DATA, (void*)&fil);
    ql_ftp_login(handle, "test", "test");
    ql_ftp_cwd(handle, "/FTP-TEST");
    ql_ftp_upload(handle, NULL, "dst_3k.txt");
    f_close(&fil);
    ql_ftp_logout(handle);
    ql_ftp_uninit(handle);
    LOG_I("ftp upload done");
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
