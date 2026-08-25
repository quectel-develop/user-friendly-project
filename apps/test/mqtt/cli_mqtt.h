#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__
#ifndef __CLI_MQTT_H__
#define __CLI_MQTT_H__
#include "ql_mqtt.h"
#include "qosa_def.h"

void cli_mqtt_get_help(void);
int cli_mqtt_test(s32_t argc, char *argv[]);
#endif /* __CLI_MQTT_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
