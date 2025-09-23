#ifndef __BROADCAST_SERVICE_H__
#define __BROADCAST_SERVICE_H__

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#include "qosa_def.h"
#include "qosa_system.h"

#define MAX_MSG_COUNT               (16)
#define MAX_BROADCAST_RECEIVE       (32)
#define QL_BROADCAST_INVALID        0x0000


typedef enum
{
	QL_BROADCAST_MAIN_STATE                 = 0x1000,

	QL_BROADCAST_MODULE_INIT                = 0x2000,
	QL_BROADCAST_MODULE_FAILURE,
	QL_BROADCAST_SIM_START,
	QL_BROADCAST_SIM_READY_FAILURE,
	QL_BROADCAST_NW_START,
	QL_BROADCAST_NET_NET_REG_SUCCESS,
	QL_BROADCAST_NET_NET_REG_FAILURE,
	QL_BROADCAST_DATACALL_START,
	QL_BROADCAST_NET_DATACALL_SUCCESS,
	QL_BROADCAST_NET_DATACALL_FAILURE,
	QL_BROADCAST_NET_KEEPALIVE,
	QL_BROADCAST_SD_CARD_DETECT,

	QL_BROADCAST_SOCKET_INIT_SUCCESS        = 0x3000,
	QL_BROADCAST_SOCKET_CONNECT_SUCCESS,
	QL_BROADCAST_SOCKET_CONNECT_FAILURE,
	QL_BROADCAST_SOCKET_SEND_DATA_SUCCESS,
	QL_BROADCAST_SOCKET_SEND_DATA_FAILURE,
	QL_BROADCAST_SOCKET_RECV_DATA_SUCCESS,
	QL_BROADCAST_SOCKET_RECV_DATA_FAILURE,

	QL_BROADCAST_FTP_INIT_SUCCESS           = 0x4000,
	QL_BROADCAST_FTP_CONNENT_SUCCESS,
	QL_BROADCAST_FTP_UP_SUCCESS,
	QL_BROADCAST_FTP_UP_FAIL,
	QL_BROADCAST_FTP_GET_SUCCESS,
	QL_BROADCAST_FTP_GET_FAIL,
	QL_BROADCAST_FTP_CLOSE_SUCCESS,

	QL_BROADCAST_HTTP_INIT_SUCCESS          = 0x5000,
	QL_BROADCAST_HTTP_REQUEST,
	QL_BROADCAST_HTTP_REQUEST_SUCCESS,
	QL_BROADCAST_HTTP_REQUEST_FAILURE,
} MSG_WHAT_STATE;


typedef struct
{
	s32_t what;
	s32_t arg1;
	s32_t arg2;
	s32_t arg3;
} msg_node, *msg_node_t;

typedef struct
{
	s32_t what;
	osa_msgq_t msgqid; 	/* Msg queue id */
} broadcast_recv, *broadcast_recv_t;



s32_t broadcast_service_create(void);
s32_t broadcast_service_destory(void);

s32_t broadcast_reg_receive_msg(s32_t what, osa_msgq_t msgqid);
s32_t broadcast_unreg_receive_msg (s32_t what, osa_msgq_t msgqid);

s32_t broadcast_send_msg_myself(s32_t what, s32_t arg1, s32_t arg2, s32_t arg3);
s32_t broadcast_send_msg(osa_msgq_t msg_id, s32_t what, s32_t arg1, s32_t arg2, s32_t arg3);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __BROADCAST_SERVICE_H__ */
