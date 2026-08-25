#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_PSM__
#include "cli_psm.h"
#include "qosa_log.h"

void cli_psm_get_help(void)
{
    LOG_I("| psm                                                                |");
    LOG_I("|    psm disable                                                     |");
    LOG_I("|    psm setting TAU/active time (ex setting 3600 60))               |");
    LOG_I("|    psm stat - show psm status                                      |");
    LOG_I("|    psm pon_trig value(0 or 1)                                      |");
    LOG_I("|                             1: enter psm                           |");
    LOG_I("|                             0: exit psm                            |");
    LOG_I("|    psm powerkey - exit psm                                         |");
}

int cli_psm_test(s32_t argc, char *argv[])
{
    ql_psm_setting_s psm_setting_test;

    if (strcmp((const char *)argv[1], "disable") == 0)
    {
        LOG_I("PSM mode disabled");
        psm_setting_test.Mode = QOSA_FALSE;
        psm_setting_test.Requested_Periodic_TAU = 3600;
        psm_setting_test.Requested_Active_Time = 60;
        if (ql_psm_settings_write(at_client_get_first(), psm_setting_test) == 0)
        {
            LOG_I("PSM settings OK");
        }
        else
        {
            LOG_E("PSM settings error");
        }
    }
    else if (strcmp((const char *)argv[1], "setting") == 0)
    {
        psm_setting_test.Mode = QOSA_TRUE;
        psm_setting_test.Requested_Periodic_TAU = atoi(argv[2]);
        psm_setting_test.Requested_Active_Time = atoi(argv[3]);

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
    
    }
    else if (strcmp((const char *)argv[1], "stat") == 0)
    {
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
    else if (strcmp((const char *)argv[1], "pon_trig") == 0)
    {
        ql_psm_pon_trig_ctrl(at_client_get_first(), atoi(argv[2]));
    }
    else if (strcmp((const char *)argv[1], "powerkey") == 0)
    {
        ql_psm_wakeup(at_client_get_first());
    }
    else
    {
        LOG_E("Invalid parameter");
        cli_psm_get_help();
    }
 
    return 0;
}

#else
void cli_psm_get_help(void)
{
    LOG_W("This function is not supported");
}
int cli_psm_test(s32_t argc, char *argv[])
{
    LOG_W("This function is not supported");
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_PSM__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
