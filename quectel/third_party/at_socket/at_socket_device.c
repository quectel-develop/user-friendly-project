/*
 * File      : at_device.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-05-08     chenyong     first version
 */
#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__
#include <stdlib.h>
#include <string.h>
#include "qosa_log.h"
#include <at_socket_device.h>
#include "qosa_def.h"

#define DBG_TAG              "at.dev"
#define DBG_LVL              DBG_INFO

static struct at_device g_at_device = { 0 };
static struct at_device_data g_at_device_data = { 0 };

struct at_device *at_device_get(void)
{
    return &g_at_device;
}

int at_device_socket_register(u32_t socket_num, struct at_socket_ops *socket_ops, struct at_client *client, ip_addr_t *ip_addr)
{
    int ret = QOSA_OK;

    g_at_device.socket_num = socket_num;
    g_at_device.socket_ops = socket_ops;
    g_at_device.client     = client;
    g_at_device.user_data  = (void *)&g_at_device_data;
    // memcpy(&g_at_device.ip_addr, ip_addr, sizeof(ip_addr_t));

    g_at_device.sockets = (struct at_socket *) calloc(socket_num, sizeof(struct at_socket));
    if (g_at_device.sockets == QOSA_NULL)
    {
        LOG_E("no memory for AT Socket number(%d) create.", socket_num);
        return QOSA_ERROR_NO_MEMORY;
    }

    ret = qosa_event_create(&g_at_device.socket_event);
    if (ret != QOSA_OK)
    {
        LOG_E("no memory for AT device socket event create.");
        free(g_at_device.sockets);
        return QOSA_ERROR_NO_MEMORY;
    }

    return QOSA_OK;
}

int at_device_socket_unregister(void)
{
    if (g_at_device.sockets)
        free(g_at_device.sockets);

    if (g_at_device.socket_event)
        qosa_event_delete(g_at_device.socket_event);

    return QOSA_OK;
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__ */
