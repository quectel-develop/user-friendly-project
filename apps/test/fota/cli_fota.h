#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifndef __CLI_FOTA_H__
#define __CLI_FOTA_H__
#include "ql_fota.h"

void cli_fota_get_help(void);
int cli_fota_test(s32_t argc, char *argv[]);
#endif /* __CLI_FOTA_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
