/*
 * sd_fatfs.c
 *
 *  Created on: Oct 18, 2023
 *      Author: barry
 */
#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_TFCARD__
#include "sd_fatfs.h"
#include "debug_service.h"
#include "fatfs.h"
#include "hal_common.h"
#include "qosa_log.h"


extern SD_HandleTypeDef hsd;
static bool s_is_init = false;


void ql_sd_hardware_init(void)
{
	HAL_GPIO_WritePin(UFP_SD_EN_PORT, UFP_SD_EN_PIN, GPIO_PIN_SET);
}

void ql_sd_hardware_deinit(void)
{
	HAL_GPIO_WritePin(UFP_SD_EN_PORT, UFP_SD_EN_PIN, GPIO_PIN_RESET);
}

bool ql_sd_init(void)
{
	FRESULT RES;
	uint64_t total_bytes;
	uint32_t total_gb;
	uint32_t decimal_part;
	DWORD free_clusters;
    FATFS* fs_ptr;
	if (BSP_SD_IsDetected() != SD_PRESENT)
	{
		LOG_E("*** No SD card detected...\r\n");
		return QOSA_FALSE;
	}
	LOG_D("SD card detected !");

	RES = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
	if(RES == FR_OK)
	{
		/* FatFs Initialization Error */
		LOG_D("Fat System OK");
		if(HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER)
		{
			LOG_D("Initialize SD card successfully!");
			LOG_D("SD card information!");
			// can not print float
			total_bytes = (uint64_t)hsd.SdCard.BlockSize * hsd.SdCard.BlockNbr;
			total_gb = total_bytes / (1024 * 1024 * 1024);      // Integer part (GB)
			decimal_part = (total_bytes % (1024 * 1024 * 1024)) / (1024 * 1024) * 100 / 1024; // Decimal part
			// Print result (Format: X.Y GB)
			LOG_D("CardCapacity  : %lu.%02uGB", total_gb, decimal_part);// Total storage capacity (BlockSize × BlockNbr)

			RES = f_getfree("0:", &free_clusters, &fs_ptr);
			if (FR_OK == RES)
			{
				total_bytes = (uint64_t)free_clusters * fs_ptr->csize * hsd.SdCard.BlockSize;
				total_gb = total_bytes / (1024 * 1024 * 1024);
				decimal_part = (total_bytes % (1024 * 1024 * 1024)) / (1024 * 1024) * 100 / 1024;
				LOG_D("FreeSpace     : %lu.%02uGB", total_gb, decimal_part);
			}
			LOG_D("CardBlockSize : %d", hsd.SdCard.BlockSize);   // Physical block size
			LOG_D("LogBlockNbr   : %d", hsd.SdCard.LogBlockNbr);	// Number of logical blocks
			LOG_D("LogBlockSize  : %d", hsd.SdCard.LogBlockSize);// Logical block size
			LOG_D("RCA           : 0x%X", hsd.SdCard.RelCardAdd);  // Relative Card Address
			LOG_D("CardType      : %d (0: <= 2GB; 1: 2GB-32GB; 2: >32GB)", hsd.SdCard.CardType);    // Card type
			//
			HAL_SD_CardCIDTypeDef sdcard_cid;
			HAL_SD_GetCardCID(&hsd,&sdcard_cid);
			LOG_D("ManufacturerID: 0x%02x (0x03: SanDisk; 0x1A: ADATA; 0x1B: Samsung; 0x41: Kingston)", sdcard_cid.ManufacturerID); // Manufacturer ID
			LOG_I("sd card mount success! ");
			s_is_init = true;
			return QOSA_TRUE;
		}
	}

	LOG_E("sd card mount failed [%d]", RES);
	return QOSA_FALSE;
}

void ql_sd_deinit(void)
{
	uint8_t RES;

	RES = f_mount(NULL, "0:", 1);
	ql_sd_hardware_deinit();
	if(RES ==FR_OK)
	{
		/* FatFs Initialization Error */
		LOG_I("sd card umount success!");
	}
	MX_FATFS_DeInit();
	s_is_init = false;
}

bool ql_sd_is_init(void)
{
	return s_is_init;
}

uint64_t get_sdcard_free_space(void)
{
	uint8_t RES;
	DWORD free_clusters;
    FATFS* fs_ptr;
	RES = f_getfree("0:", &free_clusters, &fs_ptr);
	if (FR_OK != RES)
		return 0;
	return (uint64_t)free_clusters * fs_ptr->csize * hsd.SdCard.BlockSize / (1024 * 1024) ;
}

#else
void ql_sd_hardware_init(void)
{
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_TFCARD__ */
