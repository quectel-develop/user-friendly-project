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
 * @brief 线程创建并初始化函数
 *
 * @param[in] osa_task_t * taskRef
 *          - 线程指针句柄
 *
 * @param[in] u32_t stackSize
 *          - 创建线程的栈空间大小
 *
 * @param[in] uint8_t priority
 *          - 创建线程的优先级
 *
 * @param[in] char * taskName
 *          - 创建线程的名称
 *
 * @param[in] void * taskStart
 *          - 线程创建成功后,新线程的入口函数
 *
 * @param[in] void * argv
 *          - 要传递给新线程入口函数的自定义参数
 *
 * @return int
 *       - 函数执行成功返回OSA_OK, 否则返回一个负数
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
 * @brief 线程停止并销毁函数
 *
 * @param[in] osa_task_t taskRef
 *          - 线程指针句柄
 *
 * @return int
 *        - 函数执行成功返回OSA_OK, 否则返回一个负数
 *
 * @note  注意在执行此函数时，需要保证在线程中申请的资源都已经释放，否则可能会引起内存泄露。
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
 * @brief 线程毫秒定时器
 *
 * @param[in] u32_t ms
 *          - 所要休眠的毫秒时间
 *
 */
void qosa_task_sleep_ms(u32_t ms)
{
    osDelay(tick_from_millisecond(ms));
}

/**
 * @brief 线程秒级定时器
 *
 * @param[in] u32_t s
 *          - 所要休眠的时间,单位秒
 *
 */
void qosa_task_sleep_sec(u32_t s)
{
    osDelay(tick_from_millisecond(s*1000));
}

/**
 * @brief 获取当前运行的task指针句柄
 *
 * @param[out] osa_task_t * taskRef
 *           - 返回的当前线程指针句柄
 *
 * @return int
 *       - 0 返回执行成功
 *       - 其他 表示执行失败
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
