/*
 * Copyright (c) 2025, FAE
 * @file quectel_mqtt_client.h 
 * @brief Quectel mqtt client interface definitions
 * Date           Author           Notes
 * 2025-7-16      Wells         first version
 */

#ifndef __QUECTEL_MQTT_H__
#define __QUECTEL_MQTT_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include "at.h"
#include "ql_ssl.h"
#include "quectel_common_def.h"


typedef void (*quectel_sub_cb_func)(const char* topic, size_t size, const char* msg, void* arg);

// MQTT QoS levels
typedef enum
{
    QT_MQTT_QOS0 = 0, // At most once
    QT_MQTT_QOS1,     // At least once
    QT_MQTT_QOS2      // exactly once
} QtMqttQoSLevel;


/*
    Not all of the following options are supported by the module.
    Please configure according to your requirements by referring to the documentation.
*/
typedef enum
{
    QT_MQTT_OPT_PDPCID,            // uint8_t (1-16)
    QT_MQTT_OPT_WILL,              // qt_mqtt_will_options_s
    QT_MQTT_OPT_TIMEOUT,           // qt_mqtt_timeout_options_s
    QT_MQTT_OPT_CLEAN_SESSION,     // default true, clean  disconnect client all messages
    QT_MQTT_OPT_KEEP_ALIVETIME,    // default 120s, range 0~3600s,  if set 0, server do not disconnect client when no message received
    QT_MQTT_OPT_SSL_ENABLE,        // qt_mqtt_ssl_options_s do not set，auto set if call quectel_mqtt_set_ssl function
    QT_MQTT_OPT_ALI_AUTH,          // qt_mqtt_ali_auth_options_s, if set this,  please set username/password to NULL when calling quectel_mqtt_connect.
    QT_MQTT_OPT_RECV_MODE,         // do not set, already set inside
    QT_MQTT_OPT_SUB_CALLBACK,      // sub callback  quectel_sub_cb_func
    QT_MQTT_OPT_USER_DATA,         // user data is used by sub callback
    QT_MQTT_OPT_UNKNOWN
} QtMqttClientOption;

/*
 * MQTT Client Status
*/
typedef enum
{
    QT_MQTT_STATUS_CLOSED,
    QT_MQTT_STATUS_OPEN,
    QT_MQTT_STATUS_CONNECTED,
    QT_MQTT_STATUS_DISCONNECTED,
} QtMqttClientStatus;

typedef struct qt_mqtt_will_options
{
    bool flag; //default false
    QtMqttQoSLevel qos;
    bool retain;
    const char *topic;
    const char *msg;
} qt_mqtt_will_options_s;

typedef struct qt_mqtt_timeout_options
{
    int pkt_timeout;    // default 5, range 1~60
    int retry_times;    // default 3, range 0~10
    bool timeout_notice; // default false, disable notice
} qt_mqtt_timeout_options_s;

typedef struct qt_mqtt_ssl_options
{
    bool ssl_enable;    // default false (not use ssl)
    int ssl_ctx_id;     // range 0~5
} qt_mqtt_ssl_options_s;

typedef struct qt_mqtt_ali_auth_options
{
    const char *product_key;    // Product key issued by AliCloud.
    const char *device_name;    // Device name issued by AliCloud
    const char *device_secret;  // Device secret key issued by AliCloud
} qt_mqtt_ali_auth_options_s;

typedef struct quectel_mqtt_client
{
    at_client_t client;
    int client_idx;
    uint16_t mid;
    osa_sem_t sem;
    osa_mutex_t lock;
    quectel_sub_cb_func sub_cb;
    void *user_data;
    QtMqttClientStatus status;
    ql_SSL_Config ssl;

} quectel_mqtt_client_s;


typedef quectel_mqtt_client_s* quectel_mqtt_client_t;

/**
 * @brief Create the MQTT client instance
 * 
 * @param client AT client handle used for underlying communication
 * @return quectel_mqtt_client_t Handle to the created MQTT client instance
 */
quectel_mqtt_client_t quectel_mqtt_client_create(at_client_t client);

/**
 * @brief Set MQTT options
 * 
 * @param handle MQTT client handle returned by quectel_mqtt_client_create()
 * @param option Option type to set (see QtMqttClientOption enum)
 * @param ... Variable arguments depending on the option being set
 * @return true if option was set successfully, false otherwise
 */
bool quectel_mqtt_setopt(quectel_mqtt_client_t handle, QtMqttClientOption option, ...);

/**
 * @brief Set SSL configuration for an MQTT client
 * @param handle MQTT client handle returned by quectel_http_init()
 * @param config SSL configuration
 */
void quectel_mqtt_set_ssl(quectel_mqtt_client_t handle, ql_SSL_Config config);

/**
 * @brief Connect to MQTT broker
 * 
 * @param handle MQTT client handle returned by quectel_mqtt_client_create()
 * @param server mqtt server address
 * @param port  mqtt server port
 * @param username mqtt username if not required, set to NULL
 * @param password mqtt password if not required, set to NULL
 * @return QtHttpErrCode Error code indicating connect status
 */
QtMqttErrCode quectel_mqtt_connect(quectel_mqtt_client_t handle, const char* server, int port, const char* username, const char* password);

/**
 * @brief Disconnect MQTT client
 * @param handle MQTT client handle
 */
void quectel_mqtt_disconnect(quectel_mqtt_client_t handle);

/*
 * @brief Publish message to MQTT server
 * @param handle MQTT client handle
 * @param topic Topic name
 * @param msg Message
 * @param qos Quality of service
 * @param retain Retain message
 * @return QT_MQTT_OK if success, otherwise error code
*/
QtMqttErrCode quectel_mqtt_pub(quectel_mqtt_client_t handle, const char *topic, const char *message, QtMqttQoSLevel qos, bool retain);

/*
 * @brief Subscribe to MQTT topic
 * @param handle MQTT client handle
 * @param topic Topic name
 * @param qos Quality of service level
 * @return QT_MQTT_OK if success, otherwise error code
*/
QtMqttErrCode quectel_mqtt_sub(quectel_mqtt_client_t handle, const char *topic, QtMqttQoSLevel qos);

/*
 * @brief Subscribe to MQTT topic
 * @param handle MQTT client handle
 * @param topic Topic name
*/
QtMqttErrCode quectel_mqtt_unsub(quectel_mqtt_client_t handle, const char *topic);

/*
 * @brief Destroy MQTT client
 * @param handle MQTT client handle returned by quectel_mqtt_client_create()
*/
void qutecel_mqtt_destroy(quectel_mqtt_client_t handle);

#ifdef __cplusplus
}
#endif

#endif // __QUECTEL_MQTT_CLIENT_H__