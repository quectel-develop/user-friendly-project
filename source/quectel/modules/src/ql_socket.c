#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__
#include "qosa_log.h"
#include "qosa_utils.h"
#include "ql_socket.h"

#define SOCKET_MAX_FD 12
#define ONE_PKT_MAX_LENGTH 1024
static bool s_global_socket_init = false;
static osa_mutex_t s_fd_lock = NULL; // never delete
static osa_sem_t s_fd_sem = NULL;
typedef struct ql_socket_instance
{
    ql_socket_t handle;
    bool used;
} ql_socket_instance_s;

static ql_socket_instance_s s_socket_indices[SOCKET_MAX_FD] = {0};

/*
* @brief find vaild fd, then bind fd to handle
* if fd vaild,used fd, otherwise auto find a free fd
*/
static int ql_socket_take_vaild_fd(ql_socket_t handle, int fd)
{
    qosa_mutex_lock(s_fd_lock, QOSA_WAIT_FOREVER);
    if (fd >= 0 && fd < SOCKET_MAX_FD)
    {
        if (s_socket_indices[fd].used) // means user created this socket, but not used yet
        {
            LOG_W("fd already used.");
            ql_close(fd);
        }
        s_socket_indices[fd].used = true;
        s_socket_indices[fd].handle = handle;
        qosa_mutex_unlock(s_fd_lock);
        return fd;
    }
    for (int i = 0; i < SOCKET_MAX_FD; i++)
    {
        if (!s_socket_indices[i].used)
        {
            s_socket_indices[i].used = true; // Mark as used
            s_socket_indices[i].handle = handle;
            qosa_mutex_unlock(s_fd_lock);
            return i; // Return first available index
        }
    }
    qosa_mutex_unlock(s_fd_lock);
    return -1;
}

static void ql_socket_release_fd(int fd)
{
    qosa_mutex_lock(s_fd_lock, QOSA_WAIT_FOREVER);
    if (fd >= 0 && fd < SOCKET_MAX_FD)
    {
        s_socket_indices[fd].used = false;
    }
    qosa_mutex_unlock(s_fd_lock);
}
/**
 * @brief find handle by index
 * @param fd index
 * @return success handle，otherwise NULL
 */
static ql_socket_t ql_socket_find_handle_by_fd(int fd)
{
    qosa_mutex_lock(s_fd_lock, QOSA_WAIT_FOREVER);
    if (fd >= 0 && fd < SOCKET_MAX_FD &&
        s_socket_indices[fd].used)
    {
        qosa_mutex_unlock(s_fd_lock);
        return s_socket_indices[fd].handle;
    }
    qosa_mutex_unlock(s_fd_lock);
    LOG_E("can not find handle by fd %d", fd);
    return NULL;
}

static void ql_socket_pkt_put(ql_socket_t handle, ql_socket_pkt_t pkt)
{
    if (NULL == handle->pkt_head)
    {
        handle->pkt_head = pkt;
        return;
    }
    ql_socket_pkt_t last = handle->pkt_head;
    while (last->next != NULL)
    {
        last = last->next;
    }
    last->next = pkt;
}

static ql_socket_pkt_t ql_socket_pkt_get(ql_socket_t handle)
{
    return handle->pkt_head;
}

static void ql_socket_pkt_delete_head(ql_socket_t handle)
{
    ql_socket_pkt_t tmp = handle->pkt_head;
    if (tmp != NULL)
        handle->pkt_head = tmp->next;

    if (tmp->data != NULL)
        free(tmp->data);
    if (tmp != NULL)
        free(tmp);
}

static void ql_socket_pkt_delete_all(ql_socket_t handle)
{
    while (handle->pkt_head != NULL)
    {
        ql_socket_pkt_delete_head(handle);
    }
}

static ql_socket_t ql_socket_alloc(int fd)
{
    ql_socket_t handle = (ql_socket_t)malloc(sizeof(ql_socket_s));
    if (NULL == handle)
    {
        LOG_E("no memory for socket handle.");
        return NULL;
    }
    handle->client = at_client_get_first();
    handle->fd = ql_socket_take_vaild_fd(handle, fd);
    if (handle->fd < 0 || handle->fd > SOCKET_MAX_FD)
    {
        LOG_E("no available socket fd.");
        free(handle);
        return NULL;
    }
    handle->err = QL_SOCKET_OK;
    handle->pkt_head = NULL;
    handle->timeout = QOSA_WAIT_FOREVER;
    qosa_sem_create(&handle->sem, 0);
    qosa_mutex_create(&handle->lock);
    return handle;
}

static void ql_socket_urc_close(struct at_client *client, const char *data, s32_t size, void *arg)
{
    int fd = -1;
    QOSA_ASSERT(data && size);

    sscanf(data, "+QIURC: \"closed\",%d", &fd);
    ql_socket_t handle = ql_socket_find_handle_by_fd(fd);
    if (NULL == handle)
        return;
    handle->state = QL_SOCKET_CLOSING;
    qosa_sem_release(handle->sem);
    qosa_sem_release(s_fd_sem);
}

static void ql_socket_urc_recv(struct at_client *client, const char *data, s32_t size, void *arg)
{
    int fd = -1;
    int dot_count = 0;
    char recv_ip[16] = {0};
    int total_len = 0;
    ql_socket_pkt_t pkt = (ql_socket_pkt_t)malloc(sizeof(ql_socket_pkt_s));
    pkt->offset = 0;
    pkt->next = NULL;
    for (int i = 0; i < size; i++)
    {
        if (*(data + i) == '.')
            dot_count++;
    }
    if (dot_count == 3)
    {
        if (sscanf(data, "+QIURC: \"recv\",%d,%d,\"%15[^\"]\",%hu", &fd, (int *) &total_len, recv_ip, &pkt->port) != 4)
        {
            LOG_E("parse recv data failed");
            free(pkt);
            return;
        }
        inet_aton(recv_ip, &pkt->sin_addr);
    }
    else
    {
        sscanf(data, "+QIURC: \"recv\",%d,%d", &fd, &total_len);
        pkt->port = 0;
        pkt->sin_addr = 0;
    }
    ql_socket_t handle = ql_socket_find_handle_by_fd(fd);
    if (NULL == handle)
    {
        free(pkt);
        return;
    }
    while (total_len > 0)
    {
        size_t recv_len = (total_len > ONE_PKT_MAX_LENGTH ? ONE_PKT_MAX_LENGTH : total_len);
        pkt->data = (char *)malloc(recv_len);
        size_t ret = at_client_obj_recv(handle->client, pkt->data, recv_len, 2000, false);
        total_len -= ret;
        pkt->total = ret;
        qosa_mutex_lock(handle->lock, QOSA_WAIT_FOREVER);
        ql_socket_pkt_put(handle, pkt);
        qosa_mutex_unlock(handle->lock);
        qosa_sem_release(handle->sem);
        qosa_sem_release(s_fd_sem);
    }
}

static void ql_socket_urc_incoming(struct at_client *client, const char *data, s32_t size, void *arg)
{
    int clientfd = -1;
    int serverfd = -1;
    int dot_count = 0;
    char recv_ip[16] = {0};

    for (int i = 0; i < size; i++)
    {
        if (*(data + i) == '.')
            dot_count++;
    }
    ql_socket_pkt_t pkt = (ql_socket_pkt_t)malloc(sizeof(ql_socket_pkt_s));
    pkt->offset = 0;
    pkt->next = NULL;
    if (dot_count == 3)
    {
        if (sscanf(data, "+QIURC: \"incoming\",%d,%d,\"%15[^\"]\",%hu", &clientfd, &serverfd, recv_ip, &pkt->port) != 4)
        {
            LOG_E("parse recv data failed");
            free(pkt);
            return;
        }
        ql_socket_t handle = ql_socket_find_handle_by_fd(serverfd);
        if (NULL == handle)
        {
            free(pkt);
            return;
        }
        inet_aton(recv_ip, &pkt->sin_addr);
        pkt->total = sizeof(int);
        pkt->data = (char*)malloc(sizeof(int));
        *(int*)pkt->data = clientfd;
        LOG_I("client_fd = %d, server_fd = %d", clientfd, serverfd);
        ql_socket_t new_handle = ql_socket_alloc(clientfd);
        if (NULL == new_handle)
        {
            LOG_E("create new peer socket failed.");
            close(clientfd);
            return;
        }
        new_handle->state = QL_SOCKET_CONNECTED;
        new_handle->type = handle->type;
        qosa_mutex_lock(handle->lock, QOSA_WAIT_FOREVER);
        ql_socket_pkt_put(handle, pkt);
        qosa_mutex_unlock(handle->lock);
        qosa_sem_release(handle->sem);
        qosa_sem_release(s_fd_sem);
    }
}

static void ql_socket_urc_default(struct at_client *client, const char *data, size_t size, void *arg)
{
    LOG_I("URC data : %s", data);
}

static void ql_socket_urc_connect(struct at_client *client, const char *data, size_t size, void *arg)
{
    int fd = -1;
    int err = 0;
	sscanf(data, "+QIOPEN: %d,%d", &fd, &err);
    ql_socket_t handle = ql_socket_find_handle_by_fd(fd);
    if (NULL == handle)
        return;
    handle->err = (QL_SOCKET_ERR_CODE_E)err;
    qosa_sem_release(handle->sem);
}

static void ql_socket_urc_qiurc(struct at_client *client, const char *data, size_t size, void *arg)
{
    QOSA_ASSERT(data && size);

    switch(*(data + 9))
    {
        case 'c' : ql_socket_urc_close(client, data, size, arg);    break;//+QIURC: "closed"
        case 'r' : ql_socket_urc_recv(client, data, size, arg);     break;//+QIURC: "recv"
        case 'i' : ql_socket_urc_incoming(client, data, size, arg); break;//+QIURC: "incoming"
        default  : ql_socket_urc_default(client, data, size, arg);  break;
    }
}

static const struct at_urc s_socket_urc_table[] =
{
    {"+QIOPEN:",    "\r\n",                 ql_socket_urc_connect},
    {"+QIURC:",     "\r\n",                 ql_socket_urc_qiurc}
};

static int ql_socket_connect(ql_socket_t handle, const struct sockaddr *addr, socklen_t addrlen, bool is_client)
{
    struct sockaddr_in *sock_addr = (struct sockaddr_in *)addr;
    char *ip = inet_ntoa(sock_addr->sin_addr);
    uint16_t port = ntohs(sock_addr->sin_port);
    at_response_t resp = NULL;
    int fd = handle->fd;

    LOG_V("ip = %s, port = %d, is_client = %d", ip, port, is_client);

    QOSA_ASSERT(port >= 0);
    const char *protocol = NULL;
    const char *remote_ip = is_client ? ip : "127.0.0.1";
    uint16_t remote_port = is_client ? port : 0;
    uint16_t local_port = is_client ? 0 : port;
    switch (handle->type)
    {
    case SOCK_STREAM:
        protocol = is_client ? "TCP" : "TCP LISTENER";
        break;
    case SOCK_DGRAM:
        protocol = is_client ? "UDP" : "UDP SERVICE";
        break;
    default:
        LOG_E("Not supported socket type: %d", handle->type);
        return -1;
    }
    resp = at_create_resp_new(128, 0, 150 * RT_TICK_PER_SECOND, NULL);
    if (at_obj_exec_cmd(handle->client, resp, "AT+QIOPEN=1,%d,\"%s\",\"%s\",%d,%d,1", fd, protocol, remote_ip, remote_port, local_port) < 0)
    {
        at_delete_resp(resp);
        return -1;
    }
    if (qosa_sem_wait(handle->sem, 151 * RT_TICK_PER_SECOND) == 0 && QL_SOCKET_OK == handle->err)
    {
        at_delete_resp(resp);
        return 0;
    }
    at_delete_resp(resp);
    return -1;
}

static int ql_socket_send(ql_socket_t handle, const char *buf, size_t len, const struct sockaddr *to, socklen_t tolen)
{
    LOG_V("buff = %s, len = %d", buf, len);
    char cmd[128] = {0};
    if ((handle->type == SOCK_DGRAM) && handle->state == QL_SOCKET_LISTEN)
    {
        struct sockaddr_in *sock_addr = (struct sockaddr_in *)to;
        char *ip = inet_ntoa(sock_addr->sin_addr);
        uint16_t port = ntohs(sock_addr->sin_port);
        snprintf(cmd, sizeof(cmd), "AT+QISEND=%d,%%d,%s,%d", handle->fd, ip, port);
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "AT+QISEND=%d,%%d", handle->fd);
    }
    return at_obj_exec_cmd_with_data(handle->client, cmd, buf, len);
}

int ql_socket(int domain, int type, int protocol)
{
    if (!s_global_socket_init)
    {
        qosa_mutex_create(&s_fd_lock);
        qosa_sem_create(&s_fd_sem, 0);
        at_set_urc_table(s_socket_urc_table, sizeof(s_socket_urc_table) / sizeof(s_socket_urc_table[0]));
        s_global_socket_init = true;
    }
    ql_socket_t handle = ql_socket_alloc(-1);
    if (NULL == handle)
        return -1;
    QOSA_ASSERT(domain == AF_AT || domain == AF_INET); // not used actually

    if (type != SOCK_STREAM && type != SOCK_DGRAM)
    {
        LOG_E("socket only support SOCK_STREAM and SOCK_DGRAM.");
        return -1;
    }
    handle->type = type;
    handle->state = QL_SOCKET_OPEN;
    return handle->fd;
}

int ql_close(int sockfd)
{
    ql_socket_t handle = ql_socket_find_handle_by_fd(sockfd);
    if (NULL == handle)
    {
        return -1;
    }
    if (handle->err != QL_SOCKET_ERR_FD_USED) // socket is being used elsewhere and cannot be closed 
    {
        at_response_t resp = at_create_resp_new(128, 0, 10 * RT_TICK_PER_SECOND, NULL);
        at_obj_exec_cmd(handle->client, resp, "AT+QICLOSE=%d,1", sockfd);
        at_delete_resp(resp);
    } 
    ql_socket_release_fd(handle->fd);
    ql_socket_pkt_delete_all(handle);
    qosa_sem_delete(handle->sem);
    qosa_mutex_delete(handle->lock);
    free(handle);
    return 0;
}

int ql_shutdown(int sockfd, int how)
{
    return ql_close(sockfd);
}

int ql_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    ql_socket_t handle = ql_socket_find_handle_by_fd(sockfd);
    if (NULL == handle || NULL == addr)
    {
        return -1;
    }
    if (ql_socket_connect(handle, addr, addrlen, true) != 0)
    {
        return -1;
    }
    handle->state = QL_SOCKET_CONNECTED;
    return 0;
}

int ql_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    ql_socket_t handle = ql_socket_find_handle_by_fd(sockfd);
    if (NULL == handle)
    {
        return -1;
    }
    if (ql_socket_connect(handle, addr, addrlen, false) != 0)
    {
        return -1;
    }
    handle->state = QL_SOCKET_LISTEN;
    return 0;
}

int ql_listen(int sockfd, int backlog)
{
    ql_socket_t handle = ql_socket_find_handle_by_fd(sockfd);
    if (NULL == handle)
    {
        return -1;
    }
    return 0;
}

int ql_accept(int sockfd, struct sockaddr *name, socklen_t *namelen)
{
    ql_socket_t new_handle = NULL;
    ql_socket_t handle = ql_socket_find_handle_by_fd(sockfd);
    if (NULL == handle)
    {
        return -1;
    }
    if (handle->state != QL_SOCKET_LISTEN)
    {
        LOG_E("Please listen socket first");
        return -1;
    }
    while (true)
    {
        qosa_mutex_lock(handle->lock, QOSA_WAIT_FOREVER);
        ql_socket_pkt_t pkt = ql_socket_pkt_get(handle);
        qosa_mutex_unlock(handle->lock);
        if (NULL == pkt)
        {
            qosa_sem_wait(handle->sem, QOSA_WAIT_FOREVER);
            continue;
        }
        int peer_sock = *(int*)pkt->data;
        new_handle = ql_socket_find_handle_by_fd(peer_sock);
        if (NULL == new_handle)
        {
            break;
        }
        if (NULL == name || NULL == namelen)
            break;
        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_port = pkt->port;
        sin.sin_addr.s_addr = pkt->sin_addr;
        *namelen = sizeof(sin);
        memcpy(name, &sin, *namelen);
        break;
    }
    ql_socket_pkt_delete_head(handle);
    return (NULL == new_handle ? -1 : new_handle->fd);
}

int ql_select(int nfds, ql_fd_set *readfds, ql_fd_set *writefds, ql_fd_set *exceptfds, struct timeval *timeout)
{
    if (NULL == readfds || nfds <= 0 || nfds > SOCKET_MAX_FD)
    {
        LOG_E("readfds or nfds is invalid");
        return -1;
    }
    static bool has_called = false;
    if (has_called)
    {
        LOG_E("select can only be called once");
        return -1;
    }
    has_called = true;
    int count = 0;
    ql_fd_set origin_fds = *readfds;
    uint64_t start_time = qosa_get_uptime_milliseconds();
    uint64_t wait_time = QOSA_WAIT_FOREVER;
    QL_FD_ZERO(readfds);
    while (true)
    {
        for (int fd = 0; fd <nfds; fd++)
        {
            if (QL_FD_ISSET(fd, &origin_fds))
            {
                ql_socket_t handle = ql_socket_find_handle_by_fd(fd);
                if (handle != NULL && (handle->pkt_head != NULL || handle->state == QL_SOCKET_CLOSING))
                {
                    QL_FD_SET(fd, readfds);
                    count++;
                    continue;
                }
            }
        }
        if (count > 0)
            break;
        if (timeout != NULL)
        {
            uint64_t elapsed_time = qosa_get_uptime_milliseconds() - start_time;
            uint64_t total_timeout = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
            if (elapsed_time >= total_timeout)
                break;
            wait_time = total_timeout - elapsed_time;
        }
        if (qosa_sem_wait(s_fd_sem, wait_time) != QOSA_OK)
            break;
    }
    has_called = false;
    return count;
}

int ql_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
{
    ql_socket_t handle = ql_socket_find_handle_by_fd(sockfd);
    if (NULL == handle)
    {
        return -1;
    }
    if (NULL == buf || len <= 0)
    {
        LOG_E("buf or len is invalid.");
        return -1;
    }

    switch (handle->type)
    {
    case SOCK_STREAM:
        if (handle->state != QL_SOCKET_CONNECTED )
        {
            LOG_E("socket is not connected.");
            return -1;
        }
        break;

    case SOCK_DGRAM:
        if (NULL == to)
            return -1;
        if (handle->state == QL_SOCKET_OPEN)
        {
            if (ql_socket_connect(handle, to, tolen, true) != 0)
            {
                LOG_E("udp connect to server failed.");
                return -1;
            }
            handle->state = QL_SOCKET_CONNECTED;
        }
        break;
    default:
        break;
    }
    return ql_socket_send(handle, (const char*)buf, len, to, tolen);
}

int ql_send(int sockfd, const void *buf, size_t len, int flags)
{
    return ql_sendto(sockfd, buf, len, flags, NULL, 0);
}

int ql_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
    ql_socket_t handle = ql_socket_find_handle_by_fd(sockfd);
    if (NULL == handle)
    {
        return -1;
    }
    if (NULL == buf || len <= 0)
    {
        LOG_E("buf or len is invalid.");
        return -1;
    }
    while (true)
    {
        qosa_mutex_lock(handle->lock, QOSA_WAIT_FOREVER);
        ql_socket_pkt_t pkt = ql_socket_pkt_get(handle);
        qosa_mutex_unlock(handle->lock);
        if (NULL == pkt)
        {
            if (handle->state == QL_SOCKET_CLOSING) // peer closed
                break;
            if (flags & MSG_DONTWAIT)
                return -1;
            if(qosa_sem_wait(handle->sem, handle->timeout) != QOSA_OK)
                return -1;
            continue;
        }
        if (from != NULL && fromlen != NULL)
        {
            struct sockaddr_in sin;
            sin.sin_family = AF_INET;
            sin.sin_port = htons(pkt->port);
            sin.sin_addr.s_addr = pkt->sin_addr;
            *fromlen = sizeof(sin);
            memcpy(from, &sin, *fromlen);
        }
        size_t need_len = len;
        u32_t first_port = pkt->port;
        u32_t first_addr = pkt->sin_addr;
        while (pkt != NULL && need_len > 0)
        {
            size_t pkt_left = pkt->total - pkt->offset;
            if (pkt_left >= need_len)
            {
                memcpy(buf + len - need_len, pkt->data + pkt->offset, need_len);
                pkt->offset += need_len;
                need_len = 0;
                if (pkt->offset == pkt->total)
                    ql_socket_pkt_delete_head(handle);
                break;
            }
            else
            {
                memcpy(buf + len - need_len, pkt->data + pkt->offset, pkt_left);
                need_len -= pkt_left;
                ql_socket_pkt_delete_head(handle);
                pkt = ql_socket_pkt_get(handle);
                if (pkt->port != first_port || pkt->sin_addr != first_addr)
                    break;
            }
        }
        return len - need_len;
    }
    return 0;
}

int ql_recv(int sockfd, void *buf, size_t len, int flags)
{
    return ql_recvfrom(sockfd, buf, len, flags, NULL, NULL);
}

int ql_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    ql_socket_t handle = ql_socket_find_handle_by_fd(sockfd);
    if (NULL == handle)
    {
        return -1;
    }
    if (NULL == optval)
    {
        LOG_E("AT setsockopt input option value error!");
        return -1;
    }

    switch (level)
    {
    case SOL_SOCKET:
        switch (optname)
        {
        case SO_RCVTIMEO:
            handle->timeout = ((const struct timeval *) optval)->tv_sec * 1000
                    + ((const struct timeval *) optval)->tv_usec / 1000;
            break;

        case SO_SNDTIMEO:
            break;

        default:
            return -1;
        }
        break;
    case IPPROTO_TCP:
        switch (optname)
        {
        case TCP_NODELAY:
            break;
        }
        break;
    default:
        return -1;
    }

    return 0;
}

int ql_getsockopt(int socket, int level, int optname, void *optval, socklen_t *optlen)
{
    return 0;
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__ */