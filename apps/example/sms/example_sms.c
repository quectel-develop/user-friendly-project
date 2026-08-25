#include "example_sms.h"

#include "qosa_log.h"
#include "at.h"
#include "ql_sms.h"

/**
 * @brief Run SMS example workflow
 *
 * This function demonstrates a basic SMS send flow using
 * the SMS APIs.
 *
 * @return
 *         0 : success
 *        -1 : failed
 */
int example_sms_start(void)
{
    at_client_t client;
    int ret;

    client = at_client_get_first();
    if (client == NULL)
    {
        LOG_E("example_sms_start: AT client not ready");
        return -1;
    }

    /* Step 1: set SMS format to TEXT mode */
    ret = ql_sms_set_format(client, QL_SMS_MODE_TEXT);
    if (ret != 0)
    {
        LOG_E("example_sms_start: ql_sms_set_format failed, ret=%d", ret);
        return -1;
    }

    /* Step 2: set character set */
    ret = ql_sms_set_char(client, QL_SMS_CHARSET_GSM);
    if (ret != 0)
    {
        LOG_E("example_sms_start: ql_sms_set_char failed, ret=%d", ret);
        return -1;
    }

    /* Step 3: set text mode parameters */
    ret = ql_sms_set_textmd(client, 17, 167, 0, 0);
    if (ret != 0)
    {
        LOG_E("example_sms_start: ql_sms_set_textmd failed, ret=%d", ret);
        return -1;
    }

    /* Step 4: set storage */
    ret = ql_sms_set_storage(client, QL_SMS_STORAGE_ME);
    if (ret != 0)
    {
        LOG_E("example_sms_start: ql_sms_set_storage failed, ret=%d", ret);
        return -1;
    }

    /* Step 5: send SMS */
    ret = ql_sms_send(client, "+381643352257", "Hello from Quectel STM32 example");
    if (ret != 0)
    {
        LOG_E("example_sms_start: ql_sms_send failed, ret=%d", ret);
        return -1;
    }

    LOG_I("example_sms_start: SMS example completed successfully");
    return 0;
}