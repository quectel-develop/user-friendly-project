#include "broadcast_service.h"
#include "qosa_def.h"
#include "qosa_log.h"

static broadcast_recv g_broadcast_recv[MAX_BROADCAST_RECEIVE] = { {0,0} };
static osa_msgq_t g_broadcast_msg_id  = NULL;
static osa_task_t g_broadcast_thread_id =NULL;

/******************************************************************************
* function : register receive to broascast
* return: err:FAILURE  ok:SUCCESS
******************************************************************************/
s32_t broadcast_reg_receive_msg(s32_t what, osa_msgq_t msgqid)
{
	s32_t s32i = 0;
	s32_t bcast_rev_cnt = 0;

	/* judge the msg has been regist or not */
	for (s32i = 0; s32i < MAX_BROADCAST_RECEIVE; s32i++)
	{
		if (NULL != g_broadcast_recv[s32i].msgqid)
		{
			bcast_rev_cnt++;
			if ((msgqid == g_broadcast_recv[s32i].msgqid) && (what == g_broadcast_recv[s32i].what))
			{
				LOG_W ("msg has been regist msgqid = 0x%x", msgqid);
				return 0;
			}
		}
	}

	if (bcast_rev_cnt >= MAX_BROADCAST_RECEIVE)
	{
		LOG_E ("broadcast receive is full");
		return -1;
	}

	//LOG_V ("msg receiver:%lld, 0x%x, receiverq index:%d", what, msgqid, bcast_rev_cnt);

	for (s32i = 0; s32i < MAX_BROADCAST_RECEIVE; s32i++)
	{
		if (NULL == g_broadcast_recv[s32i].msgqid)
		{
			g_broadcast_recv[s32i].what = what;
			g_broadcast_recv[s32i].msgqid = msgqid;
			break;
		}
	}

	return 0;
}

/******************************************************************************
* function : unregister receive to broascast
* return: err:FAILURE  ok:SUCCESS
******************************************************************************/
s32_t broadcast_unreg_receive_msg(s32_t what, osa_msgq_t msgqid)
{
	s32_t s32i = 0;
	s32_t bcast_rev_cnt = 0;

	LOG_V ("%s",__FUNCTION__);

	/* judge the msg has been regist or not */
	for (s32i = 0; s32i < MAX_BROADCAST_RECEIVE; s32i++)
	{
		if ((msgqid == g_broadcast_recv[s32i].msgqid) && (what == g_broadcast_recv[s32i].what))
		{
			g_broadcast_recv[s32i].what = QL_BROADCAST_INVALID;
			g_broadcast_recv[s32i].msgqid = NULL;
		}
		else if (msgqid != NULL)
		{
			bcast_rev_cnt++;
		}
	}

	LOG_V ("msg unreceiver:%lld, 0x%x, receiverq index:%d", what, msgqid, bcast_rev_cnt);

	return 0;
}

/******************************************************************************
* function : send broadcast msg
* return: err:FAILURE  ok:SUCCESS
******************************************************************************/
s32_t broadcast_send_msg(osa_msgq_t msg_id, s32_t what, s32_t arg1, s32_t arg2, s32_t arg3)
{
	s32_t ret = QOSA_OK;
	msg_node msg;

	LOG_V("%s: msg_id = 0x%x, what = 0x%x, arg1 = 0x%x, arg2 = 0x%x, arg3 = 0x%x", __FUNCTION__, msg_id, what, arg1, arg2, arg3);

	msg.what = what;
	msg.arg1 = arg1;
	msg.arg2 = arg2;
	msg.arg3 = arg3;
	ret = qosa_msgq_release(msg_id, sizeof(msg_node), (void *)&msg, 0);
	if (ret != QOSA_OK)
	{
		LOG_E("Send msg failed! %d", ret);
		return -1;
	}

	return ret;
}

/******************************************************************************
* function : send broadcast msg to myself
* return: err:FAILURE  ok:SUCCESS
******************************************************************************/
s32_t broadcast_send_msg_myself(s32_t what, s32_t arg1, s32_t arg2, s32_t arg3)
{
	msg_node msg;
	LOG_V("%s: what = 0x%x, arg1 = 0x%x, arg2 = 0x%x, arg3 = 0x%x", __FUNCTION__, what, arg1, arg2, arg3);

	return broadcast_send_msg(g_broadcast_msg_id, what, arg1, arg2, arg3);
}

/******************************************************************************
* function : broadcast thread
******************************************************************************/
static void* broadcast_service_thread_proc(void* pThreadParam)
{
	s32_t ret;
	s32_t i;
	s8_t bfind;
	msg_node msgs;
	int status = QOSA_OK;

	LOG_V("%s, stack space %d",__FUNCTION__, qosa_task_get_stack_space(qosa_task_get_current_ref()));

	while (1)
	{
		memset(&msgs, 0, sizeof(msg_node));
		status = qosa_msgq_wait(g_broadcast_msg_id, (void *)&msgs, sizeof(msg_node), QOSA_WAIT_FOREVER);
		if (status != QOSA_OK)
		{
			LOG_E("receive msg from broadcast thread error!");
			continue;
		}

		//LOG_D("Receive broadcast msg is what=0x%x, arg1=0x%x, arg2=0x%x, arg3=0x%x", msgs.what, msgs.arg1, msgs.arg2, msgs.arg3);

		bfind = QOSA_FALSE;
		for (i = 0; i < MAX_BROADCAST_RECEIVE; i++)
		{
			//LOG_V ("Revceiver index:0x%x, receiver msg id: 0x%x, what:%d", i, g_broadcast_recv[i].msgqid, g_broadcast_recv[i].what);

			if (g_broadcast_recv[i].what == 0)
			{
				//LOG_V ("The broadcast receive search over!");
				break;
			}

			if (msgs.what == g_broadcast_recv[i].what)
			{
				//LOG_V ("Find the match what %d, revqid:0x%x, len:%d", msgs.what, g_broadcast_recv[i].msgqid, sizeof(msg_node));

				bfind = QOSA_TRUE;
				status = qosa_msgq_release(g_broadcast_recv[i].msgqid, sizeof(msg_node), &msgs, 0);
				if (status != QOSA_OK)
				{
					LOG_E ("Can not send msg to 0x%x", g_broadcast_recv[i].msgqid);
				}
			}
		}

		if (!bfind)
		{
			LOG_V("This broadcast msg none receiver!");
		}
	}
	LOG_V("%s over",__FUNCTION__);
	qosa_task_exit();
}

/************************************************************************
 *function: broadcast_service_create
 *brief: Broadcast service init
 *param: NULL
 *return: SUCCESS or FAILURE
**************************************************************************/
s32_t broadcast_service_create(void)
{
	int ret = QOSA_OK;

	LOG_V("%s",__FUNCTION__);

	//create broadcast msg queue
	ret = qosa_msgq_create(&g_broadcast_msg_id, sizeof(msg_node), MAX_MSG_COUNT);
	if (ret != QOSA_OK)
	{
		LOG_E("Create broadcast msg failed!");
		return -1;
	}

	ret = qosa_task_create(&g_broadcast_thread_id, 512*3, QOSA_PRIORITY_NORMAL, "Bcast_S", broadcast_service_thread_proc, NULL);
	if (ret != QOSA_OK)
	{
		LOG_E ("Broadcast thread could not start!");
		return -1;
	}
	LOG_I("%s over(%x)",__FUNCTION__, g_broadcast_thread_id);

	return 0;
}

s32_t broadcast_service_destory(void)
{
	int ret = 0;

    LOG_V("%s",__FUNCTION__);

	//1. Destroy net service
    if (NULL != g_broadcast_thread_id)
    {
        ret = qosa_task_delete(g_broadcast_thread_id);
        if (0 != ret)
        {
            LOG_E("Delete g_broadcast_thread_id thread failed! %d", ret);
            return -1;
        }
    }

    //2. Delete msg queue
    if (NULL != g_broadcast_thread_id)
    {
        ret = qosa_msgq_delete(g_broadcast_thread_id);
        if (QOSA_OK != ret)
        {
            LOG_E("Delete g_broadcast_thread_id msg failed! %d", ret);
            return -1;
        }
    }
	LOG_V("%s over",__FUNCTION__);

    return 0;
}

