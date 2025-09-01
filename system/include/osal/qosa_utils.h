/*****************************************************************/ /**
* @file qosa_utils.h
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
#ifndef _QOSA_UTILS_H__
#define _QOSA_UTILS_H__
#include <stdint.h>

uint64_t qosa_get_uptime_milliseconds(void);

int hexstr2byte(const char *buf, int len, char *bufout);

int byte2hexstr(const char *bufin, int in_len, char *bufout, int out_len);

#endif /* _QOSA_UTILS_H__ */
