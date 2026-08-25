#include "QuectelConfig.h"

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_PSM__
/*
 * Copyright (c) 2024, FAE
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author           Notes
 * 2024-1-5     kartigesan       first version
 */
#ifndef __QL_PSM_H__
#define __QL_PSM_H__

#include <stdbool.h>
#include "at.h"

typedef struct
{
	bool Mode;                  /**< PSM mode: true-enabled, false-disabled */
	int Requested_Periodic_TAU; /**< Requested Periodic TAU value (unit: seconds) */ 
	int Requested_Active_Time;  /**< Requested Active Time value (unit: seconds) */
} ql_psm_setting_s;

/*
 * @brief write psm settings
 * @param client AT client
 * @param settings psm settings
*/
int ql_psm_settings_write(at_client_t client, ql_psm_setting_s settings);

// read fucntions
int ql_psm_settings_read(at_client_t client, ql_psm_setting_s *settings);

void ql_psm_pon_trig_ctrl(at_client_t client, int value);

void ql_psm_wakeup(at_client_t client);
#endif /* __QL_PSM_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_PSM__ */

