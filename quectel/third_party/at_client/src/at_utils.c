/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-04-14     chenyong     first version
 */

#include <at.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "qosa_def.h"
#include "qosa_system.h"
#include "hal_uart.h"
#include "qosa_log.h"
/**
 * dump hex format data to console device
 *
 * @param name name for hex object, it will show on log header
 * @param buf hex buffer
 * @param size buffer size
 */
extern LogLevel g_debug_level;

void at_print_raw_cmd(const char *name, const char *buf, size_t size)
{
#define __is_print(ch)       ((unsigned int)((ch) - ' ') < 127u - ' ')
#define WIDTH_SIZE           96

    size_t i, j;
    int index = 0;
    char buffer[128] = {0};

    for (i = 0; i < size; i += WIDTH_SIZE)
    {
        index = 0;
        memset(buffer, 0, sizeof(buffer));
        index += sprintf(buffer, "\t%s: ", name);
        for (j = 0; j < WIDTH_SIZE; j++)
        {
            if (i + j < size)
            {
                //rt_kprintf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
                index += sprintf(buffer+index, "%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        LOG_D("%s", buffer);
    }
}

size_t at_utils_send(size_t pos, const void *buffer, size_t size)
{
    return qosa_uart_write(pos, buffer, size);
}

size_t at_vprintf(char *send_buf, size_t buf_size, const char *format, va_list args)
{
    size_t len = vsnprintf(send_buf, buf_size, format, args);
    if (len == 0)
    {
        return 0;
    }

#ifdef AT_PRINT_RAW_CMD
    at_print_raw_cmd("sendline", send_buf, len);
#endif

    return at_utils_send(0, send_buf, len);
}

size_t at_vprintfln(char *send_buf, size_t buf_size, const char *format, va_list args)
{
    size_t len = vsnprintf(send_buf, buf_size - 2, format, args);
    if (len == 0)
    {
        return 0;
    }

    send_buf[len++] = '\r';
    // send_buf[len++] = '\n';

#ifdef AT_PRINT_RAW_CMD
    at_print_raw_cmd("sendline", send_buf, len);
#endif

    return at_utils_send(0, send_buf, len);
}
