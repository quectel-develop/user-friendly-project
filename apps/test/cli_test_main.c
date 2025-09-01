#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#include "cli_test_main.h"
#include "debug_service.h"
#include "broadcast_service.h"
#include "qosa_system.h"
#include "qosa_log.h"
#include "ql_dev.h"

#include "cli_psm.h"
#include "cli_file.h"
#include "cli_ftp.h"
#include "cli_http.h"
#include "cli_mqtt.h"
#include "cli_socket.h"
#include "cli_tcp.h"
#include "cli_tcp_server.h"
#include "cli_udp.h"
#include "cli_udp_server.h"
#include "cli_net.h"

Cli_Menu_t cli_fun_table[] = {
    {"getversion",  cli_mcu_firmware_version,       NULL},
    {"network",     cli_net_test,                   cli_net_get_help},
    {"mqtt",        cli_mqtt_test,                  cli_mqtt_get_help},
    {"ftp",         cli_ftp_test,                   cli_ftp_get_help},
    {"http",        cli_http_test,                  cli_http_get_help},
    {"socket",      cli_socket_test,                cli_socket_get_help},
    {"file",        cli_file_test,                  cli_file_get_help},
    {"psm",         cli_psm_test,                   cli_psm_get_help},
    {"reboot",      cli_reboot,                     cli_reboot_help},
    {"at",          NULL,                           NULL},
    {"debug",       NULL,                           NULL},
    {"help",        NULL,                           NULL},
};
int32_t fun_cnt = sizeof(cli_fun_table)/sizeof(cli_fun_table[0]);
static osa_msgq_t g_main_msg_id = NULL;

void cli_test_main(void)
{
    int ret = 0;
    msg_node msgs;
    LOG_I("\t============ cli_test_main ============");

    /* 0. Flash selftest */
    ql_spi_flash_selftest();

    /* 1. Module hardware init */
    ql_module_hardware_init();

    /* 2. SDCard init */
    ql_sd_init();

    /* 3. Create msg queue */
    ret += qosa_msgq_create(&g_main_msg_id, sizeof(msg_node), MAX_MSG_COUNT);

    /* 4. Create broadcast service*/
    ret += broadcast_service_create();

    /* 5. Register which broadcasts need to be received */
    broadcast_reg_receive_msg(QL_BROADCAST_NET_DATACALL_SUCCESS,    g_main_msg_id);
    broadcast_reg_receive_msg(QL_BROADCAST_SOCKET_INIT_SUCCESS,     g_main_msg_id);
    broadcast_reg_receive_msg(QL_BROADCAST_SD_CARD_DETECT,          g_main_msg_id);

    /* 6. Create debug cli service */
    #ifdef __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_SHELL__
	ret += debug_cli_service_create();
    ret += debug_cli_func_reg(fun_cnt, cli_fun_table);
	#endif

    /* 7. AT Client init */
    ret += at_client_init(1024, 1024);

    /* 8. Create socket */
    #ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__
    #endif

    /* 9. Network init */
    ret += cli_net_test_init();

    if(ret != 0)
    {
        LOG_E("*** cli_test_main Init Failed ***");
        return;
    }

    while (1)
    {
        memset(&msgs, 0, sizeof(msg_node));
        ret = qosa_msgq_wait(g_main_msg_id, (void *)&msgs, sizeof(msg_node), QOSA_WAIT_FOREVER);
        if (ret != QOSA_OK)
        {
            LOG_E("Receive msg from broadcast thread error!");
            continue;
        }
        else
        {
            LOG_V("Receive broadcast msg is what=%d, arg1=%d, arg2=%d, arg3=%d", msgs.what, msgs.arg1, msgs.arg2, msgs.arg3);
            switch (msgs.what)
            {
                case QL_BROADCAST_NET_DATACALL_SUCCESS:
                    LOG_I("Initialization done, do your own business.");
                    debug_uart_input_notify();
                    break;

                case QL_BROADCAST_SD_CARD_DETECT:
                    ql_sdcard_hotplug_proc();
                    break;

                default:
                    LOG_D("Unrecognized message");
                    break;
            }
        }
    }
    qosa_task_exit();
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
