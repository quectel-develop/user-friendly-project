#ifndef __QL_DEV_H__
#define __QL_DEV_H__
#include "QuectelConfig.h"
#include "qosa_def.h"
#include "qosa_system.h"
#include "sd_fatfs.h"


char* ql_get_mcu_firmware_version(void);
void ql_module_hardware_init(void);
int32_t ql_spi_flash_selftest(void);
int32_t ql_sdcard_init(void);
void ql_sdcard_deinit(void);
void ql_sdcard_hotplug_proc(bool onoff);

#endif /* __QL_DEV_H__ */
