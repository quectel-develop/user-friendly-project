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
    /* Restart the module */
    ql_module_hardware_init();
    qosa_task_sleep_sec(5);
    ql_sd_init();

    /* AT Client init */
    at_client_init(128, 128);

    example_network();

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

#endif /*  __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */

    qosa_task_exit();
}
