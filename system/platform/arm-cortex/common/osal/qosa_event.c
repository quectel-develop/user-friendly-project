/*****************************************************************/ /**
* @file qosa_event.c
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
#include <stdlib.h>
#include "qosa_log.h"
// 事件结构体
typedef struct {
    osa_sem_t   event_sem;           // 信号量用于通知事件发生
    unsigned int event_flags;        // 事件标志位，每一位代表一个事件
    osa_mutex_t event_mutex;         // 互斥锁用于保护事件标志的访问
} event_t;

// 创建事件对象

int qosa_event_create(osa_event_t *eventRef)
{
    // FIX: FAEDEVELOP-135, Jerry Chen modified, 2025-06-23
    event_t* e = (event_t*)pvPortMalloc(sizeof(event_t));

    if (!e)
        return QOSA_ERROR_NO_MEMORY;

    if (qosa_sem_create(&e->event_sem, 0) != QOSA_OK)
    {
        vPortFree(e);
        return QOSA_ERROR_NO_MEMORY;
    }

    if (qosa_mutex_create(&e->event_mutex) != QOSA_OK)
    {
        qosa_sem_delete(e->event_sem);
        vPortFree(e);
        return QOSA_ERROR_NO_MEMORY;
    }
    *eventRef = e;

    e->event_flags = 0;

    return QOSA_OK;
}

// 销毁事件对象
int qosa_event_delete(osa_event_t eventRef)
{
    event_t *e = (event_t *)eventRef;

    if (!e)
        return QOSA_ERROR_PARAM_INVALID;

    // FIX: FAEDEVELOP-135, Jerry Chen modified, 2025-06-23
    // Pointer variable is wrong in the API parameter.
    qosa_sem_delete(e->event_sem);
    qosa_mutex_delete(e->event_mutex);
    vPortFree(e);

    return QOSA_OK;
}

// 等待事件（逻辑或操作）
int qosa_event_recv(osa_event_t eventRef, unsigned int event_mask, qosa_base_event_e option, int timeout)
{
    int ret, status = 0;
    event_t *e = (event_t *)eventRef;
    unsigned int received_events;

    if (!e)
        return QOSA_ERROR_PARAM_INVALID;

    while (1)
    {
        qosa_mutex_lock(e->event_mutex, QOSA_WAIT_FOREVER);

        received_events = e->event_flags & event_mask;
        if (((option & QOSA_EVENT_FLAG_OR) && (received_events)) || ((option & QOSA_EVENT_FLAG_AND) && (received_events == event_mask)))
        {
            if (!(option & QOSA_EVENT_FLAG_NO_CLEAR))
            {
                e->event_flags &= ~received_events;
            }
            qosa_mutex_unlock(e->event_mutex);
            return received_events;
        }
        else
        {
            qosa_mutex_unlock(e->event_mutex);

            //todo:需要更新超时时间
            status = qosa_sem_wait(e->event_sem, timeout);

            if (status != QOSA_OK)
            {
                if (status == QOSA_ERROR_SEMA_TIMEOUT_ERR)
                {
                    status = QOSA_ERROR_EVENT_TIMEOUT_ERR;
                }
                return status;
            }
        }
    }

    return received_events;
}

// 设置事件为触发状态
int qosa_event_send(osa_event_t eventRef, u32_t flags)
{
    event_t *e = (event_t *)eventRef;

    if (!e)
        return QOSA_ERROR_PARAM_INVALID;

    qosa_mutex_lock(e->event_mutex, QOSA_WAIT_FOREVER);
    e->event_flags |= flags;
    qosa_sem_release(e->event_sem);
    qosa_mutex_unlock(e->event_mutex);

    return QOSA_OK;
}

int qosa_event_flags_get(osa_event_t eventRef)
{
    event_t *e = (event_t *)eventRef;

    if (!e)
        return QOSA_ERROR_PARAM_INVALID;

    return e->event_flags;
}
