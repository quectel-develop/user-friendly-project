#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_UDP_SERVER__
#include "cli_udp_server.h"
#include "ql_socket.h"
#include "qosa_log.h"

extern void at_print_raw_cmd(const char *type, const char *cmd, size_t size);
int cli_udp_server_test(short sin_port, char *sin_addr, int loop_count, int loop_interval)
{
    int ser_sock_fd, ret;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    char buf[128] = "hello";
    struct sockaddr_in ser_sock_addr, cli_sock_addr;

    LOG_V("%s Start",__FUNCTION__);

    //1.creat socket(ipv4 udp)
    ser_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(ser_sock_fd<0)
    {
        LOG_E("Socket creat err %d", ser_sock_fd);
        return -1;
    }

    //2.bind server socket
    ser_sock_addr.sin_family =  AF_INET;                   //IPV4
    ser_sock_addr.sin_port   =  htons(sin_port);           //port
    ser_sock_addr.sin_addr.s_addr  = inet_addr(sin_addr);  //ip
    ret = bind(ser_sock_fd, (struct sockaddr *)&ser_sock_addr, addr_len);
    if(ret == -1)
    {
        LOG_E("Server bind failure");
        close(ser_sock_fd);
        return -1;
    }
    LOG_I("Server bind success");

    //3.recvfrom/sendto
    for (int i=0; i<loop_count; i++)
    {
        memset(buf,0,64);
        ret = recvfrom(ser_sock_fd,buf,64,0,(struct sockaddr *)&cli_sock_addr, &addr_len);
        if (ret > 0)
        {
            at_print_raw_cmd("udp server recv data", buf, ret);
            LOG_I("Udp server recv len = %d, fd = %d (from:%s, %d)", ret, ser_sock_fd, inet_ntoa(cli_sock_addr.sin_addr.s_addr), ntohs(cli_sock_addr.sin_port));
         }
        else
            LOG_E("Udp server recv %s err", buf);

        ret = sendto(ser_sock_fd,buf,strlen(buf),0,(struct sockaddr *)&cli_sock_addr, addr_len);
        if (ret > 0)
        {
            at_print_raw_cmd("udp server send data", buf, ret);
            LOG_I("Udp server send ok, len = %d, fd = %d (to:%s, %d)",ret, ser_sock_fd, inet_ntoa(cli_sock_addr.sin_addr.s_addr), ntohs(cli_sock_addr.sin_port));
        }
         else
            LOG_E("Udp server send err(to:%s, %d)", inet_ntoa(cli_sock_addr.sin_addr.s_addr), ntohs(cli_sock_addr.sin_port));
    }
    close(ser_sock_fd);

    LOG_D("%s over",__FUNCTION__);

    return 0;
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_UDP_SERVER__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
