/*****************************************************************/ /**
* @file qosa_msg_queue.c
* @brief
* @author larson.li@quectel.com
* @date 2024-12-23
*
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description
* <tr><td>2024-12-23 <td>1.0 <td>Larson.Li <td> Init
* </table>
**********************************************************************/

#include "qosa_def.h"
#include "qosa_system.h"
#include "qosa_queue_list.h"

#define event_printf(msg, ...) printf("%s,%d," msg "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

struct osa_event_data
{
    qosa_link_type_t list;
    void            *argv;
};

typedef struct
{
    qosa_type_t type_list;       /*!< 用于存储事件列表 */
    u32_t    event_max_count; /*!< 事件最大个数 */
    osa_sem_t   event_sem;       /*!< 对应事件通知的sem */
    u32_t    event_size;      /*!< 对应注册事件类型结构体大小 */
    u32_t    current_cnt;     /*!< 当前已缓存的event个数 */
    osa_mutex_t event_mutex;     /*!< Event 操作互斥锁 */
} OSA_EventHndl;

/**
 * @brief Used to create a message queue and initialize it
 *
 * @param[in] osa_msgq_t * msgQRef
 *          - The message queue pointer handle
 *
 * @param[in] u32_t size
 *          - The length of the data type to be stored in the message queue,
 *            usually the length of the data type pointer.
 *
 * @param[in] u32_t maxNumber
 *          - The maximum number of messages in the message queue
 *
 * @return int
 *        - The function returns QOSA_OK if the function executes successfully,
 *          otherwise it returns a negative number
 */
int qosa_msgq_create(osa_msgq_t *msgQRef, u32_t size, u32_t maxNumber)
{
    OSA_EventHndl *hndl = NULL;
    int32_t        status = QOSA_OK;

    if (!msgQRef || !maxNumber)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }

    hndl = (OSA_EventHndl *)malloc(sizeof(OSA_EventHndl));
    if (hndl == QOSA_OK)
    {
        return QOSA_ERROR_NO_MEMORY;
    }

    status = qosa_sem_create(&hndl->event_sem, 0);
    if (status != QOSA_OK)
    {
        return QOSA_ERROR_NO_MEMORY;
    }

    status = qosa_mutex_create(&hndl->event_mutex);
    if (status != QOSA_OK)
    {
        return QOSA_ERROR_NO_MEMORY;
    }

    qosa_init(&hndl->type_list);

    hndl->event_size = size;
    hndl->event_max_count = maxNumber;
    hndl->current_cnt = 0;

    *msgQRef = hndl;
    return status;
}

/**
 * @brief 用于删除所创建的消息队列
 *
 * @param[in] osa_msgq_t msgQRef
 *          - 消息队列指针句柄
 *
 * @return int
 *        -  函数执行成功返回QOSA_OK, 否则返回一个负数
 */
int qosa_msgq_delete(osa_msgq_t msgQRef)
{
    OSA_EventHndl         *hndl = NULL;
    struct osa_event_data *data_ptr = NULL;

    if (!msgQRef)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    hndl = (OSA_EventHndl *)msgQRef;

    qosa_sem_delete(hndl->event_sem);
    if (qosa_cnt(&hndl->type_list) != 0)
    {
        data_ptr = (struct osa_event_data *)qosa_get(&hndl->type_list);
        while (data_ptr != NULL)
        {
            free(data_ptr->argv);
            free(data_ptr);
            data_ptr = NULL;
            data_ptr = (struct osa_event_data *)qosa_get(&hndl->type_list);
        }
    }

    qosa_mutex_delete(hndl->event_mutex);

    qosa_destroy(&hndl->type_list);
    free(hndl);

    return QOSA_OK;
}

/**
 * @brief 释放(发送)一个消息
 *
 * @param[in] osa_msgq_t msgQRef
 *          - 消息队列指针句柄
 *
 * @param[in] u32_t size
 *          - 要获取存放到消息队列中的数据类型长度,要与创建时的size参数保持一致
 *
 * @param[in] uint8_t * value
 *          - 要释放的消息数据的首地址
 *
 * @param[in] u32_t timeout
 *          - OSA_WAIT_FOREVER, OSA_NO_WAIT, or timeout
 *
 * @return int
 *       - 函数执行成功返回QOSA_OK, 否则返回一个负数
 *
 * @note
 *     - 请注意，当消息队列已满时，发布可能会失败并返回负值
 */
int qosa_msgq_release(osa_msgq_t msgQRef, u32_t size, uint8_t *value, u32_t timeout)
{
    int32_t                status = QOSA_ERROR_GENERAL;
    OSA_EventHndl         *hndl = msgQRef;
    struct osa_event_data *data_ptr = NULL;
    QOSA_UNUSED(timeout);

    if (!msgQRef)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    hndl = (OSA_EventHndl *)msgQRef;

    if (size != hndl->event_size)
    {
        //TODO:
        event_printf("The current size=%d is different from the initial size=%d.", size, hndl->event_size);
        return QOSA_ERROR_EVENT_SIZE_ERR;
    }

    if (hndl->current_cnt >= hndl->event_max_count)
    {
        event_printf("event count=%d full, please check you code", hndl->current_cnt);
        return QOSA_ERROR_EVENT_FULL_ERR;
    }


    data_ptr = (struct osa_event_data *)malloc(sizeof(struct osa_event_data));
    if (data_ptr == NULL)
    {
        return QOSA_ERROR_NO_MEMORY;
    }

    data_ptr->argv = (void *)malloc(size);
    if (data_ptr->argv == NULL)
    {
        free(data_ptr);
        return QOSA_ERROR_NO_MEMORY;
    }
    qosa_mutex_lock(hndl->event_mutex, QOSA_WAIT_FOREVER);
    memcpy(data_ptr->argv, value, size);
    qosa_link(data_ptr, &data_ptr->list);
    qosa_put(&hndl->type_list, &data_ptr->list);
    hndl->current_cnt++;
    qosa_mutex_unlock(hndl->event_mutex);

    status = qosa_sem_release(hndl->event_sem);
    if (status != QOSA_OK)
    {
        return status;
    }
    return status;
}

/**
 * @brief 等待一个消息的到达,当为OSA_WAIT_FOREVER时,如果消息队列满后
 *			  要等待
 *
 * @param[in] osa_task_t task_ref
 *          - 线程指针句柄
 *
 * @param[out] uint8_t * recvMsg
 *          - 准备接收数据的首地址
 *
 * @param[in] u32_t size
 *          - 要获取存放到消息队列中的数据类型长度,要与创建时的size参数保持一致
 *
 * @param[in] u32_t timeout
 *          - OSA_WAIT_FOREVER, OSA_NO_WAIT, or timeout
 *
 * @return int
 *       - 0 返回执行成功
 *       - 其他 表示执行失败
 */
int qosa_msgq_wait(osa_msgq_t msgQRef, uint8_t *value, u32_t size, u32_t timeout)
{
    int32_t                status = QOSA_ERROR_GENERAL;
    OSA_EventHndl         *hndl = NULL;
    struct osa_event_data *data_ptr = NULL;
    QOSA_UNUSED(timeout);

    if (!msgQRef)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    hndl = (OSA_EventHndl *)msgQRef;

    if (size != hndl->event_size)
    {
        //TODO:
        return -1;
    }

    status = qosa_sem_wait(hndl->event_sem, timeout);
    if (status != QOSA_OK)
    {
        if (status == QOSA_ERROR_SEMA_TIMEOUT_ERR)
        {
            status = QOSA_ERROR_EVENT_TIMEOUT_ERR;
        }
        return status;
    }
    qosa_mutex_lock(hndl->event_mutex, QOSA_WAIT_FOREVER);

    data_ptr = (struct osa_event_data *)qosa_get(&hndl->type_list);
    memcpy(value, data_ptr->argv, size);
    free(data_ptr->argv);
    free(data_ptr);
    hndl->current_cnt--;
    qosa_mutex_unlock(hndl->event_mutex);

    return status;
}

/**
 * @brief 获取消息队列中当前所储存的消息个数
 *
 * @param[in] osa_msgq_t msgQRef
 *          - 消息队列指针句柄
 *
 * @param[in] u32_t * cnt_ptr
 *          - 返回当前队列中的项目个数
 *
 * @return int
 *       - 函数执行成功返回QOSA_OK, 否则返回一个负数
 */
int qosa_msgq_get_cnt(osa_msgq_t msgQRef, u32_t *cnt_ptr)
{
    OSA_EventHndl *hndl = NULL;

    if (!msgQRef)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    hndl = (OSA_EventHndl *)msgQRef;
    *cnt_ptr = qosa_cnt(&hndl->type_list);
    return QOSA_OK;
}
