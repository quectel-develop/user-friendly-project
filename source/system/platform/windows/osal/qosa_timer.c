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
    HANDLE          timerid;         /*!< Timer handle */
    void*           userArgv;        /*!< User parameters */
    qosa_bool_t     cyclicalEn;      /*!< Whether it is a cyclic execution function */
    qosa_bool_t     status;          /*!< Current callback execution status */
    qosa_bool_t     func_is_running; /*!< Callback function is running */
    qosa_bool_t     force_exit;      /*!< Force exit */
    OSA_TimeFncMain fncMain;         /*!< Callback function */
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
    // If it's not a cyclic execution function, set status to false after execution
    if (hndl->cyclicalEn == FALSE)
    {
        hndl->status = FALSE;
    }
}

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
 *        - Returns QOSA_OK if the function executes successfully, otherwise returns a negative number
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

    // Accurate to millisecond level
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
 * @brief Controls the system timer to start
 *
 * @param[in] osa_timer_t timerRef
 *          - System timer pointer handle
 *
 * @param[in] u32_t set_Time
 *          - Sets the timer wait interval, unit: ms
 *
 * @param[in] qosa_bool_t cyclicalEn
 *          - QOSA_TRUE indicates a cyclic timer, QOSA_FALSE indicates a one-shot timer
 *
 * @return int
 *       - Returns QOSA_OK if the function executes successfully, otherwise returns a negative number
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

    // Whether to enable cyclic timer, if not set to 0 directly, if enabled configure the same time interval
    if (cyclicalEn == TRUE)
    {
        lPeriod = set_Time;
    }
    else
    {
        lPeriod = 0;
    }

    // pDueTime: Initial trigger time.
    // Absolute time: If positive, represents UTC time point in 100 nanosecond units (starting from 1601-01-01).
    // Relative time: If negative, represents offset time from current time (still in 100 nanosecond units).
    // lPeriod: Trigger interval time in milliseconds. If 0, indicates one-time trigger. If set to non-zero value, the timer will trigger repeatedly at this period.
    // pfnCompletionRoutine: Callback function called when timer triggers, optional.
    // fResume indicates whether the timer can wake up a suspended system. If TRUE, when the timer triggers, it can wake up a suspended system (like sleep state).

    liDueTime.QuadPart = -(set_Time * 10000);  // 1ms=1000000ns

    if (!SetWaitableTimer(hndl->timerid, &liDueTime, lPeriod, TimerProc, hndl, FALSE))
    {
        printf("Failed to set timer\n");
        CloseHandle(hndl->timerid);
        return QOSA_ERROR_TIMER_START_ERR;
    }

    hndl->status = TRUE;

    return QOSA_OK;
}

/**
 * @brief Controls the system timer to stop running
 *
 * @param[in] osa_timer_t timerRef
 *          - System timer pointer handle
 *
 * @return int
 *       - Returns QOSA_OK if the function executes successfully, otherwise returns a negative number
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
        printf("Failed to stop timer");
        return QOSA_ERROR_TIMER_STOP_ERR;
    }
    hndl->status = FALSE;

    return QOSA_OK;
}

/**
 * @brief Checks if the system timer is running
 *
 * @param[in] osa_timer_t timerRef
 *          - System timer pointer handle
 *
 * @return qosa_bool_t
 *        - QOSA_TRUE indicates it is running, QOSA_FALSE indicates it is not running
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
 * @brief Used to delete the timer and release system resources occupied by the timer
 *
 * @param[in] osa_timer_t timerRef
 *          - System timer pointer handle
 *
 * @return int
 *       - Returns QOSA_OK if the function executes successfully, otherwise returns a negative number
 *
 * @note
 *     - Before use, please use osa_osa_timerStop to stop the timer first
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
