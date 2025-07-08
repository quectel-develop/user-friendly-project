/*****************************************************************/ /**
* @file qosa_temer.c
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

typedef void (*OSA_TimeFncMain)(void* argv);

typedef struct
{
    HANDLE          timerid;         /*!< 定时器句柄 */
    void*           userArgv;        /*!< 用户参数 */
    qosa_bool_t     cyclicalEn;      /*!< 是否为循环执行函数 */
    qosa_bool_t     status;          /*!< 当前 cb执行状态 */
    qosa_bool_t     func_is_running; /*!< CB 函数正在运行 */
    qosa_bool_t     force_exit;      /*!< 强制退出 */
    OSA_TimeFncMain fncMain;         /*!< 回调函数 */
} OSA_TimerHndl;

/*********************************************************************************/

static void CALLBACK TimerProc(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
    OSA_TimerHndl* hndl = NULL;

    if (lpArg == NULL)
    {
        return;
    }
    hndl = (OSA_TimerHndl*)lpArg;
    hndl->func_is_running = TRUE;
    hndl->fncMain(hndl->userArgv);
    hndl->func_is_running = FALSE;
    //如果非循环运行函数执行之后则状态为false
    if (hndl->cyclicalEn == FALSE)
    {
        hndl->status = FALSE;
    }
}

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
 *        - 函数执行成功返回QOSA_OK, 否则返回一个负数
 */
int qosa_timer_create(osa_timer_t* timerRef, void (*callBackRoutine)(void*), void* argv)
{
    OSA_TimerHndl* hndl = NULL;

    if (timerRef == NULL || callBackRoutine == NULL)
    {
        return QOSA_ERROR_PARAM_INVALID;
    }
    hndl = (OSA_TimerHndl*)malloc(sizeof(OSA_TimerHndl));
    if (hndl == NULL)
    {
        return QOSA_ERROR_NO_MEMORY;
    }

    hndl->fncMain = callBackRoutine;
    hndl->userArgv = argv;

    // 精确到 ms 级
    hndl->timerid = CreateWaitableTimer(NULL, FALSE, NULL);
    if (hndl->timerid == NULL)
    {
        printf("creat timer fail");
        QOSA_ERROR_TIMER_CREATE_ERR;
    }

    *timerRef = hndl;

    return QOSA_OK;
}

/*********************************************************************************/

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
 *          - 为QOSA_TRUE时表示为循环定时器, QOSA_FALSE表示单次定时器
 *
 * @return int
 *       - 函数执行成功返回QOSA_OK, 否则返回一个负数
 */
int qosa_timer_start(osa_timer_t timerRef, u32_t set_Time, qosa_bool_t cyclicalEn)
{
    OSA_TimerHndl* hndl = NULL;
    LARGE_INTEGER  liDueTime = {0};
    u32_t       lPeriod = 0;

    if (timerRef == NULL)
    {
        return QOSA_ERROR_PARAM_IS_NULL;
    }
    hndl = (OSA_TimerHndl*)timerRef;

    //是否打开循环定时器,如果不打开则直接配置为0即可，如果打开则要配置相同的时间间隔
    if (cyclicalEn == TRUE)
    {
        lPeriod = set_Time;
    }
    else
    {
        lPeriod = 0;
    }

    // pDueTime: 初次触发的时间。
    // 绝对时间： 如果为正值，表示以 100 纳秒为单位的 UTC 时间点（从 1601-01-01 开始）。
    // 相对时间： 如果为负值，表示以当前时间为基准的偏移时间（仍然是 100 纳秒为单位）。
    // lPeriod: 触发的间隔时间，单位是毫秒。如果为 0，表示单次触发。如果设置为非零值，则定时器会以该周期反复触发。
    // pfnCompletionRoutine: 定时器触发时调用的回调函数，可选。
    // fResume 表示定时器是否能够唤醒挂起的系统。如果为 TRUE，则当定时器触发时，可以唤醒挂起的系统（如休眠状态）。

    liDueTime.QuadPart = -(set_Time * 10000);  // 1ms=1000000ns

    if (!SetWaitableTimer(hndl->timerid, &liDueTime, lPeriod, TimerProc, hndl, FALSE))
    {
        printf("设置定时器失败\n");
        CloseHandle(hndl->timerid);
        return QOSA_ERROR_TIMER_START_ERR;
    }

    hndl->status = TRUE;

    return QOSA_OK;
}

/**
 * @brief 控制系统定时器停止运行
 *
 * @param[in] osa_timer_t timerRef
 *          - 系统定时器指针句柄
 *
 * @return int
 *       - 函数执行成功返回QOSA_OK, 否则返回一个负数
 */
int qosa_timer_stop(osa_timer_t timerRef)
{
    OSA_TimerHndl* hndl = NULL;

    if (timerRef == NULL)
    {
        return QOSA_ERROR_PARAM_IS_NULL;
    }
    hndl = (OSA_TimerHndl*)timerRef;

    if (CancelWaitableTimer(hndl->timerid) != TRUE)
    {
        printf("fail to timer_start");
        return QOSA_ERROR_TIMER_STOP_ERR;
    }
    hndl->status = FALSE;

    return QOSA_OK;
}

/**
 * @brief 判断系统定时器是否正在工作
 *
 * @param[in] osa_timer_t timerRef
 *          - 系统定时器指针句柄
 *
 * @return qosa_bool_t
 *        - QOSA_TRUE表示正在工作, QOSA_FALSE表示未在工作
 */
qosa_bool_t qosa_timer_is_running(osa_timer_t timerRef)
{
    OSA_TimerHndl* hndl = NULL;

    if (timerRef == NULL)
    {
        return FALSE;
    }
    hndl = (OSA_TimerHndl*)timerRef;

    return hndl->status;
}

/**
 * @brief 用于删除定时器,并释放定时器占用系统资源
 *
 * @param[in] osa_timer_t timerRef
 *          - 系统定时器指针句柄
 *
 * @return int
 *       - 函数执行成功返回QOSA_OK, 否则返回一个负数
 *
 * @note
 *     - 使用前,请先使用 osa_osa_timerStop 停止定时器运行
 *
 * @see  osa_timer_stop , osa_timer_create
 */
int qosa_timer_delete(osa_timer_t timerRef)
{
    OSA_TimerHndl* hndl = NULL;

    if (timerRef == NULL)
    {
        return QOSA_ERROR_PARAM_IS_NULL;
    }
    hndl = (OSA_TimerHndl*)timerRef;

    if (hndl->func_is_running == TRUE)
    {
        hndl->status = FALSE;
        hndl->force_exit = TRUE;
        return QOSA_OK;
    }

    CancelWaitableTimer(hndl->timerid);
    CloseHandle(hndl->timerid);
    free(hndl);

    return QOSA_OK;
}
