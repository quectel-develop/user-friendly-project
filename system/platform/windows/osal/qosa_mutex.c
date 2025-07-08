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
#include <windows.h>

int qosa_mutex_create(osa_mutex_t *mutexRef)
{
    HANDLE              hMutex = NULL;

    hMutex = CreateMutex(NULL, FALSE, NULL);
    if (hMutex == NULL)
    {
        return QOSA_ERROR_MUTEX_CREATE_ERR;
    }
    *mutexRef = hMutex;
    return QOSA_OK;
}

int qosa_mutex_lock(osa_mutex_t mutexRef, u32_t timeout)
{
    DWORD ret = 0;
    DWORD millisecond = 0;

    if (mutexRef == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    millisecond = timeout;
    if (timeout == QOSA_WAIT_FOREVER)
    {
        millisecond = INFINITE;
    }
    ret = WaitForSingleObject(mutexRef, millisecond);
    if (ret == WAIT_OBJECT_0)
    {
        return QOSA_OK;
    }
    else if(ret == WAIT_TIMEOUT)
    {
        return QOSA_ERROR_MUTEX_EBUSY_ERR;
    }
    else
    {
        return QOSA_ERROR_MUTEX_LOCK_ERR;
    }
}

int qosa_mutex_try_lock(osa_mutex_t mutexRef)
{
    DWORD ret = 0;

    if (mutexRef == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    ret = WaitForSingleObject(mutexRef, INFINITE);
    if (ret == WAIT_OBJECT_0)
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
    ReleaseMutex(mutexRef);
    return QOSA_OK;
}

int qosa_mutex_delete(osa_mutex_t mutexRef)
{
    if (mutexRef == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    CloseHandle(mutexRef);
    return QOSA_OK;
}
