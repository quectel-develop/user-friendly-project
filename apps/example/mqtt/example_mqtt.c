#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__
#include "qosa_log.h"
#include "ql_mqtt.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#elif __linux__
#else
#include "cmsis_os.h"
#endif

static void sub_func(const char* topic, size_t size, const char* msg, void* arg)
{
    LOG_I("topic = %s, len = %d, msg = %s %p", topic, size, msg, arg);
}
void example_mqtt()
{
    // test 1
    {
        ql_mqtt_t handle = ql_mqtt_create(at_client_get_first());
        qt_mqtt_ali_auth_options_s auth = {.product_key = "a1vvrmkn43t", .device_name = "NiFtKoHMcu6j0VIXtC6e", .device_secret = "3115a9a768482d98a28d7390e7b9376b"};
        ql_mqtt_setopt(handle, QL_MQTT_OPT_ALI_AUTH, &auth);
        ql_mqtt_setopt(handle, QL_MQTT_OPT_SUB_CALLBACK, sub_func);
        ql_mqtt_setopt(handle, QL_MQTT_OPT_USER_DATA, handle);
        QL_MQTT_ERR_CODE_E err = ql_mqtt_connect(handle, "a1vvrmkn43t.iot-as-mqtt.cn-shanghai.aliyuncs.com", 1883, NULL, NULL);
        LOG_I("err = %d", err);
        if (err != QL_MQTT_OK)
        {
            ql_mqtt_destroy(handle);
            return;
        }
        ql_mqtt_sub(handle, "/a1vvrmkn43t/NiFtKoHMcu6j0VIXtC6e/user/get", QL_MQTT_QOS0);
        ql_mqtt_pub(handle, "/a1vvrmkn43t/NiFtKoHMcu6j0VIXtC6e/user/get", "hello world!", QL_MQTT_QOS0, false);
        ql_mqtt_disconnect(handle);
        ql_mqtt_destroy(handle);
    }
    // test 2
    {
        ql_mqtt_t handle = ql_mqtt_create(at_client_get_first());
        ql_mqtt_setopt(handle, QL_MQTT_OPT_SUB_CALLBACK, sub_func);
        ql_mqtt_setopt(handle, QL_MQTT_OPT_USER_DATA, NULL);
        LOG_D("handle = %p", handle);
        QL_MQTT_ERR_CODE_E err = ql_mqtt_connect(handle, "112.31.84.164", 8306, NULL, NULL);
        LOG_I("err = %d", err);
        ql_mqtt_sub(handle, "/topic/test", QL_MQTT_QOS0);
        ql_mqtt_sub(handle, "/topic/test1", QL_MQTT_QOS0);
        // ql_mqtt_unsub(handle, "/a1vvrmkn43t/NiFtKoHMcu6j0VIXtC6e/user/get");
        ql_mqtt_pub(handle, "/topic/test", "hello world!", QL_MQTT_QOS0, false);
        ql_mqtt_pub(handle, "/topic/test1", "hello world!1", QL_MQTT_QOS1, false);
        // ql_mqtt_disconnect(handle);
    }
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
