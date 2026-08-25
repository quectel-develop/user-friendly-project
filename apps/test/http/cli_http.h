#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_HTTP_S__
#ifndef __CLI_HTTP_H__
#define __CLI_HTTP_H__
#include "ql_http.h"
#include "qosa_def.h"

void cli_http_get_help(void);
int cli_http_test(s32_t argc, char *argv[]);
void cli_example_test_post();
#endif /* __CLI_HTTP_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_HTTP_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
