#include "QuectelConfig.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "hal_uart.h"
#include "debug_service.h"
#include "broadcast_service.h"
#include "ringbuffer.h"
#include "sd_fatfs.h"
#include "at.h"
#include "qosa_log.h"
#include "qosa_time.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#elif __linux__
#else
#include "ff.h"
#endif

#include "ql_ftp.h"
#include "ql_http.h"
#include "ql_net.h"
#include "ql_socket.h"
#include "cli_test_main.h"


#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_PRINT__
LogLevel g_debug_level = LOG_DEBUG;
int32_t g_debug_mode = 0; //0:debug, 1:release
int32_t g_save_debug_flag = 0;
#endif

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_SAVE__
#define LOG_FILE        "0:quectel.log"
#define BACKUP_DIR      "0:/backup"
static osa_task_t g_debug_service_thread_id = NULL;
osa_sem_t g_debug_service_sem_id = NULL;
ringbuffer_t g_log_rb = {.buffer = NULL};
static uint8_t *g_log_rb_buf = QOSA_NULL;
#endif

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_SHELL__
static osa_sem_t g_cli_input_sem_id = NULL;
static osa_task_t g_serial_input_parse_thread_id = NULL;

Cli_Menu_t* g_cli_fun_array = NULL;
int32_t   g_cli_fun_array_size = 0;


static void debug_service_cmd_parse(char *cmd, s32_t *pargc, char *argv[])
{
	int32_t argc, s32Ch, s32StrMrk;
	char *pcCmdStr;

	if(cmd == NULL)
	{
		LOG_E("cmd is null!");
		return;
	}

	pcCmdStr = cmd;
	/* skipping heading white spaces */
	while(s32Ch = *(unsigned char *)pcCmdStr, isspace(s32Ch))
	{
		++pcCmdStr;
	}
	argc = 0;

	while(s32Ch != '\0')
	{
		if(++argc >= CMD_ARGC_NUM)
		{
			LOG_W("cmd argc too many");
			--argc;
			break;
		}
		/* the leading token (cmd name) not supporting " or ' sign */
		if(argc && (s32Ch == '"' || s32Ch == '\''))
		{
			s32StrMrk = s32Ch;
			++pcCmdStr;
			*argv++ = pcCmdStr;
			while(s32Ch = *(unsigned char *)pcCmdStr, s32Ch && s32Ch != s32StrMrk)
			{
				++pcCmdStr;
			}

			if(s32Ch != '\0')
			{
				*pcCmdStr++ = '\0';
			}
			else
			{
				LOG_W("tailing >%c< expected", s32StrMrk);
				break;
			}
		}
		else
		{
			*argv++ = pcCmdStr;
			while(s32Ch && !isspace(s32Ch))
			{
				s32Ch = *(unsigned char *)++pcCmdStr;
			}

			if(s32Ch != '\0')
			{
				*pcCmdStr++ = '\0';
			}
		}
		while(s32Ch = *(unsigned char *)pcCmdStr, isspace(s32Ch))
		{
			++pcCmdStr;
		}
	}
	*argv = NULL;
	*pargc = argc;
}

int debug_service_cmd_exec(const char *cmd)
{
	char *argv[CMD_ARGC_NUM];
	s32_t argc = 0;
	s32_t i = 0;

	if(cmd == NULL)
	{
		LOG_E("Input is empty, please enter valid data");
		return -1;
	}
    /* Analyze and separate each parameter */
	debug_service_cmd_parse((char*)cmd, &argc, argv);
    /* Input none or "help" */
    if ((argv[0] == NULL) || (strcmp(argv[0], "help") == 0))
    {
        cli_test_table(argc, argv);
        return 0;
    }

    /* Search funtion base on NAME */
	for(i = 0; i < g_cli_fun_array_size; i++)
	{
		if(strcmp(argv[0], g_cli_fun_array[i].name) == 0)
		{
            if(strcmp(argv[1], "help") == 0)
            {
                if(g_cli_fun_array[i].help)
                    g_cli_fun_array[i].help();
            }
            else
            {
                if(g_cli_fun_array[i].func)
                    g_cli_fun_array[i].func(argc, argv);
            }
			break;
		}
	}
    /* Parameters match none */
	if (i == g_cli_fun_array_size)
	{
        LOG_E("Invalid parameter:%s", argv[0]);
		cli_test_table(0, NULL);
	}

	return 0;
}


void debug_uart_input_notify(void)
{
	if (g_cli_input_sem_id)
    {
		qosa_sem_release(g_cli_input_sem_id);
    }
}

static void shell_input_parse_thread_proc(void* pThreadParam)
{
	int32_t ret;
	const uint8_t *pData = NULL;
	uint16_t Size;

	while (1)
	{
		ret = qosa_sem_wait(g_cli_input_sem_id, QOSA_WAIT_FOREVER);
		if (ret != QOSA_OK)
		{
			LOG_E("qosa_sem_wait msg failed!");
			return;
		}

		qosa_uart_get_debug_input(&pData, &Size);
		// taskENTER_CRITICAL(); // Disable interrupts
        // LOG_V("\r\n");
        printf("\r\n");     // FIX: Input content will disappear when not using "LOG_VERBOSE" mode.
		LOG_V("get = %s", pData);
		// taskEXIT_CRITICAL(); // Enable interrupts
		debug_service_cmd_exec((const char*)pData);
		// LOG_H("# ");
		// fflush(stdout);
	}
	LOG_V("%s over",__FUNCTION__);
	qosa_task_exit();
}


int debug_cli_service_create(void)
{
	qosa_sem_create(&g_cli_input_sem_id, 0);
    if (g_cli_input_sem_id == NULL)
    {
        LOG_E("g_cli_input_sem_id semaphore create failed!");
		return -1;
    }

    /* Externed task stack, Jerry.Chen, 2025-06-16 */
    qosa_task_create(&g_serial_input_parse_thread_id, 1024 * 16, QOSA_PRIORITY_NORMAL, "Debug_S", shell_input_parse_thread_proc, NULL);
	if (NULL == g_serial_input_parse_thread_id)
	{
		LOG_E("thread_id thread could not start!");
		return -1;
	}
	LOG_I("%s over(%x)",__FUNCTION__, g_serial_input_parse_thread_id);

	return 0;
}

int debug_cli_service_destroy(void)
{
	int ret;

	//1. Destroy debug service
	if (NULL != g_serial_input_parse_thread_id)
    {
        ret = qosa_task_delete(g_serial_input_parse_thread_id);
        if (0 != ret)
        {
            LOG_E("Delete g_serial_input_parse_thread_id thread failed! %d", ret);
            return -1;
        }
    }

    //2. Delete msg/sem queue
	if (NULL != g_cli_input_sem_id)
    {
		ret = qosa_sem_delete(g_cli_input_sem_id);
        if (QOSA_OK != ret)
        {
            LOG_E("Delete g_cli_input_sem_id msg failed! %d", ret);
            return -1;
        }
    }
	return QOSA_OK;
}


#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_SAVE__

static int debug_save_service_destroy(void)
{
	if (g_log_rb_buf != NULL)
		free(g_log_rb_buf);
	g_log_rb_buf = NULL;

	//3. Delete msg/sem queue
	if (NULL != g_debug_service_sem_id)
    {
        qosa_sem_delete(g_debug_service_sem_id);
		g_debug_service_sem_id=NULL;
    }

	return 0;
}

static void debug_service_thread_proc(void* pThreadParam)
{
	int32_t ret,size;
	FRESULT f_res;
	FIL SDFile;
	FILINFO fileInfo;
	unsigned int fnum;
	unsigned char LOG_BUFFER[DBG_BUFF_LEN]= {0};

	// Check if the backup directory exists. If it doesn't exist, create it.
	f_res = f_stat(BACKUP_DIR, &fileInfo);
	if (f_res == FR_NO_FILE)
	{
		f_res = f_mkdir(BACKUP_DIR);  // Create directory
		if (f_res != FR_OK)
		{
			LOG_E("Failed to create backup dir: %d", f_res);
		}
	} else if (f_res != FR_OK)
	{
		LOG_E("f_stat error for backup dir: %d", f_res);
	}

	// 1. Check if the file already exists
	f_res = f_stat(LOG_FILE, &fileInfo);
	if (f_res == FR_OK)
	{
		// File exists. Generate timestamp backup file name.
		char backupFile[64];
		time_t now = time(NULL);
		struct tm *tm_now = localtime(&now);

		// Format the timestamp (eg: quectel_20230704_153025.log)
		snprintf(backupFile, sizeof(backupFile),
				"%s/quectel_%04d%02d%02d_%02d%02d%02d.log",
				BACKUP_DIR,  // Can be replaced by "", and placed directly in the root directory
				tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
				tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

		// 2. Rename the original file (move it to the backup)
		f_res = f_rename(LOG_FILE, backupFile);
		if (f_res != FR_OK)
		{
        	LOG_E("Failed to backup log file: %d", f_res);
		}
		else
		{
			LOG_I("backup log file: %s", backupFile);
		}
	}

	//2. init fs
	f_res = f_open(&SDFile, "0:quectel.log", FA_CREATE_ALWAYS | FA_WRITE);
	if(f_res != FR_OK)
	{
		LOG_E("open quectel.log error : %d", f_res);
        goto __exit;
	}
	else
	{
		LOG_I("open quectel.log success");
	}
	g_save_debug_flag = 1;
	//2. write to sd card
	while(g_save_debug_flag)
	{
		ret = qosa_sem_wait(g_debug_service_sem_id, portMAX_DELAY);
		if (ret != QOSA_OK)
		{
			LOG_E("qosa_sem_wait msg failed!");
			f_close(&SDFile);
            goto __exit;
		}

		while (g_save_debug_flag && ringbuffer_length(&g_log_rb))
		{
			memset(LOG_BUFFER, 0, DBG_BUFF_LEN);
			ringbuffer_get(&g_log_rb, LOG_BUFFER, sizeof(size));
			memcpy(&size, LOG_BUFFER, sizeof(size));
			if (size > DBG_BUFF_LEN)
			{
				LOG_I("size too big:data size = %d, buff size = %d", size, DBG_BUFF_LEN);
			}
			while (size > 0)
			{
				int need = MIN(DBG_BUFF_LEN, size);
				ringbuffer_get(&g_log_rb, LOG_BUFFER, need);
				//printf("A:%s", LOG_BUFFER);
				f_res = f_write(&SDFile, LOG_BUFFER, need, &fnum);
				if(f_res != FR_OK)
				{
					LOG_E("write file error : %d", f_res);
					if (FR_INVALID_OBJECT == f_res)
					{
						f_close(&SDFile);
						goto __exit;
					}
				}
				size -= need;
			}
		}
		f_sync(&SDFile);
	}

	//4. close file
	f_close(&SDFile);

__exit:
	LOG_I("%s over",__FUNCTION__);
	g_save_debug_flag = 0;
	debug_save_service_destroy();
	qosa_task_exit();
}

static int debug_save_service_create(int rb_size)
{
	//1. Init ringbuffer
	g_log_rb_buf = malloc(rb_size);
	if (g_log_rb_buf == NULL )
	{
		LOG_E("ring buffer malloc error : %d, %d", rb_size, qosa_task_get_free_heap_size());
		return -1;
	}
	ringbuffer_init(&g_log_rb, g_log_rb_buf, rb_size);

	//2. Init sem
	qosa_sem_create(&g_debug_service_sem_id, 0);
    if (g_debug_service_sem_id == NULL)
    {
        LOG_E("AT client initialize failed! g_debug_service_sem_id semaphore create failed!");
		return -1;
    }

	//3. Create thread
    qosa_task_create(&g_debug_service_thread_id, 1024 * 4, QOSA_PRIORITY_ABOVE_NORMAL, "Log_Save", debug_service_thread_proc, NULL);
	if (NULL == g_debug_service_thread_id)
	{
		LOG_E("g_debug_service_thread_id thread could not start!");
		return -1;
	}
	LOG_I("%s over(%x)",__FUNCTION__, g_debug_service_thread_id);
	return 0;
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_SAVE__ */


int cli_test_table(int argc, char *argv[])
{
	int i = 0;
    for (i = 0; i < argc; i++)
        LOG_V("%d = %s", i, argv[i]);

    if ((argc == 0) || (strcmp((const char *)argv[0], "help")==0))
	{
        LOG_I("--------------------------------------------");
        LOG_I("| CLI Test Table:                          |");
        LOG_I("|------------------------------------------|");
		for(i = 0; i < g_cli_fun_array_size; i++)
		{
			LOG_I("| %-40s |", g_cli_fun_array[i].name);
		}
		LOG_I("--------------------------------------------");
	}

	return -1;
}

static void cli_at_get_help(void)
{
    LOG_I("--------------------------------------------");
    LOG_I("| Example: at ati                          |");
    LOG_I("|          at at+cpin?                     |");
    LOG_I("|          at at+cfun=1                    |");
    LOG_I("|------------------------------------------|");
}

static void cli_debug_get_help(void)
{
    LOG_I("--------------------------------------------");
    LOG_I("| debug mode <val>                         |");
    LOG_I("|            <val>  0: debug               |");
    LOG_I("|                   1: release             |");
    LOG_I("| debug save <val>                         |");
    LOG_I("|            <val>  0: off                 |");
    LOG_I("|                   1: on                  |");
    LOG_I("|                   2: status              |");
    LOG_I("| debug level <val>                        |");
    LOG_I("|             <val> 0: Verbose             |");
    LOG_I("|                   1: Debug               |");
    LOG_I("|                   2: Info                |");
    LOG_I("|                   3: Warn                |");
    LOG_I("|                   4: Error               |");
    LOG_I("| debug test                               |");
    LOG_I("--------------------------------------------");
}

static int cli_debug_test(s32_t argc, char *argv[])
{
	uint64_t total_bytes;
	uint32_t total_gb;
	uint32_t decimal_part;
	if (argc < 2)
	{
		LOG_E("Invalid parameter");
		cli_debug_get_help();
		return -1;
	}
    if (strcmp((const char *)argv[1], "mode") == 0)
	{
		g_debug_mode = atoi(argv[2]);
	}
    else if (strcmp((const char *)argv[1], "level") == 0)
	{
		g_debug_level = atoi(argv[2]);
		LOG_I("debug level set to %d", g_debug_level);
	}
    else if (strcmp((const char *)argv[1], "test") == 0)
	{
		LOG_V("[V]: Welcome to use quectel module");
		LOG_D("[D]: Welcome to use quectel module");
		LOG_I("[I]: Welcome to use quectel module");
		LOG_W("[W]: Welcome to use quectel module");
		LOG_E("[E]: Welcome to use quectel module");
	}
	else if (strcmp((const char *)argv[1], "save") == 0)
	{
		#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_SAVE__
		if ((argc > 2))
		{
			if ( atoi(argv[2]) == 2 || atoi(argv[2]) == g_save_debug_flag )  //status
			{
				LOG_I("Current log saving status is %s, free heap size is %d", (g_save_debug_flag == 1 ? "on" : "off"), qosa_task_get_free_heap_size());
			}
			else if (atoi(argv[2]) == 1)  //on
			{
				LOG_I("Current free heap space : %d kb", qosa_task_get_free_heap_size() / 1024);
				total_bytes	= get_sdcard_free_space();
				if (0 == total_bytes)
				{
					LOG_I("sd card free space      : NA");
				}
				else
				{
					total_gb = total_bytes / 1024;
					decimal_part = total_bytes % 1024 * 100 / 1024;
					LOG_I("sd card free space      : %lu.%02uGB", total_gb, decimal_part);
				}
				LOG_I("use 2048 byte for input buffer size");
				if (argc <= 3)
				{
					debug_save_service_create(2048);

				}
				else
				{
					debug_save_service_create(atoi(argv[3]));
				}
			}
			else  //off
			{
				LOG_I("Now stop saving the log");
				g_save_debug_flag = 0;
				if (g_debug_service_sem_id)
					qosa_sem_release(g_debug_service_sem_id);
				qosa_task_sleep_ms(1000);
			}
		}
		#else
        LOG_W("This function is not supported");
		#endif
	}

    return 0;
}

static int cli_at_test(s32_t argc, char *argv[])
{
	if (argc < 2)
	{
		LOG_E("Invalid parameter");
		cli_at_get_help();
	}
    if(argv[1] != NULL)
    {
        at_response_t resp;
		resp = at_create_resp(1024, 0, 3000);
		at_exec_cmd(resp, argv[1]);
    	at_delete_resp(resp);
    }
    return 0;
}

int debug_cli_func_reg(int32_t cnt, Cli_Menu_t cli_menu[])
{
    int32_t i;

    g_cli_fun_array = cli_menu;
    g_cli_fun_array_size = cnt;

    for(i = 0; i < g_cli_fun_array_size; i++)
    {
        if(strcmp("debug", g_cli_fun_array[i].name) == 0)
        {
            g_cli_fun_array[i].func = cli_debug_test;
            g_cli_fun_array[i].help = cli_debug_get_help;
        }
        if(strcmp("at", g_cli_fun_array[i].name) == 0)
        {
            g_cli_fun_array[i].func = cli_at_test;
            g_cli_fun_array[i].help = cli_at_get_help;
        }
    }

    return 0;
}

#else
void debug_uart_input_notify(void)
{
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_SHELL__ */
