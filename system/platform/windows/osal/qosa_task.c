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
#include "qosa_def.h"
#include "qosa_system.h"
#include "qosa_queue_list.h"
#include <windows.h>
#include "qosa_log.h"

#define QOSA_TASK_NAME_MAX     10  //task名字最大长度
#define QOSA_DESTROY_MSG_MAX   30  //销毁msg最大数量
#define QOSA_DESTROY_TASK_SIZE (8 * 1024)

typedef void* (*qosa_task_entry_func)(void*);
typedef void (*qosa_task_func_main)(void* argv);

typedef struct
{
    qosa_link_type_t    list;                          /*!< 存储线程信息 */
    HANDLE              task;                          /*!< 线程句柄 */
    DWORD               thread_id;                     /*!< 线程id */
    void*               user_argv;                     /*!< 线程入口函数参数 */
    char                task_name[QOSA_TASK_NAME_MAX]; /*!< 线程名称 */
    uint8_t             priority;                      /*!< 线程优先级 */
    qosa_task_func_main func_main;                     /*!< app入口函数 */
    osa_sem_t           sem;                           /*!< 用于同步数据 */
    u32_t            cur_state;                        /*!< 线程运行状态 */
} qosa_task_hndl;

/**
 * @brief 传递task handle
 */
typedef struct
{
    qosa_task_hndl* hndl;
} osa_destroy_msg_t;

static int32_t     g_task_init_flag = FALSE;  // 是否已经被初始化
static qosa_type_t g_task_list;               //保存task信息的链表
static osa_task_t  g_destroy_task = NULL;     // 销毁task
static osa_msgq_t  g_destroy_msg = NULL;      // 销毁队列

static void        qosa_task_self_destroy_handle(void* argv);
static qosa_bool_t pthread_compare_with_name(void* node1, void* node2);

/*********************************************************************************/
static qosa_bool_t pthread_compare_with_name(void* node1, void* node2)
{
    DWORD*          thread_id = (DWORD*)node2;
    qosa_task_hndl* hndl = (qosa_task_hndl*)node1;

    //LOG_D("%d,%d,%p", *thread_id, hndl->thread_id, hndl);
    if (*thread_id == hndl->thread_id)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief 子线程
 */
static void qosa_thread_func(LPVOID lpParam)
{
    qosa_task_hndl* hndl = (qosa_task_hndl*)lpParam;

    if (hndl == NULL)
    {
        LOG_D("hndl null");
        return;
    }
    if (hndl->func_main == NULL)
    {
        LOG_D("func_main null");
        return NULL;
    }

    //LOG_D("thread_id=%d", hndl->thread_id);
    // 等待 create 把线程信息存入链表后再执行
    qosa_sem_wait(hndl->sem, QOSA_WAIT_FOREVER);
    qosa_sem_delete(hndl->sem);
    hndl->sem = NULL;

    // 上层task入口
    hndl->func_main(hndl->user_argv);
    qosa_task_delete(hndl);
}

/**
 * @brief 销毁task,线程内核资源需要在其他线程释放
 */
static void qosa_task_self_destroy_handle(void* argv)
{
    int32_t           err = 0;
    osa_destroy_msg_t msg = {0};
    qosa_task_hndl*   hndl = NULL;
    qosa_task_hndl*   check_hndl = NULL;

    for (;;)
    {
        memset(&msg, 0, sizeof(osa_destroy_msg_t));
        err = qosa_msgq_wait(g_destroy_msg, (uint8_t*)&msg, sizeof(osa_destroy_msg_t), QOSA_WAIT_FOREVER);
        if (err != 0)
        {
            continue;
        }
        if (msg.hndl == NULL)
        {
            continue;
        }
        hndl = msg.hndl;
        // 检查等待销毁的task是否还在链表中,不存在则说明已经销毁了
        check_hndl = qosa_linear_search(&g_task_list, pthread_compare_with_name, (void*)&hndl->thread_id);
        if (check_hndl == NULL)
        {
            continue;
        }

        // 其他task销毁
        hndl->cur_state = 0;
        hndl->func_main = NULL;

        if (hndl->sem)
        {
            qosa_sem_delete(hndl->sem);
            hndl->sem = NULL;
        }

        CloseHandle(hndl->task);
        LOG_D("task destroy:%s", hndl->task_name);
        qosa_delete(&g_task_list, &hndl->list);
        free(hndl);
    }
}

/*********************************************************************************/

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
    int             status = QOSA_OK;
    qosa_task_hndl* hndl = NULL;

    //初始化销毁线程
    if (g_task_init_flag == FALSE)
    {
        qosa_init(&g_task_list);
        g_task_init_flag = TRUE;
        status |= qosa_task_create(&g_destroy_task, QOSA_DESTROY_TASK_SIZE, QOSA_PRIORITY_NORMAL, "destroy_task", qosa_task_self_destroy_handle, NULL);
        status |= qosa_msgq_create(&g_destroy_msg, sizeof(osa_destroy_msg_t), QOSA_DESTROY_MSG_MAX);
        if (status != QOSA_OK)
        {
            return QOSA_ERROR_GENERAL;
        }
    }

    hndl = (qosa_task_hndl*)malloc(sizeof(qosa_task_hndl));
    if (hndl == NULL)
    {
        LOG_D("malloc err");
        return QOSA_ERROR_NO_MEMORY;
    }
    memset(hndl, 0, sizeof(qosa_task_hndl));
    memcpy(hndl->task_name, taskName, MIN(QOSA_TASK_NAME_MAX, strlen(taskName)));
    hndl->user_argv = argv;
    hndl->priority = priority;
    hndl->func_main = taskStart;
    hndl->cur_state = 0;

    status = qosa_sem_create(&hndl->sem, 0);
    if (status != QOSA_OK)
    {
        LOG_D("sem create error");
        goto exit;
    }

    //创建线程并返回线程伪句柄
    hndl->task = CreateThread(NULL, stackSize, qosa_thread_func, hndl, 0, NULL);
    if (hndl->task == NULL)
    {
        DWORD error = GetLastError();
        LOG_D("task create error=%d", error);
        status = QOSA_ERROR_MSGQ_CREATE_ERR;
        goto exit;
    }

    // 修改线程优先级
    if (!SetThreadPriority(hndl->task, priority))
    {
        DWORD error = GetLastError();
        LOG_D("Error setting priority for error=%d", error);
    }

    hndl->cur_state = 1;
    //线程 id 是唯一的,线程伪句柄是不唯一的
    hndl->thread_id = GetThreadId(hndl->task);

    // 保存到链表,用作查询
    qosa_link(hndl, &hndl->list);
    qosa_put(&g_task_list, &hndl->list);

    *taskRef = hndl;
    qosa_sem_release(hndl->sem);
    return;
exit:
    if (hndl == NULL)
    {
        return;
    }
    if (hndl->sem)
    {
        qosa_sem_delete(hndl->sem);
        hndl->sem = NULL;
    }
    free(hndl);
    return QOSA_OK;
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
    qosa_task_hndl*   hndl = NULL;
    int               err = 0;
    osa_destroy_msg_t msg = {0};

    if (taskRef == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }

    hndl = (qosa_task_hndl*)taskRef;

    hndl->cur_state = 0;
    hndl->func_main = NULL;

    if (hndl->sem)
    {
        qosa_sem_delete(hndl->sem);
        hndl->sem = NULL;
    }

    // 伪句柄
    DWORD thread_id = GetCurrentThreadId();
    if (thread_id == hndl->thread_id)
    {
        // 在当前线程自毁
        msg.hndl = hndl;
        err = qosa_msgq_release(g_destroy_msg, sizeof(osa_destroy_msg_t), (uint8_t*)&msg, QOSA_NO_WAIT);
        if (err != 0)
        {
            LOG_D("msg release fail");
            return -1;
        }
        //结束当前线程,可以不调用，默认return后线程执行完
        ExitThread(0);
    }
    else
    {
        // 强制结束其他线程
        TerminateThread(hndl->task, 0);
        // 释放资源
        CloseHandle(hndl->task);
        qosa_delete(&g_task_list, &hndl->list);
        LOG_D("task destroy:%s", hndl->task_name);
        free(hndl);
    }
}

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
int qosa_task_get_status(osa_task_t task_ref, int32_t* status)
{
    qosa_task_hndl* hndl = NULL;

    if (task_ref == NULL)
    {
        *status = 0;
        return QOSA_ERROR_PARAM_IS_NULL;
    }

    hndl = (qosa_task_hndl*)task_ref;
    *status = hndl->cur_state;
    return QOSA_OK;
}

/**
 * @brief 线程毫秒定时器
 *
 * @param[in] u32_t ms
 *          - 所要休眠的毫秒时间
 *
 */
void qosa_task_sleep_ms(u32_t ms)
{
    Sleep(ms);
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
    Sleep(s * 1000);
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
    qosa_task_hndl* hndl = NULL;
    DWORD           thread_id = 0;

    thread_id = GetCurrentThreadId();
    hndl = qosa_linear_search(&g_task_list, pthread_compare_with_name, (void*)&thread_id);
    if (hndl != NULL)
    {
        return hndl;
    }
    return NULL;
}

int qosa_task_get_stack_space(osa_task_t taskRef)
{
    return 0;
}

int qosa_task_exit(void)
{
    return 0;
}

int qosa_task_get_free_heap_size(void)
{
    return 0;
}