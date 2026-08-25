#ifndef __QL_DEV_H__
#define __QL_DEV_H__
#include "QuectelConfig.h"
#include "qosa_def.h"
#include "qosa_system.h"
#include "sd_fatfs.h"
#include "at.h"


void ql_module_hardware_init(void);
int cli_mcu_firmware_version(s32_t argc, char *argv[]);
int cli_reboot(s32_t argc, char *argv[]);
void cli_reboot_help(void);
int32_t ql_spi_flash_selftest(void);
void ql_sdcard_hotplug_proc(void);
int ql_wait_module_ready(at_client_t client, u32_t timeout);
int ql_echo_mode_enable(at_client_t client, bool onoff);
int ql_uart_flow_control_enable(at_client_t client, bool onoff);
int ql_at_uart_init(at_client_t client);

#endif /* __QL_DEV_H__ */
