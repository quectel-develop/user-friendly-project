/*****************************************************************/ /**
* @file qosa_task.c
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
#include "qosa_queue_list.h"
#include "qosa_log.h"

/**
 * @brief Thread creation and initialization function
 *
 * @param[in] osa_task_t * taskRef
 *          - Thread pointer handle
 *
 * @param[in] u32_t stackSize
 *          - Stack size of the created thread
 *
 * @param[in] uint8_t priority
 *          - Priority of the created thread
 *
 * @param[in] char * taskName
 *          - Name of the created thread
 *
 * @param[in] void * taskStart
 *          - Entry function of the new thread after successful creation
 *
 * @param[in] void * argv
 *          - Custom parameters to be passed to the new thread's entry function
 *
 * @return int
 *       - Returns OSA_OK if the function executes successfully, otherwise returns a negative number
 */
int qosa_task_create(osa_task_t* taskRef, u32_t stackSize, uint8_t priority, char* taskName, void (*taskStart)(void*), void* argv, ...)
{
    int ret = QOSA_OK;

    osThreadAttr_t attr = {.name = taskName, .stack_size = stackSize, .priority = (osPriority_t)priority};
    *taskRef = osThreadNew(taskStart, argv, &attr);
    if (*taskRef == NULL)
    {
        ret = QOSA_ERROR_NO_MEMORY;
        LOG_E("Create task failed...QOSA_ERROR_NO_MEMORY");
    }

    return ret;
}

/**
 * @brief Thread stop and destroy function
 *
 * @param[in] osa_task_t taskRef
 *          - Thread pointer handle
 *
 * @return int
 *        - Returns OSA_OK if the function executes successfully, otherwise returns a negative number
 *
 * @note  When executing this function, ensure that all resources applied for in the thread have been released, otherwise it may cause memory leaks.
 */
int qosa_task_delete(osa_task_t taskRef)
{
    return osThreadTerminate(taskRef);
}

osa_task_t qosa_task_get_task_id(void)
{
    return osThreadGetId();
}


u32_t tick_from_millisecond(s32_t ms)
{
    u32_t tick;

    if (ms < 0)
    {
        tick = (u32_t)QOSA_WAIT_FOREVER;
    }
    else
    {
        tick = RT_TICK_PER_SECOND * (ms / 1000);
        tick += (RT_TICK_PER_SECOND * (ms % 1000) + 999) / 1000;
    }

    /* return the calculated tick */
    return tick;
};

/**
 * @brief Thread millisecond timer
 *
 * @param[in] u32_t ms
 *          - Millisecond time to sleep
 */
void qosa_task_sleep_ms(u32_t ms)
{
    osDelay(tick_from_millisecond(ms));
}

/**
 * @brief Thread second timer
 *
 * @param[in] u32_t s
 *          - Time to sleep, unit: seconds
 */
void qosa_task_sleep_sec(u32_t s)
{
    osDelay(tick_from_millisecond(s*1000));
}

/**
 * @brief Gets the current running task pointer handle
 *
 * @param[out] osa_task_t * taskRef
 *           - Returns the current thread pointer handle
 *
 * @return int
 *       - 0 indicates successful execution
 *       - Other values indicate execution failure
 */
osa_task_t qosa_task_get_current_ref(void)
{
    return osThreadGetId();
}

u32_t qosa_task_get_stack_space(osa_task_t taskRef)
{
    return osThreadGetStackSpace(taskRef);
}

int qosa_task_exit(void)
{
    osThreadExit();
    return 0;
}

size_t qosa_task_get_free_heap_size(void)
{
    return xPortGetFreeHeapSize();
}
