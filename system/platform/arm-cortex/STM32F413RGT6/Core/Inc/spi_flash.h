#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H
#include <stdint.h>
#include "spi.h"
#include "common_hal.h"

// W25XQ series chips list
#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17
#define W25Q256 0XEF18	   

extern uint16_t SPI_FLASH_TYPE;    //Define the type of spi flash	

#define	FLASH_CS_HIGH      HAL_GPIO_WritePin(UFP_FLASH_CS_PORT, UFP_FLASH_CS_PIN, GPIO_PIN_SET)      // Select CS
#define	FLASH_CS_LOW       HAL_GPIO_WritePin(UFP_FLASH_CS_PORT, UFP_FLASH_CS_PIN, GPIO_PIN_RESET)    // Unselect CS

////////////////////////////////////////////////////////////////////////////////// 
// Command List
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg1		0x05 
#define W25X_ReadStatusReg2		0x35 
#define W25X_ReadStatusReg3		0x15 
#define W25X_WriteStatusReg1    0x01 
#define W25X_WriteStatusReg2    0x31 
#define W25X_WriteStatusReg3    0x11 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 
#define W25X_Enable4ByteAddr    0xB7
#define W25X_Exit4ByteAddr      0xE9
////////////////////////////////////////////////////////////////////////////////// 

void SPI_Flash_Init(void);                          // Init Spi flash IO
uint16_t SPI_Flash_ReadID(void);                    // Read ID of spi flash
uint8_t  SPI_Flash_ReadSR(uint8_t regno);           // Read Status Register of spi flash
void SPI_Flash_Write_SR(uint8_t regno,uint8_t sr);  // Write Status Register of spi flash
void SPI_Flash_Write_Enable(void);                  // Enable Write of spi flash (Set WEL)
void W25QXX_Write_Disable(void);                    // Disable Write of spi flash (Reset WEL)
void SPI_Flash_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite); // Write spi flash (Without check)
void SPI_Flash_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);    // Read a specified length of data at a specified address in spi flash
void SPI_Flash_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite); // Write data of specified length to specified address of spi flash
void SPI_Flash_Erase_Chip(void);                    // Erase the entire chip
void SPI_Flash_Erase_Sector(uint32_t Dst_Addr);     // Erase a sector
void SPI_Flash_Wait_Busy(void);                     // Wait to be idle
void SPI_Flash_PowerDown(void);                     // Enter power down mode
void SPI_Flash_WakeUp(void);                        // Wakeup

#endif
