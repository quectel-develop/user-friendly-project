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
 * @brief Used to delete the created message queue
 *
 * @param[in] osa_msgq_t msgQRef
 *          - Message queue pointer handle
 *
 * @return int
 *        - Returns OSA_OK if the function executes successfully, otherwise returns a negative number
 */
int qosa_msgq_delete(osa_msgq_t msgQRef);

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
 *       - Returns OSA_OK if the function executes successfully, otherwise returns a negative number
 *
 * @note
 *     - Note that publishing may fail and return a negative value when the message queue is full
 */
int qosa_msgq_release(osa_msgq_t msgQRef, u32_t size, uint8_t* value, u32_t timeout);

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
int qosa_msgq_wait(osa_msgq_t msgQRef, uint8_t* value, u32_t size, u32_t timeout);

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
 *       - Returns OSA_OK if the function executes successfully, otherwise returns a negative number
 */
int qosa_msgq_get_cnt(osa_msgq_t msgQRef, u32_t* cnt_ptr);

/**
 * @brief Creates a system timer
 *
 * @param[in] osa_timer_t * timerRef
 *          - System timer pointer handle
 *
 * @param[in] void * callBackRoutine
 *          - Function to actively notify the user when the timer expires
 *
 * @param[in] void * argv
 *          - User-defined parameter for the callBackRoutine function
 *
 * @return int
 *        - Returns OSA_OK if the function executes successfully, otherwise returns a negative number
 */
int qosa_timer_create(osa_timer_t* timerRef, void (*callBackRoutine)(void*), void* argv);

/**
 * @brief Controls the system timer to start
 *
 * @param[in] osa_timer_t timerRef
 *          - System timer pointer handle
 *
 * @param[in] u32_t set_Time
 *          - Sets the timer wait interval, unit: ms
 *
 * @param[in] qosa_bool_t cyclicalEn
 *          - OSA_TRUE indicates a cyclic timer, OSA_FALSE indicates a one-shot timer
 *
 * @return int
 *       - Returns OSA_OK if the function executes successfully, otherwise returns a negative number
 */
int qosa_timer_start(osa_timer_t timerRef, u32_t set_Time, qosa_bool_t cyclicalEn);

/**
 * @brief Controls the system timer to stop running
 *
 * @param[in] osa_timer_t timerRef
 *          - System timer pointer handle
 *
 * @return int
 *       - Returns OSA_OK if the function executes successfully, otherwise returns a negative number
 */
int qosa_timer_stop(osa_timer_t timerRef);

/**
 * @brief Checks if the system timer is running
 *
 * @param[in] osa_timer_t timerRef
 *          - System timer pointer handle
 *
 * @return qosa_bool_t
 *        - OSA_TRUE indicates it is running, OSA_FALSE indicates it is not running
 */
qosa_bool_t qosa_timer_is_running(osa_timer_t timerRef);

/**
 * @brief Used to delete the timer and release system resources occupied by the timer
 *
 * @param[in] osa_timer_t timerRef
 *          - System timer pointer handle
 *
 * @return int
 *       - Returns OSA_OK if the function executes successfully, otherwise returns a negative number
 *
 * @note
 *     - Before use, please use osa_osa_timerStop to stop the timer first
 *
 * @see  osa_timer_stop , osa_timer_create
 */
int qosa_timer_delete(osa_timer_t timerRef);

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
int qosa_task_create(osa_task_t* taskRef, u32_t stackSize, uint8_t priority, char* taskName, void (*taskStart)(void*), void* argv, ...);

/**
 * @brief Thread stop and destroy function
 *
 * @param[in] osa_task_t taskRef
 *          - Thread pointer handle
 *
 * @return int
 *        - Returns OSA_OK if the function executes successfully, otherwise returns a negative number
 */
int qosa_task_delete(osa_task_t taskRef);

/**
 * @brief Gets the current thread running status
 *
 * @param[in] osa_task_t task_ref
 *          - Thread pointer handle
 *
 * @param[out] int32_t * status
 *           - Returns 1 if running, 0 if stopped
 *
 * @return int
 *       - Returns OSA_OK if the function executes successfully, otherwise returns a negative number
 */
int qosa_task_get_status(osa_task_t task_ref, int32_t* status);

/**
 * @brief Thread millisecond timer
 *
 * @param[in] u32_t ms
 *          - Millisecond time to sleep
 *
 */
void qosa_task_sleep_ms(u32_t ms);

/**
 * @brief Thread second-level timer
 *
 * @param[in] u32_t s
 *          - Time to sleep, unit: seconds
 *
 */
void qosa_task_sleep_sec(u32_t s);

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
