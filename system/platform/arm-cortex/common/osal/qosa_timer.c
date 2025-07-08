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
    void*           timerid;         /*!< 定时器句柄 */
    void*           userArgv;        /*!< 用户参数 */
    qosa_bool_t     cyclicalEn;      /*!< 是否为循环执行函数 */
    qosa_bool_t     status;          /*!< 当前 cb执行状态 */
    qosa_bool_t     func_is_running; /*!< CB 函数正在运行 */
    qosa_bool_t     force_exit;      /*!< 强制退出 */
    OSA_TimeFncMain fncMain;         /*!< 回调函数 */
} OSA_TimerHndl;

/*********************************************************************************/

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
    return 1;
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

    return QOSA_OK;
}
