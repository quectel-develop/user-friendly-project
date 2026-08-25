/*
 * Copyright (c) 2025, FAE
 * @file ql_mqtt.h 
 * @brief Quectel mqtt interface definitions
 * Date           Author           Notes
 * 2025-7-16      Wells         first version
 */

#ifndef __QL_MQTT_H__
#define __QL_MQTT_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include "at.h"
#include "ql_ssl.h"
#include "ql_common_def.h"


typedef void (*ql_sub_cb_func)(const char* topic, size_t size, const char* msg, void* arg);

// MQTT QoS levels
typedef enum
{
    QL_MQTT_QOS0 = 0, // At most once
    QL_MQTT_QOS1,     // At least once
    QL_MQTT_QOS2      // exactly once
} QL_MQTT_QOS_LEVEL_E;


/*
    Not all of the following options are supported by the module.
    Please configure according to your requirements by referring to the documentation.
*/
typedef enum
{
    QL_MQTT_OPT_CONTEXT_ID,        // uint8_t (1-16)
    QL_MQTT_OPT_WILL,              // qt_mqtt_will_options_s
    QL_MQTT_OPT_TIMEOUT,           // qt_mqtt_timeout_options_s
    QL_MQTT_OPT_CLEAN_SESSION,     // default true, clean  disconnect client all messages
    QL_MQTT_OPT_KEEP_ALIVETIME,    // default 120s, range 0~3600s,  if set 0, server do not disconnect client when no message received
    QL_MQTT_OPT_SSL_ENABLE,        // qt_mqtt_ssl_options_s do not set，auto set if call ql_mqtt_set_ssl function
    QL_MQTT_OPT_ALI_AUTH,          // qt_mqtt_ali_auth_options_s, if set this,  please set username/password to NULL when calling ql_mqtt_connect.
    QL_MQTT_OPT_RECV_MODE,         // do not set, already set inside
    QL_MQTT_OPT_SUB_CALLBACK,      // sub callback  ql_sub_cb_func
    QL_MQTT_OPT_USER_DATA,         // user data is used by sub callback
    QL_MQTT_OPT_UNKNOWN
} QL_MQTT_OPTION_E;

/*
 * MQTT Client Status
*/
typedef enum
{
    QL_MQTT_STATUS_CLOSED,
    QL_MQTT_STATUS_OPEN,
    QL_MQTT_STATUS_CONNECTING,
    QL_MQTT_STATUS_CONNECTED,
    QL_MQTT_STATUS_DISCONNECTING,
    QL_MQTT_STATUS_DISCONNECTED,
} QL_MQTT_STATUS_E;

typedef struct qt_mqtt_will_options
{
    bool flag; //default false
    QL_MQTT_QOS_LEVEL_E qos;
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

typedef struct ql_mqtt
{
    at_client_t client;
    int client_idx;
    uint16_t mid;
    int pkt_timeout;
    int retry_times;
    osa_sem_t sem;
    osa_mutex_t lock;
    ql_sub_cb_func sub_cb;
    void *user_data;
    QL_MQTT_STATUS_E status;
    ql_SSL_Config ssl;

} ql_mqtt_s;


typedef ql_mqtt_s* ql_mqtt_t;

/**
 * @brief Create the MQTT client instance
 * 
 * @param client AT client handle used for underlying communication
 * @return ql_mqtt_t Handle to the created MQTT client instance
 */
ql_mqtt_t ql_mqtt_create(at_client_t client);

/**
 * @brief Set MQTT options
 * 
 * @param handle MQTT client handle returned by ql_mqtt_create()
 * @param option Option type to set (see QL_MQTT_OPTION_E enum)
 * @param ... Variable arguments depending on the option being set
 * @return true if option was set successfully, false otherwise
 */
bool ql_mqtt_setopt(ql_mqtt_t handle, QL_MQTT_OPTION_E option, ...);

/**
 * @brief Set SSL configuration for an MQTT client
 * @param handle MQTT client handle returned by ql_http_init()
 * @param config SSL configuration
 * @return true if SSL configuration was set successfully, false otherwise
 */
bool ql_mqtt_set_ssl(ql_mqtt_t handle, ql_SSL_Config config);

/**
 * @brief Connect to MQTT broker
 * 
 * @param handle MQTT client handle returned by ql_mqtt_create()
 * @param server mqtt server address
 * @param port  mqtt server port
 * @param username mqtt username if not required, set to NULL
 * @param password mqtt password if not required, set to NULL
 * @return QL_MQTT_ERR_CODE_E Error code indicating connect status
 */
QL_MQTT_ERR_CODE_E ql_mqtt_connect(ql_mqtt_t handle, const char* server, int port, const char* username, const char* password);

/**
 * @brief Disconnect MQTT client
 * @param handle MQTT client handle
 */
void ql_mqtt_disconnect(ql_mqtt_t handle);

/*
 * @brief Publish message to MQTT server
 * @param handle MQTT client handle
 * @param topic Topic name
 * @param msg Message
 * @param qos Quality of service
 * @param retain Retain message
 * @return QL_MQTT_OK if success, otherwise error code
*/
QL_MQTT_ERR_CODE_E ql_mqtt_pub(ql_mqtt_t handle, const char *topic, const char *message, QL_MQTT_QOS_LEVEL_E qos, bool retain);

/*
 * @brief Subscribe to MQTT topic
 * @param handle MQTT client handle
 * @param topic Topic name
 * @param qos Quality of service level
 * @return QL_MQTT_OK if success, otherwise error code
*/
QL_MQTT_ERR_CODE_E ql_mqtt_sub(ql_mqtt_t handle, const char *topic, QL_MQTT_QOS_LEVEL_E qos);

/*
 * @brief Subscribe to MQTT topic
 * @param handle MQTT client handle
 * @param topic Topic name
*/
QL_MQTT_ERR_CODE_E ql_mqtt_unsub(ql_mqtt_t handle, const char *topic);

/*
 * @brief Destroy MQTT client
 * @param handle MQTT client handle returned by ql_mqtt_create()
*/
void ql_mqtt_destroy(ql_mqtt_t handle);

#ifdef __cplusplus
}
#endif

#endif // __QL_MQTT_H__