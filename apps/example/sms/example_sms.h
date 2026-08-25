#ifndef __EXAMPLE_SMS_H__
#define __EXAMPLE_SMS_H__

#include "QuectelConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run SMS example workflow
 *
 * This function demonstrates a basic SMS workflow:
 * - set SMS format
 * - set character set
 * - set text mode parameters
 * - set storage
 * - send SMS
 *
 * @return
 *         0 : success
 *        -1 : failed
 */
int example_sms_start(void);

#ifdef __cplusplus
}
#endif

#endif /* __EXAMPLE_SMS_H__ */