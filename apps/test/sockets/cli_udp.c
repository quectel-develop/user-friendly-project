#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_UDP_CLIENT__
#include <time.h>
#include "cli_udp.h"
#include "ql_socket.h"
#include "qosa_log.h"

#define SOCKET_MAX_LENGTH 512
extern void at_print_raw_cmd(const char *type, const char *cmd, size_t size);

int cli_udp_client_test(short sin_port, char *sin_addr, int loop_count, int loop_interval)
{
    int fd, ret;
    char *buf = NULL;
    struct sockaddr_in ser_sockaddr;
    socklen_t len = sizeof(struct sockaddr_in);

    LOG_I("%s Start",__FUNCTION__);

    //1.creat socket(ipv4 tcp)
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd<0)
    {
        LOG_E("socket create err");
        return -1;
    }
    struct timeval timeout = {60, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    //2. creat server addr
    ser_sockaddr.sin_family =  AF_INET;                 //IPV4
    ser_sockaddr.sin_port   =  htons(sin_port);         //port
    ser_sockaddr.sin_addr.s_addr  = inet_addr(sin_addr);//ip
    buf = (char *)malloc(SOCKET_MAX_LENGTH);
    memset(buf,0,SOCKET_MAX_LENGTH);
    sprintf(buf, "%d", fd);
    for (int i=0; i<loop_count; i++)
    {
        ret = sendto(fd,buf,strlen(buf),0,(struct sockaddr *)&ser_sockaddr, len);
        if (ret > 0)
        {
           at_print_raw_cmd("udp send data", buf, ret);
           LOG_I("Udp client send ok, len = %d, fd = %d", ret, fd);
        }
        else
        {
            LOG_E("Udp client send %s err %d, %d", buf, ret, fd);
            break;
        }

        memset(buf,0,SOCKET_MAX_LENGTH);
        ret = recvfrom(fd, buf, SOCKET_MAX_LENGTH, 0,(struct sockaddr *)&ser_sockaddr, &len);
        if (ret > 0)
        {
            at_print_raw_cmd("udp recv data", buf, ret);
            LOG_I("Udp client recv len: %d, fd = %d", ret, fd);
        }
        else
        {
            LOG_E("Udp recv timeout");
            break;
        }

        qosa_task_sleep_ms(loop_interval);
    }
    close(fd);

    LOG_V("%s over",__FUNCTION__);

    return 0;
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_UDP_CLIENT__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
