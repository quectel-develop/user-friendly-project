/*****************************************************************/ /**
* @file qosa_system.h
* @brief
* @author larson.li@quectel.com
* @date 2024-12-23
*
* @copyright Copyright (c) 2024 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description"
* <tr><td>2024-12-23 <td>1.0 <td>Larson.Li <td> Init
* </table>
**********************************************************************/

#ifndef __QOSA_SYSTEM_H__
#define __QOSA_SYSTEM_H__

#include "qosa_def.h"

/** osa error code base */
#define QOSA_ERRCODE_OS_BASE (QOSA_COMPONENT_OSI << 16)

/*! osa base component error */
typedef enum
{
    QOSA_ERROR_MSGQ_CREATE_ERR = 1 | QOSA_ERRCODE_OS_BASE,    /*!< Message queue creation failed */
    QOSA_ERROR_MSGQ_FULL_ERR,                                 /*!< message queue full error */
    QOSA_ERROR_MSGQ_RECV_ERR,                                 /*!< message queue failed to receive */
    QOSA_ERROR_MSGQ_TIMEOUT_ERR,                              /*!< message queue failed for timeout */
    QOSA_ERROR_MUTEX_CREATE_ERR = 100 | QOSA_ERRCODE_OS_BASE, /*!< Failed to create mutex lock */
    QOSA_ERROR_MUTEX_LOCK_ERR,                                /*!< Failed to lock the mutex */
    QOSA_ERROR_MUTEX_EBUSY_ERR,                               /*!< other task to use this mutex  */
    QOSA_ERROR_SEMA_CREATE_ERR = 200 | QOSA_ERRCODE_OS_BASE,  /*!< semaphore creation failed */
    QOSA_ERROR_SEMA_TIMEOUT_ERR,                              /*!< Timeout did not receive a valid semaphore */
    QOSA_ERROR_TASK_CREATE_ERR = 300 | QOSA_ERRCODE_OS_BASE,  /*!< Task creation failed */
    QOSA_ERROR_TIMER_CREATE_ERR = 400 | QOSA_ERRCODE_OS_BASE, /*!< Timer creation failed */
    QOSA_ERROR_TIMER_START_ERR,                               /*!< Timer start failed */
    QOSA_ERROR_TIMER_STOP_ERR,                                /*!< Timer stop failed */
    QOSA_ERROR_TIMER_DELETE_ERR,                              /*!< Timer deletion failed */
    QOSA_ERROR_EVENT_CREATE_ERR = 500 | QOSA_ERRCODE_OS_BASE, /*!< event create error*/
    QOSA_ERROR_EVENT_FULL_ERR,                                /*!< event full error*/
    QOSA_ERROR_EVENT_SIZE_ERR,                                /*!< size is not the same error */
    QOSA_ERROR_EVENT_TIMEOUT_ERR,                             /*!< Timeout for event */

    OSA_ERROR_MAX
} osa_eercode_os_e;

typedef void* osa_msgq_t;
typedef void* osa_mutex_t;
typedef void* osa_sem_t;
typedef void* osa_task_t;
typedef void* osa_timer_t;
typedef void* osa_event_t;
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
 *        - The function returns OSA_OK if the function executes successfully,
 *          otherwise it returns a negative number
 */
int qosa_msgq_create(osa_msgq_t* msgQRef, u32_t size, u32_t maxNumber);

/**
 * @brief 用于删除所创建的消息队列
 *
 * @param[in] osa_msgq_t msgQRef
 *          - 消息队列指针句柄
 *
 * @return int
 *        -  函数执行成功返回OSA_OK, 否则返回一个负数
 */
int qosa_msgq_delete(osa_msgq_t msgQRef);

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
 *       - 函数执行成功返回OSA_OK, 否则返回一个负数
 *
 * @note
 *     - 请注意，当消息队列已满时，发布可能会失败并返回负值
 */
int qosa_msgq_release(osa_msgq_t msgQRef, u32_t size, uint8_t* value, u32_t timeout);

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
int qosa_msgq_wait(osa_msgq_t msgQRef, uint8_t* value, u32_t size, u32_t timeout);

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
 *       - 函数执行成功返回OSA_OK, 否则返回一个负数
 */
int qosa_msgq_get_cnt(osa_msgq_t msgQRef, u32_t* cnt_ptr);

/**
 * @brief 创建系统定时器
 *
 * @param[in] osa_timer_t * timerRef
 *          - 系统定时器指针句柄
 *
 * @param[in] void * callBackRoutine
 *          - 用于定时器到达时间后,主动通知用户的函数
 *
 * @param[in] void * argv
 *          - 用户自定义callBackRoutine函数的入参
 *
 * @return int
 *        - 函数执行成功返回OSA_OK, 否则返回一个负数
 */
int qosa_timer_create(osa_timer_t* timerRef, void (*callBackRoutine)(void*), void* argv);

/**
 * @brief 控制系统定时器启动
 *
 * @param[in] osa_timer_t timerRef
 *          - 系统定时器指针句柄
 *
 * @param[in] u32_t set_Time
 *          - 设置定时器等待的时间间隔,单位ms
 *
 * @param[in] qosa_bool_t cyclicalEn
 *          - 为OSA_TRUE时表示为循环定时器, OSA_FALSE表示单次定时器
 *
 * @return int
 *       - 函数执行成功返回OSA_OK, 否则返回一个负数
 */
int qosa_timer_start(osa_timer_t timerRef, u32_t set_Time, qosa_bool_t cyclicalEn);

/**
 * @brief 控制系统定时器停止运行
 *
 * @param[in] osa_timer_t timerRef
 *          - 系统定时器指针句柄
 *
 * @return int
 *       - 函数执行成功返回OSA_OK, 否则返回一个负数
 */
int qosa_timer_stop(osa_timer_t timerRef);

/**
 * @brief 判断系统定时器是否正在工作
 *
 * @param[in] osa_timer_t timerRef
 *          - 系统定时器指针句柄
 *
 * @return qosa_bool_t
 *        - OSA_TRUE表示正在工作, OSA_FALSE表示未在工作
 */
qosa_bool_t qosa_timer_is_running(osa_timer_t timerRef);

/**
 * @brief 用于删除定时器,并释放定时器占用系统资源
 *
 * @param[in] osa_timer_t timerRef
 *          - 系统定时器指针句柄
 *
 * @return int
 *       - 函数执行成功返回OSA_OK, 否则返回一个负数
 *
 * @note
 *     - 使用前,请先使用 osa_osa_timerStop 停止定时器运行
 *
 * @see  osa_timer_stop , osa_timer_create
 */
int qosa_timer_delete(osa_timer_t timerRef);

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
int qosa_task_create(osa_task_t* taskRef, u32_t stackSize, uint8_t priority, char* taskName, void (*taskStart)(void*), void* argv, ...);

/**
 * @brief 线程停止并销毁函数
 *
 * @param[in] osa_task_t taskRef
 *          - 线程指针句柄
 *
 * @return int
 *        - 函数执行成功返回OSA_OK, 否则返回一个负数
 */
int qosa_task_delete(osa_task_t taskRef);

/**
 * @brief 获取当前线程运行状态
 *
 * @param[in] osa_task_t task_ref
 *          - 线程指针句柄
 *
 * @param[out] int32_t * status
 *           - 返回1 表示正在运行, 返回0 表示停止运行
 *
 * @return int
 *       - 函数执行成功返回OSA_OK, 否则返回一个负数
 */
int qosa_task_get_status(osa_task_t task_ref, int32_t* status);

/**
 * @brief 线程毫秒定时器
 *
 * @param[in] u32_t ms
 *          - 所要休眠的毫秒时间
 *
 */
void qosa_task_sleep_ms(u32_t ms);

/**
 * @brief 线程秒级定时器
 *
 * @param[in] u32_t s
 *          - 所要休眠的时间,单位秒
 *
 */
void qosa_task_sleep_sec(u32_t s);

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
osa_task_t qosa_task_get_current_ref(void);

u32_t qosa_task_get_stack_space(osa_task_t taskRef);
size_t qosa_task_get_free_heap_size(void);

osa_task_t qosa_task_get_task_id(void);
int qosa_task_exit(void);



/**
 * @brief Create a semaphore
 *
 * @param[in] osa_sem_t * semaRef
 *          - semaphore pointer handle
 *
 * @param[in] u32_t initialCount
 *          - Semaphore initialization, usually 0.
 *
 * @return int
 *        - The function returns OSA_OK if the function executes successfully,
 *          otherwise it returns a negative number
 */
int qosa_sem_create(osa_sem_t* semaRef, u32_t initialCount);

/**
 * @brief Waiting for the semaphore to arrive
 *
 * @param[in] osa_sem_t semaRef
 *          - semaphore pointer handle
 *
 * @param[in] u32_t timeout
 *          - OSA_WAIT_FOREVER, OSA_NO_WAIT, or timeout
 *
 * @return int
 */
int qosa_sem_wait(osa_sem_t semaRef, u32_t timeout);

/**
 * @brief Release a new semaphore, and the semaphore counts after execution++
 *
 * @param[in] osa_sem_t semaRef
 *          - semaphore pointer handle
 *
 * @return int
 *       - The function returns OSA_OK if the function executes successfully,
 *         otherwise it returns a negative number
 */
int qosa_sem_release(osa_sem_t semaRef);

/**
 * @brief Get the current count value of the semaphore
 *
 * @param[in] osa_sem_t semaRef
 *          - semaphore pointer handle
 *
 * @param[out] u32_t * cnt_ptr
 *           - Returns the actual semaphore count
 *
 * @return int
 *       - The function returns OSA_OK if the function executes successfully,
 *         otherwise it returns a negative number
 */
int qosa_sem_get_cnt(osa_sem_t semaRef, u32_t* cnt_ptr);

/**
 * @brief Delete the semaphore and free resources
 *
 * @param[in] osa_sem_t semaRef
 *          - semaphore pointer handle
 *
 * @return int
 *       - The function returns OSA_OK if the function executes successfully,
 *         otherwise it returns a negative number
 */
int qosa_sem_delete(osa_sem_t semaRef);

/**
 * @brief Create a mutex and initialize it
 *
 * @param[in] osa_mutex_t * mutexRef
 *          - Mutex pointer handle
 *
 * @return int
 *       - The function returns OSA_OK if the function executes successfully,
 *         otherwise it returns a negative number
 */
int qosa_mutex_create(osa_mutex_t* mutexRef);

/**
 * @brief If the current same thread has already acquired the lock, it can
 *        continue to add the lock. When other threads acquire the lock,
 *        it cannot add and return an error.
 *
 * @param[in] osa_mutex_t mutexRef
 *          - Mutex pointer handle
 *
 * @return int
 *       - The function returns OSA_OK if the function executes successfully,
 *         otherwise it returns a negative number
 */
int qosa_mutex_try_lock(osa_mutex_t mutexRef);

/**
 * @brief lock mutex
 *
 * @param[in] osa_mutex_t mutexRef
 *          - Mutex pointer handle
 *
 * @param[in] u32_t timeout
 *          - OSA_WAIT_FOREVER, OSA_NO_WAIT, or timeout
 *
 * @return int
 *       - The function returns OSA_OK if the function executes successfully,
 *         otherwise it returns a negative number
 */
int qosa_mutex_lock(osa_mutex_t mutexRef, u32_t timeout);

/**
 * @brief unlock mutex
 *
 * @param[in] osa_mutex_t mutexRef
 *          - Mutex pointer handle
 *
 * @return int
 *       - The function returns OSA_OK if the function executes successfully,
 *         otherwise it returns a negative number
 */
int qosa_mutex_unlock(osa_mutex_t mutexRef);

/**
 * @brief     Delete the mutex and free the resource
 *
 * @param     ql_mutex_t  mutexRef  Mutex pointer handle
 *
 * @return    int  The function returns OSA_OK if the function executes successfully,
 *                 otherwise it returns a negative number
 */
/**
 * @brief Delete the mutex and free the resource
 *
 * @param[in] osa_mutex_t mutexRef
 *          - Mutex pointer handle
 *
 * @return int
 *       - The function returns OSA_OK if the function executes successfully,
 *         otherwise it returns a negative number
 */
int qosa_mutex_delete(osa_mutex_t mutexRef);


int qosa_event_create(osa_event_t *eventRef);
int qosa_event_delete(osa_event_t eventRef);
int qosa_event_recv(osa_event_t eventRef, unsigned int event_mask, qosa_base_event_e option, int timeout);
int qosa_event_send(osa_event_t eventRef, u32_t flags);
int qosa_event_flags_get(osa_event_t eventRef);

#endif /* __QOSA_SYSTEM_H__ */
