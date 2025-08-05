#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_CLIENT__
#include "debug_service.h"
#include "at_socket.h"
#include "qosa_log.h"

int example_tcp_client_test(short sin_port, char *sin_addr, int loop_count, int loop_interval)
{
    int fd, ret;
    char buf[64];
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
        closesocket(fd);
        return -1;
    }
    LOG_I("Server connection success 0x%x", ret);

    sprintf(buf, "%d", fd);
    for (int i=0; i<loop_count; i++)
    {
        ret = send(fd,buf,strlen(buf),0);
        if (ret > 0)
            LOG_I("Tcp client send %s ok %d, %d", buf, ret, fd);
        else
            LOG_E("Tcp client send %s err %d, %d", buf, ret, fd);

        memset(buf,0,64);
        ret = recv(fd,buf,64,0);
        if (0 < ret)
            LOG_I("Tcp client recv %s ok", buf);
        else
            LOG_E("Tcp client recv %s err", buf);

        qosa_task_sleep_ms(loop_interval);
    }
    closesocket(fd);

    LOG_D("%s over",__FUNCTION__);

    return 0;
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_CLIENT__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
