#include <stm32f4xx.h>
#include "stm32f4xx_hal_flash.h"
#include "diskio.h"
#include "ff.h"
#include "fatfs.h"
#include "bootloader.h"
#include "sd_fatfs.h"
#include "log_def.h"

/************************************************************************************
Function name: STM32_FLASH_GetFlashSector
Function     : Gets the flash sector in which an address resides
argument     :
    addr     : flash address
return       : The value ranges from 0 to 7, that is, the sector where addr resides
*************************************************************************************/
static uint16_t STM32_FLASH_GetFlashSector(u32_t addr)
{
	if(addr < ADDR_FLASH_SECTOR_1)          return 0;
	else if(addr < ADDR_FLASH_SECTOR_2)     return 1;
	else if(addr < ADDR_FLASH_SECTOR_3)     return 2;
	else if(addr < ADDR_FLASH_SECTOR_4)     return 3;
	else if(addr < ADDR_FLASH_SECTOR_5)     return 4;
	else if(addr < ADDR_FLASH_SECTOR_6)     return 5;
	else if(addr < ADDR_FLASH_SECTOR_7)     return 6;
    else if(addr < ADDR_FLASH_SECTOR_8)     return 7;
    else if(addr < ADDR_FLASH_SECTOR_9)     return 8;
    else if(addr < ADDR_FLASH_SECTOR_10)    return 9;
    else if(addr < ADDR_FLASH_SECTOR_11)    return 10;
	return 11;
}

/************************************************************************************
Function name: IAP_Write_Flash_AppBin
Function     : Firmware upgrade function
argument     :
	appxaddr : The start address of the application program must be the start address of a sector
	appbuf   : Application CODE
	appsize  : Application size (in bytes).
return       : NULL
*************************************************************************************/
static s16_t IAP_Write_Flash_AppBin(u32_t appxaddr,u8_t *appbuf,u32_t appsize)
{
	u32_t i = 0, align_num;
	vu32_t temp_32 = 0;
	vu8_t temp_8 = 0;
	u32_t WriteAddr = appxaddr, NumToWrite = appsize;
    s16_t ret = 0;

    if ((WriteAddr < STM32_FLASH_BASE) || (WriteAddr >= (STM32_FLASH_BASE + STM32_FLASH_SIZE)) || (WriteAddr%4))
        return -1;

    HAL_FLASH_Unlock();
	align_num = ALIGN_DOWN(NumToWrite, 4);
	for(i = 0; i < align_num; i += 4)
	{
		temp_32 = 0;
		temp_32 |= appbuf[3] << 24;
		temp_32 |= appbuf[2] << 16;
		temp_32 |= appbuf[1] << 8;
		temp_32 |= appbuf[0] << 0;
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, WriteAddr, temp_32) != HAL_OK) //Write data
        {
			ret = -1;
            break;
        }
		WriteAddr += 4;
		appbuf += 4;
		NumToWrite -= 4;
	}

	if (NumToWrite != 0)
	{
		for(i = 0; i < NumToWrite; i ++)
		{
			temp_8  = *appbuf;
			if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, WriteAddr, temp_8) != HAL_OK) //Write data
			{
                ret = -1;
                break;
            }
			WriteAddr += 1;
			appbuf += 1;
			NumToWrite -= 1;
		}
	}
	HAL_FLASH_Lock();
    return ret;
}

/************************************************************************************
Function name: IAP_Erase_Flash_AppBin
Function     :
argument     : NULL
return       : NULL
*************************************************************************************/
static void IAP_Erase_Flash_AppBin(u32_t app_size)
{
	HAL_FLASH_Unlock();
    if (APP1_START_ADDR + app_size >= ADDR_FLASH_SECTOR_11) {
	    FLASH_Erase_Sector(STM32_FLASH_GetFlashSector(ADDR_FLASH_SECTOR_11), FLASH_VOLTAGE_RANGE_3); LOG_D("Erase ADDR_FLASH_SECTOR_11");}
    if (APP1_START_ADDR + app_size >= ADDR_FLASH_SECTOR_10) {
	    FLASH_Erase_Sector(STM32_FLASH_GetFlashSector(ADDR_FLASH_SECTOR_10), FLASH_VOLTAGE_RANGE_3); LOG_D("Erase ADDR_FLASH_SECTOR_10");}
    if (APP1_START_ADDR + app_size >= ADDR_FLASH_SECTOR_9) {
	    FLASH_Erase_Sector(STM32_FLASH_GetFlashSector(ADDR_FLASH_SECTOR_9), FLASH_VOLTAGE_RANGE_3); LOG_D("Erase ADDR_FLASH_SECTOR_9");}
    if (APP1_START_ADDR + app_size >= ADDR_FLASH_SECTOR_8) {
	    FLASH_Erase_Sector(STM32_FLASH_GetFlashSector(ADDR_FLASH_SECTOR_8), FLASH_VOLTAGE_RANGE_3); LOG_D("Erase ADDR_FLASH_SECTOR_8");}
	if (APP1_START_ADDR + app_size >= ADDR_FLASH_SECTOR_7) {
	    FLASH_Erase_Sector(STM32_FLASH_GetFlashSector(ADDR_FLASH_SECTOR_7), FLASH_VOLTAGE_RANGE_3); LOG_D("Erase ADDR_FLASH_SECTOR_7");}
	if (APP1_START_ADDR + app_size >= ADDR_FLASH_SECTOR_6) {
	    FLASH_Erase_Sector(STM32_FLASH_GetFlashSector(ADDR_FLASH_SECTOR_6), FLASH_VOLTAGE_RANGE_3); LOG_D("Erase ADDR_FLASH_SECTOR_6");}
	if (APP1_START_ADDR + app_size >= ADDR_FLASH_SECTOR_5) {
	    FLASH_Erase_Sector(STM32_FLASH_GetFlashSector(ADDR_FLASH_SECTOR_5), FLASH_VOLTAGE_RANGE_3); LOG_D("Erase ADDR_FLASH_SECTOR_5");}
	if (APP1_START_ADDR + app_size >= ADDR_FLASH_SECTOR_4) {
	    FLASH_Erase_Sector(STM32_FLASH_GetFlashSector(ADDR_FLASH_SECTOR_4), FLASH_VOLTAGE_RANGE_3); LOG_D("Erase ADDR_FLASH_SECTOR_4");}
    /* If bootloader <64KB,  below should be added.*/
    // if (APP1_START_ADDR + app_size >= ADDR_FLASH_SECTOR_3) {
	//     FLASH_Erase_Sector(STM32_FLASH_GetFlashSector(ADDR_FLASH_SECTOR_3), FLASH_VOLTAGE_RANGE_3); LOG_D("Erase ADDR_FLASH_SECTOR_3");}
    // if (APP1_START_ADDR + app_size >= ADDR_FLASH_SECTOR_2) {
	//     FLASH_Erase_Sector(STM32_FLASH_GetFlashSector(ADDR_FLASH_SECTOR_2), FLASH_VOLTAGE_RANGE_3); LOG_D("Erase ADDR_FLASH_SECTOR_2");}
	HAL_FLASH_Lock();
}

/************************************************************************************
Function name: Update_Firmware
Function     : Firmware upgrade function
argument     : NULL
return       : NULL
*************************************************************************************/
static s16_t Update_Firmware(void)
{
    u8_t i=0;
    u8_t ret=0;
    u32_t br;
    u16_t readlen;
    u32_t addrx;
    u32_t Receive_data=0; 								//Calculate the total amount of data received
    u32_t file_size=0;    								//File size
    u8_t Receive_dat_buff[STM32_SECTOR_SIZE];           //Data receive cache array
    u8_t percent=0;       								//Percentage of firmware upgrades

	/*Find if the Bin file you want to upgrade exists*/
    ret = f_open(&SDFile, "0:FotaFile.bin", FA_OPEN_EXISTING | FA_READ);
    file_size = f_size(&SDFile);    					//Read file size Byte

	if(ret != FR_OK) return 1;
    LOG_D("Read firmware size: %d Bytes", file_size);
    addrx = APP1_START_ADDR;

	/*Perform major IAP functions*/
    LOG_D("Start updating firmware...");
	while(1)
	{
		/*Read 16K of data each time into the memory buffer buffer*/
	    ret = f_read(&SDFile, Receive_dat_buff, STM32_SECTOR_SIZE, &br);
        readlen = br;
        Receive_data += br;   //Total number of bytes read
        if (ret || br == 0)
        {
            if (br != 0)
                ret = -1;
            break;
        }
		if (i++ == 0) //The first packet determines the validity of the data and erases the corresponding partition
		{
			if(((*(vu32_t*)(Receive_dat_buff+4))&0xFF000000)!=0x08000000) //Check whether the value is 0X08XXXXXX
			{
				LOG_D("Invalid upgrade package 0x%p", Receive_dat_buff);
                ret = -1;
				break;
			}
			IAP_Erase_Flash_AppBin(file_size);
		}

        if(IAP_Write_Flash_AppBin(addrx, Receive_dat_buff, readlen) != 0) //The read data is written to the Flash
        {
            LOG_D("***** Write flash error occur!");
            ret = -1;
            break;
        }
        addrx += STM32_SECTOR_SIZE;
        percent = Receive_data*100/file_size;
		LOG_D("[--%d%%--] recive %d, total %d", percent, Receive_data, file_size);
    }
    f_close(&SDFile);
    /* Delete firmware after update */
    if(ret == 0)
        f_unlink("0:FotaFile.bin");
    return ret;
}


/************************************************************************************
Function name: Jump2App
Function     : Jump from the Bootloader to the user APP address space
argument     : NULL
return       : NULL
*************************************************************************************/
static void Jump2App(u32_t AppAddr)
{
    typedef void (*app_entry_t)(void);
    vu32_t* app_vector_table = (vu32_t*)AppAddr;

    /* The first address is MSP, and address +1(4-Bytes) is Reset_Handler */
    u32_t app_stack_top = *app_vector_table;
    u32_t app_entry_point = *(app_vector_table + 1);
    LOG_D("App stack top: 0x%08x, app entry point: 0x%08x", app_stack_top, app_entry_point);

    /* Stack top pointer validation */
	if(app_stack_top >= 0x20000000)
	{
		LOG_D("Now jump to APP [0x%08x]...\n", app_entry_point);

        /* 1. Disable global interrupts */
        __disable_irq();

        /* 2. Disable peripheral interrupts used by Bootloader */
		HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);
		HAL_NVIC_DisableIRQ(DMA2_Stream6_IRQn);
		HAL_NVIC_DisableIRQ(SDIO_IRQn);
		HAL_NVIC_DisableIRQ(USART6_IRQn);

        /* 3. Reset SysTick timer */
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL = 0;

        /* 4. Reset RCC (if application expects to reconfigure clocks) */
        HAL_RCC_DeInit();

        /* 5. Set Main Stack Pointer (the first Byte in app.bin is used to store the stack top address) */
        __set_MSP(app_stack_top);

        /* 6. Get App entry point and jump */
        app_entry_t app_entry = (app_entry_t)(app_entry_point);
        app_entry();

        /* Jump failed handler */
        LOG_D("***** Jump to App failed. Shouldn't be here...");
        while (1);
	}
    else
    {
        LOG_D("***** Invalid App image.");
    }
}


void bootloader(void)
{
	s16_t ret;

    if (BSP_SD_IsDetected() == SD_PRESENT)
	{
		LOG_D("SD card detected.");
		ql_sd_init();
		ret = Update_Firmware();
        switch (ret)
        {
            case 1:  LOG_D("Not found [FotaFile.bin], or no need to update."); break;
            case 0:  LOG_D("Update successfully!"); break;
            case -1: LOG_D("***** Error occured while updating. Aborted!"); break;
            default: break;
        }
	}
	else
	{
		LOG_D("No SD card detected.");
	}
	Jump2App(APP1_START_ADDR);
}
