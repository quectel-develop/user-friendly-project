#include "QuectelConfig.h"
#include "spi_flash.h"
#include "bsp_driver_sd.h"
#include "stm32f4xx_hal_cortex.h"
#include "fatfs.h"
#include "sdio.h"
#include "main.h"
#include "hal_common.h"
#include "hal_uart.h"
#include "qosa_system.h"
#include "qosa_log.h"
#include "sd_fatfs.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#include "windows.h"
#elif __linux__
#else
#include "cmsis_os.h"
#endif


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

int cli_mcu_firmware_version(s32_t argc, char *argv[])
{
    LOG_I("Current version: %s", QUECTEL_PROJECT_VERSION);
    return 0;
}

int cli_reboot(s32_t argc, char *argv[])
{
    LOG_I("system reboot...");
    HAL_NVIC_SystemReset();
    return 0;
}

void cli_reboot_help(void)
{
    LOG_I("reboot immediately.");
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


void ql_sdcard_hotplug_proc(void)
{
    qosa_task_sleep_ms(200);
    bool onoff = BSP_SD_IsDetected();
    if (onoff == ql_sd_is_init())
    {
        LOG_I("sd cur state is %s", onoff ? "ON" : "OFF");
        return;
    }
    LOG_D("sd card status change %s", onoff ? "ON" : "OFF");
    if (onoff)
    {
        MX_SDIO_SD_Init();
        qosa_task_sleep_ms(50);
        MX_FATFS_Init();
        if(!ql_sd_init())
        {
            LOG_E("Hot-swap failed. please try again later.");
            ql_sd_deinit();
            qosa_task_sleep_ms(2000);
        }
    }
    else
    {
        ql_sd_deinit();
    }
}
