#include "qosa_def.h"
#include "qosa_log.h"
#include "ql_module_compat.h"

static char s_module_name[64] = "UNKNOWN";

static const char *s_modules_not_support_net_scanmode[] =
{
    "BG95", "BG96", "BG77", 
    "BG600L",
    NULL
};

static const char *s_modules_support_net_iotopmode[] =
{
    "BG95", "BG96", "BG77", 
    "BG600L",
    NULL
};

static const char *s_modules_not_using_http_stop[] =
{
    "BG95", "BG96", "BG77", 
    "BG600L", "EC800E", "EC801E",
    NULL
};

static const char *s_modules_not_support_ftp_nlist[] =
{
    "BG95", "BG96", "BG77", 
    "BG600L",
    NULL
};

static const char *s_modules_not_support_ftp_data_addr_type_2[] =
{
    "BG95", "BG96", "BG77", 
    "BG600L",
    NULL
};

static const char *s_modules_using_mqtt_pubex[] =
{
    "BG95", "BG96", "BG77", 
    "BG600L",
    NULL
};

static const char *s_modules_not_using_mqtt_close[] =
{
    "BG770", 
    NULL
};

static const char *s_modules_using_cpsms_for_psm[] =
{
    "BG95", "BG96", "BG77", 
    "BG600L",
    NULL
};

static const char *s_modules_support_pon_trig[] =
{
    "BG770",
    NULL
};

static const char *s_modules_need_ctrl_dtr[] =
{
    "EC800E","EC801E", "EC800Z",
    NULL
};

static const char *s_modules_need_ctrl_ims[] =
{
    "EG915U","EG912U", "EC200U",
    NULL
};

static const char *s_modules_urc_cfg_is_main[] =
{
    "BG770",
    NULL
};

void ql_set_module_model(const char* model)
{
    if (NULL == model || strlen(model) <= 3)
        return;

    LOG_I("model = %s", model);
    strcpy(s_module_name, model);
    s_module_name[strlen(s_module_name) - 1] = '\0';  // delete \r\n
}

bool ql_support_net_scanmode()
{
    if (strstr(s_module_name, "BG95-M3") != NULL
        || strstr(s_module_name, "BG95-M5") != NULL
        || strstr(s_module_name, "BG95-M8") != NULL)
        return true;

    for (int i = 0; s_modules_not_support_net_scanmode[i] != NULL; i++)
    {
        if (strstr(s_module_name, s_modules_not_support_net_scanmode[i]) != NULL)
        {
            return false;
        }
    }
    return true;
}

bool ql_support_net_iotopmode()
{
    if (strstr(s_module_name, "BG95-M1") != NULL)
        return false;

    for (int i = 0; s_modules_support_net_iotopmode[i] != NULL; i++)
    {
        if (strstr(s_module_name, s_modules_support_net_iotopmode[i]) != NULL)
        {
            return true;
        }
    }
    return false;
}

bool ql_use_http_stop()
{
    for (int i = 0; s_modules_not_using_http_stop[i] != NULL; i++)
    {
        if (strstr(s_module_name, s_modules_not_using_http_stop[i]) != NULL)
        {
            return false;
        }
    }
    return true;
}

bool ql_support_ftp_nlist()
{
    for (int i = 0; s_modules_not_support_ftp_nlist[i] != NULL; i++)
    {
        if (strstr(s_module_name, s_modules_not_support_ftp_nlist[i]) != NULL)
        {
            return false;
        }
    }
    return true;
}

bool ql_support_ftp_data_addr_type_2()
{
    for (int i = 0; s_modules_not_support_ftp_data_addr_type_2[i] != NULL; i++)
    {
        if (strstr(s_module_name, s_modules_not_support_ftp_data_addr_type_2[i]) != NULL)
        {
            return false;
        }
    }
    return true;
}

bool ql_use_mqtt_pubex()
{
    for (int i = 0; s_modules_using_mqtt_pubex[i] != NULL; i++)
    {
        if (strstr(s_module_name, s_modules_using_mqtt_pubex[i]) != NULL)
        {
            return true;
        }
    }
    return false;
}

bool ql_use_mqtt_close()
{
    for (int i = 0; s_modules_not_using_mqtt_close[i] != NULL; i++)
    {
        if (strstr(s_module_name, s_modules_not_using_mqtt_close[i]) != NULL)
        {
            return false;
        }
    }
    return true;
}

bool ql_use_cpsms_for_psm()
{
    for (int i = 0; s_modules_using_cpsms_for_psm[i] != NULL; i++)
    {
        if (strstr(s_module_name, s_modules_using_cpsms_for_psm[i]) != NULL)
        {
            return true;
        }
    }
    return false;
}

bool ql_support_pon_trig()
{
    for (int i = 0; s_modules_support_pon_trig[i] != NULL; i++)
    {
        if (strstr(s_module_name, s_modules_support_pon_trig[i]) != NULL)
        {
            return true;
        }
    }
    return false;
}

bool ql_need_ctrl_dtr()
{
    for (int i = 0; s_modules_need_ctrl_dtr[i] != NULL; i++)
    {
        if (strstr(s_module_name, s_modules_need_ctrl_dtr[i]) != NULL)
        {
            return true;
        }
    }
    return false; 
}

bool ql_need_ctrl_ims()
{
    for (int i = 0; s_modules_need_ctrl_ims[i] != NULL; i++)
    {
        if (strstr(s_module_name, s_modules_need_ctrl_ims[i]) != NULL)
        {
            return true;
        }
    }
    return false; 
}

bool ql_urccfg_is_main()
{
    for (int i = 0; s_modules_urc_cfg_is_main[i] != NULL; i++)
    {
        if (strstr(s_module_name, s_modules_urc_cfg_is_main[i]) != NULL)
        {
            return true;
        }
    }
    return false;  
}