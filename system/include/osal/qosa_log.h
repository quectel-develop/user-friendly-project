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


#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
void log_message_impl(const int level, const char *msg, const char *prefix, const char *suffix, const char *file, const char *func, const int line, const char *fmt,...);
#define log_message     log_message_impl
#elif __linux__
#else
void debug_print(const int level, const char *msg, const char *prefix, const char *suffix, const char *file, const char *func, const int line, const char *fmt,...);
#define log_message     debug_print
#endif

#ifndef __FILE_NAME__
#define __FILE_NAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#endif
#define LOG_V( ...) log_message(LOG_VERBOSE, "VER",   "\033[0;37m", "\033[0m\r\n", __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_D( ...) log_message(LOG_DEBUG,   "DEBUG", "\033[0;37m", "\033[0m\r\n", __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_I( ...) log_message(LOG_INFO,    "INFO",  "\033[0;34m", "\033[0m\r\n", __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_H( ...) log_message(LOG_INFO,    "INPUT", "\033[0;92m", "\033[0m",     __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_W( ...) log_message(LOG_WARN,    "WARN",  "\033[0;33m", "\033[0m\r\n", __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_E( ...) log_message(LOG_ERROR,   "ERR",   "\033[0;31m", "\033[0m\r\n", __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)



/**
 * @brief Initializes the logger
 *
 * @param[in] LogLevel level
 *          - Log printing level
 * @param[in] const char *file_path
 *          - Log output file path
 * @note Initializes log printing. If file_path is NULL, logs will not be written to a file
 */
void init_logger(LogLevel level, const char *file_path);

/**
 * @brief Closes the logger
 *
 * @note Closes the log file after completion
 */
void destroy_logger(void);

/**
 * @brief Adjusts the log printing level
 */
void set_log_level(LogLevel level);

/**
 * @param[in] int enable
 *          - Enables writing logs to a file
 *
 * @brief 0: Disabled 1: Enabled
 */
void enable_log_to_file(int enable);


#endif /* __QOSA_LOG_H__ */
