#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__
#ifndef __CLI_FTP_H__
#define __CLI_FTP_H__
#include "ql_ftp.h"
#include "qosa_def.h"

void cli_ftp_get_help(void);
int cli_ftp_test(s32_t argc, char *argv[]);

void cli_example_ftp();
#endif /* __CLI_FTP_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
