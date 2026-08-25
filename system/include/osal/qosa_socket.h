/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-10-12     armink       first version
 */

/*
 * NOTE: DO NOT include this file on the header file.
 */
#ifndef __QOSA_SOCKET_H__
#define __QOSA_SOCKET_H__

#include "stdio.h"
#include <stdarg.h>
#include "string.h"
#include <assert.h>
#include "qosa_def.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#elif __linux__
#else
#include "cmsis_os2.h"
#endif  

/*********************************************User system type definitions****************************************/
#define LWIP_IPV6_SCOPES 1
#define NETDEV_IPV4 1
#define LWIP_ASSERT(message, assertion)

/********************************************Basic data type definitions end****************************************/

/********************************************Socket type definitions ***********************************************/
#define AF_AT                           45  /* AT socket */
#define AT_DEVICE_NAMETYPE_NETDEV       0x02
#define AF_INET                         2

/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM                     1
#define SOCK_DGRAM                      2
#define SOCK_RAW                        3

/* Flags we can use with send and recv. */
#define MSG_PEEK       0x01    /* Peeks at an incoming message */
#define MSG_WAITALL    0x02    /* Unimplemented: Requests that the function block until the full amount of data requested can be returned */
#define MSG_OOB        0x04    /* Unimplemented: Requests out-of-band data. The significance and semantics of out-of-band data are protocol-specific */
#define MSG_DONTWAIT   0x08    /* Nonblocking i/o for this operation only */
#define MSG_MORE       0x10    /* Sender will send more */
/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define  SOL_SOCKET  0xfff    /* options for socket level */

#define AF_UNSPEC       0
#define AF_INET         2
#define AF_INET6        10
#define PF_INET         AF_INET
#define PF_UNSPEC       AF_UNSPEC

#define IPPROTO_IP      0
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#define IPPROTO_UDPLITE 136
/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF    0x1001    /* Unimplemented: send buffer size */
#define SO_RCVBUF    0x1002    /* receive buffer size */
#define SO_SNDLOWAT  0x1003    /* Unimplemented: send low-water mark */
#define SO_RCVLOWAT  0x1004    /* Unimplemented: receive low-water mark */
#define SO_SNDTIMEO  0x1005    /* Unimplemented: send timeout */
#define SO_RCVTIMEO  0x1006    /* receive timeout */
#define SO_ERROR     0x1007    /* get error status and clear */
#define SO_TYPE      0x1008    /* get socket type */
#define SO_CONTIMEO  0x1009    /* Unimplemented: connect timeout */
#define SO_NO_CHECK  0x100a    /* don't create UDP checksum */

/*
 * Options for level IPPROTO_TCP
 */
#define TCP_NODELAY    0x01    /* don't delay send to coalesce packets */
#define TCP_KEEPALIVE  0x02    /* send KEEPALIVE probes when idle for pcb->keep_idle milliseconds */
#define TCP_KEEPIDLE   0x03    /* set pcb->keep_idle  - Same as TCP_KEEPALIVE, but use seconds for get/setsockopt */
#define TCP_KEEPINTVL  0x04    /* set pcb->keep_intvl - Use seconds for get/setsockopt */
#define TCP_KEEPCNT    0x05    /* set pcb->keep_cnt   - Use number of probes sent for get/setsockopt */

/** DNS maximum host name length supported in the name table. */
#define DNS_MAX_NAME_LENGTH             256

#define EAI_NONAME      200
#define EAI_SERVICE     201
#define EAI_FAIL        202
#define EAI_MEMORY      203
#define EAI_FAMILY      204

#define HOST_NOT_FOUND  210
#define NO_DATA         211
#define NO_RECOVERY     212
#define TRY_AGAIN       213

/* input flags for structure addrinfo */
#define AI_PASSIVE      0x01
#define AI_CANONNAME    0x02
#define AI_NUMERICHOST  0x04
#define AI_NUMERICSERV  0x08
#define AI_V4MAPPED     0x10
#define AI_ALL          0x20
#define AI_ADDRCONFIG   0x40

/** Copy IP address - faster than ip_addr_set: no NULL check */
#define ip_addr_copy(dest, src) ((dest).addr = (src).addr)
#define ip_addr_cmp(addr1, addr2) ((addr1)->addr == (addr2)->addr)
#define rt_inline      static __inline

#define lwip_in_range(c, lo, up)  ((u8_t)(c) >= (lo) && (u8_t)(c) <= (up))
#define lwip_isdigit(c)           lwip_in_range((c), '0', '9')
#define lwip_isxdigit(c)          (lwip_isdigit(c) || lwip_in_range((c), 'a', 'f') || lwip_in_range((c), 'A', 'F'))
#define lwip_islower(c)           lwip_in_range((c), 'a', 'z')
#define lwip_isspace(c)           ((c) == ' ' || (c) == '\f' || (c) == '\n' || (c) == '\r' || (c) == '\t' || (c) == '\v')
#define lwip_isupper(c)           lwip_in_range((c), 'A', 'Z')
#define lwip_tolower(c)           (lwip_isupper(c) ? (c) - 'A' + 'a' : c)
#define lwip_toupper(c)           (lwip_islower(c) ? (c) - 'a' + 'A' : c)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
// #include "inaddr.h"
// #include "winsock.h"
#elif __linux__
#else
#endif

typedef struct ip4_addr
{
    u32_t addr;
} ip4_addr_t;

typedef uint32_t in_addr_t;
struct in_addr
{
   in_addr_t s_addr;
};

typedef ip4_addr_t ip_addr_t;

char* ipaddr_ntoa(const ip_addr_t *addr);
int osal_ip4addr_aton(const char *cp, ip4_addr_t *addr);
#define inet_aton(cp, addr)   osal_ip4addr_aton(cp,(ip4_addr_t*)addr)
#define inet_ntoa(addr)       ipaddr_ntoa((ip_addr_t*)&(addr))
/** IPv4 only: set the IP address given as an u32_t */
#define ip4_addr_set_u32(dest_ipaddr, src_u32) ((dest_ipaddr)->addr = (src_u32))

typedef unsigned int socklen_t;

struct sockaddr {
 u8_t sa_len;
 u8_t sa_family;
 char sa_data[14];
};

/* members are in network byte order */
struct sockaddr_in {
 u8_t sin_len;
 u8_t sin_family;
 u16_t sin_port;
 struct in_addr sin_addr;
 char sin_zero[8];
};
typedef uint8_t sa_family_t;
struct sockaddr_storage
{
    uint8_t        s2_len;
    sa_family_t    ss_family;
    char           s2_data1[2];
    u32_t       s2_data2[3];
#if NETDEV_IPV6
    u32_t       s2_data3[3];
#endif /* NETDEV_IPV6 */
};

struct hostent {
   char  *h_name;      /* Official name of the host. */
   char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                          terminated by a null pointer. */
   int    h_addrtype;  /* Address type. */
   int    h_length;    /* The length, in bytes, of the address. */
   char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                          network byte order) for the host, terminated by a null pointer. */
#define h_addr h_addr_list[0] /* for backward compatibility */
};

struct addrinfo {
    int               ai_flags;      /* Input flags. */
    int               ai_family;     /* Address family of socket. */
    int               ai_socktype;   /* Socket type. */
    int               ai_protocol;   /* Protocol of socket. */
    unsigned int      ai_addrlen;    /* Length of socket address. */
    struct sockaddr  *ai_addr;       /* Socket address of socket. */
    char             *ai_canonname;  /* Canonical name of service location. */
    struct addrinfo  *ai_next;       /* Pointer to next in list. */
};

/* These macros should be calculated by the preprocessor and are used
   with compile-time constants only (so that there is no little-endian
   overhead at runtime). */
#define PP_HTONS(x) ((((x) & 0x00ffUL) << 8) | (((x) & 0xff00UL) >> 8))
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) ((((x) & 0x000000ffUL) << 24) | \
                     (((x) & 0x0000ff00UL) <<  8) | \
                     (((x) & 0x00ff0000UL) >>  8) | \
                     (((x) & 0xff000000UL) >> 24))
#define PP_NTOHL(x) PP_HTONL(x)

#define htons(x) (uint16_t)PP_HTONS(x)
#define ntohs(x) (uint16_t)PP_NTOHS(x)
#define htonl(x) (u32_t)PP_HTONL(x)
#define ntohl(x) (u32_t)PP_NTOHL(x)

u32_t ql_ipaddr_addr(const char *cp);

#define inet_addr(cp)         ql_ipaddr_addr(cp)
/** IPv4 only: get the IP address as an u32_t */
#define ip4_addr_get_u32(src_ipaddr) ((src_ipaddr)->addr)
/** 255.255.255.255 */
#define IPADDR_NONE         ((u32_t)0xffffffffUL)


/********************************************Socket type definitions end********************************************/

/********************************************Slist type definitions ************************************************/
#define RT_SLIST_OBJECT_INIT(object) { QOSA_NULL }
/**
 * rt_slist_for_each - iterate over a single list
 * @pos:    the rt_slist_t * to use as a loop cursor.
 * @head:   the head for your single list.
 */
#define rt_slist_for_each(pos, head) \
    for (pos = (head)->next; pos != QOSA_NULL; pos = pos->next)

/**
 * rt_container_of - return the member address of ptr, if the type of ptr is the
 * struct type.
 */
#define rt_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
 * @brief get the struct for this single list node
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define rt_slist_entry(node, type, member) \
    rt_container_of(node, type, member)



/**
 * Single List structure
 */
struct rt_slist_node
{
    struct rt_slist_node *next;                         /**< point to next node. */
};
typedef struct rt_slist_node rt_slist_t;                /**< Type for single list. */

/**
 * @brief initialize a single list
 *
 * @param l the single list to be initialized
 */
rt_inline void rt_slist_init(rt_slist_t *l)
{
    l->next = QOSA_NULL;
}

rt_inline void rt_slist_append(rt_slist_t *l, rt_slist_t *n)
{
    struct rt_slist_node *node;

    node = l;
    while (node->next) node = node->next;

    /* append the node to the tail */
    node->next = n;
    n->next = QOSA_NULL;
}

rt_inline int rt_slist_isempty(rt_slist_t *l)
{
    return l->next == QOSA_NULL;
}

rt_inline rt_slist_t *rt_slist_remove(rt_slist_t *l, rt_slist_t *n)
{
    /* remove slist head */
    struct rt_slist_node *node = l;
    while (node->next && node->next != n) node = node->next;

    /* remove node */
    if (node->next != (rt_slist_t *)0) node->next = node->next->next;

    return l;
}

rt_inline rt_slist_t *rt_slist_next(rt_slist_t *n)
{
    return n->next;
}

rt_inline rt_slist_t *rt_slist_first(rt_slist_t *l)
{
    return l->next;
}

rt_inline void rt_slist_insert(rt_slist_t *l, rt_slist_t *n)
{
    n->next = l->next;
    l->next = n;
}
/********************************************Slist type definitions end*********************************************/
#endif /* __QOSA_SOCKET_H__ */
