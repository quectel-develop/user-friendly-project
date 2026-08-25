#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__
#include "qosa_log.h"
#include "ql_file.h"

#define FILE_PATH       "UFS:ufp_test.txt"
#define FILE_CONTENT    "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

void example_file(void* argv)
{
    QL_FILE file_handle = NULL;
    int ret = 0;
    char read_data[64] = {0};
    u16_t read_len = 0;

    /* OPEN */
    file_handle = ql_fopen(FILE_PATH, QL_FILE_MODE_CREATE_OR_OPEN);
    if(NULL == file_handle)
    {
        LOG_E("***** [%s] Open failed.");
        return;
    }
    LOG_I("Open %s successed, file_handle: %d", FILE_PATH, file_handle->fd);

    /* WRITE */
    ret = ql_fwrite((char *)FILE_CONTENT, 1, strlen(FILE_CONTENT), file_handle);
    if(ret < 0)
    {
        LOG_E("***** Write failed, err: %d", ret);
        ql_fclose(file_handle);
        return;
    }
    LOG_I("Write successed, write data: %s", FILE_CONTENT);

    /* SEEK */
    ret = ql_fseek(file_handle, 0, 0);
    if(ret < 0)
    {
        LOG_E("***** Seek failed");
        ql_fclose(file_handle);
        return;
    }
    LOG_I("Seek successed.");

    /* READ */
    read_len = ql_fread(read_data, 1, sizeof(read_data), file_handle);
    if(read_len < 0)
    {
        LOG_E("***** Read failed, err: %d", file_handle->err);
        ql_fclose(file_handle);
        return;
    }
    LOG_I("Read successed, read len[%d] bytes, read data: %s", read_len, read_data);
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
