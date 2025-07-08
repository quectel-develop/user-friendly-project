#include "QuectelConfig.h"
#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_FTP_S_EXAMPLE__
#include "bg95_ftp.h"
#include "debug_service.h"
#include "broadcast_service.h"
#include "debug_service.h"
#include "bg95_ssl.h"
#include "example_ftp.h"
#include "qosa_def.h"
#include "qosa_log.h"

FTP_Config user_ftp_config = {
    .file = 0,
    .read_cb = NULL,
    .write_cb = NULL,

};

FTP_Config Ql_get_ftp_Config()
{
    return user_ftp_config;
}
// 鍥炶皟鍑芥暟锛岀敤浜庝粠鏂囦欢涓鍙栨暟鎹?
size_t read_data_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    FIL *file = (FIL *)stream; // 灏唖tream杞崲涓篎IL鎸囬拡
    UINT br;                   // 瀹為檯璇诲彇鐨勫瓧鑺傛暟
    FRESULT fr;

    // 浠庢枃浠朵腑璇诲彇鏁版嵁
    fr = f_read(file, ptr, size * nmemb, &br);
    if (fr != QOSA_OK)
    {
        LOG_E("Error reading file.\n");
        return 0; // 鍙戠敓閿欒鏃惰繑鍥?0
    }

    return br; // 杩斿洖瀹為檯璇诲彇鐨勫瓧鑺傛暟
}
// 鍐欏叆鍥炶皟鍑芥暟
size_t ftp_write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    // LOG_E("Entering ftp_write_callback");

    static int fd = -1;
    UINT bw;
    fd = f_write(&user_ftp_config.file, ptr, size * nmemb, &bw);

    if (fd != QOSA_OK)
    {
        // f_write 鍙戠敓閿欒锛屾墦鍗伴敊璇俊鎭?
        LOG_E("f_write error: %d", fd);
        return 0; // 杩斿洖 0 琛ㄧず鍐欏叆澶辫触
    }

    if (bw < size * nmemb)
    {
        // 鍐欏叆鐨勫瓧鑺傛暟灏戜簬棰勬湡锛屽彲鑳芥剰鍛崇潃纾佺洏宸叉弧鎴栧叾浠栭棶棰?
        LOG_I("Partial write: %u bytes written out of %u", bw, size * nmemb);
    }
    else
    {
        LOG_I("Write successful: %u bytes written", bw);
    }

    return bw; // 杩斿洖瀹為檯鍐欏叆鐨勫瓧鑺傛暟
}
int ftp_test_open(ftp_test_config *config)
{
    FTP_Config myConfig = Ql_get_ftp_Config();
    if (QL_ftp_cfg(&myConfig) != 0)
    {
        LOG_D("QL_ftp_cfg error");
        return -1;
    }
    if (QL_ftp_open(&myConfig) != 0)
    {
        LOG_D("QL_ftp_open error");
        return -1;
    }
}
int ftp_test_get_List(ftp_test_config *config)
{

    // FTP_Config myConfig = Ql_get_ftp_Config();
    // if (QL_ftp_cfg(&myConfig) != 0)
    // {
    //     LOG_D("QL_ftp_cfg error");
    //     return -1;
    // }
    // if (QL_ftp_open(&myConfig) != 0)
    // {
    //     LOG_D("QL_ftp_open error");
    //      return -1;
    // }
    // Set the current directory on the FTP server
    if (QL_ftp_path_cfg(config->directoryToSet) != 0)
    {
        LOG_D("Failed to set FTP current directory\n");
        return -1;
    }
    else
    {
        LOG_D("FTP current directory set to: %s\n", config->directoryToSet);
    }

    FileInfo fileList[5]; // Array to store information of up to 10 files
    int fileCount = QL_ftp_list_get(config->directoryToSet, "COM:", fileList, 5);
    if (fileCount >= 0)
    {
        PrintFileList(fileList, fileCount);
    }

    return 0;
}
int ftp_test_uploader(ftp_test_config *config)
{
     if (QL_ftp_path_cfg(config->directoryToSet) != 0)
    {
        LOG_D("Failed to set FTP current directory\n");
        return -1;
    }
    else
    {
        LOG_D("FTP current directory set to: %s\n", config->directoryToSet);
    }

    int uploadResult = ql_ftp_client_put_ex(config->local_name, config->rem_name, read_data_callback);
    if (uploadResult == 0)
    {
        LOG_I("File uploaded successfully.");
        ql_ftp_close();
      //  ftp_send_bcast_msg(MSG_WHAT_BG95_FTP_UP_SUCCESS, 0, 0, 0);
    }
    else
    {
        LOG_E("Failed to upload file. Error code: %d", uploadResult);
        ql_ftp_close();

      //  ftp_send_bcast_msg(MSG_WHAT_BG95_FTP_UP_FAIL, 0, 0, 0);
        return -1;
    }
    return 0;
}
int ftp_test_get(ftp_test_config *config)
{
    if (QL_ftp_path_cfg(config->directoryToSet) != 0)
    {
        LOG_D("Failed to set FTP current directory\n");
        return -1;
    }
    else
    {
        LOG_D("FTP current directory set to: %s\n", config->directoryToSet);
    }

    int result = ql_ftp_client_get_ex(config->rem_name, config->local_name, ftp_write_callback);
    if (result != 0)
    {

        LOG_E("Failed to download file");
        ql_ftp_close();

        return -1;
    }
    else
    {
        LOG_I("File downloaded successfully");
        ql_ftp_close();
    }
    return 0;
}
void ftp_test_close(void)
{
    ql_ftp_close();
}

int example_ftp_test(void *argument)
{
    ftp_test_config *config = (ftp_test_config *)argument;
    user_ftp_config.context_id = config->context_id;
    strcpy(user_ftp_config.username, config->username);
    strcpy(user_ftp_config.password, config->password);
    user_ftp_config.filetype = config->filetype;
    user_ftp_config.transmode = config->transmode;
    user_ftp_config.rsptimeout = config->rsptimeout;
    strcpy(user_ftp_config.hostname, config->request_url);
    user_ftp_config.port = config->port;

    #ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_SSL__
    user_ftp_config.ssl_config.sslenble = config->ssl_config.sslenble;
    if (user_ftp_config.ssl_config.sslenble)
    {
        user_ftp_config.ssl_config.ssltype = config->ssl_config.ssltype;
        user_ftp_config.ssl_config.sslctxid = config->ssl_config.sslctxid;
        user_ftp_config.ssl_config.ciphersuite = config->ssl_config.ciphersuite;
        user_ftp_config.ssl_config.seclevel = config->ssl_config.seclevel;
        user_ftp_config.ssl_config.sslversion = config->ssl_config.sslversion;
    }
    #endif

    if (config->ftp_type == 1)
    {
        ftp_test_open(config);
        ftp_test_get_List(config);
        ftp_test_close();
        // ftp_test_get(config);
    }
    else if (config->ftp_type == 2)
    {
        ftp_test_open(config);
        ftp_test_get(config);
    }
    else if (config->ftp_type == 3)
    {
        ftp_test_open(config);
        ftp_test_uploader(config);
    }

    LOG_V("%s over", __FUNCTION__);
    qosa_task_exit();
    return 0;
}

int user_ftp_test(ftp_test_config *config)
{
    osa_task_t thread_id = NULL;
    int ret = QOSA_OK;

    LOG_V("%s", __FUNCTION__);

    // Create http service
    ret = qosa_task_create(&thread_id, 256 * 16, QOSA_PRIORITY_NORMAL, "Ftp_T", example_ftp_test, (void *)config);
    if (ret != QOSA_OK)
    {
        LOG_E("thread_id thread could not start!");
        return -1;
    }
    LOG_I("%s over(%x)", __FUNCTION__, thread_id);
}

#endif  /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_FTP_S_EXAMPLE__ */
