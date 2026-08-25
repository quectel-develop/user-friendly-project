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
#include "qosa_time.h"
#include "sd_fatfs.h"
#include "ql_module_compat.h"
#include "ql_dev.h"

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

    LOG_D("Now restart the module...");
    HAL_GPIO_WritePin(UFP_RESET_PORT, UFP_RESET_PIN, GPIO_PIN_SET);
    qosa_task_sleep_ms(500);
    HAL_GPIO_WritePin(UFP_RESET_PORT, UFP_RESET_PIN, GPIO_PIN_RESET);
    qosa_task_sleep_ms(100);
    HAL_GPIO_WritePin(UFP_PWRKEY_PORT, UFP_PWRKEY_PIN, GPIO_PIN_SET);
    qosa_task_sleep_ms(2000);
    HAL_GPIO_WritePin(UFP_PWRKEY_PORT, UFP_PWRKEY_PIN, GPIO_PIN_RESET);
    qosa_task_sleep_ms(2000);
    LOG_D("Restart module done.");
    qosa_uart_hardware_init();
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

int ql_wait_module_ready(at_client_t client, u32_t timeout)
{
    int result = 0;

    at_response_t resp = at_create_resp(128, 0, 5000);
    u64_t start_time = time(NULL);

    while (1)
    {
        /* Check whether it is timeout */
        if (time(NULL) - start_time > timeout)
        {
            LOG_E("wait AT client connect timeout(%d tick).", timeout);
            result = -1;
            break;
        }

        if (at_obj_exec_cmd(client, resp, "AT") == QOSA_OK)
        {
            result = 0;
            break;
        }
    }

    at_delete_resp(resp);
    return result;
}

int ql_echo_mode_enable(at_client_t client, bool onoff)
{
    at_response_t resp = at_create_resp_new(128, 0, (5000), NULL);
    char* cmd = onoff ? "ATE1" : "ATE0";

    at_obj_exec_cmd(client, resp, cmd);
    at_delete_resp(resp);
    return 0;
}

int ql_set_urc_cfg(at_client_t client, bool main)
{
    at_response_t resp = at_create_resp_new(128, 0, (5000), NULL);
    char* content = main ? "main" : "uart1";

    at_obj_exec_cmd(client, resp, "AT+QURCCFG=\"urcport\",\"%s\"", content);
    at_delete_resp(resp);
    return 0;
}

int ql_uart_flow_control_enable(at_client_t client, bool onoff)
{
    at_response_t resp = at_create_resp_new(128, 0, (5000), NULL);
    char* cmd = onoff ? "AT+IFC=2,2" : "AT+IFC=0,0";

    if (at_obj_exec_cmd(client, resp, cmd) < 0)
    {
        at_delete_resp(resp);
        LOG_E("AT+IFC failed.");
        return -1;
    }
    at_delete_resp(resp);
    return 0;
}
static void ql_net_query_moudle_model(at_client_t client)
{
    at_response_t query_resp = NULL;
    // Creating a response object for AT command
    query_resp = at_create_resp(256, 0, (5000));
    if (NULL == query_resp)
    {
        LOG_E("No memory for response object.");
        return;
    }

    // Sending the AT command to query EMM reject cause value
    if (at_obj_exec_cmd(client, query_resp, "ATI") < 0)
    {
        LOG_E("ATI query failed.");
        at_delete_resp(query_resp);
        return;
    }

    // Parsing the response to extract EMM reject cause value
    for (int i = 0; i < query_resp->line_counts; i++)
    {
        const char *line = at_resp_get_line(query_resp, i + 1);
        if (strstr(line, "ATI") != NULL || strstr(line, "Quectel") != NULL
            || strstr(line, "Revision") != NULL || strcmp(line, "\r") == 0
            || strstr(line, "OK") != NULL)
            continue;
        ql_set_module_model(line);
    }

    // If the expected format is not found in the response
    at_delete_resp(query_resp);
}
int ql_at_uart_init(at_client_t client)
{
    int ret = 0;
    qosa_task_sleep_ms(500);

    ret += at_client_init(client, 1024, 1024);
    ret += ql_wait_module_ready(client, 20000);
    ret += ql_uart_flow_control_enable(client, true);
    ret += ql_echo_mode_enable(client, false);
    ql_net_query_moudle_model(client);
    ql_set_urc_cfg(client, ql_urccfg_is_main());

    return ret;
}
