#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__
#include "ql_fs.h"
#include "qosa_system.h"
#include "qosa_log.h"

#define FILE_PATH       "UFS:ufp_test.txt"
#define FILE_CONTENT    "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

static osa_task_t fs_task = NULL;

static void example_fs_proc(void* argv)
{
    int file_handle = -1;
    int ret = 0;
    char read_data[64] = {0};
    u16_t read_len = 0;

    LOG_D("======== example_fs_proc start ========");

    /* OPEN */
    file_handle = ql_fs_open(FILE_PATH, 0);
    if(file_handle < 0)
    {
        LOG_E("***** [%s] Open failed.");
        goto __EXIT;
    }
    LOG_D("[%s] Open successed, file_handle: %d", FILE_PATH, file_handle);

    /* WRITE */
    ret = ql_fs_write(file_handle, strlen(FILE_CONTENT), (char *)FILE_CONTENT);
    if(ret < 0)
    {
        LOG_E("***** Write failed, err: %d", ret);
        goto __EXIT;
    }
    LOG_D("Write successed, write data:");
    LOG_D("%s", FILE_CONTENT);

    /* SEEK */
    ret = ql_fs_seek(file_handle, 0, 0);
    if(ret < 0)
    {
        LOG_E("***** Seek failed, err: %d", ret);
        goto __EXIT;
    }
    LOG_D("Seek successed.");

    /* READ */
    read_len = ql_fs_read(file_handle, sizeof(read_data), read_data);
    if(read_len <= 0)
    {
        LOG_E("***** Read failed, err: %d", read_len);
        goto __EXIT;
    }
    LOG_D("Read successed, read len[%d] bytes, read data:", read_len);
    LOG_D("%s", read_data);

__EXIT:
    /* CLOSE */
    if(file_handle >= 0)
    {
        ret = ql_fs_close(file_handle);
        if(ret == 0)
            LOG_D("[%s] Close successed.", FILE_PATH);
        else
            LOG_E("***** [%s] Close failed, file_handle: %d, err: %d", FILE_PATH, file_handle, ret);
    }

    LOG_D("======== example_fs_proc end ========");
    qosa_task_exit();
}


void ql_fs_example_init(void)
{
	int ret = QOSA_OK;

	ret = qosa_task_create(&fs_task, 1024*4, QOSA_PRIORITY_NORMAL, "example_fs", example_fs_proc, NULL);
	if (ret != QOSA_OK)
	{
		LOG_E ("***** fs task create failed.");
		return -1;
	}
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
