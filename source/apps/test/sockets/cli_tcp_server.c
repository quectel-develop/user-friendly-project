#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_SERVER__
#include "cli_tcp_server.h"
#include "ql_socket.h"
#include "qosa_log.h"

extern void at_print_raw_cmd(const char *type, const char *cmd, size_t size);
static void tcp_server_incoming_proc(void *argument)
{
    int ret = QOSA_OK;
    socket_tcp_server_config *config = (socket_tcp_server_config *)argument;
    unsigned int cli_sock_fd = config->sock_fd;
    char buf[64];

    LOG_V("%s Start(%d)",__FUNCTION__, cli_sock_fd);

    while (true)
	{
		memset(buf,0,64);
		ret = recv(cli_sock_fd,buf,sizeof(buf),0);
        if (ret > 0)
        {
            at_print_raw_cmd("tcp server recv data", buf, ret);
            LOG_I("Tcp server recv len: %d, fd :%d", ret, cli_sock_fd);
        }
        else
        {
            LOG_I("peer disconnect");
            break;
        }
        ret = send(cli_sock_fd,buf,strlen(buf),0);
        if (ret > 0)
        {
            at_print_raw_cmd("tcp server send data", buf, ret);
            LOG_I("Tcp server send ok len = %d, fd = %d", ret, cli_sock_fd);
        }
        else
        {
            LOG_E("Tcp server send %s err %d", buf, ret);
            break;
        }
    }
    close(cli_sock_fd);

    LOG_V("%s over",__FUNCTION__);
    qosa_task_exit();
}

int cli_tcp_server_test(short sin_port, char *sin_addr, int max_connect_num, int loop_count, int loop_interval)
{
    int ser_sock_fd, i, ret, cli_sock_fd;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in ser_sock_addr, cli_sock_addr;
    static socket_tcp_server_config config;

    osa_task_t thread_id =NULL;

    LOG_V("%s Start",__FUNCTION__);

    //1.creat socket(ipv4 tcp)
    ser_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(ser_sock_fd<0)
    {
        LOG_E("Socket creat err %d", ser_sock_fd);
        return -1;
    }

    //2.bind server socket
    ser_sock_addr.sin_family =  AF_INET;                //IPV4
    ser_sock_addr.sin_port   =  htons(sin_port);        //port
    ser_sock_addr.sin_addr.s_addr  = inet_addr(sin_addr);  //ip
    ret = bind(ser_sock_fd, (struct sockaddr *)&ser_sock_addr, addr_len);
    if(ret == -1)
    {
        LOG_E("Server bind failure");
        close(ser_sock_fd);
        return -1;
    }
    LOG_I("Server bind success");

    for (i=0; i<max_connect_num; i++)
    {
    //3. listen
        ret = listen(ser_sock_fd, 0); //The second parameter is temporarily invalid
        if(ret == -1)
        {
            LOG_E("Server listen failure");
            close(ser_sock_fd);
            return -1;
        }

    //4. accept
        cli_sock_fd = accept(ser_sock_fd, (struct sockaddr*)(&cli_sock_addr), &addr_len);
        LOG_I("cli_sock_fd = %d", cli_sock_fd);
        if(cli_sock_fd == -1)
        {
            LOG_E("Server accept failure");
            break;
        }
        LOG_I("New client connect(%s, %d), %d", inet_ntoa(cli_sock_addr.sin_addr.s_addr), ntohs(cli_sock_addr.sin_port), cli_sock_fd);

    //5. create net service
        config.loop_count = loop_count;
        config.loop_interval = loop_interval;
        config.sock_fd = cli_sock_fd;
        ret = qosa_task_create(&thread_id, 256*20, QOSA_PRIORITY_NORMAL, "TCP_S", tcp_server_incoming_proc, (void *)&config);
        if (ret != QOSA_OK)
        {
            LOG_E ("thread_id thread could not start!");
            close(cli_sock_fd);
            continue;
        }
        LOG_I("%s (%x)",__FUNCTION__, thread_id);
    }
    close(ser_sock_fd);

    LOG_D("%s over",__FUNCTION__);

    return 0;
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_SERVER__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
