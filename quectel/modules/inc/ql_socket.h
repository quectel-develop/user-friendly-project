#include "QuectelConfig.h"

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__
#ifndef __QL_SOCKET_H__
#define __QL_SOCKET_H__ 

#ifdef __cplusplus
extern "C" {
#endif
#include "time.h"
#include "at.h"
#include "qosa_socket.h"
#include "ql_common_def.h"

typedef enum
{
    QL_SOCKET_OPEN,
    QL_SOCKET_LISTEN,
    QL_SOCKET_CONNECTED,
    QL_SOCKET_CLOSING,
    QL_SOCKET_CLOSED
} QlSocketState;

typedef uint16_t ql_fd_set;

#define QL_FD_ZERO(set)        ((*(set)) = 0)
#define QL_FD_SET(sock, set)   ((*(set)) |= (1 << (sock)))
#define QL_FD_CLR(sock, set)   ((*(set)) &= ~(1 << (sock)))
#define QL_FD_ISSET(sock, set) ((*(set)) & (1 << (sock)))

typedef struct ql_socket_pkt
{
    u16_t port;
    u32_t sin_addr;
    size_t total;
    size_t offset;
    char *data;
    struct ql_socket_pkt* next;
} ql_socket_pkt_s;
typedef ql_socket_pkt_s* ql_socket_pkt_t;

typedef struct ql_socket
{
    at_client_t client;
    int fd;
    int type;
    QlSocketState state;
    QL_SOCKET_ERR_CODE_E err;
    ql_socket_pkt_t pkt_head;
    uint32_t timeout;
    osa_sem_t sem;
    osa_mutex_t lock;
} ql_socket_s;

typedef ql_socket_s* ql_socket_t;

int ql_socket(int domain, int type, int protocol);

int ql_close(int sockfd);

int ql_shutdown(int sockfd, int how);

int ql_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int ql_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int ql_listen(int sockfd, int backlog);

int ql_accept(int sockfd, struct sockaddr *name, socklen_t *namelen);

int ql_select(int nfds, ql_fd_set *readfds, ql_fd_set *writefds, ql_fd_set *exceptfds, struct timeval *timeout);

/*when sending in udp client, the remote ip or port can not change, if you want to send mutil ip or port, please create mutil socket*/
int ql_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);

int ql_send(int sockfd, const void *buf, size_t len, int flags);

int ql_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);

int ql_recv(int sockfd, void *buf, size_t len, int flags);

int ql_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

int ql_getsockopt(int socket, int level, int optname, void *optval, socklen_t *optlen);

int ql_getaddrinfo(at_client_t client, const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
void ql_free_addrinfo(struct addrinfo *res);
#define socket(domain, type, protocol)                      ql_socket(domain, type, protocol)
#define close(socket)                                       ql_close(socket)
#define shutdown(socket, how)                               ql_shutdown(socket, how)
#define bind(socket, name, namelen)                         ql_bind(socket, name, namelen)
#define connect(socket, name, namelen)                      ql_connect(socket, name, namelen)
#define sendto(socket, data, size, flags, to, tolen)        ql_sendto(socket, data, size, flags, to, tolen)
#define send(socket, data, size, flags)                     ql_send(socket, data, size, flags)
#define recvfrom(socket, buf, len, flags, from, fromlen)    ql_recvfrom(socket, buf, len, flags, from, fromlen)
#define recv(socket, buf, len, flags)                       ql_recv(socket, buf, len, flags)
#define getsockopt(socket, level, optname, optval, optlen)  ql_getsockopt(socket, level, optname, optval, optlen)
#define setsockopt(socket, level, optname, optval, optlen)  ql_setsockopt(socket, level, optname, optval, optlen)
#define listen(socket, backlog)                             ql_listen(socket, backlog)
#define accept(socket, name, namelen)                       ql_accept(socket, name, namelen)
#define select(nfds, readfds, writefds, exceptfds, timeout) ql_select(nfds, readfds, writefds, exceptfds, timeout)
#define getaddrinfo(client, node, service, hints, res)      ql_getaddrinfo(client, node, service, hints, res)    
#define free_addrinfo(res)                                  ql_free_addrinfo(res)
#ifdef __cplusplus
}
#endif
#endif // __QL_SOCKET_H__
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__ */