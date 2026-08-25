#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__
#ifndef __CLI_FILE_H__
#define __CLI_FILE_H__

void cli_file_get_help(void);
int cli_file_test(s32_t argc, char *argv[]);

#endif /* __CLI_FILE_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
