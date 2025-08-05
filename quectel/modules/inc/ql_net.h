/**
 * @file quectel_network.h 
 * @brief Quectel module network interface definitions
 */

#ifndef __QUECTEL_NETWORK_H__
#define __QUECTEL_NETWORK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "at.h"
#include "quectel_common_def.h"

typedef enum
{
    QT_NETWORK_PARAM_ECHO,
    QT_NETWORK_PARAM_SCANMODE,
    QT_NETWORK_PARAM_SCANSEQ,
    QT_NETWORK_PARAM_IOTOPMODE,
    QT_NETWORK_PARAM_SAVE,

    QT_NETWORK_PARAM_UNKNOWN
} QtNetworkParams;

typedef struct quectel_network
{
    at_client_t client;
    int echo;
    int scanmode;
    int iotopmode;
    int save;
    int contextid;
    char scan_seq[8];
    char ip[16];
} quectel_network_s;

typedef quectel_network_s* quectel_network_t;

quectel_network_t quectel_network_init(at_client_t client);

void quectel_network_set_param(quectel_network_t handle, QtNetworkParams type, void* value);

QtNetworkErrCode quectel_network_attach(quectel_network_t handle);
int quectel_network_get_rssi(quectel_network_t handle);

char* quectel_network_get_ip(quectel_network_t handle);

void quectel_module_reboot(quectel_network_t handle);

void quectel_network_detach(quectel_network_t handle);

void quectel_network_deinit(quectel_network_t handle);

#ifdef __cplusplus
}
#endif

#endif // __QUECTEL_NETWORK_H__