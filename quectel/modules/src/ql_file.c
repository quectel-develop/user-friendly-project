
#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__
#include "qosa_log.h"
#include "ql_file.h"
#define QL_MAX_ONCE_LEN (1024)
static const char* s_end_description = "\r\nOK\r\n";
static void ql_read_cb(const char *data, size_t len, void* arg)
{
    QL_FILE stream = (QL_FILE)arg;
    char body_desc[32] = {0};
    char body_data[QL_MAX_ONCE_LEN] = {0};
    int total_len = 0;
    at_client_self_recv(stream->client, body_desc, sizeof(body_desc), 2 * RT_TICK_PER_SECOND, 1, true);
    if (sscanf(body_desc, "CONNECT %d", &total_len) != 1)
    {
        sscanf(body_desc, "+CME ERROR: %d", &stream->err);
        qosa_sem_release(stream->sem);
        return;
    }
    int read_len = 0;
    int left_len = total_len;
    stream->len = total_len;
    while (left_len > 0)
    {
        read_len = left_len > QL_MAX_ONCE_LEN ? QL_MAX_ONCE_LEN : left_len;
        read_len = at_client_obj_recv(stream->client, body_data, read_len, 2 * RT_TICK_PER_SECOND, false);
        memcpy(stream->ptr + total_len - left_len, body_data, read_len);
        left_len -= read_len;
    }
    memset(body_desc, 0, sizeof(body_desc));
    at_client_obj_recv(stream->client, body_desc, strlen(s_end_description), 2 * RT_TICK_PER_SECOND, true);
    if (memcmp(body_desc, s_end_description, strlen(s_end_description)) != 0)
    {
        LOG_W("GET OK FAILED %s", body_desc);
    }
    qosa_sem_release(stream->sem);
}

static void ql_write_cb(const char *data, size_t len, void* arg)
{
    QL_FILE stream = (QL_FILE)arg;
    char buffer[32] = {0};
    at_client_self_recv(stream->client, buffer, sizeof(buffer), 2000, 1, true);
    if (strstr(buffer, "CONNECT") != NULL)
    {
        int left_len = stream->len;
        while (left_len > 0)
        {
            int write_len = left_len > QL_MAX_ONCE_LEN ? QL_MAX_ONCE_LEN : left_len;
            at_client_obj_send_nolock(stream->client, stream->ptr + stream->len - left_len, write_len, false);
            left_len -= write_len;
        }
        memset(buffer, 0, sizeof(buffer));
        at_client_self_recv(stream->client, buffer, sizeof(buffer), 60 * 1000, 1, true);
        if (sscanf(buffer, "+QFWRITE: %d,%*d", &stream->len) == 1)
        {
            stream->err = 0;
            while (true)
            {
                memset(buffer, 0, sizeof(buffer));
                int ret = at_client_self_recv(stream->client, buffer, sizeof(buffer), 60 * 1000, 1, true);
                if ( (0 == ret) || (strstr(buffer, "OK") != NULL))
                    break;
            }
        }
        else
        {
            LOG_E("write failed %s", buffer);
            stream->err = -1;
        }
    }
    else
    {
        LOG_E("write failed %s", buffer);
        stream->err = -1;
    }
    qosa_sem_release(stream->sem);
}


static void ql_file_list_append(ql_file_info_s **head, ql_file_info_s *new_node)
{
    if (!head || !new_node) return;

    if (*head == NULL)
    {
        *head = new_node;
    }
    else
    {
        ql_file_info_s *current = *head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_node;
    }
}

static void ql_list_cb(const char *data, size_t len, void* arg)
{
    QL_FILE stream = (QL_FILE)arg;
    char buffer[256] = {0};
    size_t recv_len  = 0;
    while (true)
    {
        memset(buffer, 0, 256);
        recv_len = at_client_self_recv(stream->client, buffer, 256, 2000, 1, true);
        if (recv_len <= 0)
        {
            stream->err = -1;
            break;
        }
        if (strstr(buffer, "OK") != NULL)
            break;
        else if (strstr(buffer, "ERROR") != NULL)
        {
            LOG_E("list failed %s", buffer);
            stream->err = -1;
            break;
        }
        ql_file_info_s *file_info = (ql_file_info_s*)malloc(sizeof(ql_file_info_s));
        memset(file_info, 0, sizeof(ql_file_info_s));
        if (sscanf(buffer, "+QFLST: \"%[^\"]\",%u", file_info->filename, &file_info->filesize) == 2)
        {
            ql_file_list_append(&stream->file_list, file_info);
        }
        else 
        {
            free(file_info);
        }
    }
    qosa_sem_release(stream->sem);
}

QL_FILE ql_fopen(const char *file_name, QL_FILE_MODE_E mode)
{
    QL_FILE stream = (QL_FILE)malloc(sizeof(ql_flie_s));
    stream->client = at_client_get_first();
    at_response_t resp = NULL;

    resp = at_create_resp_new(128, 0, (500), NULL);

    if (at_obj_exec_cmd(stream->client, resp, "AT+QFOPEN=\"%s\",%d", file_name, mode) < 0)
    {
        LOG_E("open %s failed.", file_name);
        at_delete_resp(resp);
        return NULL;
    }
     
    int file_handle = -1;
    for (u8_t i = 0; i < resp->line_counts; i++)
    {
        const char *line = at_resp_get_line(resp, i + 1);
        LOG_D("resp line [%d]: %s", i, line);

        if (sscanf(line, "+QFOPEN: %d", &file_handle) == 1)
        {
            break;
        }
    }
    at_delete_resp(resp);
    if (file_handle < 0)
    {
        LOG_E("open %s failed.", file_name);
        free(stream);
        return NULL;
    }
    stream->fd = file_handle;
    stream->err = 0;
    stream->len = 0;
    qosa_sem_create(&stream->sem, 0);
    return stream;
}

int ql_fclose(QL_FILE stream)
{
    if (NULL == stream)
        return -1;
    at_response_t resp = NULL;

    resp = at_create_resp_new(128, 0, (500), NULL);

    if (at_obj_exec_cmd(stream->client, resp, "AT+QFCLOSE=%d", stream->fd) < 0)
    {
        LOG_E("close file failed %d.", stream->fd);
        at_delete_resp(resp);
        qosa_sem_delete(stream->sem);
        free(stream);
        return -1;
    }
    at_delete_resp(resp);
    qosa_sem_delete(stream->sem);
    free(stream);
    return 0;
}

int ql_fread(void *ptr, size_t size, size_t nmemb, QL_FILE stream)
{
    if (NULL == ptr || NULL == stream || size * nmemb == 0)
        return -1;
    at_response_t resp = NULL;
    stream->ptr = ptr;
    stream->len = 0;
    resp = at_create_resp_by_selffunc_new(256, 0, (20000), ql_read_cb, stream);

    if (at_obj_exec_cmd(stream->client, resp, "AT+QFREAD=%d,%d", stream->fd, size * nmemb) < 0)
    {
        LOG_E("read file failed %d.", stream->fd);
        at_delete_resp(resp);
        return -1;
    }   

    at_delete_resp(resp);
    qosa_sem_wait(stream->sem, QOSA_WAIT_FOREVER);
    if (stream->err != 0 && stream->err != 402)
    {
        LOG_E("read file failed %d.", stream->fd);
        return -1;
    }
    return stream->len / size;
}

int ql_fwrite(const void *ptr, size_t size, size_t nmemb, QL_FILE stream)
{
    if (NULL == ptr || NULL == stream || size * nmemb == 0)
        return -1;
    at_response_t resp = at_create_resp_by_selffunc_new(256, 0, (20000), ql_write_cb, stream);
    qosa_mutex_lock(stream->client->lock, QOSA_WAIT_FOREVER);
    stream->ptr = (void *)ptr;
    stream->len = size * nmemb;
    if (at_obj_exec_cmd(stream->client, resp, "AT+QFWRITE=%d,%d", stream->fd, size * nmemb) < 0)
    {
        LOG_E("write file failed %d.", stream->fd);
        at_delete_resp(resp);
        qosa_mutex_unlock(stream->client->lock);
        return -1;
    }
    qosa_sem_wait(stream->sem, QOSA_WAIT_FOREVER);
    qosa_mutex_unlock(stream->client->lock);
    return (stream->err == -1) ? -1 : (stream->len / size);
}

int ql_fseek(QL_FILE stream, long offset, QL_SEEK_MODE_E whence)
{
    at_response_t resp = NULL;

    resp = at_create_resp_new(128, 0, 1000, NULL);
    if (at_obj_exec_cmd(stream->client, resp, "AT+QFSEEK=%d,%d,%d", stream->fd, offset, whence) < 0)
    {
        LOG_E("AT+QFSEEK failed.");
        at_delete_resp(resp);
        return -1;
    }

    LOG_I("QFSEEK ok.");
    at_delete_resp(resp);
    return 0;
}

int ql_ftell(QL_FILE stream)
{
    at_response_t resp = NULL;

    resp = at_create_resp_new(128, 4, 1000, NULL);
    if (at_obj_exec_cmd(stream->client, resp, "AT+QFPOSITION=%d", stream->fd) < 0)
    {
        LOG_E("AT+QFSEEK failed.");
        at_delete_resp(resp);
        return -1;
    }
    const char *line =NULL;
    int position = -1;  
    for (int i = 0; i < resp->line_counts; i++)
    {
        line = at_resp_get_line(resp, i + 1);
        if (sscanf(line, "+QFWRITE: %d,%*d", &position) == 1)
        {
            break;
        }
    }
    at_delete_resp(resp);
    return position;
}

int ql_fflush(QL_FILE stream)
{
    at_response_t resp = NULL;

    resp = at_create_resp_new(128, 0, 1000, NULL);
    if (at_obj_exec_cmd(stream->client, resp, "AT+QFFLUSH=%d", stream->fd) < 0)
    {
        LOG_E("AT+QFFLUSH failed.");
        at_delete_resp(resp);
        return -1;
    }
    at_delete_resp(resp);
    return 0;  
}

int ql_remove(at_client_t client, const char *filename)
{
    at_response_t resp = NULL;

    resp = at_create_resp_new(128, 0, (5000), NULL);

    if (at_obj_exec_cmd(client, resp, "AT+QFDEL=\"%s\"", filename) < 0)
    {
        LOG_E("AT+QFDEL failed.");
        at_delete_resp(resp);
        return -1;
    }
    at_delete_resp(resp);
    return 0;
}

int ql_get_storage_space(at_client_t client, const char *filename, size_t *free, size_t *total)
{
    if (NULL == free || NULL == total)
        return -1;
    at_response_t resp = NULL;

    resp = at_create_resp_new(256, 0, (1000), NULL);
    if (at_obj_exec_cmd(client, resp, "AT+QFLDS=\"%s\"", filename) < 0)
    {
        LOG_E("AT+QFLDS query failed");
        at_delete_resp(resp);
        return -1;
    }

    for (int i = 0; i < resp->line_counts; i++)
    {
        const char *line = at_resp_get_line(resp, i + 1);

        if (sscanf(line, "+QFLDS: %d,%d", free, total) == 2)
        {
            at_delete_resp(resp);
            return 0;
        }
    }
    LOG_E("AT+QFLDS query ok, but no valid response found.");
    at_delete_resp(resp);
    return -1;
}

int ql_file_list(at_client_t client, const char *filename, ql_file_info_s **list)
{
    if (NULL == filename)
        return -1;
    at_response_t resp = NULL;
    QL_FILE stream = (QL_FILE)malloc(sizeof(ql_flie_s));
    stream->err = 0;
    stream->client = client;
    stream->file_list = NULL;
    qosa_sem_create(&stream->sem, 0);
    *list = NULL;
    resp = at_create_resp_by_selffunc_new(256, 0, (20000), ql_list_cb, stream);
    if (at_obj_exec_cmd(stream->client, resp, "AT+QFLST=\"%s\"", filename) < 0)
    {
        LOG_E("get file list failed %s.", filename);
        qosa_sem_delete(stream->sem);
        free(stream);
        at_delete_resp(resp);
        return -1;
    }   

    qosa_sem_wait(stream->sem, QOSA_WAIT_FOREVER);
    at_delete_resp(resp);
    qosa_sem_delete(stream->sem);
    *list = stream->file_list;
    int ret = (stream->err != 0) ? -1 : 0;
    free(stream);
    return ret;
}

void ql_free_file_list(ql_file_info_s *list)
{
    ql_file_info_s *current = list;
    while (current != NULL)
    {
        ql_file_info_s *temp = current;
        current = current->next;
        free(temp);
    }
}

static void ql_upload_cb(const char *data, size_t len, void* arg)
{
    QL_FILE stream = (QL_FILE)arg;
    char buffer[QL_MAX_ONCE_LEN] = {0};
    UINT br = 0;
    at_client_self_recv(stream->client, buffer, QL_MAX_ONCE_LEN, 2000, 1, true);
    if (strstr(buffer, "CONNECT\r\n") != NULL)
    {
        while (true)
        {
            if (f_read(&stream->file, buffer, QL_MAX_ONCE_LEN, &br) != FR_OK || 0 == br)
            {
                break;
            }
            at_client_obj_send(stream->client, buffer, br, false);
        }
    }
    else if (strstr(buffer, "ERROR") != NULL)
    {
        stream->err = -1;
        sscanf(buffer, "+CME ERROR: %d", &stream->err);
        if (407 == stream->err)
            LOG_I("file already exists");
        qosa_sem_release(stream->sem);
        return;
    }

    memset(buffer, 0, QL_MAX_ONCE_LEN);
    while (1)
    {
        at_client_self_recv(stream->client, buffer, QL_MAX_ONCE_LEN, 2000, 1, true);
        if (strstr(buffer, "OK\r\n") != NULL)
            break;  
    }
    qosa_sem_release(stream->sem);

}

int ql_file_upload(at_client_t client, const char *localname, const char *remotename)
{
    int ret = 0;
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_TFCARD__
    FILINFO file_info;
    FRESULT result;
    if (NULL == localname || NULL == remotename)
        return -1;
    result = f_stat(localname, &file_info);
    if (result != FR_OK)
    {
        LOG_E("File does not exist %s", localname);
        return -1;
    }
    QL_FILE stream = (QL_FILE)malloc(sizeof(ql_flie_s));
    stream->err = 0;
    stream->client = client;
    qosa_sem_create(&stream->sem, 0);
    FRESULT res = f_open(&stream->file, localname, FA_READ);
    if (res != FR_OK)
    {
        LOG_E("Error opening file for reading %d", res);
        qosa_sem_delete(stream->sem);
        free(stream);
        return -1;
    }
    at_response_t resp = at_create_resp_by_selffunc_new(128, 0, (5000), ql_upload_cb, stream);

    if (at_exec_cmd(resp, "AT+QFUPL=\"%s\",%d,20000", remotename, file_info.fsize) < 0)
    {
        f_close(&stream->file);
        qosa_sem_delete(stream->sem);
        free(stream);
        return -1;
    }
    qosa_sem_wait(stream->sem, QOSA_WAIT_FOREVER);
    ret = (stream->err == 0 || stream->err == 407) ? 0 : -1;
    at_delete_resp(resp);
    f_close(&stream->file);
    qosa_sem_delete(stream->sem);
    free(stream);
#endif
    return ret;
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__ */
