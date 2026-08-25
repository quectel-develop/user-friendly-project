#include "QuectelConfig.h"
#ifndef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_PSM__
#include "ql_psm.h"
#include "qosa_log.h"


void example_psm(void)
{
    ql_psm_setting_s psm_setting_test;
    psm_setting_test.Mode = QOSA_TRUE;
    psm_setting_test.Requested_Periodic_TAU = 3600;
    psm_setting_test.Requested_Active_Time = 60;

    LOG_I("PSM mode enabled");
    LOG_I("PSM requested periodic TAU: %d", psm_setting_test.Requested_Periodic_TAU);
    LOG_I("PSM requested active time: %d", psm_setting_test.Requested_Active_Time);
    if (ql_psm_settings_write(at_client_get_first(), psm_setting_test) == 0)
    {
        LOG_I("PSM settings OK");
    }
    else
    {
        LOG_E("PSM settings error");
    }

    // query PSM settings
    if (ql_psm_settings_read(at_client_get_first(), &psm_setting_test) == 0)
    {
        LOG_I("PSM query OK");
        LOG_I("PSM mode %s", psm_setting_test.Mode ? "enabled" : "disabled");
        LOG_I("PSM requested periodic TAU: %d", psm_setting_test.Requested_Periodic_TAU);
        LOG_I("PSM requested active time: %d", psm_setting_test.Requested_Active_Time);
    }
    else
    {
        LOG_E("PSM query error");
    }
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_PSM__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
