#ifndef __DEBUG_SERVICE_H__
#define __DEBUG_SERVICE_H__
#include <ctype.h>
#include "QuectelConfig.h"
#include "qosa_system.h"

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
	char name[NAME_MAX_LEN];
	int (*func)(s32_t argc, char *argv[]);
    void (*help)(void);
}Cli_Menu_t;

int debug_service_cmd_exec(const char *cmd);

int debug_cli_func_reg(int32_t cnt, Cli_Menu_t cli_menu[]);
int debug_cli_service_create(void);
int debug_cli_service_destroy(void);

void debug_uart_input_notify(void);

int cli_test_table(int argc, char *argv[]);
#endif /* __DEBUG_SERVICE_H__ */

