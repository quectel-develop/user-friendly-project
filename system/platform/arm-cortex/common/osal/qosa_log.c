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
#include "stdarg.h"
// 日志模块
typedef struct
{
    LogLevel         level;        // 当前日志等级
    FILE            *log_file;     // 日志文件
    int              log_to_file;  // 是否写入文件 (0: 否, 1: 是)
    osa_mutex_t      lock;         // 锁
} Logger;

// 全局日志实例
static Logger logger = {LOG_INFO, NULL};
static char   g_printf_buf[1024] = {0};
static char   time_str[20] = {0};
static osa_task_t g_serial_input_thread_id =NULL;

// 获取当前时间字符串
static void get_current_time(char *buffer, size_t size)
{
    // time_t    now = time(NULL);
    // struct tm tm_info;
    // localtime_s(&tm_info, &now);
    // strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info);
}

#define MAX_LENGTH 256
static uint8_t g_uart_input_buf[MAX_LENGTH];
static uint32_t g_uart_input_buf_get_len = 0;

static void* serial_input_parse_thread_proc(void* pThreadParam)
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
            debug_service_cmd_proc(g_uart_input_buf);
        }
	}
	LOG_V("%s over",__FUNCTION__);
}

// 初始化日志模块
void init_logger(LogLevel level, const char *file_path)
{
    int ret = QOSA_OK;
    logger.level = level;
    qosa_mutex_create(&logger.lock);

    if (file_path != NULL)
    {
        // logger.log_file = fopen(file_path, "a");
        // if (!logger.log_file)
        // {
        //     fprintf(stderr, "Failed to open log file: %s\n", file_path);
        //     exit(EXIT_FAILURE);
        // }
    }

	ret = qosa_task_create(&g_serial_input_thread_id, 512*6, QOSA_PRIORITY_NORMAL, "Debug_S", serial_input_parse_thread_proc, NULL);
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
        // fclose(logger.log_file);
        logger.log_file = NULL;
    }
    qosa_mutex_delete(logger.lock);
}

// 设置日志等级
void set_log_level(LogLevel level)
{
    qosa_mutex_lock(logger.lock, QOSA_WAIT_FOREVER);
    logger.level = level;
    qosa_mutex_unlock(logger.lock);
}

// 设置日志等级
int get_log_level(void)
{
    return logger.level;
}

// 启用或禁用文件日志
void enable_log_to_file(int enable)
{
    qosa_mutex_lock(logger.lock, QOSA_WAIT_FOREVER);
    logger.log_to_file = enable;
    qosa_mutex_unlock(logger.lock);
}

// 打印日志 (延迟格式化)
void log_message_impl(const int level, const char *msg, const char *prefix, const char *suffix, const char *file, const char *func, const int line, const char *fmt,...)
{
    if (level < logger.level)
    {
        return;  // 日志等级低于当前设置，直接返回
    }

    qosa_mutex_lock(logger.lock, QOSA_WAIT_FOREVER);

    memset(g_printf_buf, 0, sizeof(g_printf_buf));
    memset(time_str, 0, sizeof(time_str));
    //get_current_time(time_str, sizeof(time_str));

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
        // fprintf(logger.log_file, "[%s][%-5s][%24s][%32s():%04d] %s\n", time_str, msg, file, func, line, g_printf_buf);
        // fflush(logger.log_file);
    }

    qosa_mutex_unlock(logger.lock);
}

#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_PRINT__
extern LogLevel g_debug_level;
extern int32_t g_debug_mode; //0:debug, 1:release
extern int32_t g_save_debug_flag;

#include "ringbuffer.h"
extern osa_sem_t g_debug_service_sem_id;
extern struct ringbuffer g_log_rb;

void debug_print(const int level, const char *msg, const char *prefix, const char *suffix, const char *file, const char *func, const int line, const char *fmt,...)
{
    va_list arglst;
    static osa_mutex_t debug_slock = NULL;
	int ret = 0, total_size=0, size1=0, size2=0, size3=0;
	osa_task_t thread_id = qosa_task_get_task_id();
	unsigned char DBG_BUFFER[DBG_BUFF_LEN]= {0};
    int32_t status = QOSA_OK;

	if (debug_slock == NULL)
	{
        status = qosa_mutex_create(&debug_slock);
        if (status != QOSA_OK)
        {
            LOG_E("No memory for debug allocation lock!");
            return;
        }
	}
	if (level >= g_debug_level)
	{
        qosa_mutex_lock(debug_slock, QOSA_WAIT_FOREVER);//FreeRtos fifo is not supported
		va_start(arglst,fmt);
		// vsnprintf(DBG_BUFFER,sizeof(DBG_BUFFER),fmt,arglst);
		// printf("%s[%-5s][%24s][%32s():%04d][%4d] %s%s", prefix, msg, file, func, line, qosa_task_get_stack_space(thread_id), DBG_BUFFER, suffix);
		memset(DBG_BUFFER, 0, sizeof(DBG_BUFFER));
		if (g_debug_mode == 1)
			size1 = snprintf(DBG_BUFFER+sizeof(total_size), sizeof(DBG_BUFFER), "%s", prefix);
		else
			size1 = snprintf(DBG_BUFFER+sizeof(total_size), sizeof(DBG_BUFFER), "%s[%-5s][%24s][%32s():%04d][%4d][%x] ", prefix, msg, file, func, line, qosa_task_get_stack_space(thread_id), thread_id);
		//size1 = snprintf(DBG_BUFFER+sizeof(total_size), sizeof(DBG_BUFFER), "%s[%-5s][%24s][%32s():%04d][%4d/%4d][%x] ", prefix, msg, file, func, line, qosa_task_get_free_heap_size(thread_id), os_thread_get_free_heap_size(), thread_id);
		//size1 = snprintf(DBG_BUFFER+sizeof(total_size), sizeof(DBG_BUFFER), "%s[%-5s][%24s][%32s():%04d][%4d][%4d/%4d][%x] ", prefix, msg, file, func, line, qosa_task_get_free_heap_size(thread_id), os_thread_get_min_free_heap_size(), os_thread_get_free_heap_size(), thread_id);
		size2 = vsnprintf(DBG_BUFFER+size1+sizeof(total_size), sizeof(DBG_BUFFER)-size1, fmt, arglst);
		size3 = snprintf(DBG_BUFFER+size1+size2+sizeof(total_size), sizeof(DBG_BUFFER)-size1-size2, "%s", suffix);
		total_size = size1+size2+size3;
		memcpy(DBG_BUFFER, &total_size, sizeof(total_size));
		printf("%s", DBG_BUFFER+sizeof(total_size));
		#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SAVE__
		if (g_save_debug_flag && g_log_rb.buffer)
		{
			ringbuffer_putstr(&g_log_rb, DBG_BUFFER, total_size + sizeof(total_size));
			if (g_debug_service_sem_id)
				qosa_sem_release(g_debug_service_sem_id);
		}
		#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SAVE__ */
		va_end(arglst);
        qosa_mutex_unlock(debug_slock);

	}
}
#endif  /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_PRINT__ */
