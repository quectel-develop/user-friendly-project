#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_UDP_SERVER__
#ifndef __EXAMPLE_UDP_SERVER_H__
#define __EXAMPLE_UDP_SERVER_H__

int example_udp_server_test(short sin_port, char *sin_addr, int loop_count, int loop_interval);

#endif /* __EXAMPLE_UDP_SERVER_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_UDP_SERVER__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
