#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_CLIENT__
#include "cli_tcp.h"
#include "ql_socket.h"
#include "qosa_log.h"
#include "ff.h"

#define SOCKET_MAX_LENGTH 512
extern void at_print_raw_cmd(const char *type, const char *cmd, size_t size);

int cli_tcp_client_test(short sin_port, char *sin_addr, int loop_count, int loop_interval, const char* file_name)
{
    int fd, ret;
    char *buf = NULL;
    struct sockaddr_in ser_sockaddr;
    FRESULT f_res = FR_NO_FILE;
    FIL SDFile;
    UINT bytes_read;

    LOG_D("%s Start",__FUNCTION__);

    if (file_name != NULL && file_name[0] != '\0')
    {
        f_res = f_open(&SDFile, file_name, FA_READ);
        if(f_res != FR_OK)
        {
            LOG_E("can not open file: %s", file_name);
            return -1;
        }
    }

    //1.creat socket(ipv4 tcp)
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd<0)
    {
        LOG_E("Socket create err");
        f_close(&SDFile);
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
        f_close(&SDFile);
        close(fd);
        return -1;
    }
    struct timeval timeout = {60, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    LOG_I("Server connection success %d, %d", ret, fd);
    buf = (char *)malloc(SOCKET_MAX_LENGTH);
    memset(buf,0,SOCKET_MAX_LENGTH);
    if (FR_OK == f_res)
    {
        int toatl_size = 0;
        FSIZE_t file_size = f_size(&SDFile);
        LOG_I("File size: %lu bytes\n", (unsigned long)file_size);
        while (!f_eof(&SDFile))
        {
            f_res = f_read(&SDFile, buf, SOCKET_MAX_LENGTH, &bytes_read);
            if (f_res != FR_OK || bytes_read == 0)
            {
                LOG_I("file send end %d %u", f_res, bytes_read);
                break;
            }
            ret = send(fd, buf, bytes_read, 0);
            if (ret > 0)
            {
                LOG_I("Tcp client send ok len = %d, fd = %d", ret, fd);
            }
            else
            {
                LOG_E("Tcp client send err %d, %d", ret, fd);
                break;
            }
            toatl_size += ret;
            memset(buf, 0, SOCKET_MAX_LENGTH);
            ret = recv(fd, buf, SOCKET_MAX_LENGTH, MSG_DONTWAIT);
            if (ret > 0) 
            {
                LOG_I("Tcp client recv len: %d, fd :%d", ret, fd);
            }
            else if (ret == 0)
            {
                LOG_I("peer close");
                break;
            }
            LOG_I("Tcp client send total len: %d, fd :%d", toatl_size, fd);
        }
        f_close(&SDFile);
        // qosa_task_sleep_ms(loop_interval);
    }
    else
    {
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
    }
    free(buf);
    close(fd);

    LOG_D("%s over",__FUNCTION__);

    return 0;
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_CLIENT__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
