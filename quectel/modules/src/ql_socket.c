#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__
#include <at.h>
#include "at_socket.h"
#include "ql_socket.h"
#include "ql_net.h"
#include "debug_service.h"
#include "broadcast_service.h"
#include "at_socket_device.h"
#include "qosa_def.h"
#include "qosa_log.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#include "windows.h"
#elif __linux__
#else
#include "cmsis_os.h"
#endif


static osa_msgq_t g_socket_msg_id = NULL;
static osa_task_t g_socket_thread_id =NULL;


/* set real event by current socket and current state */
#define SET_EVENT(socket, event)       (((socket + 1) << 16) | (event))

/* AT socket event type */
#define QL_EVENT_CONN_OK             (1L << 0)
#define QL_EVENT_SEND_OK             (1L << 1)
#define QL_EVENT_RECV_OK             (1L << 2)
#define QL_EVNET_CLOSE_OK            (1L << 3)
#define QL_EVENT_CONN_FAIL           (1L << 4)
#define QL_EVENT_SEND_FAIL           (1L << 5)
#define QL_EVENT_DOMAIN_OK           (1L << 6)
#define QL_EVENT_INCOMING_OK         (1L << 7)

static at_evt_cb_t at_evt_cb_set[] = {
        [AT_SOCKET_EVT_RECV] = NULL,
        [AT_SOCKET_EVT_CLOSED] = NULL,
};

static void at_tcp_ip_errcode_parse(int result)//TCP/IP_QIGETERROR
{
    LOG_V("%s, %d",__FUNCTION__, result);

    switch(result)
    {
    case 0   : LOG_D("%d : Operation successful",         result); break;
    case 550 : LOG_E("%d : Unknown error",                result); break;
    case 551 : LOG_E("%d : Operation blocked",            result); break;
    case 552 : LOG_E("%d : Invalid parameters",           result); break;
    case 553 : LOG_E("%d : Memory not enough",            result); break;
    case 554 : LOG_E("%d : Create socket failed",         result); break;
    case 555 : LOG_E("%d : Operation not supported",      result); break;
    case 556 : LOG_E("%d : Socket bind failed",           result); break;
    case 557 : LOG_E("%d : Socket listen failed",         result); break;
    case 558 : LOG_E("%d : Socket write failed",          result); break;
    case 559 : LOG_E("%d : Socket read failed",           result); break;
    case 560 : LOG_E("%d : Socket accept failed",         result); break;
    case 561 : LOG_E("%d : Open PDP context failed",      result); break;
    case 562 : LOG_E("%d : Close PDP context failed",     result); break;
    case 563 : LOG_W("%d : Socket identity has been used", result); break;
    case 564 : LOG_E("%d : DNS busy",                     result); break;
    case 565 : LOG_E("%d : DNS parse failed",             result); break;
    case 566 : LOG_E("%d : Socket connect failed",        result); break;
    // case 567 : LOG_W("%d : Socket has been closed",       result); break;
    case 567 : break;
    case 568 : LOG_E("%d : Operation busy",               result); break;
    case 569 : LOG_E("%d : Operation timeout",            result); break;
    case 570 : LOG_E("%d : PDP context broken down",      result); break;
    case 571 : LOG_E("%d : Cancel send",                  result); break;
    case 572 : LOG_E("%d : Operation not allowed",        result); break;
    case 573 : LOG_E("%d : APN not configured",           result); break;
    case 574 : LOG_E("%d : Port busy",                    result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}

static int ql_socket_event_send(struct at_device *device, u32_t event)
{
    LOG_V("%s, 0x%x",__FUNCTION__, event);
    return (int) qosa_event_send(device->socket_event, event);
}

static int ql_socket_event_recv(struct at_device *device, u32_t event, u32_t timeout, u8_t option)
{
    u32_t recved;

    LOG_V("%s, event = 0x%x, timeout = %d, option = %d",__FUNCTION__, event, timeout, option);

    recved = qosa_event_recv(device->socket_event, event, option, timeout);
    if (recved < 0)
    {
        return -QOSA_ERROR_TIMEOUT;
    }

    return recved;
}

/**
 * close socket by AT commands.
 *
 * @param current socket
 *
 * @return  0: close socket success
 *         -1: send AT commands error
 *         -2: wait socket event timeout
 *         -5: no memory
 */
static int ql_socket_at_closesocket(struct at_socket *socket)
{
    int result = QOSA_OK;
    at_response_t resp = QOSA_NULL;
    int device_socket = (int) socket->user_data;
    struct at_device *device = (struct at_device *) socket->device;

    LOG_V("%s",__FUNCTION__);

    resp = at_create_resp(64, 0, 10 * RT_TICK_PER_SECOND);
    if (resp == QOSA_NULL)
    {
        LOG_E("no memory for resp create.");
        return QOSA_ERROR_NO_MEMORY;
    }

    /* default connection timeout is 10 seconds, but it set to 1 seconds is convenient to use.*/
    result = at_obj_exec_cmd(device->client, resp, "AT+QICLOSE=%d,1", device_socket);

    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;
}

/**
 * create TCP/UDP client or server connect by AT commands.
 *
 * @param socket current socket
 * @param ip server or client IP address
 * @param port server or client port
 * @param type connect socket type(tcp, udp)
 * @param is_client connection is client
 *
 * @return   0: connect success
 *          -1: connect failed, send commands error or type error
 *          -2: wait socket event timeout
 *          -5: no memory
 */
static int ql_socket_at_connect(struct at_socket *socket, char *ip, int32_t port, enum at_socket_type type, qosa_bool_t is_client)
{
    u32_t event = 0;
    qosa_bool_t retryed = QOSA_FALSE;
    at_response_t resp = QOSA_NULL;
    int result = 0, event_result = 0;
    int device_socket = (int) socket->user_data;
    struct at_device *device = (struct at_device *) socket->device;

    LOG_V("%s, ip = %s, port = %d, type = %d, is_client = %d", __FUNCTION__, ip, port, type, is_client);

    QOSA_ASSERT(port >= 0);
    resp = at_create_resp(128, 0, 150 * RT_TICK_PER_SECOND);
    if (resp == QOSA_NULL)
    {
        LOG_E("no memory for resp create.");
        return QOSA_ERROR_NO_MEMORY;
    }
__retry:
    /* clear socket connect event */
    event = SET_EVENT(device_socket, QL_EVENT_CONN_OK | QL_EVENT_CONN_FAIL);
    ql_socket_event_recv(device, event, 0, QOSA_EVENT_FLAG_OR);
    if (is_client)
    {
        switch (type)
        {
        case AT_SOCKET_TCP:
            /* send AT commands(AT+QIOPEN=<contextID>,<socket>,"<TCP/UDP>","<IP_address>/<domain_name>", */
            /* <remote_port>,<local_port>,<access_mode>) to connect TCP server */
            /* contextID   = 1 : use same contextID as AT+QICSGP & AT+QIACT */
            /* local_port  = 0 : local port assigned automatically */
            /* access_mode = 1 : Direct push mode */
            if (at_obj_exec_cmd(device->client, resp,
                                "AT+QIOPEN=1,%d,\"TCP\",\"%s\",%d,0,1", device_socket, ip, port) < 0)
            {
                result = -QOSA_ERROR_GENERAL;
                goto __exit;
            }
            break;

        case AT_SOCKET_UDP:
            if (at_obj_exec_cmd(device->client, resp,
                                "AT+QIOPEN=1,%d,\"UDP\",\"%s\",%d,0,1", device_socket, ip, port) < 0)
            {
                result = -QOSA_ERROR_GENERAL;
                goto __exit;
            }
            break;

        default:
            LOG_E("not supported connect type : %d.", type);
            return -QOSA_ERROR_GENERAL;
        }
    }
    else /* TCP/UDP Server*/
    {
        switch (type)
        {
        case AT_SOCKET_TCP:
            /* send AT commands(AT+QIOPEN=<contextID>,<socket>,"<TCP/UDP>","<IP_address>/<domain_name>", */
            /* <remote_port>,<local_port>,<access_mode>) to connect TCP server */
            /* contextID   = 1 : use same contextID as AT+QICSGP & AT+QIACT */
            /* local_port  = 0 : local port assigned automatically */
            /* access_mode = 1 : Direct push mode */
            if (at_obj_exec_cmd(device->client, resp,
                                "AT+QIOPEN=1,%d,\"TCP LISTENER\",\"127.0.0.1\",0,%d,1", device_socket, port) < 0)
            {
                result = -QOSA_ERROR_GENERAL;
                goto __exit;
            }
            break;

        case AT_SOCKET_UDP:
            if (at_obj_exec_cmd(device->client, resp,
                                "AT+QIOPEN=1,%d,\"UDP SERVICE\",\"127.0.0.1\",0,%d,1", device_socket, port) < 0)
            {
                result = -QOSA_ERROR_GENERAL;
                goto __exit;
            }
            break;

        default:
            LOG_E("not supported connect type : %d.", type);
            return -QOSA_ERROR_GENERAL;
        }

    }

    /* waiting result event from AT URC, the device default connection timeout is 75 seconds, but it set to 10 seconds is convenient to use.*/
    if (ql_socket_event_recv(device, SET_EVENT(device_socket, 0), 150 * RT_TICK_PER_SECOND, QOSA_EVENT_FLAG_OR|QOSA_EVENT_FLAG_NO_CLEAR) < 0)
    {
        LOG_E("device socket(%d) wait connect result timeout.", device_socket);
        result = -QOSA_ERROR_TIMEOUT;
        goto __exit;
    }
    /* waiting OK or failed result */
    event_result = ql_socket_event_recv(device, QL_EVENT_CONN_OK | QL_EVENT_CONN_FAIL, 1 * RT_TICK_PER_SECOND, QOSA_EVENT_FLAG_OR);
    if (event_result < 0)
    {
        LOG_E("device socket(%d) wait connect OK|FAIL timeout.", device_socket);
        result = -QOSA_ERROR_TIMEOUT;
        goto __exit;
    }
    /* check result */
    if (event_result & QL_EVENT_CONN_FAIL)
    {
        if (retryed == QOSA_FALSE)
        {
            LOG_D("device socket(%d) connect failed, the socket was not be closed and now will connect retey.",
                    device_socket);
            /* default connection timeout is 10 seconds, but it set to 1 seconds is convenient to use.*/
            if (ql_socket_at_closesocket(socket) < 0)
            {
                result = -QOSA_ERROR_GENERAL;
                goto __exit;
            }
            retryed = QOSA_TRUE;
            goto __retry;
        }
        LOG_E("device socket(%d) connect failed.", device_socket);
        result = -QOSA_ERROR_GENERAL;
        goto __exit;
    }
__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }
    LOG_V("%s over", __FUNCTION__);

    return result;
}

// static int ql_socket_at_socket(struct at_device *device, enum at_socket_type type)
// {

//     LOG_V("%s at_socket_type = %d",__FUNCTION__, type);
// }

static int at_get_send_size(struct at_socket *socket, size_t *size, size_t *acked, size_t *nacked)
{
    int result = 0;
    at_response_t resp = QOSA_NULL;
    int device_socket = (int) socket->user_data;
    struct at_device *device = (struct at_device *) socket->device;

    LOG_V("%s",__FUNCTION__);

    resp = at_create_resp(128, 0, 5 * RT_TICK_PER_SECOND);
    if (resp == QOSA_NULL)
    {
        LOG_E("no memory for resp create.");
        result = QOSA_ERROR_NO_MEMORY;
        goto __exit;
    }

    if (at_obj_exec_cmd(device->client, resp, "AT+QISEND=%d,0", device_socket) < 0)
    {
        result = -QOSA_ERROR_GENERAL;
        goto __exit;
    }

    if (at_resp_parse_line_args_by_kw(resp, "+QISEND:", "+QISEND: %d,%d,%d", size, acked, nacked) <= 0)
    {
        result = -QOSA_ERROR_GENERAL;
        goto __exit;
    }

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }
    LOG_V("%s over", __FUNCTION__);

    return result;
}

static int at_wait_send_finish(struct at_socket *socket, size_t settings_size)
{
    /* get the timeout by the input data size */
    u32_t timeout = settings_size;
    u32_t last_time = qosa_get_uptime_milliseconds();
    size_t size = 0, acked = 0, nacked = 0xFFFF;

    LOG_V("%s, settings_size = %d",__FUNCTION__, settings_size);

    while (qosa_get_uptime_milliseconds() - last_time <= timeout)
    {
        at_get_send_size(socket, &size, &acked, &nacked);
        if (nacked == 0)
        {
            return QOSA_OK;
        }
        qosa_task_sleep_ms(50);
    }

    return -QOSA_ERROR_TIMEOUT;
}

/**
 * send data to server or client by AT commands.
 *
 * @param socket current socket
 * @param buff send buffer
 * @param bfsz send buffer size
 * @param type connect socket type(tcp, udp)
 *
 * @return >=0: the size of send success
 *          -1: send AT commands error or send data error
 *          -2: waited socket event timeout
 *          -5: no memory
 */
static int ql_socket_at_send(struct at_socket *socket, const char *buff, size_t bfsz, char *ip, int32_t port, enum at_socket_type type, qosa_bool_t is_client)
{
    int device_socket = (int) socket->user_data;
    struct at_device *device = (struct at_device *) socket->device;

    LOG_V("%s, buff = %s, bfsz = %d, type = %d", __FUNCTION__, buff, bfsz, type);

    char cmd[128] = {0};
    if ((type == AT_SOCKET_UDP) && !is_client)
    {
        snprintf(cmd, sizeof(cmd), "AT+QISEND=%d,%%d,%s,%d", device_socket, ip, port);
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "AT+QISEND=%d,%%d", device_socket);
    }
    return at_obj_exec_cmd_with_data(device->client, cmd, buff, bfsz);
}

/**
 * domain resolve by AT commands.
 *
 * @param name domain name
 * @param ip parsed IP address, it's length must be 16
 *
 * @return  0: domain resolve success
 *         -1: send AT commands error or response error
 *         -2: wait socket event timeout
 *         -5: no memory
 */
static int ql_socket_at_domain_resolve(const char *name, char ip[16])
{
#define RESOLVE_RETRY                  3

    int i, result;
    at_response_t resp = QOSA_NULL;
    struct at_device *device = QOSA_NULL;

    LOG_V("%s, name = %s", __FUNCTION__, name);

    QOSA_ASSERT(name);
    QOSA_ASSERT(ip);

    device = at_device_get();
    if (device == QOSA_NULL)
    {
        LOG_E("get first init device failed.");
        return -QOSA_ERROR_GENERAL;
    }

    /* the maximum response time is 60 seconds, but it set to 10 seconds is convenient to use. */
    resp = at_create_resp(128, 0, 10 * RT_TICK_PER_SECOND);
    if (!resp)
    {
        LOG_E("no memory for resp create.");
        return QOSA_ERROR_NO_MEMORY;
    }

    /* clear QL_EVENT_DOMAIN_OK */
    ql_socket_event_recv(device, QL_EVENT_DOMAIN_OK, 0, QOSA_EVENT_FLAG_OR);

    result = at_obj_exec_cmd(device->client, resp, "AT+QIDNSGIP=1,\"%s\"", name);
    if (result < 0)
    {
        goto __exit;
    }

    if (result == QOSA_OK)
    {
        for(i = 0; i < RESOLVE_RETRY; i++)
        {
            /* waiting result event from AT URC, the device default connection timeout is 60 seconds.*/
            if (ql_socket_event_recv(device, QL_EVENT_DOMAIN_OK, 10 * RT_TICK_PER_SECOND, QOSA_EVENT_FLAG_OR) < 0)
            {
                continue;
            }
            else
            {
                struct at_device_data *module = (struct at_device_data *) device->user_data;
                char *recv_ip = (char *) module->socket_data;

                if (strlen(recv_ip) < 8)
                {
                    qosa_task_sleep_ms(100);
                    /* resolve failed, maybe receive an URC CRLF */
                    result = -QOSA_ERROR_GENERAL;
                    continue;
                }
                else
                {
                    strncpy(ip, recv_ip, 15);
                    ip[15] = '\0';
                    result = QOSA_OK;
                    break;
                }
            }
        }

        /* response timeout */
        if (i == RESOLVE_RETRY)
        {
            result = QOSA_ERROR_NO_MEMORY;
        }
    }

 __exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;

}

/**
 * set AT socket event notice callback
 *
 * @param event notice event
 * @param cb notice callback
 */
static void ql_socket_at_set_event_cb(at_socket_evt_t event, at_evt_cb_t cb)
{
    LOG_V("%s, event = %d",__FUNCTION__, event);

    if (event < sizeof(at_evt_cb_set) / sizeof(at_evt_cb_set[1]))
    {
        at_evt_cb_set[event] = cb;
    }
}

static int ql_get_socket_num(struct at_device *device, int device_socket)
{
    int i;
    struct at_socket *socket = QOSA_NULL;

    LOG_V("%s",__FUNCTION__);

    QOSA_ASSERT(device);

    for (i=0; i<device->socket_num; i++)
    {
        if ((int)device->sockets[i].user_data == device_socket)
            break;
    }

    QOSA_ASSERT(i!=device->socket_num);

    return i;
}

static void urc_connect_func(struct at_client *client, const char *data, s32_t size)
{
    int device_socket = 0, result = 0;
    struct at_device *device = QOSA_NULL;

    QOSA_ASSERT(data && size);

    LOG_V("%s, size = %d",__FUNCTION__, size);

    device = at_device_get();
    if (device == QOSA_NULL)
    {
        LOG_E("get device failed.");
        return;
    }

    sscanf(data, "+QIOPEN: %d,%d", &device_socket , &result);

    if (result == 0)
    {
        ql_socket_event_send(device, SET_EVENT(device_socket, QL_EVENT_CONN_OK));
    }
    else
    {
        at_tcp_ip_errcode_parse(result);
        ql_socket_event_send(device, SET_EVENT(device_socket, QL_EVENT_CONN_FAIL));
    }
}

static void urc_close_func(struct at_client *client, const char *data, s32_t size)
{
    int device_socket = 0;
    struct at_socket *socket = QOSA_NULL;
    struct at_device *device = QOSA_NULL;

    LOG_V("%s, size = %d",__FUNCTION__, size);

    QOSA_ASSERT(data && size);

    device = at_device_get();
    if (device == QOSA_NULL)
    {
        LOG_E("get device failed.");
        return;
    }

    sscanf(data, "+QIURC: \"closed\",%d", &device_socket);
    /* get at socket object by device socket descriptor */
    socket = &(device->sockets[ql_get_socket_num(device, device_socket)]);
    /* notice the socket is disconnect by remote */
    if (at_evt_cb_set[AT_SOCKET_EVT_CLOSED])
    {
        at_evt_cb_set[AT_SOCKET_EVT_CLOSED](socket, AT_SOCKET_EVT_CLOSED, NULL, 0, 0);
    }
}

static void urc_recv_func(struct at_client *client, const char *data, s32_t size)
{
    int device_socket = 0, i = 0, j = 0;
    s32_t timeout;
    s32_t bfsz = 0, temp_size = 0;
    char *recv_buf = QOSA_NULL, temp[8] = {0};
    struct at_socket *socket = QOSA_NULL;
    struct at_device *device = QOSA_NULL;
    char recv_ip[16] = {0}, *p = QOSA_NULL;
    at_socket_addr at_socket_info;

    LOG_V("%s, size = %d",__FUNCTION__, size);

    QOSA_ASSERT(data && size);
    memset(&at_socket_info, 0, sizeof(at_socket_addr));

    device = at_device_get();
    if (device == QOSA_NULL)
    {
        LOG_E("get device failed.");
        return;
    }

    for (i = 0; i < size; i++)
    {
        if (*(data + i) == '.')
            j++;
    }

    if (j == 3)
    {
        /* get the current socket and receive buffer size by receive data */
        sscanf(data, "+QIURC: \"recv\",%d,%d,\"%[^\"]\"", &device_socket, (int *) &bfsz, recv_ip);
        recv_ip[15] = '\0';
        p = strrchr(data, ',');
        p++;
        at_socket_info.addr.port = atoi(p);
        at_socket_info.addr.sin_addr = inet_addr(recv_ip); //ip
        LOG_V("incoming:%d, %d, %s, %d", device_socket, bfsz, recv_ip, at_socket_info.addr.port);
    }
    else
    {
        /* get the current socket and receive buffer size by receive data */
        sscanf(data, "+QIURC: \"recv\",%d,%d", &device_socket, (int *) &bfsz);
    }

    /* set receive timeout by receive buffer length, not less than 10 ms */
    timeout = bfsz > 10 ? bfsz : 10;
    if (device_socket < 0 || bfsz == 0)
    {
        return;
    }

    recv_buf = (char *) calloc(1, bfsz);
    if (recv_buf == QOSA_NULL)
    {
        LOG_E("no memory for URC receive buffer(%d).", bfsz);
        /* read and clean the coming data */
        while (temp_size < bfsz)
        {
            if (bfsz - temp_size > sizeof(temp))
            {
                at_client_obj_recv(client, temp, sizeof(temp), timeout, true);
            }
            else
            {
                at_client_obj_recv(client, temp, bfsz - temp_size, timeout, true);
            }
            temp_size += sizeof(temp);
        }
        return;
    }
    /* sync receive data */
    if (at_client_obj_recv(client, recv_buf, bfsz, timeout, true) != bfsz)
    {
        LOG_E("device receive size(%d) data failed.", bfsz);
        free(recv_buf);
        return;
    }

    /* get at socket object by device socket descriptor */
    socket = &(device->sockets[ql_get_socket_num(device, device_socket)]);

    /* notice the receive buffer and buffer size */
    if (at_evt_cb_set[AT_SOCKET_EVT_RECV])
    {
        at_evt_cb_set[AT_SOCKET_EVT_RECV](socket, AT_SOCKET_EVT_RECV, recv_buf, bfsz, at_socket_info.data);
    }
}

static void urc_pdpdeact_func(struct at_client *client, const char *data, s32_t size)
{
    int connectID = 0;

    LOG_V("%s, size = %d",__FUNCTION__, size);

    QOSA_ASSERT(data && size);

    sscanf(data, "+QIURC: \"pdpdeact\",%d", &connectID);

    LOG_E("context (%d) is deactivated.", connectID);
}

static void urc_incoming_func(struct at_client *client, const char *data, s32_t size)
{
    int i = 0, j = 0;
    char recv_ip[16] = {0},*p =NULL;
    int result, ip_count, dns_ttl;
    struct at_device *device = QOSA_NULL;
    struct at_device_data *module = QOSA_NULL;
    struct at_socket *socket = QOSA_NULL;
    struct at_incoming_info *incoming_info = NULL;

    LOG_V("%s, size = %d",__FUNCTION__, size);

    QOSA_ASSERT(data && size);

    device = at_device_get();
    if (device == QOSA_NULL)
    {
        LOG_E("get device failed.");
        return;
    }
    module = (struct at_device_data *) device->user_data;

    for (i = 0; i < size; i++)
    {
        if (*(data + i) == '.')
            j++;
    }

    if (j == 3)
    {
        incoming_info = (char *) calloc(1, sizeof(struct at_incoming_info));
        if (incoming_info == QOSA_NULL)
        {
            LOG_E("no memory for incoming URC receive buffer(%d).", sizeof(struct at_incoming_info));
            return;
        }
        //sscanf(data, "+QIURC: \"incoming\",%d,%d,\"%[^\"]\",%d", &incoming_info->socket, &incoming_info->device_socket, incoming_info->ip, &incoming_info->port);
        sscanf(data, "+QIURC: \"incoming\",%d,%d,\"%[^\"]\"", &incoming_info->socket, &incoming_info->device_socket, recv_ip);
        recv_ip[15] = '\0';
        p = strrchr(data, ',');
        p++;
        incoming_info->socket_addr.addr.port = atoi(p);
        incoming_info->socket_addr.addr.sin_addr = inet_addr(recv_ip);

        if (incoming_info->socket < 0 || incoming_info->device_socket < 0)
        {
            return;
        }

        /* get at socket object by device socket descriptor */
        socket = &(device->sockets[ql_get_socket_num(device, incoming_info->device_socket)]);
        LOG_I("socket = %d, device_socket = %d, ip = %s, port = %d", incoming_info->socket, incoming_info->device_socket, recv_ip, incoming_info->socket_addr.addr.port);

        /* notice the receive buffer and buffer size */
        if (at_evt_cb_set[AT_SOCKET_EVT_RECV])
        {
            at_evt_cb_set[AT_SOCKET_EVT_RECV](socket, AT_SOCKET_EVT_RECV, incoming_info, sizeof(struct at_incoming_info), 0);
        }
    }
}

static void urc_dnsqip_func(struct at_client *client, const char *data, s32_t size)
{
    int i = 0, j = 0;
    char recv_ip[16] = {0};
    int result, ip_count, dns_ttl;
    struct at_device *device = QOSA_NULL;
    struct at_device_data *module = QOSA_NULL;

    LOG_V("%s, size = %d",__FUNCTION__, size);

    QOSA_ASSERT(data && size);

    device = at_device_get();
    if (device == QOSA_NULL)
    {
        LOG_E("get device failed.");
        return;
    }
    module = (struct at_device_data *) device->user_data;

    for (i = 0; i < size; i++)
    {
        if (*(data + i) == '.')
            j++;
    }
    /* There would be several dns result, we just pickup one */
    if (j == 3)
    {
        sscanf(data, "+QIURC: \"dnsgip\",\"%[^\"]", recv_ip);
        recv_ip[15] = '\0';

        /* set module information socket data */
        if (module->socket_data == QOSA_NULL)
        {
            module->socket_data = calloc(1, sizeof(recv_ip));
            if (module->socket_data == QOSA_NULL)
            {
                return;
            }
        }
        memcpy(module->socket_data, recv_ip, sizeof(recv_ip));
        ql_socket_event_send(device, QL_EVENT_DOMAIN_OK);
    }
    else
    {
        sscanf(data, "+QIURC: \"dnsgip\",%d,%d,%d", &result, &ip_count, &dns_ttl);
        if (result)
        {
            at_tcp_ip_errcode_parse(result);
        }
    }
}

static void urc_func(struct at_client *client, const char *data, s32_t size)
{
    QOSA_ASSERT(data);

    LOG_I("URC data : %.*s", size, data);
}

static void urc_qiurc_func(struct at_client *client, const char *data, s32_t size)
{
    QOSA_ASSERT(data && size);

    switch(*(data + 9))
    {
        case 'c' : urc_close_func(client, data, size);    break;//+QIURC: "closed"
        case 'r' : urc_recv_func(client, data, size);     break;//+QIURC: "recv"
        case 'p' : urc_pdpdeact_func(client, data, size); break;//+QIURC: "pdpdeact"
        case 'd' : urc_dnsqip_func(client, data, size);   break;//+QIURC: "dnsgip"
        case 'i' : urc_incoming_func(client, data, size); break;//+QIURC: "incoming"
        default  : urc_func(client, data, size);          break;
    }
}

static const struct at_urc urc_table[] =
{
    {"+QIOPEN:",    "\r\n",                 urc_connect_func},
    {"+QIURC:",     "\r\n",                 urc_qiurc_func},
};

/*****************************************************************************************/

static void ql_socket_service_proc(void *argument)
{
    msg_node msgs;
    int status = QOSA_OK;

	LOG_V("%s, stack space %d",__FUNCTION__, qosa_task_get_stack_space(qosa_task_get_current_ref()));

    /* A service that provides network socket interface, and keep the services provided effective */
    while(1)
    {
        memset(&msgs, 0, sizeof(msg_node));
        status = qosa_msgq_wait(g_socket_msg_id, (void *)&msgs, sizeof(msg_node), QOSA_WAIT_FOREVER);
		if (status != QOSA_OK)
		{
			LOG_E("Receive msg from broadcast thread error!");
			continue;
		}
        else
        {
            LOG_V("Receive broadcast msg is what=0x%x, arg1=0x%x, arg2=0x%x, arg3=0x%x", msgs.what, msgs.arg1, msgs.arg2, msgs.arg3);
            switch (msgs.what)
            {
                case QL_BROADCAST_SOCKET_CONNECT_SUCCESS:
                    LOG_D("QL_BROADCAST_SOCKET_CONNECT_SUCCESS");
                    break;

                case QL_BROADCAST_SOCKET_CONNECT_FAILURE:
                    LOG_D("QL_BROADCAST_SOCKET_CONNECT_FAILURE");
                    break;

                case QL_BROADCAST_SOCKET_SEND_DATA_SUCCESS:
                    LOG_D("QL_BROADCAST_SOCKET_SEND_DATA_SUCCESS");
                    break;

                case QL_BROADCAST_SOCKET_SEND_DATA_FAILURE:
                    LOG_D("QL_BROADCAST_SOCKET_SEND_DATA_FAILURE");
                    break;

                case QL_BROADCAST_SOCKET_RECV_DATA_SUCCESS:
                    LOG_D("QL_BROADCAST_SOCKET_RECV_DATA_SUCCESS");
                    break;

                case QL_BROADCAST_SOCKET_RECV_DATA_FAILURE:
                    LOG_D("QL_BROADCAST_SOCKET_RECV_DATA_FAILURE");
                    break;

                case QL_BROADCAST_NET_DATACALL_SUCCESS:
                    {
                        struct at_socket_ops ql_at_socket_ops;
                        at_client_t client = at_client_get_first();
                        ip_addr_t module_addr;
                        #ifdef __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__
                        // module_addr = ql_net_get_ip();
                        #endif

                        memset(&ql_at_socket_ops, 0, sizeof(struct at_socket_ops));

                        ql_at_socket_ops.at_connect        = ql_socket_at_connect;
                        ql_at_socket_ops.at_closesocket    = ql_socket_at_closesocket;
                        ql_at_socket_ops.at_send           = ql_socket_at_send;
                        ql_at_socket_ops.at_domain_resolve = ql_socket_at_domain_resolve;
                        ql_at_socket_ops.at_set_event_cb   = ql_socket_at_set_event_cb;
                        // ql_at_socket_ops.at_socket         = ql_socket_at_socket;
                        at_device_socket_register(12, &ql_at_socket_ops, client, &module_addr);
                        LOG_D("at_device_socket_register success");
                        broadcast_send_msg_myself(QL_BROADCAST_SOCKET_INIT_SUCCESS, 0, 0, 0);  //Send tcp init successful broadcast
                    }
                    break;

                default:
                    LOG_D("Unrecognized message");
                    break;
            }
        }
    }
    LOG_V("%s over",__FUNCTION__);
    qosa_task_exit();
}

int ql_socket_service_create(void)
{
    int ret = QOSA_OK;
    int status = QOSA_OK;

    LOG_V("%s",__FUNCTION__);

    //1. Creat msg queue
    status = qosa_msgq_create(&g_socket_msg_id, sizeof(msg_node), MAX_MSG_COUNT);
	if (status != QOSA_OK)
	{
		LOG_E("Create ql_socket_service msg failed!");
		return -1;
	}

    //2. Register which broadcasts need to be processed
    broadcast_reg_receive_msg(QL_BROADCAST_SOCKET_CONNECT_SUCCESS,   g_socket_msg_id);
    broadcast_reg_receive_msg(QL_BROADCAST_SOCKET_CONNECT_FAILURE,   g_socket_msg_id);
    broadcast_reg_receive_msg(QL_BROADCAST_SOCKET_SEND_DATA_SUCCESS, g_socket_msg_id);
    broadcast_reg_receive_msg(QL_BROADCAST_SOCKET_SEND_DATA_FAILURE, g_socket_msg_id);
    broadcast_reg_receive_msg(QL_BROADCAST_SOCKET_RECV_DATA_SUCCESS, g_socket_msg_id);
    broadcast_reg_receive_msg(QL_BROADCAST_SOCKET_RECV_DATA_FAILURE, g_socket_msg_id);
    broadcast_reg_receive_msg(QL_BROADCAST_NET_DATACALL_SUCCESS,  g_socket_msg_id);

    //3. Deal with tcp problems
    /* register URC data execution function  */
    at_set_urc_table(urc_table, sizeof(urc_table) / sizeof(urc_table[0]));

    //4. Create net service
    ret = qosa_task_create(&g_socket_thread_id, 512*10, QOSA_PRIORITY_NORMAL, "Tcp_S", ql_socket_service_proc, NULL);
	if (ret != QOSA_OK)
	{
		LOG_E ("ql_socket_service_create thread could not start!");
		return -1;
	}

    LOG_I("%s over(%x)",__FUNCTION__, g_socket_thread_id);

    return 0;
}

int ql_socket_service_destroy(void)
{
    int ret = QOSA_OK;

    LOG_V("%s",__FUNCTION__);

    //1. Deal with tcp problems

    //2. UnRegister which broadcasts need to be processed
    broadcast_unreg_receive_msg(QL_BROADCAST_SOCKET_CONNECT_SUCCESS,   g_socket_msg_id);
    broadcast_unreg_receive_msg(QL_BROADCAST_SOCKET_CONNECT_FAILURE,   g_socket_msg_id);
    broadcast_unreg_receive_msg(QL_BROADCAST_SOCKET_SEND_DATA_SUCCESS, g_socket_msg_id);
    broadcast_unreg_receive_msg(QL_BROADCAST_SOCKET_SEND_DATA_FAILURE, g_socket_msg_id);
    broadcast_unreg_receive_msg(QL_BROADCAST_SOCKET_RECV_DATA_SUCCESS, g_socket_msg_id);
    broadcast_unreg_receive_msg(QL_BROADCAST_SOCKET_RECV_DATA_FAILURE, g_socket_msg_id);
    broadcast_unreg_receive_msg(QL_BROADCAST_NET_DATACALL_SUCCESS,     g_socket_msg_id);

    //3. Destroy net service
    if (NULL != g_socket_thread_id)
    {
        ret = qosa_task_delete(g_socket_thread_id);
        if (0 != ret)
        {
            LOG_E("Delete g_socket_thread_id thread failed! %d", ret);
            return -1;
        }
    }

    //4. Delete msg queue
    if (NULL != g_socket_msg_id)
    {
        ret = qosa_msgq_delete(g_socket_msg_id);
        if (QOSA_OK != ret)
        {
            LOG_E("Delete g_socket_msg_id msg failed! %d", ret);
            return -1;
        }
    }
    LOG_V("%s over",__FUNCTION__);

    return 0;
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__ */
