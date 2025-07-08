#include "QuectelConfig.h"
#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_FILESYSTEM_EXAMPLE__
#ifndef __EXAMPLE_FS_H__
#define __EXAMPLE_FS_H__

#define FILE_NAME_MAX_LEN    64
#define WRITE_BUF_MAX_LEN    30

#include "bg95_filesystem.h"
typedef struct {
    int fs_type;//0 :list
    char name_pattern[FILE_NAME_MAX_LEN];
    u8_t open_mode;
    u16_t file_handle;
    u16_t wirte_read_size; //
    char wirte_buffer[WRITE_BUF_MAX_LEN];
}fs_test_config;
#endif /* __EXAMPLE_FTP_H__ */
#endif  /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_FILESYSTEM_EXAMPLE__ */
