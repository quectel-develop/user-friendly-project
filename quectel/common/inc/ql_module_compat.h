#ifndef __QL_MODULE_COMPAT_H__
#define __MODULE_INFO_H__
#include <stdbool.h>

void ql_set_module_model(const char* model);

bool ql_support_net_scanmode();

bool ql_support_net_iotopmode();

bool ql_use_http_stop();

bool ql_support_ftp_nlist();

bool ql_support_ftp_data_addr_type_2();

bool ql_use_mqtt_pubex();

bool ql_use_mqtt_close();

bool ql_use_cpsms_for_psm();

bool ql_support_pon_trig();

bool ql_need_ctrl_dtr();

bool ql_need_ctrl_ims();

bool ql_urccfg_is_main();

#endif /* __QL_MODULE_COMPAT_H__ */
