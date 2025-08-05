#include "QuectelConfig.h"

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__
/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-10-12     armink       first version
 */
#ifndef __QL_SOCKET_H__
#define __QL_SOCKET_H__


int ql_socket_service_create(void);
int ql_socket_service_destroy(void);

#endif /* __QL_SOCKET_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__ */

