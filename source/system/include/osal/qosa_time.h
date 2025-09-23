/*****************************************************************/ /**
* @file qosa_time.h
* @brief
* @author wells.li@quectel.com
* @date 2025-07-01
*
* @copyright Copyright (c) 2024 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description"
* <tr><td>2025-07-01 <td>1.0 <td>Wells.Li <td> Init
* </table>
**********************************************************************/
#ifndef _QOSA_TIME_H__
#define _QOSA_TIME_H__

#include <time.h>

time_t time(time_t *t);

void update_ntp_time(time_t new_time);

#endif /* _QOSA_UTILS_H__ */