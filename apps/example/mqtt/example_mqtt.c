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
void example_mqtt_test()
{
    {
        quectel_mqtt_client_t handle = quectel_mqtt_client_create(at_client_get_first());
        // quectel_mqtt_setopt(handle, QT_MQTT_OPT_PDPCID, 1);
        // qt_mqtt_will_options_s will = {true, QT_MQTT_QOS0, true, "will_topic", "will_msg"};
        // quectel_mqtt_setopt(handle, QT_MQTT_OPT_WILL, &will);
        // qt_mqtt_timeout_options_s timeout = {4, 5, true};
        // quectel_mqtt_setopt(handle, QT_MQTT_OPT_TIMEOUT, &timeout);
        // quectel_mqtt_setopt(handle, QT_MQTT_OPT_CLEAN_SESSION, false);
        // quectel_mqtt_setopt(handle, QT_MQTT_OPT_KEEP_ALIVETIME, 110);
        // qt_mqtt_ssl_options_s ssl = {true, 1};
        // quectel_mqtt_setopt(handle, QT_MQTT_OPT_SSL_ENABLE, &ssl);
        qt_mqtt_ali_auth_options_s auth = {.product_key = "a1vvrmkn43t", .device_name = "NiFtKoHMcu6j0VIXtC6e", .device_secret = "3115a9a768482d98a28d7390e7b9376b"};
        quectel_mqtt_setopt(handle, QT_MQTT_OPT_ALI_AUTH, &auth);
        quectel_mqtt_setopt(handle, QT_MQTT_OPT_SUB_CALLBACK, sub_func);
        quectel_mqtt_setopt(handle, QT_MQTT_OPT_USER_DATA, handle);
        QtMqttErrCode err = quectel_mqtt_connect(handle, "a1vvrmkn43t.iot-as-mqtt.cn-shanghai.aliyuncs.com", 1883, NULL, NULL);
        LOG_I("err = %d", err);
        if (err != QT_MQTT_OK)
        {
            qutecel_mqtt_destroy(handle);
            return;
        }
        quectel_mqtt_sub(handle, "/a1vvrmkn43t/NiFtKoHMcu6j0VIXtC6e/user/get", QT_MQTT_QOS0);
        quectel_mqtt_pub(handle, "/a1vvrmkn43t/NiFtKoHMcu6j0VIXtC6e/user/get", "hello world!", QT_MQTT_QOS0, false);
        quectel_mqtt_disconnect(handle);
        qutecel_mqtt_destroy(handle);
    }
    {
        quectel_mqtt_client_t handle = quectel_mqtt_client_create(at_client_get_first());
        // quectel_mqtt_setopt(handle, QT_MQTT_OPT_PDPCID, 1);
        // qt_mqtt_will_options_s will = {true, QT_MQTT_QOS0, true, "will_topic", "will_msg"};
        // quectel_mqtt_setopt(handle, QT_MQTT_OPT_WILL, &will);
        // qt_mqtt_timeout_options_s timeout = {4, 5, true};
        // quectel_mqtt_setopt(handle, QT_MQTT_OPT_TIMEOUT, &timeout);
        // quectel_mqtt_setopt(handle, QT_MQTT_OPT_CLEAN_SESSION, false);
        // quectel_mqtt_setopt(handle, QT_MQTT_OPT_KEEP_ALIVETIME, 110);
        // qt_mqtt_ssl_options_s ssl = {true, 1};
        // quectel_mqtt_setopt(handle, QT_MQTT_OPT_SSL_ENABLE, &ssl);
        quectel_mqtt_setopt(handle, QT_MQTT_OPT_SUB_CALLBACK, sub_func);
        quectel_mqtt_setopt(handle, QT_MQTT_OPT_USER_DATA, NULL);
        LOG_D("handle = %p", handle);
        QtMqttErrCode err = quectel_mqtt_connect(handle, "112.31.84.164", 8306, NULL, NULL);
        LOG_I("err = %d", err);
        quectel_mqtt_sub(handle, "/topic/test", QT_MQTT_QOS0);
        quectel_mqtt_sub(handle, "/topic/test1", QT_MQTT_QOS0);
        // quectel_mqtt_unsub(handle, "/a1vvrmkn43t/NiFtKoHMcu6j0VIXtC6e/user/get");
        quectel_mqtt_pub(handle, "/topic/test", "hello world!", QT_MQTT_QOS0, false);
        quectel_mqtt_pub(handle, "/topic/test1", "hello world!1", QT_MQTT_QOS1, false);
        // quectel_mqtt_disconnect(handle);
    }
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
