#ifndef __MODULE_INFO_H__
#define __MODULE_INFO_H__
#include <stdbool.h>

typedef enum
{
    MOD_TYPE_DEFAULT,   // Default
    MOD_TYPE_BG95,      // BG95
    MOD_TYPE_BG96,      // BG96
    MOD_TYPE_EC800M,    // EC800M
    MOD_TYPE_EC25,      // EC25
    MOD_TYPE_MAX,
} Module_Type;

typedef struct
{
    const char* name; // Module name
    Module_Type type; // Module type
} module_type_map_t;


bool set_module_type(const char* type);
Module_Type get_module_type();
const char* get_module_type_name();

#endif /* __MODULE_INFO_H__ */
