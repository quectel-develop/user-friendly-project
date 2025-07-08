/****************************************************************************
*
* Copy right: 2020-, Copyrigths of Quectel Ltd.
****************************************************************************
* File name: dbg_log.c
* History: Rev1.0 2023-08-19
****************************************************************************/
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "debug_service.h"
#include "broadcast_service.h"
#include "ringbuffer.h"
#include "at.h"
#include "qosa_log.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#elif __linux__
#else
#include "ff.h"
#endif

#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_FTP_S__
#include "bg95_ftp.h"
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_FTP_S__ */
#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_EXAMPLE_MAIN__
#include "user_main.h"
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_EXAMPLE_MAIN__ */
#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_HTTP_S__
#include "bg95_http.h"
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_HTTP_S__ */
#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_NETWORK__
#include "bg95_net.h"
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_NETWORK__ */
#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_SOCKET__
#include "bg95_socket.h"
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_SOCKET__ */



#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SHELL__
static osa_sem_t g_debug_input_sem_id = NULL;
static osa_task_t g_serial_input_parse_thread_id = NULL;
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SHELL__*/

#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SAVE__
static osa_task_t g_debug_service_thread_id = NULL;
osa_sem_t g_debug_service_sem_id = NULL;
struct ringbuffer g_log_rb = {.buffer = NULL};
static uint8_t *g_log_rb_buf = QOSA_NULL;
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SAVE__ */

#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_PRINT__
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_PRINT__ */

LogLevel g_debug_level = LOG_DEBUG;

int32_t g_debug_mode = 0; //0:debug, 1:release
int32_t g_save_debug_flag = 0;
static int32_t debug_service_test(s32_t argc, char *argv[]);
static int32_t usage_help(s32_t argc, char *argv[]);

dbg_module g_debug_fun_array[] =
{
#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_NETWORK__
	{"bg95_net",  	bg95_net_service_test},
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_NETWORK__ */
#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_SOCKET__
	{"bg95_socket", bg95_socket_service_test},
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_SOCKET__ */
#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_HTTP_S__
	{"bg95_http", 	bg95_http_service_test},
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_HTTP_S__ */
#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_FTP_S__
	{"bg95_ftp",  	bg95_ftp_service_test},
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_FTP_S__ */
	{"bcast",     	bcast_service_test},
	{"debug",     	debug_service_test},
#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_EXAMPLE_MAIN__
	{"main",      	user_main_test},
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_EXAMPLE_MAIN__ */
	{"help",      	usage_help},
};


static void debug_service_cmd_parse(char *cmd, s32_t *pargc, char *argv[])
{
	int32_t argc;
	int32_t s32StrMrk;
	char *pcCmdStr;
	int32_t s32Ch;

	LOG_V("%s",__FUNCTION__);

	if(cmd == NULL)
	{
		LOG_E("Cmd is null!");
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

int debug_service_cmd_proc(const char *cmd)
{
	char *argv[CMD_ARGC_NUM];
	int32_t argc, ret = -1;;
	static int8_t enter_fun_index = -1;
	int32_t i = 0, debug_fun_array_size = sizeof(g_debug_fun_array)/sizeof(g_debug_fun_array[0]);

	LOG_V("%s, %s",__FUNCTION__, cmd);

	if(cmd == NULL)
	{
		LOG_E ("Input is empty, please enter valid data");
		return -1;
	}

	debug_service_cmd_parse(cmd,&argc,argv);
	if (enter_fun_index != -1)
	{
		ret = g_debug_fun_array[enter_fun_index].fp(argc, argv);
		if (ret == -1)
		{
			enter_fun_index = -1;
			argc = 1;
			argv[0] = "help";
			usage_help(argc, argv);
		}
		return ;
	}
	for(i = 0; i < debug_fun_array_size; i++)
	{
		LOG_V("%s -> %s", argv[0], g_debug_fun_array[i].module_name);
		if (argv[0] == NULL)
			break;

		if(strcmp(argv[0], g_debug_fun_array[i].module_name) == 0)
		{
			enter_fun_index = i;
			argc = 1;
			argv[0] = "help";
			ret = g_debug_fun_array[i].fp(argc, argv);
			if (ret == -1)
				enter_fun_index = -1;
			break;
		}
	}
	if ((i == debug_fun_array_size) || (argv[0] == NULL))
	{
		enter_fun_index = -1;
		argc = 1;
		argv[0] = "help";
		usage_help(argc, argv);
	}
	return 0;
}

#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SHELL__
void serial_input_parse_thread_wake_up()
{
	if (g_debug_input_sem_id)
		qosa_sem_release(g_debug_input_sem_id);

}
static void* serial_input_parse_thread_proc(void* pThreadParam)
{
	int32_t ret;
	uint8_t *pData = NULL;
	uint16_t Size;

	LOG_V("%s",__FUNCTION__);

	while (1)
	{
		ret = qosa_sem_wait(g_debug_input_sem_id, QOSA_WAIT_FOREVER);
		if (ret != QOSA_OK)
		{
			LOG_E("qosa_sem_wait msg failed!");
			return -1;
		}

		USER_GET_DEBUG_INPUT_DATA(&pData, &Size);
		pData[Size] = '\0';
		// taskENTER_CRITICAL(); // 禁用中断
        // LOG_V("\r\n");
        printf("\r\n");     // FIX: Input content will disappear when not using "LOG_VERBOSE" mode.
		LOG_V("get = %s", pData);
		//   taskEXIT_CRITICAL(); // 重新启用中断
		debug_service_cmd_proc(pData);
		LOG_H("# ");
		fflush(stdout);
	}
	LOG_V("%s over",__FUNCTION__);
	qosa_task_exit();
}

static int debug_input_service_create(void)
{
	int ret = QOSA_OK;

	LOG_V("%s",__FUNCTION__);

	ret = qosa_sem_create(&g_debug_input_sem_id, 0);
    if (g_debug_input_sem_id == NULL)
    {
        LOG_E("AT client initialize failed! g_debug_input_sem_id semaphore create failed!");
		return -1;
    }

    /* Externed task stack, Jerry.Chen, 2025-06-16 */
    ret = qosa_task_create(&g_serial_input_parse_thread_id, 1024 * 16, QOSA_PRIORITY_NORMAL, "Debug_S", serial_input_parse_thread_proc, NULL);
	if (NULL == g_serial_input_parse_thread_id)
	{
		LOG_E ("Broadcast g_serial_input_parse_thread_id could not start!");
		return -1;
	}
	LOG_I("%s over(%x)",__FUNCTION__, g_serial_input_parse_thread_id);

	return 0;
}
static int debug_input_service_destroy(void)
{
	int ret;

	//1. Destroy net service
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
	if (NULL != g_debug_input_sem_id)
    {
		ret = qosa_sem_delete(g_debug_input_sem_id);
        if (QOSA_OK != ret)
        {
            LOG_E("Delete g_debug_input_sem_id msg failed! %d", ret);
            return -1;
        }
    }
	return QOSA_OK;
}
#else
void serial_input_parse_thread_wake_up()
{

}
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SHELL__ */


#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SAVE__
static void* debug_service_thread_proc(void* pThreadParam)
{
	int32_t ret,size;
	FRESULT f_res;
	FIL SDFile;
	unsigned int fnum;
	unsigned char LOG_BUFFER[DBG_BUFF_LEN]= {0};

	// LOG_I("%s",__FUNCTION__);

	//2. init fs
	f_res = f_open(&SDFile, "0:quectel.log", FA_CREATE_ALWAYS | FA_WRITE);
	if(f_res != FR_OK)
	{
		LOG_E("open file error : %d", f_res);
        goto __exit;
	}
	else
	{
		LOG_I("open file success");
	}

	//3. write to sd card
	while(g_save_debug_flag)
	{
		ret = qosa_sem_wait(g_debug_service_sem_id, portMAX_DELAY);
		if (ret != QOSA_OK)
		{
			LOG_E("qosa_sem_wait msg failed!");
            goto __exit;
			return -1;
		}

		while (g_save_debug_flag&&ringbuffer_data_len(&g_log_rb))
		{
			memset(LOG_BUFFER, 0, DBG_BUFF_LEN);
			ringbuffer_getstr(&g_log_rb, LOG_BUFFER, sizeof(size));
			memcpy(&size, LOG_BUFFER, sizeof(size));
			if (size > DBG_BUFF_LEN)
			{
				LOG_E("size too big:data size = %d, buff size = %d", size, DBG_BUFF_LEN);
			}
			ringbuffer_getstr(&g_log_rb, LOG_BUFFER, size);
			//printf("A:%s", LOG_BUFFER);
			f_res = f_write(&SDFile, LOG_BUFFER, size, &fnum);
			if(f_res != FR_OK)
			{
				LOG_E("write file error : %d", f_res);
			}
		}
		f_sync(&SDFile);
	}

	//4. close file
	f_res = f_close(&SDFile);
	if(f_res != FR_OK)
	{
		LOG_E("close file error : %d", f_res);
        goto __exit;
		return;
	}

__exit:
	LOG_I("%s over",__FUNCTION__);
	qosa_task_exit();
}

static int debug_save_service_create(int rb_size)
{
    int ret = -1;

	//1. Init ringbuffer
	g_log_rb_buf = malloc(rb_size);
	if (g_log_rb_buf == NULL )
	{
		LOG_E("ring buffer malloc error : %d, %d", rb_size, qosa_task_get_free_heap_size());
		return -1;
	}
	ringbuffer_init(&g_log_rb, g_log_rb_buf, rb_size);

	//2. Init sem
	ret = qosa_sem_create(&g_debug_service_sem_id, 0);
    if (g_debug_service_sem_id == NULL)
    {
        LOG_E("AT client initialize failed! g_debug_service_sem_id semaphore create failed!");
		return -1;
    }

	//3. Create thread
    ret = qosa_task_create(&g_debug_service_thread_id, 1024 * 4, QOSA_PRIORITY_ABOVE_NORMAL, "Log_Save", debug_service_thread_proc, NULL);
	if (NULL == g_debug_service_thread_id)
	{
		LOG_E("g_debug_service_thread_id thread could not start!");
		return -1;
	}
	LOG_I("%s over(%x)",__FUNCTION__, g_debug_service_thread_id);
}

static int debug_save_service_destroy(void)
{
	int ret;

	// //1. Destroy net service
    // if (NULL != g_debug_service_thread_id)
    // {
    //     ret = qosa_task_delete(g_debug_service_thread_id);
    //     if (0 != ret)
    //     {
    //         LOG_E("Delete g_debug_service_thread_id thread failed! %d", ret);
    //         return -1;
    //     }
	// 	g_debug_service_thread_id=NULL;
    // }

	//2. Delete msg/sem queue
	if (NULL != g_debug_service_sem_id)
    {
        ret = qosa_sem_delete(g_debug_service_sem_id);
        if (QOSA_OK != ret)
        {
            LOG_E("Delete g_debug_service_sem_id msg failed! %d", ret);
            return -1;
        }
		g_debug_service_sem_id=NULL;
    }

	//3. Free ringbuffer
	if (g_log_rb_buf != NULL)
		free(g_log_rb_buf);
	g_log_rb_buf = NULL;

	return 0;
}
#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SAVE__ */


int debug_service_create(void)
{
	int32_t ret;

	LOG_V("%s",__FUNCTION__);

	#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SHELL__
	debug_input_service_create();
	#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SHELL__ */

	return 0;
}

int debug_service_destroy(void)
{
	int32_t ret = 0;

    LOG_V("%s",__FUNCTION__);
	#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SHELL__
	debug_input_service_destroy();
	#endif /* __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SHELL__ */
	LOG_V("%s over",__FUNCTION__);

    return 0;
}


static int32_t usage_help(s32_t argc, char *argv[])
{
	if ((argc == 0) || (strcmp((const char *)argv[0], "help")==0))
	{
		LOG_I("--------------------------------------------");
		// LOG_I("| Usage:(%s)                |", __QUECTEL_USER_FRIENDLY_PROJECT_VERSION);
		LOG_I("| Usage:(%s)", QUECTEL_PROJECT_VERSION);
		int32_t i = 0, debug_fun_array_size = sizeof(g_debug_fun_array)/sizeof(g_debug_fun_array[0]);

		for(i = 0; i < debug_fun_array_size; i++)
		{
			LOG_I("| %-40s |", g_debug_fun_array[i].module_name);
		}
		LOG_I("--------------------------------------------");
	}

	return -1;
}

static int32_t debug_service_test(s32_t argc, char *argv[])
{
    int32_t i, save_debug_flag;

    //LOG_V("%s",__FUNCTION__);

    for (i=0; i<argc; i++)
    {
        //LOG_V("%d = %s", i, argv[i]);
    }

	if ((argc == 0) || (strcmp((const char *)argv[0], "help")==0))
	{
		LOG_I("--------------------------------------------");
		LOG_I("| debug:                                   |");
		LOG_I("--------------------------------------------");
        LOG_I("| at (cmd)                                 |");
        LOG_I("| mode  ( 0:debug, 1:release    )          |");
		LOG_I("| save  ( 0:off, 1:on, 2:status )          |");
        LOG_I("| level ( 0:V, 1:D, 2:I, 3:W, 4:E )        |");
		LOG_I("| test                                     |");
        LOG_I("| help                                     |");
        LOG_I("| exit                                     |");
		LOG_I("--------------------------------------------");
	}
	else if (strcmp((const char *)argv[0], "exit")==0)
	{
        LOG_D("exit %s", __FUNCTION__);
        return -1;
	}
	else if (strcmp((const char *)argv[0], "test")==0)
	{
		LOG_V("[V]: Welcome to use quectel module");
		LOG_D("[D]: Welcome to use quectel module");
		LOG_I("[I]: Welcome to use quectel module");
		LOG_W("[W]: Welcome to use quectel module");
		LOG_E("[E]: Welcome to use quectel module");
	}
	else if (strcmp((const char *)argv[0], "level")==0)
	{
		g_debug_level = atoi(argv[1]);
	}
	else if (strcmp((const char *)argv[0], "mode")==0)
	{
		g_debug_mode = atoi(argv[1]);
	}
	else if (strcmp((const char *)argv[0], "save")==0)
	{
		#ifdef __QUECTEL_USER_FRIENDLY_PROJECT_FEATURE_SUPPORT_DEBUG_SAVE__
		if ((argc > 1))
		{
			if ( atoi(argv[1]) == 2 || atoi(argv[1]) == g_save_debug_flag )  //status
			{
				LOG_I("Current log saving status is %s, free heap size is %d", (g_save_debug_flag == 1 ? "on" : "off"), qosa_task_get_free_heap_size());
			}
			else if (atoi(argv[1]) == 1)  //on
			{
				if (argc != 3)
				{
					LOG_I("Current free heap space(%d), default use 2048 byte for input buffer size", qosa_task_get_free_heap_size());
					g_save_debug_flag = 1;
					debug_save_service_create(2048);
				}
				else
				{
					LOG_I("Now start saving the log to sd card, input buffer size is %d, free heap size is %d", atoi(argv[2]), qosa_task_get_free_heap_size());
					g_save_debug_flag = 1;
					debug_save_service_create(atoi(argv[2]));
				}
			}
			else  //off
			{
				LOG_I("Now stop saving the log");
				g_save_debug_flag = 0;
				if (g_debug_service_sem_id)
					qosa_sem_release(g_debug_service_sem_id);
				qosa_task_sleep_ms(1000);
				debug_save_service_destroy();
			}
		}
		#else
        LOG_W("This function is not supported");
		#endif
	}
	else if (strcmp((const char *)argv[0], "at")==0)
	{
		at_response_t resp;
		LOG_V("%s", argv[1]);
		resp = at_create_resp(1024, 0, 3000);
		at_exec_cmd(resp, argv[1]);
    	at_delete_resp(resp);
	}
    else
    {
        LOG_E("Invalid parameter:%s", argv[0]);
    }
	LOG_V("%s over",__FUNCTION__);

    return 0;
}
