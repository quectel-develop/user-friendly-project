#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_CLIENT__
#ifndef __CLI_TCP_H__
#define __CLI_TCP_H__

int cli_tcp_client_test(short sin_port, char *sin_addr, int loop_count, int loop_interval, const char* file_name);

#endif /* __CLI_TCP_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_CLIENT__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
