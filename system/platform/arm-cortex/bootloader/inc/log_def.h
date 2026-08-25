#ifndef __LOG_DEF_H__
#define __LOG_DEF_H__
#include <stdio.h>
#define LOG_D(fmt, ...) printf("[Bootloader][%s, %d] "fmt"\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#endif /* __LOG_DEF_H__ */
