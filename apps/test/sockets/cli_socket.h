#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__
#ifndef __CLI_SOCKET_H__
#define __CLI_SOCKET_H__
#include "qosa_def.h"

typedef enum {
    SOCKET_TCP,
    SOCKET_UDP,
    SOCKET_TCP_SERVER,
    SOCKET_UDP_SERVER,
} socket_test_type;

typedef struct {
    socket_test_type type;
    unsigned int max_connect_num;
    unsigned short sin_port;
    unsigned int loop_count;
    unsigned int loop_interval;  //In milliseconds
    char sin_addr[32];
    char file_name[64];
    void *user_data;
}socket_test_config;

void cli_socket_get_help(void);
int cli_socket_test(s32_t argc, char *argv[]);

#endif /* __CLI_SOCKET_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
