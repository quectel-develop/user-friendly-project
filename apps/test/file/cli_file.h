#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__
#ifndef __CLI_FILE_H__
#define __CLI_FILE_H__
#include "ql_fs.h"

#define FILE_NAME_MAX_LEN    64
#define WRITE_BUF_MAX_LEN    30

typedef struct {
    int fs_type;    //0 :list
    char name_pattern[FILE_NAME_MAX_LEN];
    u8_t open_mode;
    u16_t file_handle;
    u16_t write_read_size;
    char write_buffer[WRITE_BUF_MAX_LEN];
}fs_test_config;

void cli_file_get_help(void);
int cli_file_test(int argc, char *argv[]);

#endif /* __CLI_FILE_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
