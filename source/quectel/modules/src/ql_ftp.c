#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__
#include <stdarg.h>
#include "qosa_log.h"
#include "module_info.h"
#include "ql_ftp.h"

static bool s_global_ftp_init = false;

static const char* s_ql_ftp_option_strings[] =
{
    [QL_FTP_OPT_ACCOUNT]       = "account",
    [QL_FTP_OPT_FILE_TYPE]     = "filetype",
    [QL_FTP_OPT_CONTEXT_ID]    = "contextid",
    [QL_FTP_OPT_TRANMODE]    = "transmode",
    [QL_FTP_OPT_RSP_TIMEOUT]   = "rsptimeout",
    [QL_FTP_OPT_SSL_TYPE]      = "ssltype",
    [QL_FTP_OPT_SSL_CTXID]     = "sslctxid",
    [QL_FTP_OPT_SSL_DATA_ADDR] = "data_address",
    [QL_FTP_OPT_REST_ENABLE]   = "restenable",
    [QL_FTP_OPT_UNKNOWN]      = "unknown"
};

static void ql_ftp_urc_open(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int protocol_err = -1;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPOPEN: %d,%d", &err, &protocol_err);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    handle->protocol_err = (QL_FTP_ERR_CODE_E)protocol_err;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_cwd(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int protocol_err = -1;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPCWD: %d,%d", &err, &protocol_err);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    handle->protocol_err = (QL_FTP_ERR_CODE_E)protocol_err;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_pwd(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int protocol_err = 0;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    if (sscanf(data, "+QFTPPWD: %d,", &err) == 1)
    {
        if (0 == err)
            sscanf(data, "+QFTPPWD: 0,\"%255[^\"]\"", handle->pwd);
        else
            sscanf(data, "+QFTPPWD: %*d,%d", &protocol_err);
    }
    else
    {
        LOG_E("FTP PWD error");
    }
    handle->err = (QL_FTP_ERR_CODE_E)err;
    handle->protocol_err = (QL_FTP_ERR_CODE_E)protocol_err;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_mkdir(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int protocol_err = -1;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPMKDIR: %d,%d", &err, &protocol_err);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    handle->protocol_err = (QL_FTP_ERR_CODE_E)protocol_err;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_rmdir(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int protocol_err = -1;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPMKDIR: %d,%d", &err, &protocol_err);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    handle->protocol_err = (QL_FTP_ERR_CODE_E)protocol_err;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_list(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int value = -1;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPLIST: %d,%d", &err, &value);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    if (handle->err != QL_FTP_OK)
        handle->protocol_err = (QL_FTP_ERR_CODE_E)value;
    else
        handle->file_size = value;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_size(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int value = -1;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPSIZE: %d,%d", &err, &value);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    if (handle->err != QL_FTP_OK)
        handle->protocol_err = (QL_FTP_ERR_CODE_E)value;
    else
        handle->file_size = value;
    LOG_D("file size: %d", handle->file_size);
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_put(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int value = -1;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPPUT: %d,%d", &err, &value);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    if (handle->err != QL_FTP_OK)
        handle->protocol_err = (QL_FTP_ERR_CODE_E)value;
    else
        handle->file_size = value;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_get(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int value = -1;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPGET: %d,%d", &err, &value);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    if (handle->err != QL_FTP_OK)
        handle->protocol_err = (QL_FTP_ERR_CODE_E)value;
    else
        handle->file_size = value;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_del(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int protocol_err = -1;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPDEL: %d,%d", &err, &protocol_err);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    handle->protocol_err = (QL_FTP_ERR_CODE_E)protocol_err;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_mlsd(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int value = -1;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPMLSD: %d,%d", &err, &value);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    if (handle->err != QL_FTP_OK)
        handle->protocol_err = (QL_FTP_ERR_CODE_E)value;
    else
        handle->file_size = value;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_mdtm(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int protocol_err = 0;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    if (sscanf(data, "+QFTPMDTM: %d,", &err) == 1)
    {
        if (0 == err)
            sscanf(data, "+QFTPMDTM: 0,\"%255[^\"]\"", handle->modify_time);
        else
            sscanf(data, "+QFTPMDTM: %*d,%d", &protocol_err);
    }
    else
    {
        LOG_E("FTP MDTM error");
    }
    handle->err = (QL_FTP_ERR_CODE_E)err;
    handle->protocol_err = (QL_FTP_ERR_CODE_E)protocol_err;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_rename(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int protocol_err = -1;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPRENAME: %d,%d", &err, &protocol_err);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    handle->protocol_err = (QL_FTP_ERR_CODE_E)protocol_err;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_close(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int protocol_err = -1;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPCLOSE: %d,%d", &err, &protocol_err);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    handle->protocol_err = (QL_FTP_ERR_CODE_E)protocol_err;
    qosa_sem_release(handle->sem);
}

static void ql_ftp_urc_stat(struct at_client *client, const char *data, size_t size, void *arg)
{
    int err = -1;
    int stat = 4;
    ql_ftp_t handle = (ql_ftp_t)arg;
    if (NULL == handle)
        return;
    sscanf(data, "+QFTPSTAT: %d,%d", &err, &stat);
    handle->err = (QL_FTP_ERR_CODE_E)err;
    handle->stat = stat;
    qosa_sem_release(handle->sem);
}

static const struct at_urc s_ftp_urc_table[] =
{
    {"+QFTPOPEN", "\r\n", ql_ftp_urc_open},
    {"+QFTPCWD", "\r\n", ql_ftp_urc_cwd},
    {"+QFTPPWD", "\r\n", ql_ftp_urc_pwd},
    {"+QFTPMKDIR", "\r\n", ql_ftp_urc_mkdir},
    {"+QFTPMKDIR", "\r\n", ql_ftp_urc_rmdir},
    {"+QFTPSIZE", "\r\n", ql_ftp_urc_size},
    {"+QFTPPUT", "\r\n", ql_ftp_urc_put},
    {"+QFTPGET:", "\r\n", ql_ftp_urc_get},
    {"+QFTPDEL:", "\r\n", ql_ftp_urc_del},
    {"+QFTPLIST", "\r\n", ql_ftp_urc_list},
    {"+QFTPMLSD", "\r\n", ql_ftp_urc_mlsd},
    {"+QFTPMDTM", "\r\n", ql_ftp_urc_mdtm},
    {"+QFTPRENAME", "\r\n", ql_ftp_urc_rename},
    {"+QFTPCLOSE:", "\r\n", ql_ftp_urc_close},
    {"+QFTPSTAT:", "\r\n", ql_ftp_urc_stat}
};
static void ql_ftp_upload_cb(const char *data, size_t len, void* arg)
{
    ql_ftp_t handle = (ql_ftp_t)arg;
    char buffer[1024] = {0};
    UINT br = 0;
    at_client_self_recv(handle->client, buffer, 1024, 2000, 1, true);
    if (strstr(buffer, "CONNECT\r\n") != NULL)
    {
        while (true)
        {
            if (f_read(&handle->file, buffer, 1024, &br) != FR_OK || 0 == br)
                break;
            at_client_obj_send(handle->client, buffer, br, false); // Send data to the modem
        }
        qosa_task_sleep_ms(2000);
        at_client_obj_send(handle->client, "+++", 3, true);
    }
    else if (strstr(buffer, "ERROR") != NULL || strstr(buffer, "+CME ERROR") != NULL)
    {
        int err = 0;
        if (sscanf(buffer, "+CME ERROR: %d", &err) == 1)
        handle->err = (QL_FTP_ERR_CODE_E)err;
        qosa_sem_release(handle->sem);
    }
    f_close(&handle->file);
}

static void ql_ftp_download_cb(const char *data, size_t len, void* arg)
{
    ql_ftp_t handle = (ql_ftp_t)arg;
    char buffer[1024] = {0};
    UINT br = 0;
    at_client_self_recv(handle->client, buffer, 1024, 2000, 1, true);
    size_t total_len = 0;
    size_t recv_len  = 0;
    if (strstr(buffer, "CONNECT\r\n") != NULL)
    {
        while (total_len < handle->file_size)
        {
            recv_len = ((handle->file_size - total_len) > 1024) ? 1024 : (handle->file_size - total_len);
            recv_len = at_client_self_recv(handle->client, buffer, recv_len, (handle->timeout - 1) * 1000 , 0, false);
            if (recv_len <= 0)
            {
                LOG_E("download over ");
                break;
            }
            total_len += recv_len;
            if ((handle->file.flag & FA_WRITE) == 0)
                continue;
            FRESULT ret = f_write(&handle->file, buffer, recv_len, &br);
            if(ret != FR_OK)
            {
                LOG_E("write file error : %d", ret);
                if (FR_INVALID_OBJECT == ret)
                {
                    f_close(&handle->file);
                }
            }
            LOG_I("write %d bytes", total_len);
        }
    }
    else if (strstr(buffer, "ERROR") != NULL || strstr(buffer, "+CME ERROR") != NULL)
    {
        int err = 0;
        if (sscanf(buffer, "+CME ERROR: %d", &err) == 1)
        handle->err = (QL_FTP_ERR_CODE_E)err;
        qosa_sem_release(handle->sem);
    }
    if ((handle->file.flag & FA_WRITE) == FA_WRITE)
    {
        f_close(&handle->file);
    }
}

static ql_ftp_file_info_s* ql_parse_entry_info(const char *line)
{
    if (NULL == line || strlen(line) < 10)
        return NULL;

    ql_ftp_file_info_s *info = malloc(sizeof(ql_ftp_file_info_s));
    if (NULL == info)
        return NULL;

    memset(info, 0, sizeof(ql_ftp_file_info_s));

    char month[4], day[4], time[6];
    int scanned = sscanf(
        line,
        "%c%9s %d %31s %31s %ld %3s %3s %5s %255[^\n]",
        &info->type, info->permissions, &info->links,
        info->owner, info->group, &info->size,
        month, day, time, info->name
    );
    if (scanned < 10) {
        free(info);
        return NULL;
    }

    snprintf(info->date, 255, "%s %s %s", month, day, time);

    info->next = NULL;
    return info;
}

static void ql_entry_list_append(ql_ftp_file_info_s **head, ql_ftp_file_info_s *new_node)
{
    if (!head || !new_node) return;

    if (*head == NULL) {
        *head = new_node;
    }
    else
    {
        ql_ftp_file_info_s *current = *head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_node;
    }
}
static void ql_ftp_list_cb(const char *data, size_t len, void* arg)
{
    ql_ftp_t handle = (ql_ftp_t)arg;
    char buffer[1024] = {0};
    at_client_self_recv(handle->client, buffer, 1024, 2000, 1, true);
    size_t recv_len  = 0;
    if (strstr(buffer, "CONNECT\r\n") != NULL)
    {
        while (true)
        {
            memset(buffer, 0, 1024);
            recv_len = at_client_self_recv(handle->client, buffer, 1024, (handle->timeout - 1) * 1000, 1, true);
            if (recv_len <= 0)
            {
                LOG_E("list over ");
                break;
            }
            if (strstr(buffer, "OK\r\n") != NULL || strstr(buffer, "ERROR") != NULL)
                break;
            ql_ftp_file_info_s *info = ql_parse_entry_info(buffer);
            if (info != NULL)
                ql_entry_list_append(&handle->file_list, info);
        }
    }
    else if (strstr(buffer, "ERROR") != NULL || strstr(buffer, "+CME ERROR") != NULL)
    {
        int err = -1;
        if (sscanf(buffer, "+CME ERROR: %d", &err) == 1)
        handle->err = (QL_FTP_ERR_CODE_E)err;
        qosa_sem_release(handle->sem);
    }
}

static void ql_ftp_nlist_cb(const char *data, size_t len, void* arg)
{
    ql_ftp_t handle = (ql_ftp_t)arg;
    char buffer[1024] = {0};
    at_client_self_recv(handle->client, buffer, 1024, 2000, 1, true);
    size_t recv_len  = 0;
    if (strstr(buffer, "CONNECT\r\n") != NULL)
    {
        while (true)
        {
            memset(buffer, 0, 1024);
            recv_len = at_client_self_recv(handle->client, buffer, 1024, 3000, 1, true);
            LOG_D("buffer = %s", buffer);
            if (recv_len <= 0)
            {
                LOG_E("list over ");
                break;
            }
            if (strstr(buffer, "OK\r\n") != NULL)
                break;
            ql_ftp_file_info_s *info = ql_parse_entry_info(buffer);
            if (info != NULL)
                ql_entry_list_append(&handle->file_list, info);
        }
    }
    else if (strstr(buffer, "ERROR") != NULL || strstr(buffer, "+CME ERROR") != NULL)
    {
        int err = 0;
        if (sscanf(buffer, "+CME ERROR: %d", &err) == 1)
            handle->err = (QL_FTP_ERR_CODE_E)err;
        qosa_sem_release(handle->sem);
    }
}

static void ql_ftp_mlsd_cb(const char *data, size_t len, void* arg)
{
    ql_ftp_t handle = (ql_ftp_t)arg;
    char buffer[1024] = {0};
    at_client_self_recv(handle->client, buffer, 1024, 2000, 1, true);
    size_t recv_len  = 0;
    if (strstr(buffer, "CONNECT\r\n") != NULL)
    {
        while (true)
        {
            memset(buffer, 0, 1024);
            recv_len = at_client_self_recv(handle->client, buffer, 1024, 3000, 1, true);
            LOG_D("buffer = %s", buffer);
            if (recv_len <= 0)
            {
                LOG_E("list over ");
                break;
            }
            if (strstr(buffer, "OK\r\n") != NULL)
                break;
            ql_ftp_file_info_s *info = ql_parse_entry_info(buffer);
            memcpy(handle->file_list, info, sizeof(ql_ftp_file_info_s));
        }
    }
    else if (strstr(buffer, "ERROR") != NULL || strstr(buffer, "+CME ERROR") != NULL)
    {
        LOG_E("error:%s", buffer);
        int err = 0;
        if (sscanf(buffer, "+CME ERROR: %d", &err) == 1)
            handle->err = (QL_FTP_ERR_CODE_E)err;
        qosa_sem_release(handle->sem);
    }
}


static QL_FTP_ERR_CODE_E ql_ftp_exec_cmd(ql_ftp_t handle, at_response_t resp, QL_FTP_ERR_CODE_E default_err, const char *cmd_format, ...)
{
    va_list args;
    int ret;
    char formatted_cmd[1024] = {0};

    va_start(args, cmd_format);
    vsnprintf(formatted_cmd, sizeof(formatted_cmd), cmd_format, args);
    va_end(args);
    // qosa_mutex_lock(handle->client->lock, QOSA_WAIT_FOREVER);
    ret = at_obj_exec_cmd(handle->client, resp, formatted_cmd);
    if (ret < 0)
    {
        for (int i = 0; i < resp->line_counts; i++)
        {
            int err = -1;
            const char *line = at_resp_get_line(resp, i + 1);
            LOG_I("line = %s", line);
            if (strstr(line, "+CME ERROR:") != NULL) {
                sscanf(line, "+CME ERROR: %d", &err);
                at_delete_resp(resp);
                LOG_E("ql_ftp_exec_cmd: %d", err);
                // qosa_mutex_unlock(handle->client->lock);
                return err;
            }
        }
        at_delete_resp(resp);
        // qosa_mutex_unlock(handle->client->lock);
        return default_err;
    }

    qosa_sem_wait(handle->sem, (handle->timeout + 1) * 1000);
    at_delete_resp(resp);
    // qosa_mutex_unlock(handle->client->lock);
    return handle->err;
}

static QL_FTP_ERR_CODE_E ql_ftp_stat(ql_ftp_t handle)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    at_response_t resp = at_create_resp_new(256, 0, handle->timeout * 1000, handle);
    return ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_STAT, "AT+QFTPSTAT");
}
ql_ftp_t ql_ftp_init(const char* host, u16_t port, at_client_t client)
{
    if (NULL == host || strlen(host) == 0)
    {
        LOG_E("host is null.");
        return NULL;
    }
    ql_ftp_t handle = (ql_ftp_t)malloc(sizeof(ql_ftp_s));
    if (NULL == handle)
    {
        LOG_E("no memory for AT client response object.");
        return NULL;
    }
    handle->client = client;
    handle->port = port;
    handle->host = strdup(host);
    handle->timeout = 90;
    handle->err = QL_FTP_OK;
    handle->protocol_err = QL_FTP_OK;
    handle->stat = 4; // ftp closed
    handle->pwd = NULL;
    handle->file_size = 0;
    handle->file_list = NULL;
    handle->ssl.ssltype = 0;
    qosa_sem_create(&handle->sem, 0);
    if (!s_global_ftp_init)
    {
        at_set_urc_table(s_ftp_urc_table, sizeof(s_ftp_urc_table) / sizeof(s_ftp_urc_table[0]));
        s_global_ftp_init = true;
    }
    ql_ftp_setopt(handle, QL_FTP_OPT_TRANMODE, 1);
    return handle;
}

bool ql_ftp_setopt(ql_ftp_t handle, QL_FTP_OPTION_E option, ...)
{
    va_list arg;
    at_response_t resp = NULL;
    bool ret = false;
    char *option_content = NULL;
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;
    resp = at_create_resp_new(1024, 0, 300, handle);
    va_start(arg, option);

   switch (option)
    {
        case QL_FTP_OPT_ACCOUNT:
        {
            const char* value = va_arg(arg, const char*);
            option_content = (char*)malloc(1024);
            snprintf(option_content, 1024, "%s", value);
            ret = true;
            break;
        }
        case QL_FTP_OPT_FILE_TYPE:
        {
            int type = va_arg(arg, int);
            if (type > 1 || type < 0)
                break;
            option_content = (char*)malloc(12);
            snprintf(option_content, 12, "%d", type);
            ret = true;
            break;
        }
        case QL_FTP_OPT_CONTEXT_ID:
        {
            int id = va_arg(arg, int);
            if (id > 16 || id < 1)
                break;
            option_content = (char*)malloc(12);
            snprintf(option_content, 12, "%d", id);
            ret = true;
            break;
        }
        case QL_FTP_OPT_TRANMODE:
        {
            option_content = (char*)malloc(12);
            snprintf(option_content, 12, "%d", 1);
            ret = true;
            break;
        }
        case QL_FTP_OPT_RSP_TIMEOUT:
        {
            handle->timeout = va_arg(arg, int);
            if ( handle->timeout > 180 ||  handle->timeout < 20)
                break;
            option_content = (char*)malloc(12);
            snprintf(option_content, 12, "%d",  handle->timeout);
            ret = true;
            break;
        }
        case QL_FTP_OPT_SSL_TYPE:
        {
            int value = va_arg(arg, int);
            if (value > 2 || value < 0)
                break;
            option_content = (char*)malloc(12);
            snprintf(option_content, 12, "%d", value);
            ret = true;
            break;
        }
        case QL_FTP_OPT_SSL_CTXID:
        {
            int value = va_arg(arg, int);
            if (value > 5 || value < 0)
                break;
            option_content = (char*)malloc(12);
            snprintf(option_content, 12, "%d", value);
            ret = true;
            break;
        }
        case QL_FTP_OPT_SSL_DATA_ADDR:
        {
            int value = va_arg(arg, int);
            if (strstr(get_module_type_name(), "BG95") != NULL)
            {
                if (value > 1 || value < 0)
                    break;
                option_content = (char*)malloc(12);
                snprintf(option_content, 12, "%d", value);
            }
            else
            {
                if (value > 2 || value < 0)
                    break;
                option_content = (char*)malloc(12);
                snprintf(option_content, 12, "%d,%d", value, 25);
            }
                ret = true;
            break;
        }
        case QL_FTP_OPT_REST_ENABLE:
        {
            bool enable = (bool)va_arg(arg, int);
            option_content = (char*)malloc(12);
            snprintf(option_content, 12, "%d", enable);
            ret = true;
            break;
        }
        default:
            ret = 0;
            break;
    }
    if (ret)
    {
        if (at_obj_exec_cmd(handle->client, resp, "AT+QFTPCFG=\"%s\",%s", s_ql_ftp_option_strings[option], option_content) < 0)
            ret = false;
    }
    at_delete_resp(resp);
    if (option_content)
        free(option_content);
    va_end(arg);
    return ret;
}

bool ql_ftp_set_ssl(ql_ftp_t handle, ql_SSL_Config config)
{
    handle->ssl = config;
    if (NULL == handle->ssl.cacert_dst_path)
        handle->ssl.cacert_dst_path = "ftp_ca.pem";
    if (NULL == handle->ssl.clientcert_dst_path)
        handle->ssl.clientcert_dst_path = "ftp_user.pem";
    if (NULL == handle->ssl.clientkey_dst_path)
        handle->ssl.clientkey_dst_path = "ftp_user_key.pem";
    handle->ssl.client = handle->client;
    return true;
}

QL_FTP_ERR_CODE_E ql_ftp_login(ql_ftp_t handle, const char* username, const char* password)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;
    if (username != NULL && password != NULL)
    {
        char account[1024] = {0};
        snprintf(account, sizeof(account), "\"%s\",\"%s\"", username, password);
        ql_ftp_setopt(handle, QL_FTP_OPT_ACCOUNT, account);
    }
    if (0 == handle->ssl.ssltype)
        ql_ftp_setopt(handle, QL_FTP_OPT_SSL_TYPE,  handle->ssl.ssltype);
    else
    {       
        ql_ftp_setopt(handle, QL_FTP_OPT_SSL_TYPE,  handle->ssl.ssltype);
        ql_ftp_setopt(handle, QL_FTP_OPT_SSL_CTXID,  handle->ssl.sslctxid);
        if (configure_ssl(&handle->ssl) != 0)
            return QL_FTP_ERR_SSL_CONFIG;
    }

    at_response_t resp = at_create_resp_new(128, 0, (125000), handle);
    return ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_OPEN, "AT+QFTPOPEN=\"%s\",%d", handle->host, handle->port);
}

QL_FTP_ERR_CODE_E ql_ftp_cwd(ql_ftp_t handle, const char* path)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == path || strlen(path) == 0)
        return QL_FTP_ERR_INVALID_PARAM;
    at_response_t resp = at_create_resp_new(256, 0, handle->timeout * 1000, handle);
    return ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_CWD, "AT+QFTPCWD=\"%s\"", path);
}

QL_FTP_ERR_CODE_E ql_ftp_pwd(ql_ftp_t handle, char* path)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == path)
        return QL_FTP_ERR_INVALID_PARAM;
    handle->pwd = path;
    at_response_t resp = at_create_resp_new(256, 0, handle->timeout * 1000, handle);
    return ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_PWD, "AT+QFTPPWD");
}

QL_FTP_ERR_CODE_E ql_ftp_mkdir(ql_ftp_t handle, const char* path)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == path || strlen(path) == 0)
        return QL_FTP_ERR_INVALID_PARAM;
    at_response_t resp = at_create_resp_new(256, 0, handle->timeout * 1000, handle);
    return ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_MKDIR, "AT+QFTPMKDIR=\"%s\"", path);
}

QL_FTP_ERR_CODE_E ql_ftp_rmdir(ql_ftp_t handle, const char* path)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == path || strlen(path) == 0)
        return QL_FTP_ERR_INVALID_PARAM;
    at_response_t resp = at_create_resp_new(256, 0, handle->timeout * 1000 * 2, handle);
    return ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_RMDIR, "AT+QFTPRMDIR=\"%s\"", path);
}

QL_FTP_ERR_CODE_E ql_ftp_rename(ql_ftp_t handle, const char* old_name, const char* new_name)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == old_name || strlen(old_name) == 0 || NULL == new_name || strlen(new_name) == 0)
        return QL_FTP_ERR_INVALID_PARAM;
    at_response_t resp = at_create_resp_new(256, 0, handle->timeout * 1000 * 2, handle);
    return ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_RENAME, "AT+QFTPRENAME=\"%s\",\"%s\"", old_name, new_name);
}

QL_FTP_ERR_CODE_E ql_ftp_list(ql_ftp_t handle, const char *path, ql_ftp_file_info_s **head)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == path || strlen(path) == 0)
        return QL_FTP_ERR_INVALID_PARAM;

    at_response_t resp = NULL;
    *head = NULL;
    resp = at_create_resp_by_selffunc_new(128, 0, handle->timeout * 1000 * 2, ql_ftp_list_cb, handle);
    QL_FTP_ERR_CODE_E err = ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_LIST, "AT+QFTPLIST=\"%s\"", path);
    if (err != QL_FTP_OK)
        return err;
    *head = handle->file_list;
    return QL_FTP_OK;
}

QL_FTP_ERR_CODE_E ql_ftp_nlist(ql_ftp_t handle, const char *path, ql_ftp_file_info_s **head)
{
    if (strstr(get_module_type_name(), "BG95") != NULL)
    {
        return QL_FTP_ERR_NOT_SUPPORT;
    }
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == path || strlen(path) == 0)
        return QL_FTP_ERR_INVALID_PARAM;

    at_response_t resp = NULL;
    *head = NULL;
    resp = at_create_resp_by_selffunc_new(128, 0, handle->timeout * 1000 * 2, ql_ftp_nlist_cb, handle);
    QL_FTP_ERR_CODE_E err = ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_NLIST, "AT+QFTPNLIST=\"%s\"", path);
    if (err != QL_FTP_OK)
        return err;
    *head = handle->file_list;
    return QL_FTP_OK;
}

QL_FTP_ERR_CODE_E ql_ftp_file_size(ql_ftp_t handle, const char *remote_file_name, size_t *file_size)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == file_size || NULL == remote_file_name || strlen(remote_file_name) == 0)
        return QL_FTP_ERR_INVALID_PARAM;

    *file_size = 0;
    at_response_t resp = at_create_resp_new(256, 0, handle->timeout * 1000 * 2, handle);
    QL_FTP_ERR_CODE_E err = ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_FILE_SIZE, "AT+QFTPSIZE=\"%s\"", remote_file_name);
    if (QL_FTP_OK != err)
        return err;
    *file_size = handle->file_size;
    return QL_FTP_OK;
}

QL_FTP_ERR_CODE_E ql_ftp_upload(ql_ftp_t handle, const char *local_file_name, const char *remote_file_name)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == local_file_name || NULL == remote_file_name || strlen(local_file_name) == 0 || strlen(remote_file_name) == 0)
        return QL_FTP_ERR_INVALID_PARAM;
    at_response_t resp = NULL;
    const char *local_name = NULL;
    if (strstr(local_file_name, "0:") != NULL)
    {

        if (f_open(&handle->file, local_file_name, FA_READ) != FR_OK)
        {
            LOG_E("Failed to open file %s", local_file_name);
            return QL_FTP_ERR_UPLOAD;
        }
        local_name = "COM:";
        resp = at_create_resp_by_selffunc_new(128, 0, handle->timeout * 1000 * 2, ql_ftp_upload_cb, handle);
    }
    else
    {
        local_name = local_file_name;
        resp = at_create_resp_new(128, 0, handle->timeout * 1000 * 2, handle);
    }
    return ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_UPLOAD, "AT+QFTPPUT=\"%s\",\"%s\",0", remote_file_name, local_name);
}

QL_FTP_ERR_CODE_E ql_ftp_download(ql_ftp_t handle, const char *remote_file_name, const char *local_file_name)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == local_file_name || NULL == remote_file_name || strlen(local_file_name) == 0 || strlen(remote_file_name) == 0)
        return QL_FTP_ERR_INVALID_PARAM;

    QL_FTP_ERR_CODE_E err = ql_ftp_file_size(handle, remote_file_name, &handle->file_size);
    if (err != QL_FTP_OK)
        return err;
    at_response_t resp = NULL;
    const char *local_name = NULL;
    if (strstr(local_file_name, "0:") != NULL)
    {

        if (f_open(&handle->file, local_file_name, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        {
            LOG_E("Failed to open file %s", local_file_name);
            return QL_FTP_ERR_DOWNLOAD;
        }
        local_name = "COM:";
        resp = at_create_resp_by_selffunc_new(128, 0, handle->timeout * 1000 * 2, ql_ftp_download_cb, handle);
    }
    else
    {
        local_name = local_file_name;
        resp = at_create_resp_new(128, 0, handle->timeout * 1000 * 2, handle);
    }
    return ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_DOWNLOAD, "AT+QFTPGET=\"%s\",\"%s\"", remote_file_name, local_name);
}

QL_FTP_ERR_CODE_E ql_ftp_file_delete(ql_ftp_t handle, const char *remote_file_name)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == remote_file_name || strlen(remote_file_name) == 0)
        return QL_FTP_ERR_INVALID_PARAM;

    at_response_t resp = at_create_resp_new(256, 0, handle->timeout * 1000 * 2, handle);
    QL_FTP_ERR_CODE_E err = ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_FILE_DELETE, "AT+QFTPDEL=\"%s\"", remote_file_name);
    if (QL_FTP_OK != err)
        return err;
    return QL_FTP_OK;
}

QL_FTP_ERR_CODE_E ql_ftp_mlsd(ql_ftp_t handle, const char *path, ql_ftp_file_info_s *info)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == path || strlen(path) == 0)
        return QL_FTP_ERR_INVALID_PARAM;

    at_response_t resp = NULL;
    handle->file_list = info;
    resp = at_create_resp_by_selffunc_new(128, 0, handle->timeout * 1000 * 2, ql_ftp_mlsd_cb, handle);
    QL_FTP_ERR_CODE_E err = ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_MLSD, "AT+QFTPMLSD=\"%s\",\"RAM:nlst.txt\"", path);
    handle->file_list = NULL;
    if (err != QL_FTP_OK)
        return err;
    return QL_FTP_OK;
}

QL_FTP_ERR_CODE_E ql_ftp_file_get_modify_time(ql_ftp_t handle, const char *remote_file_name, char *time)
{
        if (NULL == handle)
        return QL_FTP_ERR_NOINIT;

    if (NULL == remote_file_name || strlen(remote_file_name) == 0 || NULL == time)
        return QL_FTP_ERR_INVALID_PARAM;
    handle->modify_time = time;
    at_response_t resp = NULL;
    resp = at_create_resp_new(128, 0, handle->timeout * 1000 * 2, handle);
    return ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_MDTM, "AT+QFTPMDTM=\"%s\"", remote_file_name);
}

QL_FTP_ERR_CODE_E ql_ftp_logout(ql_ftp_t handle)
{
    if (NULL == handle)
        return QL_FTP_ERR_NOINIT;
    ql_ftp_stat(handle);
    if (handle->err != QL_FTP_OK || handle->stat == 4)
        return QL_FTP_ERR_LOGOUT;
    at_response_t resp = at_create_resp_new(256, 0, handle->timeout * 1000 * 2, handle);
    return ql_ftp_exec_cmd(handle, resp, QL_FTP_ERR_LOGOUT, "AT+QFTPCLOSE");
}

void ql_ftp_uninit(ql_ftp_t handle)
{
    if (NULL == handle)
    {
        return;
    }
    free(handle->host);
    qosa_sem_delete(handle->sem);
    free(handle);
}

QL_FTP_ERR_CODE_E ql_get_protocol_err(ql_ftp_t handle)
{
    return handle->protocol_err;
}

void ql_ftp_list_free(ql_ftp_file_info_s *head)
{
    ql_ftp_file_info_s *current = head;
    while (current != NULL)
    {
        ql_ftp_file_info_s *temp = current;
        current = current->next;
        free(temp);
    }
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__ */
