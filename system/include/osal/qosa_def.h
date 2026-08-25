/*****************************************************************/ /**
* @file qosa_def.h
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

#ifndef _QOSA_DEF_H__
#define _QOSA_DEF_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#include "windows.h"
#elif __linux__
#else
#include "cmsis_os2.h"
#endif

typedef signed    int                   qosa_bool_t; /**< boolean type */
typedef uint8_t   u8_t;
typedef int8_t    s8_t;
typedef uint16_t  u16_t;
typedef int16_t   s16_t;
typedef uint32_t  u32_t;
typedef int32_t   s32_t;

#ifdef ARCH_CPU_64BIT
typedef signed long                     s64_t;      /**< 64bit integer type */
typedef unsigned long                   u64_t;      /**< 64bit unsigned integer type */
#else
typedef signed long long                s64_t;      /**< 64bit integer type */
typedef unsigned long long              u64_t;      /**< 64bit unsigned integer type */
#endif


#define QOSA_TRUE         1
#define QOSA_FALSE        0

#define QOSA_SUCCESS      0
#define QOSA_FAILURE      -1

#define QOSA_NO_WAIT      0                 //no timeout
#define QOSA_WAIT_FOREVER (-1)              //wait forever
#define QOSA_MAX_DELAY    (0xffffffffUL)    //max delay


#define QOSA_NULL         0
#define QOSA_INVALID_ADDRESS        0xdddddddd

#define QOSA_UNUSED(x)    ((void)x)         //Fix the false warning for the unused defined parameters

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

#ifndef ALIGN_UP
#define ALIGN_UP(x, a) ( ( ((x) + ((a) - 1) ) / a ) * a )
#endif

#ifndef ALIGN_DOWN
#define ALIGN_DOWN(x, a) ( ( (x) / (a)) * (a) )
#endif


#define QOSA_HW_INTERRUPT_DISABLE()      NULL
#define QOSA_HW_INTERRUPT_ENABLE(level)  NULL

#define QOSA_PARAMETER_RANGE_CHECK(value, min, max, default)  ({if (((value) < (min)) || ((value) > (max))) {(value) = default;LOG_W("The parameter is out of bounds. Restore the default value");}})


#define RT_TICK_PER_SECOND	           (1000) //configTICK_RATE_HZ     /* Tick per Second*/


#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#define QOSA_ASSERT(a)                                                                                                                                         \
    if (!(a))                                                                                                                                                  \
    {                                                                                                                                                          \
        OutputDebugString("Assertion failed: " #a "\n");                                                                                                       \
        DebugBreak();                                                                                                                                          \
    }
#elif __linux__
#else
#include "assert.h"
#define QOSA_ASSERT(a)       ({if (!(a)) {assert(0);}})
#endif

/*! base event mask */
typedef enum
{
    QOSA_EVENT_FLAG_OR = 1,          /**< logic or */
    QOSA_EVENT_FLAG_AND = 2,         /**< logic and */
    QOSA_EVENT_FLAG_NO_CLEAR = 4,    /**< no clear flag */
} qosa_base_event_e;


/*! base component error mask */
typedef enum
{
    QOSA_COMPONENT_NONE = 0,
    QOSA_COMPONENT_COMMON = 0x8000, /*!< system common error base */
    QOSA_COMPONENT_OSI = 0x8000,    /*!< system qosa error base */
    OSA_COMPONENT_MAX = 0x9FFF      /*!< component base end */
} qosa_base_component_e;

/*! common error code */
typedef enum
{
    QOSA_OK = 0,
    QOSA_ERROR_GENERAL = 1 | (QOSA_COMPONENT_COMMON << 16), /*!< generic error,this erroris only returned when a special exception occurs */
    QOSA_ERROR_NO_MEMORY,                                   /*!< memory malloc failed */
    QOSA_ERROR_PARAM_INVALID,                               /*!< Parameter input error */
    QOSA_ERROR_PARAM_IS_NULL,                               /*!< Parameter is empty error */
    QOSA_ERROR_TIMEOUT,                                     /*!< timeout */
    QOSA_ERROR_BUSY,                                        /*!< busy */
} qosa_eercode_common_e;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
typedef enum
{
    QOSA_PRIORITY_REALTIME = THREAD_PRIORITY_TIME_CRITICAL,
    QOSA_PRIORITY_HIGH = THREAD_PRIORITY_HIGHEST,
    QOSA_PRIORITY_ABOVE_NORMAL = THREAD_PRIORITY_ABOVE_NORMAL,
    QOSA_PRIORITY_NORMAL = THREAD_PRIORITY_NORMAL,  //Common priority level
    QOSA_PRIORITY_BELOW_NORMAL = THREAD_PRIORITY_BELOW_NORMAL,
    QOSA_PRIORITY_LOW = THREAD_PRIORITY_LOWEST,
    QOSA_PRIORITY_IDLE = THREAD_PRIORITY_IDLE,
} qosa_thread_priority_e;
#else
typedef enum
{
    QOSA_PRIORITY_REALTIME = osPriorityRealtime,
    QOSA_PRIORITY_HIGH = osPriorityHigh,
    QOSA_PRIORITY_ABOVE_NORMAL = osPriorityAboveNormal,
    QOSA_PRIORITY_NORMAL = osPriorityNormal,    //Common priority level
    QOSA_PRIORITY_BELOW_NORMAL = osPriorityBelowNormal,
    QOSA_PRIORITY_LOW = osPriorityLow,
    QOSA_PRIORITY_IDLE = osPriorityIdle,
} qosa_thread_priority_e;
#endif

#endif /* _QOSA_DEF_H__ */
