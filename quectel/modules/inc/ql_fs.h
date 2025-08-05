#include "QuectelConfig.h"

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__
/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-10-12     armink       first version
 */
#ifndef __QL_FS_H__
#define __QL_FS_H__
#include "ff.h"
#include "qosa_system.h"
typedef struct {
    char filename[256];
    u32_t filesize;
} File_Moudle_Info;

struct file_device
{
    struct at_client *client;   /* AT Client object for AT device */
    osa_event_t file_event;
    #ifdef __QUECTEL_UFP_FEATURE_SUPPORT_TFCARD__
    FIL file;                   /* AT device socket event */
    #endif
};

int ql_fs_open(char *localfile,u8_t mode);
int ql_fs_close(u16_t file_handle);
int ql_fs_write(u16_t file_handle,u16_t wirte_size, char *write_buffer);
int ql_fs_read(u16_t file_handle, u16_t read_size, char *out_data);
int ql_file_del(const char *dirname);
int ql_fs_seek(u16_t file_handle, u16_t offset, u16_t pos);
int ql_file_put_ex(char *localfile, char *remotefile,u32_t up_size );
int ql_fs_get_free(char *localfile, s32_t *free_size, s32_t *total_size);
int ql_module_list_get(const char *dirname, File_Moudle_Info *fileList, u8_t maxFiles, u8_t mode);

#endif /* __QL_FS_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__ */

