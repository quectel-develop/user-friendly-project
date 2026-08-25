#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_PSM__
#ifndef __CLI_PSM_H__
#define __CLI_PSM_H__
#include "ql_psm.h"
#include "qosa_def.h"

void cli_psm_get_help(void);
int cli_psm_test(s32_t argc, char *argv[]);

#endif /* __CLI_PSM_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_PSM__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
