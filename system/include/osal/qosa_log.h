/*****************************************************************/ /**
* @file qosa_log.h
* @brief
* @author larson.li@quectel.com
* @date 2024-12-23
*
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description"
* <tr><td>2024-12-23 <td>1.0 <td>Larson.Li <td> Init
* </table>
**********************************************************************/
#ifndef __QOSA_LOG_H__
#define __QOSA_LOG_H__

typedef enum
{
    LOG_VERBOSE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

void debug_print(const int level, const char *msg, const char *prefix, const char *suffix, const char *file, const char *func, const int line, const char *fmt,...);

// void log_message_impl(const int level, const char *msg, const char *prefix, const char *suffix, const char *file, const char *func, const int line, const char *fmt,...);
// // 实际日志打印实现
#ifndef __FILE_NAME__
    #define __FILE_NAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#endif
#define LOG_V( ...) debug_print(LOG_VERBOSE, "VER",   "\033[0;37m", "\033[0m\r\n", __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_D( ...) debug_print(LOG_DEBUG,   "DEBUG", "\033[0;37m", "\033[0m\r\n", __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_I( ...) debug_print(LOG_INFO,    "INFO",  "\033[0;34m", "\033[0m\r\n", __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_H( ...) debug_print(LOG_INFO,    "INPUT", "\033[0;92m", "\033[0m",     __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_W( ...) debug_print(LOG_WARN,    "WARN",  "\033[0;33m", "\033[0m\r\n", __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_E( ...) debug_print(LOG_ERROR,   "ERR",   "\033[0;31m", "\033[0m\r\n", __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)


/**
 * @brief 初始化日志
 *
 * @param[in] LogLevel level
 *          - 日志打印等级
 * @param[in] const char *file_path
 *          - 日志打印文件
 * @note 初始化日志打印，file_path 可以为NULL，为NULL时不再打印到文件
 */
void init_logger(LogLevel level, const char *file_path);

/**
 * @brief 关闭日志
 *
 * @note 结束后会关闭日志文件
 */
void destroy_logger(void);

/**
 * @brief 调整日志打印等级
 */
void set_log_level(LogLevel level);

/**
 * @param[in] int enable
 *          -使能日志写入文件
 *
 * @brief 0:不在写入 1:写入
 */
void enable_log_to_file(int enable);


#endif /* __QOSA_LOG_H__ */
