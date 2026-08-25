#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__
#include "cli_socket.h"
#include "ql_net.h"
#include "qosa_system.h"
#include "qosa_log.h"
#include "cli_tcp.h"
#include "cli_udp.h"
#include "cli_tcp_server.h"
#include "cli_udp_server.h"

void cli_socket_get_help(void)
{
    LOG_I("| socket socket_type ip port count interval_ms max_connect_num/file_name                                     |");
    LOG_I("|      socket_type   : socket type                                                                 |");
    LOG_I("|                      0: TCP                                                                      |");
    LOG_I("|                      1: UDP                                                                      |");
    LOG_I("|                      2: TCP SERVER                                                               |");
    LOG_I("|                      3: UDP SERVER                                                               |");
    LOG_I("|     ip              : ip address                                                                 |");
    LOG_I("|     port            : port                                                                       |");
    LOG_I("|     count           : Number of times the TCP/UDP client sends data                              |");
    LOG_I("|     interval_ms     : Time interval between TCP/UDP client data transmissions                    |");
    LOG_I("|     max_connect_num : Max number connect request(only tcp server need set)                       |");
    LOG_I("|     file_name       : when socket_type == 0, this parameter specifies the file to upload.        |");
}

extern ql_net_t s_net_handle;
static void socket_service_proc(void *argument)
{
    socket_test_config *config = (socket_test_config *)argument;
    const char *ip = ql_net_get_ip(s_net_handle);
    LOG_I("IP Address: %s", ip);
    LOG_V("%s Start(heap size = %d)\r\n\r\n", __FUNCTION__, qosa_task_get_free_heap_size());
    if (config->type == SOCKET_TCP)
    {
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_CLIENT__
        cli_tcp_client_test(config->sin_port, config->sin_addr, config->loop_count, config->loop_interval, config->file_name);
#endif
    }
    else if (config->type == SOCKET_UDP)
    {
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_UDP_CLIENT__
        cli_udp_client_test(config->sin_port, config->sin_addr, config->loop_count, config->loop_interval);
#endif
    }
    else if (config->type == SOCKET_TCP_SERVER)
    {
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_SERVER__
        strcpy(config->sin_addr, ip);
        cli_tcp_server_test(config->sin_port, config->sin_addr, config->max_connect_num, config->loop_count, config->loop_interval);
#endif
    }
    else if (config->type == SOCKET_UDP_SERVER)
    {
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_UDP_SERVER__
        strcpy(config->sin_addr, ip);
        cli_udp_server_test(config->sin_port, config->sin_addr, config->loop_count, config->loop_interval);
#endif
    }
    else
    {
        LOG_E("Invalid parameter %d", config->type);
    }
    LOG_V("%s over(heap size = %d)", __FUNCTION__, qosa_task_get_free_heap_size());
    qosa_task_exit();
}


int cli_socket_test(s32_t argc, char *argv[])
{
    osa_task_t thread_id = NULL;
    int ret = QOSA_OK;
    static socket_test_config config;
    
    config.type = atoi(argv[1]);
    strcpy(config.sin_addr, argv[2]);
    config.sin_port = atoi(argv[3]);
    config.loop_count = atoi(argv[4]);
    config.loop_interval = atoi(argv[5]);
    config.file_name[0] = '\0';
    if (config.type == 0 && argc >= 7)
        strcpy(config.file_name, argv[6]);
    LOG_I("type                : %d", config.type);
    LOG_I("ip                  : %s", config.sin_addr);
    LOG_I("port                : %d", config.sin_port);
    LOG_I("loop_count          : %d", config.loop_count);
    LOG_I("loop_interval       : %d", config.loop_interval);
    LOG_I("file_name           : %s", config.file_name);
    if (argc == 7)
    {
        config.max_connect_num = atoi(argv[6]);
        LOG_I("max_connect_num     : %d", config.max_connect_num);
    }

    // Create net service
    ret = qosa_task_create(&thread_id, 256 * 15, QOSA_PRIORITY_NORMAL, "user_socket_test", socket_service_proc, (void *)&config);
    if (ret != QOSA_OK)
    {
        LOG_E("thread_id thread could not start! %d", qosa_task_get_free_heap_size());
        return -1;
    }
    return 0;
}


#else
void cli_socket_get_help(void)
{
    LOG_W("This function is not supported");
}
int cli_socket_test(s32_t argc, char *argv[])
{
    LOG_W("This function is not supported");
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
