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

// Structure of the event
typedef struct {
    osa_sem_t       event_sem;          // The semaphore is used to notify the occurrence of an event
    unsigned int    event_flags;        // Event flag bit, each bit represents one event
    osa_mutex_t     event_mutex;        // The mutex lock is used to protect the access to the event flag.
} event_t;

// Create an event object
int qosa_event_create(osa_event_t *eventRef)
{
    // FIX: FAEDEVELOP-135, Jerry Chen modified, 2025-06-23
    event_t* e = (event_t*)malloc(sizeof(event_t));

    if (!e)
        return QOSA_ERROR_NO_MEMORY;

    if (qosa_sem_create(&e->event_sem, 0) != QOSA_OK)
    {
        free(e);
        return QOSA_ERROR_NO_MEMORY;
    }

    if (qosa_mutex_create(&e->event_mutex) != QOSA_OK)
    {
        qosa_sem_delete(e->event_sem);
        free(e);
        return QOSA_ERROR_NO_MEMORY;
    }
    *eventRef = e;

    e->event_flags = 0;

    return QOSA_OK;
}

// Destroy the event object
int qosa_event_delete(osa_event_t eventRef)
{
    event_t *e = (event_t *)eventRef;

    if (!e)
        return QOSA_ERROR_PARAM_INVALID;

    // FIX: FAEDEVELOP-135, Jerry Chen modified, 2025-06-23
    // Pointer variable is wrong in the API parameter.
    qosa_sem_delete(e->event_sem);
    qosa_mutex_delete(e->event_mutex);
    free(e);

    return QOSA_OK;
}

// Wait event (logical OR operation)
int qosa_event_recv(osa_event_t eventRef, unsigned int event_mask, qosa_base_event_e option, int timeout)
{
    int status = 0;
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

            //todo: need to update timeout
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

// Set event to the triggered state
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
