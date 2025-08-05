#include "QuectelConfig.h"
#include "main.h"
#include "common_hal.h"
#include "hal_uart.h"
#include "qosa_log.h"
#include "sd_fatfs.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#include "windows.h"
#elif __linux__
#else
#include "cmsis_os.h"
#endif


char* ql_get_mcu_firmware_version(void)
{
    LOG_I("Current version: %s", QUECTEL_PROJECT_VERSION);
    return QUECTEL_PROJECT_VERSION;
}

void ql_module_hardware_init(void)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#elif __linux__
#else
    qosa_uart_hardware_init();

    LOG_D("Now restart the module...");
    HAL_GPIO_WritePin(UFP_RESET_PORT, UFP_RESET_PIN, GPIO_PIN_SET);
    qosa_task_sleep_ms(500);
    HAL_GPIO_WritePin(UFP_RESET_PORT, UFP_RESET_PIN, GPIO_PIN_RESET);
    qosa_task_sleep_ms(500);
    HAL_GPIO_WritePin(UFP_PWRKEY_PORT, UFP_PWRKEY_PIN, GPIO_PIN_SET);
    qosa_task_sleep_ms(2000);
    HAL_GPIO_WritePin(UFP_PWRKEY_PORT, UFP_PWRKEY_PIN, GPIO_PIN_RESET);
    qosa_task_sleep_ms(2000);
    LOG_D("Restart module done.");
#endif
}

int32_t ql_spi_flash_selftest(void)
{
    uint8_t Tx_Buffer[] = "Hello, this is just an external Flash self test code...";
    uint8_t Rx_Buffer[64];
    uint16_t FlashID = 0x00;
    int32_t ret = -1;

    SPI_Flash_Init();

    FlashID = SPI_Flash_ReadID();
    if(FlashID)
    {
        LOG_I("===Detected Flash! DeviceID [0X%02X]", FlashID);
        SPI_Flash_Erase_Sector(0x00000);
        SPI_Flash_Write(Tx_Buffer, 0x00000, sizeof(Tx_Buffer));
        LOG_I("===Write: %s", Tx_Buffer);
        SPI_Flash_Read(Rx_Buffer, 0x00000, sizeof(Tx_Buffer));
        LOG_I("===Read : %s", Rx_Buffer);

        if( Buffercmp(Tx_Buffer, Rx_Buffer, sizeof(Tx_Buffer)) == 0 )
        {
            LOG_I("===Matched. Flash test successfully!");
            ret = 0;
        }
        else
        {
            LOG_E("***Unmatched. Flash test Failed!");
            ret = -1;
        }
    }
    else
    {
        LOG_E("***Can not detected a flash device.");
        ret = -1;
    }
    return ret;
}


int32_t ql_sdcard_init(void)
{
    return SD_INIT();
}

void ql_sdcard_deinit(void)
{
    return SD_DEINIT();
}

void ql_sdcard_hotplug_proc(bool onoff)
{
    printf("\r\n");
    LOG_D("SD_CARD_DETECT, STATUS: %s", onoff? "ON" : "OFF");
    if (onoff == true)
    {
        MX_SDIO_SD_Init();
        MX_FATFS_Init();
        if(!SD_INIT())
            SD_DEINIT();
    }
    else
    {
        SD_DEINIT();
    }
    // LOG_H("#");
    // fflush(stdout);
}

