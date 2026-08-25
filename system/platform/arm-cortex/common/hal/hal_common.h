/*
 * hal_common.h
 *
 *  Created on: March 26, 2025
 *      Author: Jerry Chen
 */
#ifndef __HAL_COMMON_H
#define __HAL_COMMON_H

#define UFP_FLASH_CS_PORT       GPIOA
#define UFP_FLASH_CS_PIN        GPIO_PIN_4

#define UFP_NET_MOD_DET_PORT    GPIOA
#define UFP_NET_MOD_DET_PIN     GPIO_PIN_8
#define UFP_STATUS_DET_PORT     GPIOB
#define UFP_STATUS_DET_PIN      GPIO_PIN_1
#define UFP_SLEEP_DET_POR       GPIOB
#define UFP_SLEEP_DET_PIN       GPIO_PIN_2

#define UFP_SD_EN_PORT          GPIOB
#define UFP_SD_EN_PIN           GPIO_PIN_8
#define UFP_SD_DET_PORT         GPIOB
#define UFP_SD_DET_PIN          GPIO_PIN_9

#define UFP_PWRKEY_PORT         GPIOC
#define UFP_PWRKEY_PIN          GPIO_PIN_0
#define UFP_RESET_PORT          GPIOC
#define UFP_RESET_PIN           GPIO_PIN_3
#define UFP_VBAT_EN_PORT        GPIOB
#define UFP_VBAT_EN_PIN         GPIO_PIN_6

#define UFP_AT_UART_CTS_PORT    GPIOA
#define UFP_AT_UART_CTS_PIN     GPIO_PIN_0
#define UFP_AT_UART_RTS_PORT    GPIOA
#define UFP_AT_UART_RTS_PIN     GPIO_PIN_1


#endif /* __HAL_COMMON_H */
