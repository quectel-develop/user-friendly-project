#ifndef __QL_DEV_H__
#define __QL_DEV_H__
#include "QuectelConfig.h"
#include "qosa_def.h"
#include "qosa_system.h"
#include "sd_fatfs.h"
#include "at.h"

typedef enum {
    QL_UART_BAUD_AUTO       = 0,
    QL_UART_BAUD_1200       = 1200,
    QL_UART_BAUD_2400       = 2400,
    QL_UART_BAUD_4800       = 4800,
    QL_UART_BAUD_9600       = 9600,
    QL_UART_BAUD_14400      = 14400,
    QL_UART_BAUD_19200      = 19200,
    QL_UART_BAUD_28800      = 28800,
    QL_UART_BAUD_33600      = 33600,
    QL_UART_BAUD_38400      = 38400,
    QL_UART_BAUD_57600      = 57600,
    QL_UART_BAUD_115200     = 115200,
    QL_UART_BAUD_230400     = 230400,
    QL_UART_BAUD_460800     = 460800,
    QL_UART_BAUD_921600     = 921600,
	QL_UART_BAUD_1000000	= 1000000,
    QL_UART_BAUD_1843200    = 1843200,
    QL_UART_BAUD_2000000    = 2000000,
    QL_UART_BAUD_2100000    = 2100000,
}ql_uart_baud_e;

typedef struct {
    at_client_t client;     /**< AT client handle */
    size_t rx_buffer_size;  /**< Size of the receive buffer */
    size_t tx_buffer_size;  /**< Size of the transmit buffer */
    u32_t timeout;          /**< AT command timeout in milliseconds */
    bool echo_mode;         /**< Echo mode status */
    bool flow_control;      /**< Flow control status */
    ql_uart_baud_e baudrate; /**< Baud rate */
} ql_uart_config_t;


void ql_module_hardware_init(void);
int cli_mcu_firmware_version(s32_t argc, char *argv[]);
int cli_reboot(s32_t argc, char *argv[]);
void cli_reboot_help(void);
int32_t ql_spi_flash_selftest(void);
void ql_sdcard_hotplug_proc(void);
int ql_wait_module_ready(at_client_t client, u32_t timeout);
int ql_echo_mode_enable(at_client_t client, bool onoff);
int ql_uart_config(at_client_t client, ql_uart_baud_e baudrate, bool flow_control);
int ql_at_uart_init(ql_uart_config_t *config);

#endif /* __QL_DEV_H__ */
