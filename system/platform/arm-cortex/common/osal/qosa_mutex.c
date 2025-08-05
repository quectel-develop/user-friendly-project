/*****************************************************************/ /**
* @file qosa_mutex.c
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

int qosa_mutex_create(osa_mutex_t *mutexRef)
{
    osa_mutex_t              hMutex = NULL;

    osMutexAttr_t mutex_attr =
    {
        .name = "MyRecursiveMutex",
        .attr_bits = osMutexRecursive
    };
    hMutex = osMutexNew(&mutex_attr);
    if (hMutex == NULL)
    {
        return QOSA_ERROR_MUTEX_CREATE_ERR;
    }
    *mutexRef = hMutex;
    return QOSA_OK;
}

int qosa_mutex_lock(osa_mutex_t mutexRef, u32_t timeout)
{
    u32_t ret = 0;
    u32_t millisecond = 0;

    if (mutexRef == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    millisecond = timeout;
    ret = osMutexAcquire(mutexRef, millisecond);
    if (ret == 0)
    {
        return QOSA_OK;
    }
    else if(ret == -2)
    {
        return QOSA_ERROR_MSGQ_TIMEOUT_ERR;
    }
    else
    {
        return QOSA_ERROR_MUTEX_LOCK_ERR;
    }
}

int qosa_mutex_try_lock(osa_mutex_t mutexRef)
{
    u32_t ret = 0;

    if (mutexRef == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    ret = osMutexAcquire(mutexRef, 0);
    if (ret == 0)
    {
        return QOSA_OK;
    }
    else
    {
        return QOSA_ERROR_MUTEX_LOCK_ERR;
    }
}

int qosa_mutex_unlock(osa_mutex_t mutexRef)
{
    if (mutexRef == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    osMutexRelease(mutexRef);
    return QOSA_OK;
}

int qosa_mutex_delete(osa_mutex_t mutexRef)
{
    if (mutexRef == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    osMutexDelete(mutexRef);
    return QOSA_OK;
}
