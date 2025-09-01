/*****************************************************************/ /**
* @file hal_uart.h
* @brief
* @author larson.li@quectel.com
* @date 2024-12-23
*
* @copyright Copyright (c) 2024 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
**********************************************************************/
#ifndef _HAL_UART_H__
#define _HAL_UART_H__
#include <stdint.h>


#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#define AT_UART                         "COM10"
#elif __linux__
#else
/* -------------------- Debug UART -------------------- */
#define DEBUG_UART                      USART6
#define DEBUG_UART_HANDLE               huart6
#define USER_Debug_UART_RxIdleCallback  USER_UART6_RxIdleCallback

/* --------------------- AT UART ---------------------- */
#define AT_UART                         USART2
#define AT_UART_HANDLE                  huart2
#define USER_AT_UART_RxIdleCallback     USER_UART2_RxIdleCallback
#define USER_AT_UART_RxHalfCpltCallback USER_UART2_RxHalfCpltCallback
#define USER_AT_UART_RxCpltCallback     USER_UART2_RxCpltCallback
#endif


typedef int (*event_callback_t)(void);

int qosa_uart_open(void);
int qosa_uart_write(int pos, const char *buffer, size_t size);
int qosa_uart_read(int pos, const char *buffer, size_t size);
void qosa_uart_register(event_callback_t event_cb);

void qosa_uart_hardware_init(void);
void qosa_uart_get_debug_input(const uint8_t **pData, uint16_t *pSize);

#endif /* _HAL_UART_H__ */
