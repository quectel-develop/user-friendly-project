/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-30     chenyong     first version
 * 2018-08-17     chenyong     multiple client support
 */

#ifndef __AT_H__
#define __AT_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "qosa_def.h"
#include "qosa_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define AT_PRINT_RAW_CMD

#define AT_SW_VERSION                  "1.3.1"

#define AT_CMD_NAME_LEN                16
#define AT_END_MARK_LEN                4

#ifndef AT_CMD_MAX_LEN
#define AT_CMD_MAX_LEN                 128
#endif

/* the server AT commands new line sign */
#if defined(AT_CMD_END_MARK_CRLF)
#define AT_CMD_END_MARK                "\r\n"
#elif defined(AT_CMD_END_MARK_CR)
#define AT_CMD_END_MARK                "\r"
#elif defined(AT_CMD_END_MARK_LF)
#define AT_CMD_END_MARK                "\n"
#endif

typedef enum
{
    AT_STATUS_UNINITIALIZED = 0,
    AT_STATUS_INITIALIZED,
    AT_STATUS_CLI,
} at_status_t;


typedef enum
{
    AT_RESP_OK = 0,         /* AT response end is OK */
    AT_RESP_ERROR = -1,     /* AT response end is ERROR */
    AT_RESP_TIMEOUT = -2,   /* AT response is timeout */
    AT_RESP_BUFF_FULL = -3, /* AT response buffer is full */
} at_resp_status_t;

typedef void (*resp_func)(const char *, size_t, void *);
struct at_response
{
    resp_func self_func;

    /* response buffer */
    char *buf;
    /* the maximum response buffer size, it set by `at_create_resp()` function */
    size_t buf_size;
    /* the length of current response buffer */
    size_t buf_len;
    /* the number of setting response lines, it set by `at_create_resp()` function
     * == 0: the response data will auto return when received 'OK' or 'ERROR'
     * != 0: the response data will return when received setting lines number data */
    size_t line_num;
    /* the count of received response lines */
    size_t line_counts;
    /* the maximum response time */
    int32_t timeout;
    void *arg;
};

typedef struct at_response *at_response_t;

struct at_client
{
    at_status_t status;
    char        end_sign;
    char       *send_buf;
    /* The maximum supported send cmd length */
    size_t send_bufsz;
    /* The length of last cmd */
    size_t last_cmd_len;
    /* the current received one line data buffer */
    char *recv_line_buf;
    /* The length of the currently received one line data */
    size_t recv_line_len;
    /* The maximum supported receive data length */
    size_t               recv_bufsz;
    osa_sem_t            rx_notice;
    osa_mutex_t          lock;
    at_response_t        resp;
    osa_sem_t            resp_notice;
    osa_sem_t            data_send_notice;
    at_resp_status_t     resp_status;
    struct at_urc_table *urc_table;
    size_t               urc_table_size;
    const struct at_urc *urc;
    osa_task_t           parser;
    resp_func            self_func;
    void *               arg;
};
typedef struct at_client *at_client_t;

/* URC(Unsolicited Result Code) object, such as: 'RING', 'READY' request by AT server */
struct at_urc
{
    const char *cmd_prefix;
    const char *cmd_suffix;
    void (*func)(struct at_client *client, const char *data, size_t size, void *arg);
};
typedef struct at_urc *at_urc_t;

struct at_urc_table
{
    size_t urc_size;
    const struct at_urc *urc;
};
typedef struct at_urc *at_urc_table_t;


/* ========================== multiple AT client function ============================ */
/* AT client initialize and start*/
int at_client_init(at_client_t client, size_t recv_bufsz, size_t send_bufsz);
/* get AT client object */
at_client_t at_client_get(void);
at_client_t at_client_get_first(void);

/* AT client wait for connection to external devices. */

/* AT client send or receive data */
size_t at_client_obj_send(at_client_t client, const char *buf, size_t size, bool print);
size_t at_client_obj_send_nolock(at_client_t client, const char *buf, size_t size, bool print);
size_t at_client_obj_recv(at_client_t client, char *buf, size_t size, int32_t timeout, bool print);
size_t at_client_self_recv(at_client_t client, char *buf, size_t size, int32_t timeout,u8_t mode, bool print);
/* set AT client a line end sign */
void at_obj_set_end_sign(at_client_t client, char ch);

/* Set URC(Unsolicited Result Code) table */
int at_obj_set_urc_table(at_client_t client, const struct at_urc * table, size_t size);

/* AT client send commands to AT server and waiter response */
int at_obj_exec_cmd(at_client_t client, at_response_t resp, const char *cmd_expr, ...);

int at_obj_exec_cmd_with_data(at_client_t client, const char *cmd, const char *data, size_t size);
/* AT response object create and delete */
at_response_t at_create_resp(size_t buf_size, size_t line_num, int32_t timeout);

at_response_t at_create_resp_new(size_t buf_size, size_t line_num, int32_t timeout, void *arg);
void at_delete_resp(at_response_t resp);
at_response_t at_resp_set_info(at_response_t resp, size_t buf_size, size_t line_num, int32_t timeout);
at_response_t at_resp_set_info_new(at_response_t resp, size_t buf_size, size_t line_num, int32_t timeout, void *arg);
at_response_t at_create_resp_by_selffunc(size_t buf_size, size_t line_num, int32_t timeout, resp_func func);
at_response_t at_create_resp_by_selffunc_new(size_t buf_size, size_t line_num, int32_t timeout, resp_func func, void *arg);

/* AT response line buffer get and parse response buffer arguments */
const char *at_resp_get_line(at_response_t resp, size_t resp_line);
const char *at_resp_get_line_by_kw(at_response_t resp, const char *keyword);
int at_resp_parse_line_args(at_response_t resp, size_t resp_line, const char *resp_expr, ...);
int at_resp_parse_line_args_by_kw(at_response_t resp, const char *keyword, const char *resp_expr, ...);

/* ========================== single AT client function ============================ */

/**
 * NOTE: These functions can be used directly when there is only one AT client.
 * If there are multiple AT Client in the program, these functions can operate on the first initialized AT client.
 */

#define at_exec_cmd(resp, ...)                   at_obj_exec_cmd(at_client_get_first(), resp, __VA_ARGS__)
#define at_client_send(buf, size)                at_client_obj_send(at_client_get_first(), buf, size, true)
#define at_client_recv(buf, size, timeout)       at_client_obj_recv(at_client_get_first(), buf, size, timeout, true)
#define at_self_recv(buf, size, timeout,mode)           at_client_self_recv(at_client_get_first(), buf, size, timeout,mode)

#define at_set_end_sign(ch)                      at_obj_set_end_sign(at_client_get_first(), ch)
#define at_set_urc_table(urc_table, table_sz)    at_obj_set_urc_table(at_client_get_first(), urc_table, table_sz)


/* ========================== User port function ============================ */

#ifdef __cplusplus
}
#endif

#endif /* __AT_H__ */
