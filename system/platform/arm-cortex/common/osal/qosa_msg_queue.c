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

#define event_printf(msg, ...) printf("%s, %d," msg "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

struct osa_event_data
{
    qosa_link_type_t list;
    void            *argv;
};

typedef struct
{
    qosa_type_t type_list;       /*!< Used to store event list */
    u32_t       event_max_count; /*!< Maximum number of events */
    osa_sem_t   event_sem;       /*!< Semaphore for event notification */
    u32_t       event_size;      /*!< Size of the registered event type structure */
    u32_t       current_cnt;     /*!< Current number of cached events */
    osa_mutex_t event_mutex;     /*!< Event operation mutex lock */
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
 * @brief Used to delete the created message queue
 *
 * @param[in] osa_msgq_t msgQRef
 *          - Message queue pointer handle
 *
 * @return int
 *        - Returns QOSA_OK if the function executes successfully, otherwise returns a negative number
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
 * @brief Releases (sends) a message
 *
 * @param[in] osa_msgq_t msgQRef
 *          - Message queue pointer handle
 *
 * @param[in] u32_t size
 *          - Length of the data type to be stored in the message queue, must be consistent with the size parameter during creation
 *
 * @param[in] uint8_t * value
 *          - Starting address of the message data to be released
 *
 * @param[in] u32_t timeout
 *          - OSA_WAIT_FOREVER, OSA_NO_WAIT, or timeout
 *
 * @return int
 *       - Returns QOSA_OK if the function executes successfully, otherwise returns a negative number
 *
 * @note
 *     - Note that publishing may fail and return a negative value when the message queue is full
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
        event_printf("The current size=%lu is different from the initial size=%lu.", size, hndl->event_size);
        return QOSA_ERROR_EVENT_SIZE_ERR;
    }

    if (hndl->current_cnt >= hndl->event_max_count)
    {
        event_printf("event count=%lu full", hndl->current_cnt);
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
 * @brief Waits for a message to arrive. When set to OSA_WAIT_FOREVER, it will wait if the message queue is full
 *
 * @param[in] osa_task_t task_ref
 *          - Thread pointer handle
 *
 * @param[out] uint8_t * recvMsg
 *          - Starting address for receiving data
 *
 * @param[in] u32_t size
 *          - Length of the data type to be stored in the message queue, must be consistent with the size parameter during creation
 *
 * @param[in] u32_t timeout
 *          - OSA_WAIT_FOREVER, OSA_NO_WAIT, or timeout
 *
 * @return int
 *       - 0 indicates successful execution
 *       - Other values indicate execution failure
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
 * @brief Gets the current number of messages stored in the message queue
 *
 * @param[in] osa_msgq_t msgQRef
 *          - Message queue pointer handle
 *
 * @param[in] u32_t * cnt_ptr
 *          - Returns the current number of items in the queue
 *
 * @return int
 *       - Returns QOSA_OK if the function executes successfully, otherwise returns a negative number
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
