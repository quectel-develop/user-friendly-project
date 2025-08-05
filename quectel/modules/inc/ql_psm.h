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
#include <stdbool.h>
#ifndef __QL_PSM_H__
#define __QL_PSM_H__

typedef struct  {
	bool Mode;
	int Requested_Periodic_RAU;
	int Requested_GPRS_Ready_timer;
	int Requested_Periodic_TAU;
	int Requested_Active_Time;
} psm_setting;

typedef struct  {
	int threshold;
	int psm_version;

} psm_threshold_setting;

typedef struct  {
	int PSM_opt_mask;
	int max_oos_full_scans;
	int PSM_duration_due_to_oos;
	int PSM_randomization_window;
	int max_oos_time;
	int early_wakeup_time;
} psm_ext_cfg;

// read fucntions
int ql_psm_settings_read();
int ql_psm_threshold_settings_read();
int ql_psm_ext_cfg_read();
int ql_psm_ext_timer_read();
//write functions
int ql_psm_settings_write(psm_setting*);
int ql_psm_threshold_settings_write(psm_threshold_setting*);
int ql_psm_ext_cfg_write(psm_ext_cfg*);
int ql_psm_ext_timer_write(bool);

void ql_psm_stat();
void ql_psm_example();

#endif /* __QL_PSM_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_PSM__ */

