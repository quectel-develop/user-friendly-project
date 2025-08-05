#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__
#include "qosa_log.h"
#include "broadcast_service.h"
#include "ql_net.h"
void example_network(void)
{
    quectel_network_t s_network_handle = quectel_network_init(at_client_get_first());
    if (NULL == s_network_handle)
        return;
    int value = 0;
    quectel_network_set_param(s_network_handle, QT_NETWORK_PARAM_ECHO, &value);
    /*
     *   if not support , set -1 or do nothing
     *   if not set, use default value
    */
    value = -1;
    quectel_network_set_param(s_network_handle, QT_NETWORK_PARAM_SCANMODE, &value);
    value = 2;
    quectel_network_set_param(s_network_handle, QT_NETWORK_PARAM_IOTOPMODE, &value);
    while (1)
    {
        if (quectel_network_attach(s_network_handle) == QT_NETWORK_OK)
            break;
        LOG_E("network attach error!");
        quectel_module_reboot(s_network_handle);
    }
    qosa_task_sleep_ms(2000);
    int rssi = quectel_network_get_rssi(s_network_handle);
    LOG_D("network rssi = %d", rssi);
    broadcast_send_msg_myself(QL_BROADCAST_NET_DATACALL_SUCCESS, 0, 0, 0);
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
