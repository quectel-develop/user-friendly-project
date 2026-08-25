#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_UDP_CLIENT__
#ifndef __CLI_UDP_H__
#define __CLI_UDP_H__

int cli_udp_client_test(short sin_port, char *sin_addr, int loop_count, int loop_interval);

#endif /* __CLI_UDP_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_UDP_CLIENT__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
