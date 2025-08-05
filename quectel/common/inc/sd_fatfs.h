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

bool SD_INIT(void);

void SD_DEINIT(void);

uint64_t get_sdcard_free_space(void);
#endif /* COMMON_INC_SD_FATFS_H_ */

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_TFCARD__ */