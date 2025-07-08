#ifndef __DEBUG_SERVICE_H__
#define __DEBUG_SERVICE_H__
#include "QuectelConfig.h"

#define DBG_BUFF_LEN		    (1024)
#define CMD_ARGC_NUM	     	(25)
#define NAME_MAX_LEN		    (16)

#define USE_DEBUG_ASSERT

#ifdef  USE_DEBUG_ASSERT
#define dbg_assert_param(expr) ((expr) ? (void)0U : dbg_assert_failed((uint8_t *)__FILE__, __LINE__))
#else
#define assert_param(expr) ((void)0U)
#endif /* USE_DEBUG_ASSERT */



typedef struct 
{
	char module_name[NAME_MAX_LEN];
	int (*fp)(int argc, char *argv[]);
}dbg_module, *dbg_module_t;

int debug_service_cmd_proc(const char *cmd);

int debug_service_create(void);
int debug_service_destroy(void);
void serial_input_parse_thread_wake_up();

#endif /* __DEBUG_SERVICE_H__ */

