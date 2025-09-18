/*****************************************************************/ /**
* @file qosa_sem.c
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
#include "FreeRTOS.h"
#include "qosa_def.h"
#include "qosa_system.h"

typedef struct
{
    u32_t       count;      // Current semaphore count
    u32_t       maxCount;   // Maximum semaphore count
    osa_mutex_t lock;       // Synchronization lock for count
    osa_sem_t   sem;        // Actual semaphore
} OSA_SemHndl;

int qosa_sem_create(osa_sem_t *semaRef, u32_t initialCount)
{
    OSA_SemHndl *hndl = NULL;
    int          ret = 0;

    /* FIX: FAEDEVELOP-135 */
    /* Using malloc() will cause the program to stuck in */
    /* qosa_event_delete() -> qosa_mutex_delete() -> vSemaphoreDelete() -> vQueueDelete() -> vPortFree() */
    /* When using malloc() there, the memory freed by vPortFree() in future is not exist, for it wasn't allocated by pvPortMalloc() */
    /* Jerry.Chen, 2025-06-16 */
    hndl = pvPortMalloc(sizeof(OSA_SemHndl));
    if (hndl == NULL)
    {
        return QOSA_ERROR_NO_MEMORY;
    }
    memset(hndl, 0, sizeof(OSA_SemHndl));
    // Create a lock to ensure count is correct when used by multiple threads
    ret = qosa_mutex_create(&hndl->lock);
    if (ret != QOSA_OK)
    {
        vPortFree(hndl);
        return ret;
    }
    hndl->count = initialCount;
    hndl->maxCount = 0x7FFFFFFF;

    hndl->sem = osSemaphoreNew(5, 0, NULL);
    if (hndl->sem == NULL)
    {
        printf("CreateSemaphore failed with error code\n");

        qosa_mutex_delete(hndl->lock);
        vPortFree(hndl);
        return QOSA_ERROR_SEMA_CREATE_ERR;
    }

    *semaRef = hndl;
    return QOSA_OK;
}

// /*
//  * @brief If created with the actual count, it must be ensured that it cannot be released multiple times, otherwise it will cause an active DUMP, so the count limit is removed for now
//  *
//  * @param[in] osa_sem_t * semaRef
//  *          - Pointer to the semaphore
//  * @param[in] osa_uint32_t * initialCount
//  *          - Initial count of the semaphore
//  * @param[in] osa_uint32_t * max_cnt
//  *          - Maximum semaphore count
//  */
// int qosa_sem_create_ex(osa_sem_t *semaRef, u32_t initialCount, u32_t max_cnt)
// {
//     OSA_SemHndl *hndl = NULL;
//     int          ret = 0;

//     hndl = pvPortMalloc(sizeof(OSA_SemHndl));
//     if (hndl == NULL)
//     {
//         return QOSA_ERROR_NO_MEMORY;
//     }
//     memset(hndl, 0, sizeof(OSA_SemHndl));
//     ret = qosa_mutex_create(&hndl->lock);
//     if (ret != QOSA_OK)
//     {
//         vPortFree(hndl);
//         return ret;
//     }
//     hndl->count = initialCount;
//     hndl->maxCount = max_cnt;

//     hndl->sem = CreateSemaphore(NULL, initialCount, max_cnt, NULL);
//     if (hndl->sem == NULL)
//     {
//         qosa_mutex_delete(hndl->lock);
//         vPortFree(hndl);
//         return QOSA_ERROR_SEMA_CREATE_ERR;
//     }
//     *semaRef = hndl;
//     return QOSA_OK;
// }

int qosa_sem_wait(osa_sem_t semaRef, u32_t timeout)
{
    OSA_SemHndl *hndl = NULL;
    u32_t        ret = 0;
    u32_t        millisecond = 0;

    if (semaRef == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    hndl = (OSA_SemHndl *)semaRef;

    millisecond = timeout;
    ret = osSemaphoreAcquire(hndl->sem, millisecond);
    qosa_mutex_lock(hndl->lock, QOSA_WAIT_FOREVER);
    if (ret == QOSA_OK)
    {
        // Semaphore acquired successfully, count--
        hndl->count--;
        ret = QOSA_OK;
    }
    else if (ret == -2 || ret == -3) // osErrorTimeout
    {
        // Semaphore wait timeout
        ret = QOSA_ERROR_SEMA_TIMEOUT_ERR;
    }
    else
    {
        // printf("sem release error\n");
        // No other error definitions for now
        ret = QOSA_ERROR_SEMA_TIMEOUT_ERR;
    }
    qosa_mutex_unlock(hndl->lock);
    return ret;
}

int qosa_sem_release(osa_sem_t semaRef)
{
    OSA_SemHndl *hndl = NULL;
    int          ret = 0;

    if (semaRef == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    hndl = (OSA_SemHndl *)semaRef;
    qosa_mutex_lock(hndl->lock, QOSA_WAIT_FOREVER);

    // If semaphore count is less than maximum, allow releasing semaphore
    if (hndl->count < hndl->maxCount)
    {
        ret = osSemaphoreRelease(hndl->sem);
    }

    if (ret == QOSA_OK)
    {
        hndl->count++;
        ret = QOSA_OK;
    }
    else
    {
        ret = QOSA_ERROR_GENERAL;
    }
    qosa_mutex_unlock(hndl->lock);
    return ret;
}

int qosa_sem_get_cnt(osa_sem_t semaRef, u32_t *cnt_ptr)
{
    OSA_SemHndl *hndl = NULL;

    if (semaRef == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    hndl = (OSA_SemHndl *)semaRef;
    qosa_mutex_lock(hndl->lock, QOSA_WAIT_FOREVER);
    *cnt_ptr = hndl->count;
    qosa_mutex_unlock(hndl->lock);
    return QOSA_OK;
}

int qosa_sem_delete(osa_sem_t semaRef)
{
    OSA_SemHndl *hndl = NULL;

    if (semaRef == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    hndl = (OSA_SemHndl *)semaRef;

    // FIX: FAEDEVELOP-135, Jerry Chen modified, 2025-06-23
    osSemaphoreDelete(hndl->sem);
    qosa_mutex_delete(hndl->lock);
    vPortFree(hndl);
    return QOSA_OK;
}
