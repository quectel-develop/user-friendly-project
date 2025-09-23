#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_SERVER__
#include "qosa_def.h"
#include "qosa_log.h"
#include "ql_socket.h"
#include "example_tcp_server.h"

#define QL_MAX_CLIENT 12
extern void at_print_raw_cmd(const char *type, const char *cmd, size_t size);

int example_tcp_server_test(short sin_port, char *sin_addr, int max_connect_num)
{
    int ser_sock_fd, ret;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in ser_sock_addr, cli_sock_addr;

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
    //3. listen
    ret = listen(ser_sock_fd, 0); //The second parameter is temporarily invalid
    if(ret == -1)
    {
        LOG_E("Server listen failure");
        close(ser_sock_fd);
        return -1;
    }
    int client_fd[QL_MAX_CLIENT];
    for (int i = 0; i < QL_MAX_CLIENT; ++i)
        client_fd[i] = -1;

    ql_fd_set allset, rset;
    QL_FD_ZERO(&allset);
    QL_FD_SET(ser_sock_fd, &allset);
    LOG_I("allset = %d", allset);
    int maxfd = ser_sock_fd;
    int accept_count = 0;
    bool quit = false;
    while (!quit)
    {
        rset = allset;
        LOG_I("allset = %d", allset);
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int nready = select(maxfd + 1, &rset, NULL, NULL, &tv);
        if (nready < 0)
        {
            LOG_E("select failed");
            break;
        }
        else if (nready == 0)
        {
            continue;
        }
        if (QL_FD_ISSET(ser_sock_fd, &rset))
        {
            int connfd = accept(ser_sock_fd, (struct sockaddr*)(&cli_sock_addr), &addr_len);
            if (connfd <= 0)
            {
                LOG_E("accept");
                continue;
            }
            else
            {
                LOG_I("New client connect(%s, %d), %d", inet_ntoa(cli_sock_addr.sin_addr.s_addr), ntohs(cli_sock_addr.sin_port), connfd);
                int i = 0;
                for (i = 0; i < QL_MAX_CLIENT; ++i)
                {
                    if (client_fd[i] < 0)
                    {
                        client_fd[i] = connfd;
                        break;
                    }
                }
                if (i == QL_MAX_CLIENT)
                {
                    LOG_E("Too many clients, rejecting connection");
                    close(connfd);
                } 
                else
                {
                    QL_FD_SET(connfd, &allset);
                    if (connfd > maxfd) maxfd = connfd;
                    if (++accept_count >= max_connect_num)
                    {
                        LOG_W("accept max connect");
                        QL_FD_CLR(ser_sock_fd, &allset);
                        close(ser_sock_fd);
                    }
                }
            }
            if (--nready <= 0) continue; // no more fds ready
        }
        for (int i = 0; i < QL_MAX_CLIENT && nready > 0; ++i)
        {
            int sockfd = client_fd[i];
            if (sockfd < 0)
                continue;
            if (QL_FD_ISSET(sockfd, &rset))
            {
                nready--;
                char buf[64] = {0};
                int n = recv(sockfd, buf, sizeof(buf), 0);
                if (n <= 0)
                {
                    LOG_I("peer disconnect");
                    close(sockfd);
                    QL_FD_CLR(sockfd, &allset);
                    client_fd[i] = -1;
                    if (accept_count >= max_connect_num) // wait all client disconnect, server quit
                    {
                        int i = 0;
                        for (i = 0; i < QL_MAX_CLIENT; ++i)
                        {
                            if(client_fd[i] != -1)
                                break;
                        }
                        if (i == QL_MAX_CLIENT)
                            quit = true;
                    }
                }
                else
                {
                    at_print_raw_cmd("tcp server recv data", buf, ret);
                    LOG_I("Tcp server recv len: %d, fd :%d", ret, sockfd);
                    int ret = send(sockfd, buf, n, 0);
                    if (ret <= 0)
                    {
                        LOG_E("Tcp server send %s err %d", buf, ret);
                        close(sockfd);
                        QL_FD_CLR(sockfd, &allset);
                        client_fd[i] = -1;
                        continue;
                    }
                    else
                    {
                        at_print_raw_cmd("tcp server send data", buf, ret);
                        LOG_I("Tcp server send ok len = %d, fd = %d", ret, sockfd);
                    }
                }
            }
        }
    }
    LOG_D("%s over",__FUNCTION__);

    return 0;
}


#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_SERVER__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
