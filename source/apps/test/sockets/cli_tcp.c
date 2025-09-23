#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_CLIENT__
#include "cli_tcp.h"
#include "ql_socket.h"
#include "qosa_log.h"

#define SOCKET_MAX_LENGTH 512
extern void at_print_raw_cmd(const char *type, const char *cmd, size_t size);

int cli_tcp_client_test(short sin_port, char *sin_addr, int loop_count, int loop_interval)
{
    int fd, ret;
    char *buf = NULL;
    struct sockaddr_in ser_sockaddr;

    LOG_D("%s Start",__FUNCTION__);

    //1.creat socket(ipv4 tcp)
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd<0)
    {
        LOG_E("Socket creat err");
        return -1;
    }

    //2.connect server
    ser_sockaddr.sin_family =  AF_INET;                 //IPV4
    ser_sockaddr.sin_port   =  htons(sin_port);         //port
    ser_sockaddr.sin_addr.s_addr  = inet_addr(sin_addr);//ip
    ret = connect(fd, (struct sockaddr *)&ser_sockaddr, sizeof(ser_sockaddr));
    if(ret != 0)
    {
        LOG_E("Server connection failure");
        close(fd);
        return -1;
    }
    struct timeval timeout = {60, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    LOG_I("Server connection success %d, %d", ret, fd);
    buf = (char *)malloc(SOCKET_MAX_LENGTH);
    memset(buf,0,SOCKET_MAX_LENGTH);
    sprintf(buf, "%d", fd);
    for (int i=0; i<loop_count; i++)
    {
        ret = send(fd,buf,strlen(buf),0);
        if (ret > 0)
        {
            at_print_raw_cmd("tcp send data", buf, ret);
            LOG_I("Tcp client send ok len = %d, fd = %d", ret, fd);
        }
        else
        {
            LOG_E("Tcp client send %s err %d, %d", buf, ret, fd);
            break;
        }

        memset(buf,0,SOCKET_MAX_LENGTH);
        ret = recv(fd,buf,SOCKET_MAX_LENGTH,0);
        if (ret > 0) 
        {
            at_print_raw_cmd("tcp recv data", buf, ret);
            LOG_I("Tcp client recv len: %d, fd :%d", ret, fd);
        }
        else if (ret == 0)
        {
            LOG_I("peer close");
            break;
        }
        else
        {
            LOG_E("tcp recv timeout");
            break;
        }

        qosa_task_sleep_ms(loop_interval);
    }
    free(buf);
    close(fd);

    LOG_D("%s over",__FUNCTION__);

    return 0;
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_CLIENT__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
