/*****************************************************************/ /**
* @file qosa_log.c
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
#include "stdarg.h"
#include "qosa_def.h"
#include "qosa_system.h"
#include "qosa_time.h"
#include "qosa_log.h"
#include "debug_service.h"

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_PRINT__
extern LogLevel g_debug_level;
extern int32_t g_debug_mode;    // 0:debug, 1:release
extern int32_t g_save_debug_flag;

#include "ringbuffer.h"
extern osa_sem_t g_debug_service_sem_id;
extern ringbuffer_t g_log_rb;

void debug_print(const int level, const char *msg, const char *prefix, const char *suffix, const char *file, const char *func, const int line, const char *fmt,...)
{
    va_list arglst;
    static osa_mutex_t debug_slock = NULL;
	int total_size=0, size1=0, size2=0, size3=0;
	osa_task_t thread_id = qosa_task_get_task_id();
	char DBG_BUFFER[DBG_BUFF_LEN]= {0};
    int32_t status = QOSA_OK;
    // const char *tab_suffix = (strcmp(msg, "INPUT") != 0) ? "\t" : "";
    time_t now;
    char timestamp[32];
    struct tm *tm_info = NULL;
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
        time(&now);
        tm_info = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
		va_start(arglst,fmt);
		memset(DBG_BUFFER, 0, sizeof(DBG_BUFFER));
		if (g_debug_mode == 1)
			size1 = snprintf(DBG_BUFFER+sizeof(total_size), sizeof(DBG_BUFFER), "%s", prefix);
		else
        {
            if(strcmp(msg, "INPUT") == 0)
                size1 = snprintf(DBG_BUFFER+sizeof(total_size), sizeof(DBG_BUFFER), "%s%s [%-5s]  [%-10s]\t[%s]", prefix, timestamp, msg, file, func);
            else
                size1 = snprintf(DBG_BUFFER+sizeof(total_size), sizeof(DBG_BUFFER), "%s%s [%-5s]  [%-10s]\t[%s():%d][%lu]\t", prefix, timestamp, msg, file, func, line, qosa_task_get_stack_space(thread_id));
        }
		size2 = vsnprintf(DBG_BUFFER+size1+sizeof(total_size), sizeof(DBG_BUFFER)-size1, fmt, arglst);
		size3 = snprintf(DBG_BUFFER+size1+size2+sizeof(total_size), sizeof(DBG_BUFFER)-size1-size2, "%s", suffix);
		total_size = size1+size2+size3;
		memcpy(DBG_BUFFER, &total_size, sizeof(total_size));
		printf("%s", DBG_BUFFER+sizeof(total_size));
		#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_SAVE__
		if (g_save_debug_flag && g_log_rb.buffer)
		{
			ringbuffer_put(&g_log_rb, (const uint8_t*)DBG_BUFFER, total_size + sizeof(total_size));
			if (g_debug_service_sem_id)
				qosa_sem_release(g_debug_service_sem_id);
		}
		#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SAVE__ */
		va_end(arglst);
        qosa_mutex_unlock(debug_slock);
	}
}
#endif  /* __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_PRINT__ */

