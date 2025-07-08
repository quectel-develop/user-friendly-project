/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-30     chenyong     first version
 * 2018-04-12     chenyong     add client implement
 * 2018-08-17     chenyong     multiple client support
 */

#include <at.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdarg.h"
#include "qosa_def.h"
#include "qosa_log.h"

#define NAME_MAX       8

#define AT_RESP_END_OK                 "OK"
#define AT_RESP_END_ERROR              "ERROR"
#define AT_RESP_END_FAIL               "FAIL"
#define AT_END_CR_LF                   "\r\n"

static struct at_client at_client_table[AT_CLIENT_NUM_MAX] = { 0 };

extern size_t at_utils_send(size_t pos, const void *buffer, size_t size);
extern size_t at_vprintfln(char *send_buf, size_t buf_size, const char *format, va_list args);
extern void   at_print_raw_cmd(const char *type, const char *cmd, size_t size);


/**
 * Create response object.
 *
 * @param buf_size the maximum response buffer size
 * @param line_num the number of setting response lines
 *         = 0: the response data will auto return when received 'OK' or 'ERROR'
 *        != 0: the response data will return when received setting lines number data
 * @param timeout the maximum response time
 *
 * @return != NULL: response object
 *          = NULL: no memory
 */
at_response_t at_create_resp(size_t buf_size, size_t line_num, int32_t timeout)
{
    at_response_t resp = NULL;

    resp = (at_response_t) calloc(1, sizeof(struct at_response));
    if (resp == NULL)
    {
        LOG_E("AT create response object failed! No memory for response object!");
        return NULL;
    }

    resp->buf = (char *) calloc(1, buf_size);
    if (resp->buf == NULL)
    {
        LOG_E("AT create response object failed! No memory for response buffer!");
        free(resp);
        return NULL;
    }

    resp->buf_size = buf_size;
    resp->line_num = line_num;
    resp->line_counts = 0;
    resp->timeout = timeout;
    resp->self_func = NULL;

    return resp;
}

at_response_t at_create_resp_by_selffunc(size_t buf_size, size_t line_num, int32_t timeout, resp_func func)
{
    at_response_t resp = NULL;

    resp = at_create_resp(buf_size, line_num, timeout);
    resp->self_func = func;

    return resp;
}

/**
 * Delete and free response object.
 *
 * @param resp response object
 */
void at_delete_resp(at_response_t resp)
{
    if (resp && resp->buf)
    {
        free(resp->buf);
    }

    if (resp)
    {
        free(resp);
        resp = NULL;
    }
}

/**
 * Set response object information
 *
 * @param resp response object
 * @param buf_size the maximum response buffer size
 * @param line_num the number of setting response lines
 *         = 0: the response data will auto return when received 'OK' or 'ERROR'
 *        != 0: the response data will return when received setting lines number data
 * @param timeout the maximum response time
 *
 * @return  != NULL: response object
 *           = NULL: no memory
 */
at_response_t at_resp_set_info(at_response_t resp, size_t buf_size, size_t line_num, int32_t timeout)
{
    char *p_temp;
    QOSA_ASSERT(resp);

    if (resp->buf_size != buf_size)
    {
        resp->buf_size = buf_size;

        p_temp = (char *)realloc(resp->buf, buf_size);
        if (p_temp == NULL)
        {
            LOG_D("No memory for realloc response buffer size(%d).", buf_size);
            return NULL;
        }
        else
        {
            resp->buf = p_temp;
        }
    }

    resp->line_num = line_num;
    resp->timeout = timeout;

    return resp;
}

/**
 * Get one line AT response buffer by line number.
 *
 * @param resp response object
 * @param resp_line line number, start from '1'
 *
 * @return != NULL: response line buffer
 *          = NULL: input response line error
 */
const char *at_resp_get_line(at_response_t resp, size_t resp_line)
{
    char *resp_buf = resp->buf;
    char *resp_line_buf = NULL;
    size_t line_num = 1;

    QOSA_ASSERT(resp);

    if (resp_line > resp->line_counts || resp_line <= 0)
    {
        LOG_E("AT response get line failed! Input response line(%d) error!", resp_line);
        return NULL;
    }

    for (line_num = 1; line_num <= resp->line_counts; line_num++)
    {
        if (resp_line == line_num)
        {
            resp_line_buf = resp_buf;

            return resp_line_buf;
        }

        resp_buf += strlen(resp_buf) + 1;
    }

    return NULL;
}

/**
 * Get one line AT response buffer by keyword
 *
 * @param resp response object
 * @param keyword query keyword
 *
 * @return != NULL: response line buffer
 *          = NULL: no matching data
 */
const char *at_resp_get_line_by_kw(at_response_t resp, const char *keyword)
{
    char *resp_buf = resp->buf;
    char *resp_line_buf = NULL;
    size_t line_num = 1;

    QOSA_ASSERT(resp);
    QOSA_ASSERT(keyword);

    for (line_num = 1; line_num <= resp->line_counts; line_num++)
    {
        if (strstr(resp_buf, keyword))
        {
            resp_line_buf = resp_buf;

            return resp_line_buf;
        }

        resp_buf += strlen(resp_buf) + 1;
    }

    return NULL;
}

/**
 * Get and parse AT response buffer arguments by line number.
 *
 * @param resp response object
 * @param resp_line line number, start from '1'
 * @param resp_expr response buffer expression
 *
 * @return -1 : input response line number error or get line buffer error
 *          0 : parsed without match
 *         >0 : the number of arguments successfully parsed
 */
int at_resp_parse_line_args(at_response_t resp, size_t resp_line, const char *resp_expr, ...)
{
    va_list args;
    int resp_args_num = 0;
    const char *resp_line_buf = NULL;

    QOSA_ASSERT(resp);
    QOSA_ASSERT(resp_expr);

    if ((resp_line_buf = at_resp_get_line(resp, resp_line)) == NULL)
    {
        return -1;
    }

    va_start(args, resp_expr);

    resp_args_num = vsscanf(resp_line_buf, resp_expr, args);

    va_end(args);

    return resp_args_num;
}

/**
 * Get and parse AT response buffer arguments by keyword.
 *
 * @param resp response object
 * @param keyword query keyword
 * @param resp_expr response buffer expression
 *
 * @return -1 : input keyword error or get line buffer error
 *          0 : parsed without match
 *         >0 : the number of arguments successfully parsed
 */
int at_resp_parse_line_args_by_kw(at_response_t resp, const char *keyword, const char *resp_expr, ...)
{
    va_list args;
    int resp_args_num = 0;
    const char *resp_line_buf = NULL;

    QOSA_ASSERT(resp);
    QOSA_ASSERT(resp_expr);

    if ((resp_line_buf = at_resp_get_line_by_kw(resp, keyword)) == NULL)
    {
        return -1;
    }

    va_start(args, resp_expr);

    resp_args_num = vsscanf(resp_line_buf, resp_expr, args);

    va_end(args);

    return resp_args_num;
}

/**
 * Send commands to AT server and wait response.
 *
 * @param client current AT client object
 * @param resp AT response object, using NULL when you don't care response
 * @param cmd_expr AT commands expression
 *
 * @return 0 : success
 *        -1 : response status error
 *        -2 : wait timeout
 *        -7 : enter AT CLI mode
 */
int at_obj_exec_cmd(at_client_t client, at_response_t resp, const char *cmd_expr, ...)
{
    va_list args;
    int     result = QOSA_OK;

    QOSA_ASSERT(cmd_expr);

    if (client == NULL)
    {
        LOG_E("input AT Client object is NULL, please create or get AT Client object!");
        return QOSA_ERROR_GENERAL;
    }

    /* check AT CLI mode */
    if (client->status == AT_STATUS_CLI && resp)
    {
        return QOSA_ERROR_GENERAL;
    }

    qosa_mutex_lock(client->lock, QOSA_WAIT_FOREVER);

    client->resp_status = AT_RESP_OK;
    client->resp = resp;

    if (resp != NULL)
    {
        resp->buf_len = 0;
        resp->line_counts = 0;
    }

    va_start(args, cmd_expr);
    client->last_cmd_len = at_vprintfln(client->send_buf, client->send_bufsz, cmd_expr, args);
    if (client->last_cmd_len > 2)
    {
        client->last_cmd_len -= 2; /* "\r\n" */
    }
    va_end(args);

    if (resp != NULL)
    {
        if (qosa_sem_wait(client->resp_notice, resp->timeout) != QOSA_OK)
        {
            LOG_W("execute command (%.*s) timeout (%d ticks)!", client->last_cmd_len, client->send_buf, resp->timeout);
            client->resp_status = AT_RESP_TIMEOUT;
            result = QOSA_ERROR_TIMEOUT;
            goto __exit;
        }
        if (client->resp_status != AT_RESP_OK)
        {
            LOG_E("execute command (%.*s) failed!", client->last_cmd_len, client->send_buf);
            result = QOSA_ERROR_GENERAL;
            goto __exit;
        }
    }

__exit:
    client->resp = NULL;

    qosa_mutex_unlock(client->lock);

    return result;
}

/**
 * Waiting for connection to external devices.
 *
 * @param client current AT client object
 * @param timeout millisecond for timeout
 *
 * @return 0 : success
 *        -2 : timeout
 *        -5 : no memory
 */
int at_client_obj_wait_connect(at_client_t client, u32_t timeout_ms)
{
    int           result = QOSA_OK;
    at_response_t resp = NULL;
    u32_t      start_time = 0;

    if (client == NULL)
    {
        LOG_E("input AT client object is NULL, please create or get AT Client object!");
        return QOSA_ERROR_GENERAL;
    }

    resp = at_create_resp(64, 0, 1000);
    if (resp == NULL)
    {
        LOG_E("no memory for AT client response object.");
        return QOSA_ERROR_NO_MEMORY;
    }

    qosa_mutex_lock(client->lock, QOSA_WAIT_FOREVER);
    client->resp = resp;

    start_time = qosa_get_uptime_milliseconds();

    while (1)
    {
        /* Check whether it is timeout */
        if (qosa_get_uptime_milliseconds() - start_time > timeout_ms)
        {
            LOG_E("wait AT client connect timeout(%d tick).", timeout_ms);
            result = QOSA_ERROR_TIMEOUT;
            break;
        }

        /* Check whether it is already connected */
        resp->buf_len = 0;
        resp->line_counts = 0;
        #ifdef AT_PRINT_RAW_CMD
        at_print_raw_cmd("sendline", "AT\r\n", 4);
        #endif
        at_utils_send(0, "AT\r\n", 4);
        if (qosa_sem_wait(client->resp_notice, resp->timeout) != QOSA_OK)
            continue;
        else
            break;
    }

    at_delete_resp(resp);
    client->resp = NULL;
    qosa_mutex_unlock(client->lock);

    return result;
}

/**
 * Send data to AT server, send data don't have end sign(eg: \r\n).
 *
 * @param client current AT client object
 * @param buf   send data buffer
 * @param size  send fixed data size
 *
 * @return >0: send data size
 *         =0: send failed
 */
size_t at_client_obj_send(at_client_t client, const char *buf, size_t size)
{
    size_t len;

    QOSA_ASSERT(buf);

    if (client == NULL)
    {
        LOG_E("input AT Client object is NULL, please create or get AT Client object!");
        return 0;
    }

#ifdef AT_PRINT_RAW_CMD
    at_print_raw_cmd("sendline", buf, size);
#endif

    qosa_mutex_lock(client->lock, QOSA_WAIT_FOREVER);

    len = at_utils_send(0, buf, size);

    qosa_mutex_unlock(client->lock);

    return len;
}

static int at_client_getchar(at_client_t client, char *ch, int32_t timeout)
{
    int result = QOSA_OK;

    while (qosa_uart_read(0, ch, 1) == 0)
    {
        result = qosa_sem_wait(client->rx_notice, timeout);
        if (result != QOSA_OK)
        {
            return result;
        }
    }

    return QOSA_OK;
}

/**
 * AT client receive fixed-length data.
 *
 * @param client current AT client object
 * @param buf   receive data buffer
 * @param size  receive fixed data size
 * @param timeout  receive data timeout (ms)
 *
 * @note this function can only be used in execution function of URC data
 *
 * @return >0: receive data size
 *         =0: receive failed
 */
size_t at_client_obj_recv(at_client_t client, char *buf, size_t size, int32_t timeout)
{
    size_t len = 0;

    QOSA_ASSERT(buf);

    if (client == NULL)
    {
        LOG_E("input AT Client object is NULL, please create or get AT Client object!");
        return 0;
    }

    while (1)
    {
        size_t read_len;

        //rt_sem_control(client->rx_notice, RT_IPC_CMD_RESET, NULL);

        read_len = qosa_uart_read(0, buf + len, size);
        if (read_len > 0)
        {
            len += read_len;
            size -= read_len;
            if (size == 0)
            {
                break;
            }
            continue;
        }

        if (qosa_sem_wait(client->rx_notice, timeout) != QOSA_OK)
        {
            break;
        }
    }

#ifdef AT_PRINT_RAW_CMD
    at_print_raw_cmd("urc_recv", buf, len);
#endif

    return len;
}

size_t at_client_self_recv(at_client_t client, char *buf, size_t size, int32_t timeout,u8_t mode)
{
    size_t read_idx = 0;
    char ch, last_ch = 0;
    int result = QOSA_OK;

    QOSA_ASSERT(buf);

    if (client == NULL)
    {
        LOG_E("input AT Client object is NULL, please create or get AT Client object!");
        return 0;
    }

    while (1) {
           if (read_idx < size)
           {
            result = at_client_getchar(client, &ch, timeout);
            if (result != QOSA_OK)
            {
                LOG_E("AT Client receive failed, uart device get %d data error(%d)", read_idx, result);
                return read_idx;
            }

            buf[read_idx++] = ch;
            if(mode)
            {

                if (last_ch == '\r' && ch == '\n') {
                break;
                }
                 last_ch = ch;
            }


        } else {
            break;
        }
    }

    return read_idx;
}

/**
 *  AT client set end sign.
 *
 * @param client current AT client object
 * @param ch the end sign, can not be used when it is '\0'
 */
void at_obj_set_end_sign(at_client_t client, char ch)
{
    if (client == NULL)
    {
        LOG_E("input AT Client object is NULL, please create or get AT Client object!");
        return;
    }

    client->end_sign = ch;
}

/**
 * set URC(Unsolicited Result Code) table
 *
 * @param client current AT client object
 * @param table URC table
 * @param size table size
 */
int at_obj_set_urc_table(at_client_t client, const struct at_urc *urc_table, size_t table_sz)
{
    size_t idx;

    if (client == NULL)
    {
        LOG_E("input AT Client object is NULL, please create or get AT Client object!");
        return QOSA_ERROR_GENERAL;
    }

    for (idx = 0; idx < table_sz; idx++)
    {
        QOSA_ASSERT(urc_table[idx].cmd_prefix);
        QOSA_ASSERT(urc_table[idx].cmd_suffix);
    }

    if (client->urc_table_size == 0)
    {
        client->urc_table = (struct at_urc_table *) calloc(1, sizeof(struct at_urc_table));
        if (client->urc_table == NULL)
        {
            return QOSA_ERROR_NO_MEMORY;
        }

        client->urc_table[0].urc = urc_table;
        client->urc_table[0].urc_size = table_sz;
        client->urc_table_size++;
    }
    else
    {
        struct at_urc_table *old_urc_table = NULL;
        size_t old_table_size = client->urc_table_size * sizeof(struct at_urc_table);

        old_urc_table = (struct at_urc_table *) malloc(old_table_size);
        if (old_urc_table == NULL)
        {
            return QOSA_ERROR_NO_MEMORY;
        }
        memcpy(old_urc_table, client->urc_table, old_table_size);

        /* realloc urc table space */
        client->urc_table = (struct at_urc_table *) realloc(client->urc_table,
                old_table_size + sizeof(struct at_urc_table));
        if (client->urc_table == NULL)
        {
            free(old_urc_table);
            return QOSA_ERROR_NO_MEMORY;
        }
        memcpy(client->urc_table, old_urc_table, old_table_size);
        client->urc_table[client->urc_table_size].urc = urc_table;
        client->urc_table[client->urc_table_size].urc_size = table_sz;
        client->urc_table_size++;

        free(old_urc_table);
    }

    return QOSA_OK;
}

/**
 * get AT client object by AT device name.
 *
 * @dev_name AT client device name
 *
 * @return AT client object
 */
at_client_t at_client_get(void)
{
    int idx = 0;

    return &at_client_table[idx];
}

/**
 * get first AT client object in the table.
 *
 * @return AT client object
 */
at_client_t at_client_get_first(void)
{
    return &at_client_table[0];
}

static const struct at_urc *get_urc_obj(at_client_t client)
{
    size_t i, j, prefix_len, suffix_len;
    size_t bufsz;
    char *buffer = NULL;
    const struct at_urc *urc = NULL;
    struct at_urc_table *urc_table = NULL;

    if (client->urc_table == NULL)
    {
        return NULL;
    }

    buffer = client->recv_line_buf;
    bufsz = client->recv_line_len;

    for (i = 0; i < client->urc_table_size; i++)
    {
        for (j = 0; j < client->urc_table[i].urc_size; j++)
        {
            urc_table = client->urc_table + i;
            urc = urc_table->urc + j;

            prefix_len = strlen(urc->cmd_prefix);
            suffix_len = strlen(urc->cmd_suffix);
            if (bufsz < prefix_len + suffix_len)
            {
                continue;
            }
            if ((prefix_len ? !strncmp(buffer, urc->cmd_prefix, prefix_len) : 1)
                && (suffix_len ? !strncmp(buffer + bufsz - suffix_len, urc->cmd_suffix, suffix_len) : 1))
            {
                return urc;
            }
        }
    }

    return NULL;
}

static int at_recv_readline(at_client_t client)
{
    size_t      read_len = 0;
    char        ch = 0, last_ch = 0;
    qosa_bool_t is_full = QOSA_FALSE;

    memset(client->recv_line_buf, 0x00, client->recv_bufsz);
    client->recv_line_len = 0;

    while (1)
    {
        ch = 0;
        at_client_getchar(client, &ch, QOSA_WAIT_FOREVER);

        if (read_len < client->recv_bufsz)
        {
            client->recv_line_buf[read_len++] = ch;
            client->recv_line_len = read_len;
        }
        else
        {
            is_full = QOSA_TRUE;
        }

        /* is newline or URC data */
        if ((client->urc = get_urc_obj(client)) != NULL || (ch == '\n' && last_ch == '\r') || (client->end_sign != 0 && ch == client->end_sign))
        {
            if (is_full)
            {
                LOG_E("read line failed. The line data length is out of buffer size(%d)!", client->recv_bufsz);
                memset(client->recv_line_buf, 0x00, client->recv_bufsz);
                client->recv_line_len = 0;
                return QOSA_ERROR_GENERAL;
            }
            break;
        }
        last_ch = ch;
    }

#ifdef AT_PRINT_RAW_CMD
    at_print_raw_cmd("recvline", client->recv_line_buf, read_len);
#endif

    return read_len;
}

static void client_parser(void *argv)
{
    at_client_t client;

    client = (at_client_t)argv;
    while (1)
    {
        if (at_recv_readline(client) > 0)
        {
            if (client->urc != NULL)
            {
                /* current receive is request, try to execute related operations */
                if (client->urc->func != NULL)
                {
                    client->urc->func(client, client->recv_line_buf, client->recv_line_len);
                }
                client->urc = NULL;
            }
            else if (client->resp != NULL)
            {
                at_response_t resp = client->resp;

                if (resp->self_func != NULL)
                {
                    qosa_sem_release(client->resp_notice);
                    resp->self_func(client->recv_line_buf, client->recv_line_len);
                    client->resp = NULL;
                }
                else
                {
                    char end_ch = client->recv_line_buf[client->recv_line_len - 1];
                    /* current receive is response */
                    client->recv_line_buf[client->recv_line_len - 1] = '\0';
                    if (resp->buf_len + client->recv_line_len < resp->buf_size)
                    {
                        /* copy response lines, separated by '\0' */
                        memcpy(resp->buf + resp->buf_len, client->recv_line_buf, client->recv_line_len);

                        /* update the current response information */
                        resp->buf_len += client->recv_line_len;
                        resp->line_counts++;
                    }
                    else
                    {
                        client->resp_status = AT_RESP_BUFF_FULL;
                        LOG_E("Read response buffer failed. The Response buffer size is out of buffer size(%d)!", resp->buf_size);
                    }
                    /* check response result */
                    if ((client->end_sign != 0) && (end_ch == client->end_sign) && (resp->line_num == 0))
                    {
                        /* get the end sign, return response state END_OK.*/
                        client->resp_status = AT_RESP_OK;
                    }
                    else if (memcmp(client->recv_line_buf, AT_RESP_END_OK, strlen(AT_RESP_END_OK)) == 0 && resp->line_num == 0)
                    {
                        /* get the end data by response result, return response state END_OK. */
                        client->resp_status = AT_RESP_OK;
                    }
                    else if (strstr(client->recv_line_buf, AT_RESP_END_ERROR) || (memcmp(client->recv_line_buf, AT_RESP_END_FAIL, strlen(AT_RESP_END_FAIL)) == 0))
                    {
                        client->resp_status = AT_RESP_ERROR;
                    }
                    else if (resp->line_counts == resp->line_num && resp->line_num)
                    {
                        /* get the end data by response line, return response state END_OK.*/
                        client->resp_status = AT_RESP_OK;
                    }
                    else
                    {
                        //LOG_D("continue");
                        continue;
                    }

                    client->resp = NULL;
                    //LOG_D("...");
                    qosa_sem_release(client->resp_notice);
                }
            }
            else
            {
//                LOG_D("unrecognized line: %.*s", client->recv_line_len, client->recv_line_buf);
            }
        }
    }
}

static void at_client_rx_ind(void *argv)
{
    //LOG_D("rx event");
    qosa_sem_release(at_client_table[0].rx_notice);
}

/* initialize the client object parameters */
static int at_client_para_init(at_client_t client)
{
#define AT_CLIENT_LOCK_NAME            "at_c"
#define AT_CLIENT_SEM_NAME             "at_cs"
#define AT_CLIENT_RESP_NAME            "at_cr"
#define AT_CLIENT_THREAD_NAME          "at_clnt"

    int result = QOSA_OK;
    static int at_client_num = 0;
    char name[NAME_MAX];

    client->status = AT_STATUS_UNINITIALIZED;

    client->recv_line_len = 0;
    client->recv_line_buf = (char *) calloc(1, client->recv_bufsz);
    if (client->recv_line_buf == NULL)
    {
        LOG_E("AT client initialize failed! No memory for receive buffer.");
        result = QOSA_ERROR_NO_MEMORY;
        goto __exit;
    }

    client->last_cmd_len = 0;
    client->send_buf = (char *)calloc(1, client->send_bufsz);
    if (client->send_buf == NULL)
    {
        LOG_E("AT client initialize failed! No memory for send buffer.");
        result = QOSA_ERROR_NO_MEMORY;
        goto __exit;
    }

    snprintf(name, NAME_MAX, "%s%d", AT_CLIENT_LOCK_NAME, at_client_num);
    qosa_mutex_create(&client->lock);
    if (client->lock == NULL)
    {
        LOG_E("AT client initialize failed! at_client_recv_lock create failed!");
        result = QOSA_ERROR_NO_MEMORY;
        goto __exit;
    }

    snprintf(name, NAME_MAX, "%s%d", AT_CLIENT_SEM_NAME, at_client_num);
    qosa_sem_create(&client->rx_notice, 0);
    if (client->rx_notice == NULL)
    {
        LOG_E("AT client initialize failed! at_client_notice semaphore create failed!");
        result = QOSA_ERROR_NO_MEMORY;
        goto __exit;
    }

    snprintf(name, NAME_MAX, "%s%d", AT_CLIENT_RESP_NAME, at_client_num);
    qosa_sem_create(&client->resp_notice, 0);
    if (client->resp_notice == NULL)
    {
        LOG_E("AT client initialize failed! at_client_resp semaphore create failed!");
        result = QOSA_ERROR_NO_MEMORY;
        goto __exit;
    }

    client->urc_table = NULL;
    client->urc_table_size = 0;

    snprintf(name, NAME_MAX, "%s%d", AT_CLIENT_THREAD_NAME, at_client_num);
    LOG_D("name=%s", name);
    result = qosa_task_create(&client->parser, 4*1024, QOSA_PRIORITY_NORMAL, name, client_parser, (void *)client);
    if (client->parser == NULL)
    {
        result = QOSA_ERROR_NO_MEMORY;
        goto __exit;
    }

__exit:
    if (result != QOSA_OK)
    {
        if (client->lock)
        {
            qosa_mutex_delete(client->lock);
        }

        if (client->rx_notice)
        {
            qosa_sem_delete(client->rx_notice);
        }

        if (client->resp_notice)
        {
            qosa_sem_delete(client->resp_notice);
        }

        if (client->recv_line_buf)
        {
            free(client->recv_line_buf);
        }

        memset(client, 0x00, sizeof(struct at_client));
    }

    LOG_D("%s over(%x)",__FUNCTION__, client->parser);
    return result;
}

/**
 * AT client initialize.
 *
 * @param dev_name AT client device name
 * @param recv_bufsz the maximum number of receive buffer length
 *
 * @return 0 : initialize success
 *        -1 : initialize failed
 *        -5 : no memory
 */
int at_client_init(size_t recv_bufsz, size_t send_bufsz)
{
    int         idx = 0;
    int         result = QOSA_OK;
    int         open_result = QOSA_OK;
    at_client_t client = NULL;

    QOSA_ASSERT(recv_bufsz > 0);
    QOSA_ASSERT(send_bufsz > 0);

    client = &at_client_table[idx];
    client->recv_bufsz = recv_bufsz;
    client->send_bufsz = send_bufsz;

    result = at_client_para_init(client);
    if (result != QOSA_OK)
    {
        goto __exit;
    }
    open_result = qosa_uart_open();
    QOSA_ASSERT(open_result == QOSA_OK);
    qosa_uart_register(at_client_rx_ind);

__exit:
    if (result == QOSA_OK)
    {
        client->status = AT_STATUS_INITIALIZED;

        //qosa_task_delete(client->parser);

        LOG_I("AT client(V%s) initialize success.", AT_SW_VERSION);
    }
    else
    {
        LOG_E("AT client(V%s) initialize failed(%d).", AT_SW_VERSION, result);
    }

    return result;
}

