/*
 * sd_fatfs.h
 *
 *  Created on: Oct 18, 2023
 *      Author: barry
 */
#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_TFCARD__
#ifndef _SD_FATFS_H_
#define _SD_FATFS_H_
#include <stdbool.h>
#include "qosa_def.h"


bool ql_sd_init(void);
void ql_sd_deinit(void);
void ql_sd_hardware_init(void);
void ql_sd_hardware_deinit(void);
bool ql_sd_is_init(void);
uint64_t get_sdcard_free_space(void);

#endif /* _SD_FATFS_H_ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_TFCARD__ */
