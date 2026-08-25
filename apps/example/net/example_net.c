#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__
#include "qosa_log.h"
#include "ql_net.h"
void example_network(void)
{
    ql_net_t s_net_handle = ql_net_init(at_client_get_first());
    if (NULL == s_net_handle)
        return;
    if (ql_usim_get(s_net_handle) != QL_NET_OK)
    {
        ql_net_deinit(s_net_handle);
        return;
    }
    ql_net_set_opt(s_net_handle, QL_NET_OPTINON_SCANMODE, 0);
    // ql_net_set_opt(s_net_handle, QL_NET_OPTINON_SCANSEQ, "0203");
    ql_net_set_opt(s_net_handle, QL_NET_OPTINON_IOTOPMODE, 2);
    ql_net_content_s content = {0};
    content.contentid = 1;
    content.apn = "apn";
    content.username = "username";
    content.password = "password";
    ql_net_set_opt(s_net_handle, QL_NET_OPTINON_CONTENT, &content);
    int try = 3;
    while (try -- > 0)
    {
        if (ql_net_attach(s_net_handle) == QL_NET_OK)
            break;
        LOG_E("network attach error!");
        ql_module_reboot(s_net_handle);
    }
    qosa_task_sleep_ms(2000);
    int strength = 0;
    int quality = 0;
    int ret = ql_net_get_signal_info(s_net_handle, &strength, &quality);
    if (0 == ret)
        LOG_I("network strength = %d, quality = %d", strength, quality);
    else
        LOG_E("get signal strength failed");
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
