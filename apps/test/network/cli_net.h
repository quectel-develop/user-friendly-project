#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__
#ifndef __CLI_NETWORK_H__
#define __CLI_NETWORK_H__
#include "ql_net.h"

void cli_net_get_help(void);
int cli_net_test_init(void);
int cli_net_test(s32_t argc, char *argv[]);
#endif /* __CLI_NETWORK_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
