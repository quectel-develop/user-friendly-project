/*****************************************************************/ /**
* @file hal_uart.h
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
#ifndef _HAL_UART_H__
#define _HAL_UART_H__

typedef int (*event_callback_t)(void *argv);

void qosa_uart_register(event_callback_t event_cb);

int qosa_uart_open(void);

int qosa_uart_write(int pos, const char *buffer, size_t size);

int qosa_uart_read(int pos, const char *buffer, size_t size);

void qosa_uart_hardware_init(void);

#endif /* _HAL_UART_H__ */
