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
    void*           timerid;         /*!< Timer handle */
    void*           userArgv;        /*!< User parameters */
    qosa_bool_t     cyclicalEn;      /*!< Whether it is a cyclic execution function */
    qosa_bool_t     status;          /*!< Current callback execution status */
    qosa_bool_t     func_is_running; /*!< Callback function is running */
    qosa_bool_t     force_exit;      /*!< Force exit */
    OSA_TimeFncMain fncMain;         /*!< Callback function */
} OSA_TimerHndl;

/*********************************************************************************/

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
    return 1;
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

    return QOSA_OK;
}
