#ifndef __QL_DEV_H__
#define __QL_DEV_H__
#include "QuectelConfig.h"
#include "qosa_def.h"
#include "qosa_system.h"
#include "sd_fatfs.h"


void ql_module_hardware_init(void);
int cli_mcu_firmware_version(s32_t argc, char *argv[]);
int cli_reboot(s32_t argc, char *argv[]);
void cli_reboot_help(void);
int32_t ql_spi_flash_selftest(void);
void ql_sdcard_hotplug_proc(void);

#endif /* __QL_DEV_H__ */
