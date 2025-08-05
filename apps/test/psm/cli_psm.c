#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_PSM__
#include "cli_psm.h"
#include "qosa_log.h"

void cli_psm_get_help(void)
{
    LOG_I("| psm                                                                                              |");
    LOG_I("| \tpsm enable/disable                                                                             |");
    LOG_I("| \tpsm settings - TAU/active time (ex setting 00000100 00001111))                                 |");
    LOG_I("|                             0: Requested Periodic TAU                                            |");
    LOG_I("|                             1: Requested Active Time                                             |");
    LOG_I("| \tpsm threshold - sets the minimum threshold value to enter PSM(ex threshold 100))               |");
    LOG_I("| \tpsm modem Optimization - sets the Modem Optimization (ex modem 2 2 120 5 120 3))               |");
    LOG_I("|                             0: PSM opt mask                                                      |");
    LOG_I("|                             1: PSM max oos full scans                                            |");
    LOG_I("|                             2: PSM duration due to oos                                           |");
    LOG_I("|                             3: PSM randomization window                                          |");
    LOG_I("|                             4: PSM max oos time                                                  |");
    LOG_I("|                             5: PSM early wakeup time                                             |");
    LOG_I("| \tpsm stat - show all psm setting                                                                |");
}

int cli_psm_test(s32_t argc, char *argv[])
{
    static psm_setting psm_setting_test;
    static psm_threshold_setting psm_threshold_setting_test;
    static psm_ext_cfg psm_ext_cfg_test;

    if (strcmp((const char *)argv[1], "enable") == 0)
    {
        LOG_I("PSM mode enabled");
        psm_setting_test.Mode = QOSA_TRUE;
        ql_psm_settings_write(&psm_setting_test);
    }
    else if (strcmp((const char *)argv[1], "disable") == 0)
    {
        LOG_I("PSM mode disabled");
        psm_setting_test.Mode = QOSA_FALSE;
        ql_psm_settings_write(&psm_setting_test);
    }
    else if (strcmp((const char *)argv[1], "setting") == 0)
    {
        psm_setting_test.Mode = QOSA_TRUE;
        psm_setting_test.Requested_Periodic_TAU = atoi(argv[2]);
        psm_setting_test.Requested_Active_Time = atoi(argv[3]);

        LOG_I("PSM mode enabled");
        LOG_I("PSM requested periodic TAU: %08d", psm_setting_test.Requested_Periodic_TAU);
        LOG_I("PSM requested active time: %08d", psm_setting_test.Requested_Active_Time);

        ql_psm_settings_write(&psm_setting_test);
    }
    else if (strcmp((const char *)argv[1], "threshold") == 0)
    {
        psm_threshold_setting_test.threshold = atoi(argv[2]);
        LOG_I("PSM threshold: %d", psm_threshold_setting_test.threshold);
        ql_psm_threshold_settings_write(&psm_threshold_setting_test);
    }
    else if (strcmp((const char *)argv[1], "modem") == 0)
    {
        psm_ext_cfg_test.PSM_opt_mask = atoi(argv[2]);
        psm_ext_cfg_test.max_oos_full_scans = atoi(argv[3]);
        psm_ext_cfg_test.PSM_duration_due_to_oos = atoi(argv[4]);
        psm_ext_cfg_test.PSM_randomization_window = atoi(argv[5]);
        psm_ext_cfg_test.max_oos_time = atoi(argv[6]);
        psm_ext_cfg_test.early_wakeup_time = atoi(argv[7]);

        LOG_I("PSM opt mask: %d", psm_ext_cfg_test.PSM_opt_mask);
        LOG_I("PSM max oos full scans: %d", psm_ext_cfg_test.max_oos_full_scans);
        LOG_I("PSM duration due to oos: %d", psm_ext_cfg_test.PSM_duration_due_to_oos);
        LOG_I("PSM randomization window: %d", psm_ext_cfg_test.PSM_randomization_window);
        LOG_I("PSM max oos time: %d", psm_ext_cfg_test.max_oos_time);
        LOG_I("PSM early wakeup time: %d", psm_ext_cfg_test.early_wakeup_time);

        ql_psm_ext_cfg_write(&psm_ext_cfg_test);
    }
    else if (strcmp((const char *)argv[1], "stat") == 0)
        ql_psm_stat();
}

#else
void cli_psm_get_help(void)
{
    LOG_W("This function is not supported");
}
int cli_psm_test(int argc, char *argv[])
{
    LOG_W("This function is not supported");
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_PSM__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
