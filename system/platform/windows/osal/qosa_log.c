/*****************************************************************/ /**
* @file qosa_temer.c
* @brief
* @author larson.li@quectel.com
* @date 2024-12-23
*
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description
* <tr><td>2024-12-23 <td>1.0 <td>Larson.Li <td> Init
* </table>
**********************************************************************/
#include <time.h>

#include "qosa_def.h"
#include "qosa_system.h"
#include "qosa_log.h"
#include "debug_service.h"
// 日志模块
typedef struct
{
    LogLevel         level;        // 当前日志等级
    FILE            *log_file;     // 日志文件
    int              log_to_file;  // 是否写入文件 (0: 否, 1: 是)
    CRITICAL_SECTION lock;         // 线程安全锁
} Logger;

// 全局日志实例
static Logger logger = {LOG_INFO, NULL};
static char   g_printf_buf[1024] = {0};
static char   time_str[20] = {0};
static osa_task_t g_serial_input_thread_id =NULL;

// 获取当前时间字符串
static void get_current_time(char *buffer, size_t size)
{
    time_t    now = time(NULL);
    struct tm tm_info;
    localtime_s(&tm_info, &now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info);
}

#define MAX_LENGTH 256
static uint8_t g_uart_input_buf[MAX_LENGTH];
static uint32_t g_uart_input_buf_get_len = 0;

static void* cli_input_parse_thread_proc(void* pThreadParam)
{
	int32_t ret;
	uint16_t Size;

	LOG_V("%s",__FUNCTION__);

	while (1)
	{
        // 从标准输入读取一行数据
        memset(g_uart_input_buf, 0, MAX_LENGTH);
        if (fgets(g_uart_input_buf, MAX_LENGTH, stdin) != NULL)
        {
            // 去除换行符
            size_t len = strlen(g_uart_input_buf);
            if (len > 0 && g_uart_input_buf[len - 1] == '\n')
            {
                g_uart_input_buf[len - 1] = '\0';
            }
            // 输出读取到的数据
            printf("input:%s, len = %d(0x%x, 0x%x, 0x%x)\r\n", g_uart_input_buf, len, g_uart_input_buf[0], g_uart_input_buf[1], g_uart_input_buf[2]);
            debug_service_cmd_exec(g_uart_input_buf);
        }
	}
	LOG_V("%s over",__FUNCTION__);
}

// 初始化日志模块
void init_logger(LogLevel level, const char *file_path)
{
    int ret = QOSA_OK;
    logger.level = level;
    InitializeCriticalSection(&logger.lock);

    if (file_path != NULL)
    {
        logger.log_file = fopen(file_path, "a");
        if (!logger.log_file)
        {
            fprintf(stderr, "Failed to open log file: %s\n", file_path);
            exit(EXIT_FAILURE);
        }
    }

	ret = qosa_task_create(&g_serial_input_thread_id, 512*12, QOSA_PRIORITY_NORMAL, "Debug_S", cli_input_parse_thread_proc, NULL);
    if (ret != QOSA_OK)
	{
		LOG_E ("serial_input_pars thread could not start!");
		return -1;
	}
}

// 销毁日志模块
void destroy_logger(void)
{
    if (logger.log_file)
    {
        fclose(logger.log_file);
        logger.log_file = NULL;
    }
    DeleteCriticalSection(&logger.lock);
}

// 设置日志等级
void set_log_level(LogLevel level)
{
    EnterCriticalSection(&logger.lock);
    logger.level = level;
    LeaveCriticalSection(&logger.lock);
}

// 设置日志等级
int get_log_level(void)
{
    return logger.level;
}

// 启用或禁用文件日志
void enable_log_to_file(int enable)
{
    EnterCriticalSection(&logger.lock);
    logger.log_to_file = enable;
    LeaveCriticalSection(&logger.lock);
}

// 打印日志 (延迟格式化)
void log_message_impl(const int level, const char *msg, const char *prefix, const char *suffix, const char *file, const char *func, const int line, const char *fmt,...)
{
    if (level < logger.level)
    {
        return;  // 日志等级低于当前设置，直接返回
    }

    EnterCriticalSection(&logger.lock);

    memset(g_printf_buf, 0, sizeof(g_printf_buf));
    memset(time_str, 0, sizeof(time_str));
    get_current_time(time_str, sizeof(time_str));

    // 格式化日志消息
    va_list args;
    va_start(args, fmt);
    vsnprintf(g_printf_buf, sizeof(g_printf_buf), fmt, args);
    va_end(args);

    // 输出到控制台
	printf("%s[%s][%-5s][%24s][%32s():%04d] %s%s", prefix, time_str, msg, file, func, line, g_printf_buf, suffix);

    // 输出到文件
    if (logger.log_file)
    {
        fprintf(logger.log_file, "[%s][%-5s][%24s][%32s():%04d] %s\n", time_str, msg, file, func, line, g_printf_buf);
        fflush(logger.log_file);
    }

    LeaveCriticalSection(&logger.lock);
}

