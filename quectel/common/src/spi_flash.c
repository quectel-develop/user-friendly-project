/**
  ******************************************************************************
  * @file    spi_flash.c
  * @brief   This file provides code for the driver of spi flash
  ******************************************************************************
  */
#include "spi_flash.h"

extern void SPI1_SetSpeed(uint8_t SPI_BaudRatePrescaler);
extern uint8_t SPI1_ReadWriteByte(uint8_t TxData);

uint16_t SPI_FLASH_TYPE = W25Q64;	//Default W25Q64

// 4Kbytes is a Sector
// 16 Sectors is a Block
// The capacity of W25Q64 is 8MB
// 128 Blocks, 2048 Sectors totally

// Init Spi flash IO
void SPI_Flash_Init(void)
{
    uint8_t temp;
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();	// Enable GPIOB Clock

    // PA4 - NSS
    GPIO_InitStruct.Pin = UFP_FLASH_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(UFP_FLASH_CS_PORT, &GPIO_InitStruct);

	FLASH_CS_HIGH;			                	// Unselect CS
	MX_SPI1_Init();		   			        	// Init SPI
	SPI1_SetSpeed(SPI_BAUDRATEPRESCALER_2); 	// Set 42M clock, highspeed mode
	SPI_FLASH_TYPE = SPI_Flash_ReadID();		// Read FLASH ID
    if(SPI_FLASH_TYPE == W25Q64)				// SPI FLASH is W25Q64
    {
        temp = SPI_Flash_ReadSR(3);				// Read Status Register 3 to determine the address Mode
        if((temp&0X01) == 0)					// if not 4-Bytes address mode, enter 4-Bytes address mode
		{
			FLASH_CS_LOW;						// Select CS
			SPI1_ReadWriteByte(W25X_Enable4ByteAddr);	// Send the command of entering 4-Bytes address mode
			FLASH_CS_HIGH;       		        // Unselect CS
		}
    }
}

// Read Status Register of spi flash
// Return: The value of Status Register
uint8_t SPI_Flash_ReadSR(uint8_t regno)
{
	uint8_t byte = 0, command = 0;
    switch(regno)
    {
        case 1:
            command = W25X_ReadStatusReg1;	// Read Status Register 1
            break;
        case 2:
            command = W25X_ReadStatusReg2;	// Read Status Register 2
            break;
        case 3:
            command = W25X_ReadStatusReg3;	// Read Status Register 3
            break;
        default:
            command = W25X_ReadStatusReg1;
            break;
    }
	FLASH_CS_LOW;							// Select CS
	SPI1_ReadWriteByte(command);			// Send the command of reading Status Register
	byte = SPI1_ReadWriteByte(0Xff);		// Read one Byte
	FLASH_CS_HIGH;							// Unselect CS
	return byte;
}

// Write Status Register of spi flash
void SPI_Flash_Write_SR(uint8_t regno, uint8_t sr)
{
    uint8_t command = 0;
    switch(regno)
    {
        case 1:
            command = W25X_WriteStatusReg1;	// Read Status Register 1
            break;
        case 2:
            command = W25X_WriteStatusReg2;	// Read Status Register 2
            break;
        case 3:
            command = W25X_WriteStatusReg3;	// Read Status Register 3
            break;
        default:
            command = W25X_WriteStatusReg1;
            break;
    }
	FLASH_CS_LOW;							// Select CS
	SPI1_ReadWriteByte(command);            // Send the command of writing Status Register
	SPI1_ReadWriteByte(sr);                 // Write one Byte
	FLASH_CS_HIGH;							// Unselect CS
}

// Enable Write of spi flash (Set WEL)
void SPI_Flash_Write_Enable(void)
{
	FLASH_CS_LOW;							// Select CS
    SPI1_ReadWriteByte(W25X_WriteEnable);	// Send the command of writing enable
	FLASH_CS_HIGH;							// Unselect CS
}

// Disable Write of spi flash (Reset WEL)
void W25QXX_Write_Disable(void)
{
	FLASH_CS_LOW;							// Select CS
    SPI1_ReadWriteByte(W25X_WriteDisable);  // Send the command of writing disable
	FLASH_CS_HIGH;							// Unselect CS
}

// Read ID of spi flash
uint16_t SPI_Flash_ReadID(void)
{
	uint16_t Temp = 0;
	FLASH_CS_LOW;
	SPI1_ReadWriteByte(0x90);	// Send the command of reading ID
	SPI1_ReadWriteByte(0x00);
	SPI1_ReadWriteByte(0x00);
	SPI1_ReadWriteByte(0x00);
	Temp |= SPI1_ReadWriteByte(0xFF)<<8;
	Temp |= SPI1_ReadWriteByte(0xFF);
	FLASH_CS_HIGH;
	return Temp;
}

// Read a specified length of data at a specified address in spi flash
// pBuffer : buffer of reading data
// ReadAddr: 24-bit address
// NumByteToRead : Max 65535 bytes
void SPI_Flash_Read(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
 	uint16_t i;
	FLASH_CS_LOW;								// Select CS
    SPI1_ReadWriteByte(W25X_ReadData);      	// Send the command of reading data
    if(SPI_FLASH_TYPE == W25Q256)				// if spi flash is W25Q256, which has 4-Bytes adress, Send MSB (8bit)
    {
        SPI1_ReadWriteByte((uint8_t)((ReadAddr)>>24));
    }
    SPI1_ReadWriteByte((uint8_t)((ReadAddr)>>16));	// Send 24bit address
    SPI1_ReadWriteByte((uint8_t)((ReadAddr)>>8));
    SPI1_ReadWriteByte((uint8_t)ReadAddr);
    for(i=0; i<NumByteToRead; i++)
	{
        pBuffer[i] = SPI1_ReadWriteByte(0XFF);	// Read data in loop
    }
	FLASH_CS_HIGH;								// Unselect CS
}

// Write data (Max 256 bytes) to a specified address on a page
// pBuffer : buffer of writing data
// ReadAddr: 24-bit address
// NumByteToRead : Max 256 bytes. This size should not exceed the remaining bytes of the page !!
void W25QXX_Write_Page(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
 	uint16_t i;
    SPI_Flash_Write_Enable();					// SET WEL
	FLASH_CS_LOW;								// Select CS
    SPI1_ReadWriteByte(W25X_PageProgram);		// Send the command of writing page
    if(SPI_FLASH_TYPE == W25Q256)				// if spi flash is W25Q256, which has 4-Bytes adress, Send MSB (8bit)
    {
        SPI1_ReadWriteByte((uint8_t)((WriteAddr)>>24));
    }
    SPI1_ReadWriteByte((uint8_t)((WriteAddr)>>16));	// Send 24bit address
    SPI1_ReadWriteByte((uint8_t)((WriteAddr)>>8));
    SPI1_ReadWriteByte((uint8_t)WriteAddr);
    for(i=0; i<NumByteToWrite; i++)
	{
		SPI1_ReadWriteByte(pBuffer[i]);			// Write data in loop
	}
	FLASH_CS_HIGH;								// Unselect CS
	SPI_Flash_Wait_Busy();						// Wait for writing to end
}

// Write spi flash (Without check)
// It must be ensured that all the data in the address range written to is 0XFF
// or else the data written to somewhere other than 0XFF will fail !
// With automatic pagination
// Write the specified length of data at the specified address, and make sure the address doesn't go out of bounds!
// pBuffer : buffer of writing data
// ReadAddr: 24-bit address
// NumByteToRead : Max 65535 bytes
void SPI_Flash_Write_NoCheck(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint16_t pageremain;
	pageremain = 256-WriteAddr%256;		// Number of Bytes remaining in a single page
	if(NumByteToWrite<=pageremain) pageremain = NumByteToWrite;	// No larger than 256 Bytes
	while(1)
	{
		W25QXX_Write_Page(pBuffer,WriteAddr, pageremain);
		if(NumByteToWrite == pageremain) break;	// If writing data has been completed
	 	else // NumByteToWrite > pageremain
		{
			pBuffer += pageremain;
			WriteAddr += pageremain;

			NumByteToWrite -= pageremain;			// Minus the number of bytes written
			if(NumByteToWrite>256) pageremain=256;	// 256 Bytes can be written at a time
			else pageremain = NumByteToWrite;		// There are not enough 256 bytes
		}
	};
}

// Write data of specified length to specified address of spi flash
// This function with erase operation
// pBuffer : buffer of writing data
// ReadAddr: 24-bit address
// NumByteToRead : Max 65535 bytes
uint8_t FLASH_BUFFER[4096];
void SPI_Flash_Write(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint32_t secpos;
	uint16_t secoff, secremain;
 	uint16_t i;
	uint8_t * DataBuf = FLASH_BUFFER;

 	secpos = WriteAddr/4096;	// Sector address
	secoff = WriteAddr%4096;	// Offset within the sector
	secremain = 4096-secoff;	// Remaining available capacity of the sector

 	if(NumByteToWrite <= secremain) secremain = NumByteToWrite;	// No larger than 4096 Bytes
	while(1)
	{
		SPI_Flash_Read(DataBuf, secpos*4096, 4096);	// Read data of the entire sector
		for(i=0; i<secremain; i++)//check data
		{
			if(DataBuf[secoff+i] != 0XFF) break;	// Need to be erased
		}
		if(i < secremain)// Need to be erased
		{
			SPI_Flash_Erase_Sector(secpos);	// Erase this sector
			for(i=0; i<secremain; i++)		// Copy
			{
				DataBuf[i+secoff] = pBuffer[i];
			}
			SPI_Flash_Write_NoCheck(DataBuf, secpos*4096, 4096);	// Write data into the entire sector
		}
		else SPI_Flash_Write_NoCheck(pBuffer, WriteAddr, secremain);	// If the sector has been erased, Write directly
		if(NumByteToWrite == secremain) break;	// if writing data has been completed
		else									// if writing data has NOT been completed
		{
			secpos++;
			secoff=0;

		   	pBuffer += secremain;  						// Pointor offset
			WriteAddr += secremain;						// Write address offset
		   	NumByteToWrite -= secremain;				// Byte count decrement
			if(NumByteToWrite > 4096) secremain = 4096;	// If there is still not enough space to write to the next sector
			else secremain = NumByteToWrite;			// If there is enough space to write to the next sector
		}
	};
}

// Erase the entire chip
// It takes an extremely long time to wait...
void SPI_Flash_Erase_Chip(void)
{
    SPI_Flash_Write_Enable();				// SET WEL
    SPI_Flash_Wait_Busy();
  	FLASH_CS_LOW;							// Select CS
    SPI1_ReadWriteByte(W25X_ChipErase);		// Send the command of erasing chip
	FLASH_CS_HIGH;							// Unselect CS
	SPI_Flash_Wait_Busy();					// Wait for erasing to end
}

// Erase a sector
// Minimum time to erase a sector : 150ms
// Dst_Addr : Sector address, which is set according to the actual capacity
void SPI_Flash_Erase_Sector(uint32_t Dst_Addr)
{
 	Dst_Addr*=4096;
    SPI_Flash_Write_Enable();					// SET WEL
    SPI_Flash_Wait_Busy();
  	FLASH_CS_LOW;								// Select CS
    SPI1_ReadWriteByte(W25X_SectorErase);		// Send the command of erasing sector
    if(SPI_FLASH_TYPE == W25Q256)				// if spi flash is W25Q256, which has 4-Bytes adress, Send MSB (8bit)
    {
        SPI1_ReadWriteByte((uint8_t)((Dst_Addr)>>24));
    }
    SPI1_ReadWriteByte((uint8_t)((Dst_Addr)>>16));	// Send 24bit address
    SPI1_ReadWriteByte((uint8_t)((Dst_Addr)>>8));
    SPI1_ReadWriteByte((uint8_t)Dst_Addr);
	FLASH_CS_HIGH;								// Unselect CS
    SPI_Flash_Wait_Busy();						// Wait for erasing to end
}

// Wait to be idle
void SPI_Flash_Wait_Busy(void)
{
	while( (SPI_Flash_ReadSR(1)&0x01) == 0x01 );	// Wait for the BUSY bit to be emptied
}

// Enter power down mode
void SPI_Flash_PowerDown(void)
{
  	FLASH_CS_LOW;							// Select CS
    SPI1_ReadWriteByte(W25X_PowerDown);		// Send the command of PowerDown
	FLASH_CS_HIGH;							// Unselect CS
    HAL_Delay(3);							// Wait TPD
}

// Wakeup
void SPI_Flash_WakeUp(void)
{
  	FLASH_CS_LOW;								// Select CS
    SPI1_ReadWriteByte(W25X_ReleasePowerDown);	// Send the command of PowerDown
	FLASH_CS_HIGH;								// Unselect CS
    HAL_Delay(3);								// Wait TRES1
}

// Compare strings
int Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while(BufferLength--)
  {
	if(*pBuffer1 != *pBuffer2) return -1;

	pBuffer1++;
	pBuffer2++;
  }
  return 0;
}
