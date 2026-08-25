#include "main.h"
#ifdef __APP_USE_RTOS__
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "qosa_def.h"
#include "qosa_log.h"
#include "hal_uart.h"
#include "ringbuffer.h"
#include "hal_common.h"

/*###############################################################################################*/
/*---------------------------------------- Debug UART -------------------------------------------*/
/*###############################################################################################*/
extern UART_HandleTypeDef DEBUG_UART_HANDLE;
uint8_t  g_debug_uart_buf[256];
uint16_t g_debug_uart_buf_get_len = 0;
static bool g_flow_control = false;
static uint8_t *g_uart2_left_data = NULL;
static size_t g_uart2_left_data_len = 0;
extern void debug_uart_input_notify(void);


/* ------------------ Redirect printf() and scanf() ------------------ */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
// #define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART6 and Loop until the end of transmission */
    HAL_UART_Transmit(&DEBUG_UART_HANDLE, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
// GETCHAR_PROTOTYPE
// {
//     uint8_t ch = 0;
//     /* Clear the Overrun flag just before receiving the first character */
//     __HAL_UART_CLEAR_OREFLAG(&DEBUG_UART_HANDLE);
//     /* Wait for reception of a character on the USART RX line and echo this character on console */
//     HAL_UART_Receive(&DEBUG_UART_HANDLE, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
//     HAL_UART_Transmit(&DEBUG_UART_HANDLE, (uint8_t *)&ch, 1, 0);
//     return ch;
// }
/* ------------------------------------------------------------------- */


void USER_Debug_UART_RxIdleCallback(UART_HandleTypeDef *huart)
{
    static uint16_t RxXferCountOld = sizeof(g_debug_uart_buf)/sizeof(g_debug_uart_buf[0]);
    int num = RxXferCountOld - huart->RxXferCount;
    unsigned char back_space[3] = {0x8, 0x20, 0x8};

    if (huart->RxXferSize - huart->RxXferCount >= sizeof(g_debug_uart_buf) - 1)
    {
        printf("\r\n");
        LOG_E("input characters is over max length!");
        RxXferCountOld = sizeof(g_debug_uart_buf)/sizeof(g_debug_uart_buf[0]);
        g_debug_uart_buf_get_len = 0;
        huart->RxState = HAL_UART_STATE_READY;
        HAL_UART_Receive_IT(huart, (uint8_t *)g_debug_uart_buf, sizeof(g_debug_uart_buf)/sizeof(g_debug_uart_buf[0]));
        return;
    }

    uint8_t* buf = &g_debug_uart_buf[huart->RxXferSize - huart->RxXferCount - num];
    if ((num == 1) && (buf[0] == 0x8 || buf[0] == 0x7F)) // backspace
    {
        if (huart->RxXferCount == huart->RxXferSize - 1)
        {
            huart->pRxBuffPtr  -= 1;
            huart->RxXferCount += 1;
        }
        else
        {
            HAL_UART_Transmit(&DEBUG_UART_HANDLE, back_space, 3, 1000);
            huart->pRxBuffPtr  -= 2;
            huart->RxXferCount += 2;
        }
    }
    else if ((num >= 4) && (buf[0] == 0x1B) && (buf[1] == '[') && (buf[2] == '3') && (buf[3] == '~')) // delete character
    {
        if (huart->RxXferCount <= huart->RxXferSize - 5)
        {
            HAL_UART_Transmit(&DEBUG_UART_HANDLE, back_space, 3, 1000);
            huart->pRxBuffPtr  -= 5;
            huart->RxXferCount += 5;
        }
        else
        {
            huart->pRxBuffPtr  -= 4;
            huart->RxXferCount += 4;
        }
    }
    else
    {
        HAL_UART_Transmit(&DEBUG_UART_HANDLE, &g_debug_uart_buf[huart->RxXferSize-huart->RxXferCount-num], num, 1000);
    }

    RxXferCountOld = huart->RxXferCount;
    if ((huart->RxXferSize - huart->RxXferCount) >= 1 && (g_debug_uart_buf[huart->RxXferSize - huart->RxXferCount - 1] == 0x0d))
    {
        RxXferCountOld = sizeof(g_debug_uart_buf)/sizeof(g_debug_uart_buf[0]);
        huart->RxState = HAL_UART_STATE_READY;
        g_debug_uart_buf_get_len = huart->RxXferSize-huart->RxXferCount - 1;
        g_debug_uart_buf[g_debug_uart_buf_get_len] = '\0';
        debug_uart_input_notify();
    }
    else if ((huart->RxXferSize - huart->RxXferCount) >= 2 && (g_debug_uart_buf[huart->RxXferSize - huart->RxXferCount - 2] == 0x0d))
    {
        RxXferCountOld = sizeof(g_debug_uart_buf)/sizeof(g_debug_uart_buf[0]);
        huart->RxState = HAL_UART_STATE_READY;
        g_debug_uart_buf_get_len = huart->RxXferSize-huart->RxXferCount - 2;
        g_debug_uart_buf[g_debug_uart_buf_get_len] = '\0';
        debug_uart_input_notify();
    }

    HAL_UART_Receive_IT(huart, (uint8_t *)g_debug_uart_buf, sizeof(g_debug_uart_buf)/sizeof(g_debug_uart_buf[0]));
}


void qosa_uart_get_debug_input(const uint8_t **pData, uint16_t *pSize)
{
    *pData = g_debug_uart_buf;
    *pSize = g_debug_uart_buf_get_len;
}

void debug_uart_hardware_init(void)
{
    HAL_UART_Receive_IT(&DEBUG_UART_HANDLE, (uint8_t *)g_debug_uart_buf, sizeof(g_debug_uart_buf)/sizeof(g_debug_uart_buf[0])); // Interrupt reception
    __HAL_UART_ENABLE_IT(&DEBUG_UART_HANDLE, UART_IT_IDLE); // Enable IDLE interrupt
}




/*###############################################################################################*/
/*------------------------------------------ AT UART --------------------------------------------*/
/*###############################################################################################*/
extern UART_HandleTypeDef AT_UART_HANDLE;
uint8_t  g_uart_buf[256];
ringbuffer_t   g_uart_rb = {.buffer = NULL, .buffer_size = 0, .head = 0, .tail = 0};
static uint8_t g_uart_rb_buf[RINGBUFFER_SIZE] = {0};
static event_callback_t g_rx_ind_cb = NULL;

static s32_t rt_device_cb(void *buffer, s32_t size)
{
    int ret = ringbuffer_put(&g_uart_rb, buffer, size);
    if (g_rx_ind_cb)
        g_rx_ind_cb();
    if (ret < size)
    {
        if (!g_flow_control)
        {
            HAL_GPIO_WritePin(UFP_AT_UART_RTS_PORT, UFP_AT_UART_RTS_PIN, GPIO_PIN_SET);
            g_flow_control = true;
        }
        if (g_uart2_left_data != NULL && g_uart2_left_data_len > 0)
        {
            uint8_t* temp_buffer = (uint8_t*)malloc(g_uart2_left_data_len + size - ret);
            memcpy(temp_buffer, g_uart2_left_data, g_uart2_left_data_len);
            memcpy(temp_buffer + g_uart2_left_data_len, buffer + ret, size - ret);
            free(g_uart2_left_data);
            g_uart2_left_data = temp_buffer;
            g_uart2_left_data_len += size - ret;
        }
        else
        {
            g_uart2_left_data = (uint8_t*)malloc(size - ret);
            memcpy(g_uart2_left_data, buffer + ret, size - ret);
            g_uart2_left_data_len = size - ret;
        }
        return QOSA_ERROR_BUSY;
    }
    return QOSA_OK;
}

s32_t rt_device_close(void)
{
    return QOSA_OK;
}

// https://blog.csdn.net/a1598025967/article/details/120539170
static uint32_t g_head_ptr = 0;
void USER_AT_UART_RxIdleCallback(UART_HandleTypeDef *huart)
{
    uint32_t tail_ptr;
    uint32_t copy, offset;

    /*
     * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
     * |     head_ptr          tail_ptr         |
     * |         |                 |            |
     * |         v                 v            |
     * | --------*******************----------- |
     * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
     */

    /* Already received */
    if (huart->RxXferSize == __HAL_DMA_GET_COUNTER(huart->hdmarx)) // just interrupted no data
    {
        g_head_ptr = 0;
        return;
    }
    tail_ptr = huart->RxXferSize - __HAL_DMA_GET_COUNTER(huart->hdmarx);

    offset = g_head_ptr % huart->RxXferSize;
    copy = tail_ptr - offset;
    g_head_ptr += copy;

    rt_device_cb(huart->pRxBuffPtr + offset, copy);
}

void USER_AT_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    uint32_t tail_ptr;
    uint32_t offset, copy;

    /*
     * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
     * |                  half                  |
     * |     head_ptr   tail_ptr                |
     * |         |          |                   |
     * |         v          v                   |
     * | --------*******************----------- |
     * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
     */

    tail_ptr = (huart->RxXferSize >> 1) + (huart->RxXferSize & 1);

    offset = g_head_ptr % huart->RxXferSize;
    copy = tail_ptr - offset;
    g_head_ptr += copy;

    rt_device_cb(huart->pRxBuffPtr + offset, copy);
}

void USER_AT_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint32_t tail_ptr;
    uint32_t offset, copy;

    /*
     * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
     * |                  half                  |
     * |                    | head_ptr tail_ptr |
     * |                    |    |            | |
     * |                    v    v            v |
     * | ------------------------************** |
     * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
     */

    tail_ptr = huart->RxXferSize;

    offset = g_head_ptr % huart->RxXferSize;
    copy = tail_ptr - offset;
    g_head_ptr += copy;

    rt_device_cb(huart->pRxBuffPtr + offset, copy);
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    /* Ignore half-full interrupts when receiving a buf size of 1 */
    if(1 == huart->RxXferSize) { return; }

    if((huart->Instance) == AT_UART)
    {
        USER_AT_UART_RxHalfCpltCallback(huart);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if((huart->Instance) == AT_UART)
    {
        if(NULL == huart->hdmarx)
        {
            /* If in interrupt mode, restore the receive address pointer to the initialized buffer position */
            huart->pRxBuffPtr -= huart->RxXferSize;
        }

        USER_AT_UART_RxCpltCallback(huart);

        if(NULL != huart->hdmarx)
        {
            if(huart->hdmarx->Init.Mode != DMA_CIRCULAR)
            {
                while(HAL_OK != HAL_UART_Receive_DMA(huart, huart->pRxBuffPtr, huart->RxXferSize))
                {
                    __HAL_UNLOCK(huart);
                }
            }
        }
        else
        {
            while(HAL_UART_Receive_IT(huart, huart->pRxBuffPtr, huart->RxXferSize))
            {
                __HAL_UNLOCK(huart);
            }
        }
    }
}

/**
  * @brief  UART error callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    __IO uint32_t tmpErr = 0x00U;

    tmpErr = HAL_UART_GetError(huart);
    if(HAL_UART_ERROR_NONE == tmpErr)
    {
        return ;
    }

    switch(tmpErr)
    {
        case HAL_UART_ERROR_PE:
                __HAL_UART_CLEAR_PEFLAG(huart);
                break;
        case HAL_UART_ERROR_NE:
                __HAL_UART_CLEAR_NEFLAG(huart);
                break;
        case HAL_UART_ERROR_FE:
                __HAL_UART_CLEAR_FEFLAG(huart);
                break;
        case HAL_UART_ERROR_ORE:
                __HAL_UART_CLEAR_OREFLAG(huart);
                break;
        case HAL_UART_ERROR_DMA:
                break;
        default:
                break;
    }

    if(NULL != huart->hdmarx)
    {
        while(HAL_UART_Receive_DMA(huart, huart->pRxBuffPtr, huart->RxXferSize))
        {
            __HAL_UNLOCK(huart);
        }
    }
    else
    {
        /* Restore the receiving address pointer to the initial buffer position, initial address = current address - Number of data received, number of data received = Number of data to be received - number of not received*/
        while(HAL_UART_Receive_IT(huart, huart->pRxBuffPtr - (huart->RxXferSize - huart->RxXferCount), huart->RxXferSize))
        {
            __HAL_UNLOCK(huart);
        }
    }
}


void at_uart_hardware_init(void)
{
    if(g_uart_rb.buffer == NULL)
        ringbuffer_init(&g_uart_rb, g_uart_rb_buf, RINGBUFFER_SIZE);
    HAL_UART_Receive_DMA(&AT_UART_HANDLE, (uint8_t *)g_uart_buf, sizeof(g_uart_buf)/sizeof(g_uart_buf[0])); // DMA reception
    __HAL_UART_ENABLE_IT(&AT_UART_HANDLE, UART_IT_IDLE); // Enable IDLE interrupt
}

int qosa_uart_open(void)
{
    if(g_uart_rb.buffer == NULL)
        ringbuffer_init(&g_uart_rb, g_uart_rb_buf, RINGBUFFER_SIZE);

    return QOSA_OK;
}

int qosa_uart_read(int pos, const char *buffer, size_t size)
{
    if (NULL == g_uart_rb.buffer)
        return 0;
    if(ringbuffer_length(&g_uart_rb) > 0)
    {
        if (ringbuffer_length(&g_uart_rb) < g_uart_rb.buffer_size / 3 && g_flow_control)
        {
            ringbuffer_put(&g_uart_rb, g_uart2_left_data, g_uart2_left_data_len);
            free(g_uart2_left_data);
            g_uart2_left_data = NULL;
            g_uart2_left_data_len = 0;
            HAL_GPIO_WritePin(UFP_AT_UART_RTS_PORT, UFP_AT_UART_RTS_PIN, GPIO_PIN_RESET);
            g_flow_control = false;
            LOG_I("stop flow control");
        }
        int get_length =  ringbuffer_get(&g_uart_rb, (uint8_t*)buffer, size);
       return get_length;
    }

    return 0;
}

int qosa_uart_write(int pos, const char *buffer, size_t size)
{
    HAL_UART_Transmit(&AT_UART_HANDLE, (const uint8_t*)buffer, size, 0xFFFF);
    return size;
}

void qosa_uart_register(event_callback_t event_cb)
{
    QOSA_ASSERT(event_cb != QOSA_NULL);
    g_rx_ind_cb = event_cb;
}



/*###############################################################################################*/
/*------------------------------------------ COMMON ---------------------------------------------*/
/*###############################################################################################*/
void qosa_uart_hardware_init(void)
{
    at_uart_hardware_init();
    debug_uart_hardware_init();
}

#endif /* __APP_USE_RTOS__ */
