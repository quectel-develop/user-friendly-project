#ifndef _SD_FATFS_H_
#define _SD_FATFS_H_
#include <stdbool.h>
#include <stdint.h>

#define QOSA_TRUE         1
#define QOSA_FALSE        0


bool ql_sd_init(void);
void ql_sd_hardware_init(void);
bool ql_sd_is_init(void);
uint64_t get_sdcard_free_space(void);

#endif /* _SD_FATFS_H_ */
