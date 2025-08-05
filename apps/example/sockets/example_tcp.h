#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_CLIENT__
#ifndef __EXAMPLE_TCP_H__
#define __EXAMPLE_TCP_H__

int example_tcp_client_test(short sin_port, char *sin_addr, int loop_count, int loop_interval);

#endif /* __EXAMPLE_TCP_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_CLIENT__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
