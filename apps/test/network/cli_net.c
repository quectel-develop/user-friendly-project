#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__
#include "qosa_log.h"
#include "broadcast_service.h"
#include "ql_net.h"

ql_net_t s_net_handle = NULL;

void cli_net_get_help(void)
{
    LOG_I("network apn username password");
    LOG_I("apn        : Access Point Name");
    LOG_I("username   : Authentication username for the APN, Max length: 127 bytes");
    LOG_I("password   : Authentication password for the APN, Max length: 127 bytes");
}
int cli_net_test_init(void)
{
    s_net_handle = ql_net_init(at_client_get_first());
    if (NULL == s_net_handle)
        return -1;
    if (ql_usim_get(s_net_handle) != QL_NET_OK)
    {
        ql_net_deinit(s_net_handle);
        return -1;
    }
    ql_net_set_opt(s_net_handle, QL_NET_OPTINON_SCANMODE, 0);
    // ql_net_set_opt(s_net_handle, QL_NET_OPTINON_SCANSEQ, "0203");
    ql_net_set_opt(s_net_handle, QL_NET_OPTINON_IOTOPMODE, 2);
    if (ql_net_attach(s_net_handle) != QL_NET_OK)
    {
        LOG_E("Initialization is not complete. Please check if the SIM card is functioning properly and confirm whether APN/username/password needs to be configured");
        return -1;
    }
    qosa_task_sleep_ms(2000);
    int strength = 0;
    int quality = 0;
    int ret = ql_net_get_signal_info(s_net_handle, &strength, &quality);
    if (0 == ret)
        LOG_I("network strength = %d, quality = %d", strength, quality);
    else
        LOG_E("get signal strength failed");
    broadcast_send_msg_myself(QL_BROADCAST_NET_DATACALL_SUCCESS, 0, 0, 0);
    return 0;
}
int cli_net_test(s32_t argc, char *argv[])
{
    if (argc < 4)
    {
        LOG_E("Invalid parameter");
        cli_net_get_help();
        return -1;
    }
    if (NULL == s_net_handle)
    {
        s_net_handle = ql_net_init(at_client_get_first());
    }
    if (ql_usim_get(s_net_handle) != QL_NET_OK)
    {
        ql_net_deinit(s_net_handle);
        return -1;
    }
    ql_net_content_s content = {0};
    content.contentid = 1;
    content.apn = argv[1];
    content.username = argv[2];
    content.password = argv[3];
    ql_net_set_opt(s_net_handle, QL_NET_OPTINON_CONTENT, &content);
    if (ql_net_attach(s_net_handle) != QL_NET_OK)
    {
        LOG_E("network attach failed, Please check if the SIM card is functioning properly and confirm whether APN/username/password needs to be configured.");
        return -1;
    }
    qosa_task_sleep_ms(2000);
    int strength = 0;
    int quality = 0;
    int ret = ql_net_get_signal_info(s_net_handle, &strength, &quality);
    if (0 == ret)
        LOG_I("network strength = %d, quality = %d", strength, quality);
    else
        LOG_E("get signal strength failed");
    return 0;
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
