#include "qosa_def.h"
#include "qosa_log.h"
#include "module_info.h"

static Module_Type s_module = MOD_TYPE_DEFAULT;

static const module_type_map_t s_module_type_map[] = {
    {"BG95",    MOD_TYPE_BG95},
    {"BG96",    MOD_TYPE_BG96},
    {"EC800M",   MOD_TYPE_EC800M},
    {"EC25", MOD_TYPE_EC25}
    // 可以继续扩展...
};

bool set_module_type(const char* type)
{
    int i = 0;
    if (NULL == type)
        return QOSA_FALSE;
    for (i = 0; i < sizeof(s_module_type_map) / sizeof(module_type_map_t); i++)
    {
        if (strstr(type, s_module_type_map[i].name) != NULL)
        {
            s_module = s_module_type_map[i].type;
            LOG_D("module = %s type = %d", s_module_type_map[i].name, s_module);
            return QOSA_TRUE;
        }
    }
    return QOSA_FALSE;
}

Module_Type get_module_type()
{
    return s_module;
}

const char* get_module_type_name()
{
    for (size_t i = 0; i < sizeof(s_module_type_map) / sizeof(s_module_type_map[0]); i++)
    {
        if (s_module_type_map[i].type == s_module)
        {
            return s_module_type_map[i].name;
        }
    }
    return "UNKNOWN";
}