/**
 * @file ql_net.h 
 * @brief quectel network interface definitions
 */

#ifndef __QL_NET_H__
#define __QL_NET_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "at.h"
#include "ql_common_def.h"

typedef enum
{
    QL_NET_OPTINON_CONTENT, // see ql_net_content_s
    QL_NET_OPTINON_SCANMODE,
    QL_NET_OPTINON_SCANSEQ,
    QL_NET_OPTINON_IOTOPMODE,
    QL_NET_OPTINON_BAND,

    QL_NET_OPTINON_UNKNOWN
} QL_NET_OPTION_E;

typedef struct ql_net_content
{
    int contentid;
    char* apn;
    char* username;
    char* password;
} ql_net_content_s;

/*
    Hexadecimal data is stored in a char array.
    if bandx set 0, means not cnange
*/
typedef struct ql_net_band
{
    char band1[32]; // GSM or CDMA
    char band2[32]; // LTE
    char band3[32]; // CDMA or NB-IOT
} ql_net_band_s;

typedef struct ql_net
{
    at_client_t client;
    int contextid;
    char ip[16];
} ql_net_s;

typedef ql_net_s* ql_net_t;

/**
 * @brief Initialize the Network instance
 * @param client AT client handle used for underlying communication
 * @return ql_net_t Handle to creat Network instance
 */
ql_net_t ql_net_init(at_client_t client);

/*
 * @brief Get USIM status
 * @param handle net client handle returned by ql_net_init()
 * @return QL_NET_ERR_CODE_E Error code
*/
QL_NET_ERR_CODE_E ql_usim_get(ql_net_t handle);

/**
 * @brief Set network options
 * 
 * @param handle net client handle returned by ql_net_init()
 * @param option Option type to set (see QL_NET_OPTION_E enum)
 * @param ... Variable arguments depending on the option being set
 * @return true if option was set successfully, false otherwise
 */
bool ql_net_set_opt(ql_net_t handle, QL_NET_OPTION_E option, ...);

/*
 * @brief Attach to network
 * @param handle net client handle returned by ql_net_init()
 * @return QL_NET_ERR_CODE_E Error code
*/
QL_NET_ERR_CODE_E ql_net_attach(ql_net_t handle);

/*
 * @brief get signal strength and quality
 * @param handle net client handle returned by ql_net_init()
 * @param strength signal strength
 * @param quality signal quality
 * @return 0: get success, -1: get failed
*/
int ql_net_get_signal_info(ql_net_t handle, int *strength, int *quality);

/*
 * @brief Get IP address
 * @param handle net client handle returned by ql_net_init()
 * @return IP address
*/
const char* ql_net_get_ip(ql_net_t handle);

/* @brief Check if network is OK.
 * @param handle net client handle returned by ql_net_init()
 * @return true if network is OK, false otherwise
*/
bool ql_net_is_ok(ql_net_t handle);

/*
 * @brief Reconnect to network
 * @param handle net client handle returned by ql_net_init()
 * @return QL_NET_ERR_CODE_E Error code
*/
QL_NET_ERR_CODE_E ql_net_reconnect(ql_net_t handle);

/*
 * @brief Reboot the module
 * @param handle net client handle returned by ql_net_init()
 * @return void
*/
void ql_module_reboot(ql_net_t handle);

/*
 * @brief Detach from network
 * @param handle net client handle returned by ql_net_init()
 * @return void
*/
void ql_net_detach(ql_net_t handle);

/*
 * @brief Deinitialize the Network instance
 * @param handle net client handle returned by ql_net_init()
 * @return void
*/
void ql_net_deinit(ql_net_t handle);

#ifdef __cplusplus
}
#endif

#endif // __QL_NET_H__