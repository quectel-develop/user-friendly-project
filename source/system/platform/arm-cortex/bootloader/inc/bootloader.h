#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__
#include "core_cm4.h"
#include "stm32f4xx.h"

typedef unsigned char                   u8_t;
typedef signed   char                   s8_t;
typedef unsigned short                  u16_t;
typedef signed   short                  s16_t;
typedef unsigned int                    u32_t;
typedef signed   int                    s32_t;
typedef unsigned long long              u64_t;
typedef __IO u8_t                       vu8_t;
typedef __IO u16_t                      vu16_t;
typedef __IO u32_t                      vu32_t;
typedef __IO u64_t                      vu64_t;

#define ALIGN_UP(x, a) ( ( ((x) + ((a) - 1) ) / a ) * a )
#define ALIGN_DOWN(x, a) ( ( (x) / (a)) * (a) )

/* Flash module organization (See RM0430 Reference manual) */
/* Main memory */
#define ADDR_FLASH_SECTOR_0     ((u32_t)0x08000000) 	//Sector 0 start address, 16 Kbyte
#define ADDR_FLASH_SECTOR_1     ((u32_t)0x08004000) 	//Sector 1 start address, 16 Kbyte
#define ADDR_FLASH_SECTOR_2     ((u32_t)0x08008000) 	//Sector 2 start address, 16 Kbyte
#define ADDR_FLASH_SECTOR_3     ((u32_t)0x0800C000) 	//Sector 3 start address, 16 Kbyte
#define ADDR_FLASH_SECTOR_4     ((u32_t)0x08010000) 	//Sector 4 start address, 64 Kbyte
#define ADDR_FLASH_SECTOR_5     ((u32_t)0x08020000) 	//Sector 5 start address, 128 Kbyte
#define ADDR_FLASH_SECTOR_6     ((u32_t)0x08040000) 	//Sector 6 start address, 128 Kbyte
#define ADDR_FLASH_SECTOR_7     ((u32_t)0x08060000) 	//Sector 7 start address, 128 Kbyte
#define ADDR_FLASH_SECTOR_8     ((u32_t)0x08080000) 	//Sector 8 start address, 128 Kbyte
#define ADDR_FLASH_SECTOR_9     ((u32_t)0x080A0000) 	//Sector 9 start address, 128 Kbyte
#define ADDR_FLASH_SECTOR_10    ((u32_t)0x080C0000) 	//Sector 10 start address, 128 Kbyte
#define ADDR_FLASH_SECTOR_11    ((u32_t)0x080E0000) 	//Sector 11 start address, 128 Kbyte
/* System memory */
#define ADDR_FLASH_SYSTEM       ((u32_t)0x1FFF0000) 	//System memory start address, 30 Kbyte
/* OTP area */
#define ADDR_FLASH_OTP          ((u32_t)0x1FFF7800) 	//OTP area start address, 528 byte
/* Option bytes */
#define ADDR_FLASH_OPTION       ((u32_t)0x1FFFC000) 	//Option bytes start address, 16 byte

#define STM32_FLASH_SIZE        (1024*1024)             //FLASH total capacity of the selected STM32 (in bytes)
#define STM32_SECTOR_SIZE       (1024*16)               //The FLASH sector size of STM32
#define STM32_FLASH_BASE        ADDR_FLASH_SECTOR_0     //Bootloader start address
#define APP1_START_ADDR		    ADDR_FLASH_SECTOR_4  	//Application start address


void bootloader(void);
#endif /* __BOOTLOADER_H__ */
