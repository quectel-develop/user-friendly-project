#include "QuectelConfig.h"
#include "qosa_log.h"
#include "cli_test_main.h"
#include "ql_dev.h"
#include "example_fs.h"
#include "example_net.h"
#include "example_http.h"
#include "example_mqtt.h"
#include "example_ftp.h"
#include <at.h>


void user_main(void* argv)
{
    LOG_D("Welcome to Quectel User Friendly Project !");
    LOG_D("Current version: %s @%s", QUECTEL_PROJECT_VERSION, QUECTEL_BUILD_ENV);

/* Cli test mode */
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
    cli_test_main();

/* Example code mode */
#else
    /* 1. Module hardware init */
    ql_module_hardware_init();

    /* 2. SDCard init */
    ql_sd_init();

    /* 3. AT UART init */
    ql_at_uart_init(at_client_get_first());

    LOG_I("########## example start ##########");

    #ifdef __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__
    example_network();
    #endif

    #ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__
    example_file();
    #endif

    #ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__
    example_ftp();
    #endif
    #ifdef __QUECTEL_UFP_FEATURE_SUPPORT_HTTP_S__
    example_test_post();
    #endif

    #ifdef __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__
    example_mqtt();
    #endif

    qosa_task_sleep_ms(500);
    LOG_I("########## example end ##########");

#endif /*  __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */

    qosa_task_exit();
}
