#include "qosa_system.h"
#include "qosa_log.h"
#include "ff.h"
#include "ql_http.h"
#include "ql_ftp.h"
#include "ql_file.h"
#include "ql_fota.h"

typedef struct ql_fota_http_config_in
{
    fota_progress_callback cb;
    void *user_data;
    void *file;
    int received;
    int total;
    bool succeed;
    QL_FOTA_DOWNLOAD_LOCATION_E location;
} ql_fota_config_in_s;
static void ql_http_rsp_callback(QL_HTTP_USR_EVENT_E event, const char *data, size_t len, void *arg)
{
    ql_fota_config_in_s *config = (ql_fota_config_in_s *)arg;
    switch (event)
    {
    case QL_HTTP_USR_EVENT_START:
        if (config->cb != NULL)
            config->cb(QL_FOTA_INPROGRESS, 0, config->user_data);
        config->total = len;
        config->received = 0;
        break;
    case QL_HTTP_USR_EVENT_BODY_DATA:
        config->received += len;
        if (config->cb != NULL)
            config->cb(QL_FOTA_INPROGRESS,  config->received * 100 / config->total, config->user_data);
        switch (config->location)
        {
        case QL_FOTA_DOWNLOAD_SD_CARD:
            UINT bw;
            FIL* file = (FIL*)config->file;
            if (file->flag & FA_WRITE)
                f_write(file, data, len, &bw);
            break;
        case QL_FOTA_DOWNLOAD_FLASH:
            break;
        default:
            break;
        }
        break;
    case QL_HTTP_USR_EVENT_END:
        switch (config->location)
        {
        case QL_FOTA_DOWNLOAD_SD_CARD:
            f_close((FIL*)config->file);
            free(config->file);
            config->file = NULL;
            if (len == -1)
            {
                config->received = 0;
            }
            break;
        case QL_FOTA_DOWNLOAD_FLASH:
            break;
        default:
            break;
        }
        config->succeed = config->received == config->total ? true : false;
        if (config->cb != NULL)
            config->cb(config->received == config->total ? QL_FOTA_SUCCEED : QL_FOTA_FAIL,  0, config->user_data);
        break;
    default:
        break;
    }
}

 static void ql_ftp_download_callback(QL_FTP_USR_DOWNLOAD_EVENT_E event, const char *data, size_t len, void *arg)
{
    ql_fota_config_in_s *config = (ql_fota_config_in_s *)arg;
    switch (event)
    {
    case QL_FTP_USR_DOWNLOAD_START:
        if (config->cb != NULL)
            config->cb(QL_FOTA_INPROGRESS, 0, config->user_data);
        config->total = len;
        config->received = 0;
        break;
    case QL_FTP_USR_DOWNLOAD_DATA:
        config->received += len;
        if (config->cb != NULL)
            config->cb(QL_FOTA_INPROGRESS,  config->received * 100 / config->total, config->user_data);
        switch (config->location)
        {
        case QL_FOTA_DOWNLOAD_SD_CARD:
            UINT bw;
            FIL* file = (FIL*)config->file;
            if (file->flag & FA_WRITE)
                f_write(file, data, len, &bw);
            break;
        case QL_FOTA_DOWNLOAD_FLASH:
            break;
        default:
            break;
        }
        break;
    case QL_FTP_USR_DOWNLOAD_END:
        switch (config->location)
        {
        case QL_FOTA_DOWNLOAD_SD_CARD:
            f_close((FIL*)config->file);
            free(config->file);
            config->file = NULL;
            break;
        case QL_FOTA_DOWNLOAD_FLASH:
            break;
        default:
            break;
        }
        if (config->cb != NULL)
            config->cb(config->received == config->total ? QL_FOTA_SUCCEED : QL_FOTA_FAIL,  0, config->user_data);
        break;
    default:
        break;
    }
}
static int ql_fota_http_process(ql_fota_http_config_s *config)
{
    ql_fota_config_in_s config_in;
    config_in.cb = config->cb;
    config_in.user_data = config->user_data;
    config_in.location = config->location;
    switch (config->location)
    {
    case QL_FOTA_DOWNLOAD_SD_CARD:
    {
        FIL *file = (FIL*)malloc(sizeof(FIL));
        FRESULT res = f_open(file, config->save_path, FA_CREATE_ALWAYS | FA_WRITE);
        if (res != FR_OK)
        {
            LOG_E("open file failed %s", config->save_path);
            free(file);
            if (config->cb != NULL)
                config->cb(QL_FOTA_FAIL,  0, config->user_data);
            return -1;
        }
        config_in.file = (void*)file;
        break;
    }
    case QL_FOTA_DOWNLOAD_FLASH:
    default:
        if (config->cb != NULL)
            config->cb(QL_FOTA_FAIL,  0, config->user_data);
        return -1;
    }
    ql_http_t http_handle = ql_http_init(at_client_get_first());
    if (config->header[0] != '\0')
    {
        char headers[128] = {0};
        snprintf(headers, sizeof(headers), "%s", config->header);
        ql_http_setopt(http_handle, QL_HTTP_OPT_CUSTOM_HEADER, headers);
    }
    ql_http_setopt(http_handle, QL_HTTP_OPT_CONTEXT_ID, config->content_id);
    ql_http_setopt(http_handle, QL_HTTP_OPT_WRITE_FUNCTION, ql_http_rsp_callback);
    ql_http_setopt(http_handle, QL_HTTP_OPT_WRITE_DATA, (void*)&config_in);
    if (config->ssl_cfg.sslenble)
        ql_http_set_ssl(http_handle, config->ssl_cfg);
    QL_HTTP_ERR_CODE_E err = ql_http_request(http_handle, config->url, QL_HTTP_METHORD_GET, NULL, 0);
    if (err != QL_HTTP_OK)
        LOG_E("ql_http_request %d", err);

    if (config_in.file != NULL)
    {
        switch (config_in.location)
        {
        case QL_FOTA_DOWNLOAD_SD_CARD:
            f_close((FIL*)config_in.file);
            free(config_in.file);
            config_in.file = NULL;
            break;
        case QL_FOTA_DOWNLOAD_FLASH:
            break;
        default:
            break;
        }
    }
    if (!config_in.succeed)
        f_unlink(config->save_path);
    ql_http_deinit(http_handle);
    LOG_I("http download firmware end");
    if (err != QL_HTTP_OK)
    {
        if (config->cb != NULL)
            config->cb(QL_FOTA_FAIL,  0, config->user_data);
        return -1;
    }
    return 0;
}

static int ql_fota_ftp_process(ql_fota_ftp_config_s *config)
{
    ql_fota_config_in_s config_in;
    config_in.cb = config->cb;
    config_in.user_data = config->user_data;
    config_in.location = config->location;
    ql_ftp_t ftp_handle = ql_ftp_init(config->address, config->port, at_client_get_first());
    ql_ftp_setopt(ftp_handle, QL_FTP_OPT_CONTEXT_ID, config->content_id);
    ql_ftp_setopt(ftp_handle, QL_FTP_OPT_WRITE_FUNCTION, ql_ftp_download_callback);
    ql_ftp_setopt(ftp_handle, QL_FTP_OPT_WRITE_DATA, (void*)&config_in);
    ql_ftp_set_ssl(ftp_handle, config->ssl_cfg);
    if (ql_ftp_login(ftp_handle, config->username, config->password) != QL_FTP_OK )
    {
        LOG_E("ftp Login failed");
        ql_ftp_uninit(ftp_handle);
        if (config->cb != NULL)
            config->cb(QL_FOTA_FAIL,  0, config->user_data);
        return -1;
    }
    if (ql_ftp_cwd(ftp_handle, config->work_dir) != QL_FTP_OK)
    {
        LOG_E("change work dir failed");
        ql_ftp_logout(ftp_handle);
        ql_ftp_uninit(ftp_handle);
        if (config->cb != NULL)
            config->cb(QL_FOTA_FAIL,  0, config->user_data);
        return -1;
    }
    
    switch (config->location)
    {
    case QL_FOTA_DOWNLOAD_SD_CARD:
    {
        FIL *file = (FIL*)malloc(sizeof(FIL));
        FRESULT res = f_open(file, config->save_path, FA_CREATE_ALWAYS | FA_WRITE);
        if (res != FR_OK)
        {
            LOG_E("open file failed %s", config->save_path);
            free(file);
            if (config->cb != NULL)
                config->cb(QL_FOTA_FAIL,  0, config->user_data);
            return -1;
        }
        config_in.file = (void*)file;
        break;
    }
    case QL_FOTA_DOWNLOAD_FLASH:
    default:
        if (config->cb != NULL)
            config->cb(QL_FOTA_FAIL,  0, config->user_data);
        return -1;
    }
    QL_FTP_ERR_CODE_E err = ql_ftp_download(ftp_handle, config->file_name, NULL);
    if (err != QL_FTP_OK)
        LOG_E("ql_ftp_download %d", err);
    ql_ftp_logout(ftp_handle);
    ql_ftp_uninit(ftp_handle);
    LOG_I("ftp download firmware end");
    if (config_in.file != NULL)
    {
        switch (config_in.location)
        {
        case QL_FOTA_DOWNLOAD_SD_CARD:
            f_close((FIL*)config_in.file);
            free(config_in.file);
            config_in.file = NULL;
            break;
        case QL_FOTA_DOWNLOAD_FLASH:
            break;
        }
    }
    if (err != QL_FTP_OK)
    {
        if (config->cb != NULL)
            config->cb(QL_FOTA_FAIL,  0, config->user_data);
        return -1;
    }
    return 0;
}

static void ql_fota_http_download(void *arg)
{
    ql_fota_http_process((ql_fota_http_config_s*)arg);
    free(arg);
    qosa_task_exit();
}

static void ql_fota_ftp_download(void *arg)
{
    ql_fota_ftp_process((ql_fota_ftp_config_s*)arg);
    free(arg);
    qosa_task_exit();
}

static int ql_fota_firmware_download_http(ql_fota_http_config_s config)
{
    if (config.async)
    {
        osa_task_t thread_id = NULL;
        ql_fota_http_config_s *http_config = (ql_fota_http_config_s*)malloc(sizeof(ql_fota_http_config_s));
        memcpy(http_config, &config, sizeof(ql_fota_http_config_s));
        int ret = qosa_task_create(&thread_id, 256 * 25, QOSA_PRIORITY_NORMAL, "ota_http_download", ql_fota_http_download, (void *)http_config);
        if (ret != QOSA_OK)
        {
            LOG_E("thread_id thread could not start! %d", qosa_task_get_free_heap_size());
            free(http_config);
            return -1;
        }
        return 0;
    }
    return ql_fota_http_process(&config);
}

static int ql_fota_firmware_download_ftp(ql_fota_ftp_config_s config)
{
    if (config.async)
    {
        osa_task_t thread_id = NULL;
        ql_fota_ftp_config_s *ftp_config = (ql_fota_ftp_config_s*)malloc(sizeof(ql_fota_ftp_config_s));
        memcpy(ftp_config, &config, sizeof(ql_fota_ftp_config_s));
        int ret = qosa_task_create(&thread_id, 256 * 25, QOSA_PRIORITY_NORMAL, "ota_ftp_download", ql_fota_ftp_download, (void *)ftp_config);
        if (ret != QOSA_OK)
        {
            LOG_E("thread_id thread could not start! %d", qosa_task_get_free_heap_size());
            free(ftp_config);
            return -1;
        }
        return 0;
    }
    return ql_fota_ftp_process(&config);
}
int ql_fota_firmware_download(QL_FOTA_MODE_E mode, ql_fota_config_u config)
{
    if (QL_FOTA_DWNLD_MOD_HTTP == mode)
        return ql_fota_firmware_download_http(config.http_config);
    else if (QL_FOTA_DWNLD_MOD_FTP == mode)
        return ql_fota_firmware_download_ftp(config.ftp_config);
    else
        return -1;
    return 0;
}