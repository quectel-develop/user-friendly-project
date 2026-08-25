#include "ql_sms.h"
#include "qosa_log.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Global variable to store pending SMS index received via URC (+CMTI)
 */
static volatile int g_sms_pending_index = -1;

/**
 * @brief Get pending SMS index from URC
 *
 * @return
 *         >=0 : valid SMS index
 *         -1  : no pending SMS
 */
int ql_sms_get_pending_index(void)
{
    return g_sms_pending_index;
}

/**
 * @brief Clear pending SMS index
 *
 * This function resets the pending SMS index after it has been processed.
 */
void ql_sms_clear_pending(void)
{
    g_sms_pending_index = -1;
}

/**
 * @brief Handle SMS URC notifications
 *
 * This function parses incoming URC messages such as:
 *     +CMTI: "ME",7
 *
 * It extracts the SMS index and stores it for later processing.
 *
 * @param line URC string from AT interface
 */
void ql_sms_urc_handler(const char *line)
{
    int index;

    if ((line != NULL) && strstr(line, "+CMTI:"))
    {
        if (sscanf(line, "+CMTI: \"%*[^\"]\",%d", &index) == 1)
        {
            g_sms_pending_index = index;
            LOG_I(">>> SMS queued index=%d", index);
        }
    }
}


/**
 * Set SMS format
 *
 * This function sets SMS mode using AT+CMGF:
 * 0 : PDU mode
 * 1 : TEXT mode
 *
 * @param client AT client object
 * @param mode SMS mode
 *
 * @return
 *         0 : success
 *        -1 : error
 *        >0 : CME error code
 */
int ql_sms_set_format(at_client_t client, ql_sms_mode_e mode)
{
    at_response_t resp = NULL;
    int err = -1;

    if (client == NULL)
    {
        LOG_E("ql_sms_set_format: client is NULL");
        return -1;
    }

    if ((mode != QL_SMS_MODE_PDU) && (mode != QL_SMS_MODE_TEXT))
    {
        LOG_E("ql_sms_set_format: invalid mode=%d", mode);
        return -1;
    }

    resp = at_create_resp_new(128, 0, 3000, NULL);
    if (resp == NULL)
    {
        LOG_E("ql_sms_set_format: no memory for response");
        return -1;
    }

    if (at_obj_exec_cmd(client, resp, "AT+CMGF=%d", mode) < 0)
    {
        for (int i = 0; i < resp->line_counts; i++)
        {
            const char *line = at_resp_get_line(resp, i + 1);
            if ((line != NULL) && (strstr(line, "+CME ERROR:") != NULL))
            {
                sscanf(line, "+CME ERROR: %d", &err);
                break;
            }
        }

        LOG_E("AT+CMGF failed, err=%d", err);
        at_delete_resp(resp);
        return err;
    }

    at_delete_resp(resp);
    return 0;
}

/**
 * Set SMS character set
 *
 * This function configures TE character set using AT+CSCS.
 *
 * @param client AT client object
 * @param charset character set enum (GSM / IRA / UCS2)
 *
 * @return
 *         0 : success
 *        -1 : error
 *        >0 : CME error code
 */
int ql_sms_set_char(at_client_t client, ql_sms_charset_e charset)
{
    at_response_t resp = NULL;
    int err = -1;
    int i;
    const char *charset_str = NULL;

    if (client == NULL)
    {
        LOG_E("ql_sms_set_char: client is NULL");
        return -1;
    }

    /* Map enum to string */
    switch (charset)
    {
        case QL_SMS_CHARSET_GSM:
            charset_str = "GSM";
            break;

        case QL_SMS_CHARSET_IRA:
            charset_str = "IRA";
            break;

        case QL_SMS_CHARSET_UCS2:
            charset_str = "UCS2";
            break;

        default:
            LOG_E("ql_sms_set_char: invalid charset");
            return -1;
    }

    resp = at_create_resp_new(128, 0, 3000, NULL);
    if (resp == NULL)
    {
        LOG_E("ql_sms_set_char: no memory for response");
        return -1;
    }

    /* Send AT command */
    if (at_obj_exec_cmd(client, resp, "AT+CSCS=\"%s\"", charset_str) < 0)
    {
        for (i = 0; i < resp->line_counts; i++)
        {
            const char *line = at_resp_get_line(resp, i + 1);
            if ((line != NULL) && strstr(line, "+CME ERROR:"))
            {
                sscanf(line, "+CME ERROR: %d", &err);
                break;
            }
        }

        LOG_E("AT+CSCS failed, err=%d", err);
        at_delete_resp(resp);
        return err;
    }

    at_delete_resp(resp);
    return 0;
}

/**
 * Set SMS text mode parameters
 *
 * This function configures SMS parameters using AT+CSMP:
 * fo  : first octet
 * vp  : validity period
 * pid : protocol identifier
 * dcs : data coding scheme
 *
 * @param client AT client object
 * @param fo first octet
 * @param vp validity period
 * @param pid protocol identifier
 * @param dcs data coding scheme
 *
 * @return
 *         0 : success
 *        -1 : failed
 */
int ql_sms_set_textmd(at_client_t client, int fo, int vp, int pid, int dcs)
{
    at_response_t resp = NULL;
    int err = -1;
    int i;

    if (client == NULL)
    {
        LOG_E("ql_sms_set_textmd: client is NULL");
        return -1;
    }

    resp = at_create_resp_new(128, 0, 3000, NULL);
    if (resp == NULL)
    {
        LOG_E("ql_sms_set_textmd: no memory");
        return -1;
    }

    if (at_obj_exec_cmd(client, resp, "AT+CSMP=%d,%d,%d,%d", fo, vp, pid, dcs) < 0)
    {
        for (i = 0; i < resp->line_counts; i++)
        {
            const char *line = at_resp_get_line(resp, i + 1);
            if ((line != NULL) && strstr(line, "+CME ERROR:"))
            {
                sscanf(line, "+CME ERROR: %d", &err);
                break;
            }
        }

        LOG_E("AT+CSMP failed, err=%d", err);
        at_delete_resp(resp);
        return err;
    }

    at_delete_resp(resp);
    return 0;
}


/**
 * Get SMS storage information
 *
 * This function queries the SMS storage status by using AT+CPMS?.
 * It returns the number of used messages and the total storage capacity.
 *
 * @param client AT client object used to communicate with the module
 * @param used pointer to store used messages count
 * @param total pointer to store total storage capacity
 *
 * @return
 *         0 : success
 *        -1 : invalid parameter or execution failed
 */
int ql_sms_get_storage(at_client_t client, int *used, int *total)
{
    at_response_t resp = NULL;
    int ret;
    int i;

    if ((client == NULL) || (used == NULL) || (total == NULL))
    {
        LOG_E("ql_sms_get_storage: invalid parameter");
        return -1;
    }

    *used = 0;
    *total = 0;

    resp = at_create_resp_new(128, 0, 3000, NULL);
    if (resp == NULL)
    {
        LOG_E("ql_sms_get_storage: no memory");
        return -1;
    }

    ret = at_obj_exec_cmd(client, resp, "AT+CPMS?");
    if (ret < 0)
    {
        LOG_E("ql_sms_get_storage: AT+CPMS? failed");
        at_delete_resp(resp);
        return -1;
    }

    /*
     * Example response:
     * +CPMS: "ME",7,100,"ME",7,100,"ME",7,100
     */
    for (i = 0; i < resp->line_counts; i++)
    {
        const char *line = at_resp_get_line(resp, i + 1);

        if ((line != NULL) && (strstr(line, "+CPMS:") != NULL))
        {
            if (sscanf(line, "+CPMS: \"%*[^\"]\",%d,%d", used, total) == 2)
            {
                at_delete_resp(resp);
                return 0;
            }
        }
    }

    LOG_E("ql_sms_get_storage: failed to parse CPMS response");
    at_delete_resp(resp);
    return -1;
}

/**
 * Set SMS storage
 *
 * This function configures the preferred SMS storage by using
 * the AT+CPMS command.
 *
 * @param client AT client object used to communicate with the module
 * @param storage SMS storage type
 *
 * @return
 *         0 : success
 *        -1 : invalid parameter or execution failed
 *        >0 : CME error code returned by the module
 */
int ql_sms_set_storage(at_client_t client, ql_sms_storage_e storage)
{
    at_response_t resp = NULL;
    const char *storage_str = NULL;
    int err = -1;
    int i;

    if (client == NULL)
    {
        LOG_E("ql_sms_set_storage: client is NULL");
        return -1;
    }

    switch (storage)
    {
        case QL_SMS_STORAGE_SM:
            storage_str = "SM";
            break;

        case QL_SMS_STORAGE_ME:
            storage_str = "ME";
            break;

        default:
            LOG_E("ql_sms_set_storage: invalid storage");
            return -1;
    }

    resp = at_create_resp_new(128, 0, 3000, NULL);
    if (resp == NULL)
    {
        LOG_E("ql_sms_set_storage: no memory");
        return -1;
    }

    if (at_obj_exec_cmd(client, resp,
                        "AT+CPMS=\"%s\",\"%s\",\"%s\"",
                        storage_str, storage_str, storage_str) < 0)
    {
        for (i = 0; i < resp->line_counts; i++)
        {
            const char *line = at_resp_get_line(resp, i + 1);
            if ((line != NULL) && (strstr(line, "+CME ERROR:") != NULL))
            {
                sscanf(line, "+CME ERROR: %d", &err);
                break;
            }
        }

        LOG_E("AT+CPMS failed, err=%d", err);
        at_delete_resp(resp);
        return err;
    }

    at_delete_resp(resp);
    return 0;
}


/**
 * Read SMS message
 *
 * This function reads an SMS message from storage using AT+CMGR.
 *
 * @param client AT client object
 * @param index SMS index
 * @param msg_buf buffer to store SMS text
 * @param buf_len buffer size
 *
 * @return
 *         0 : success
 *        -1 : error
 */
int ql_sms_read(at_client_t client, int index, char *msg_buf, int buf_len)
{
    at_response_t resp = NULL;
    int ret;
    int i;

    if ((client == NULL) || (msg_buf == NULL) || (buf_len <= 0))
    {
        LOG_E("ql_sms_read: invalid parameter");
        return -1;
    }

    resp = at_create_resp_new(256, 0, 3000, NULL);
    if (resp == NULL)
    {
        LOG_E("ql_sms_read: no memory");
        return -1;
    }

    ret = at_obj_exec_cmd(client, resp, "AT+CMGR=%d", index);
    if (ret < 0)
    {
        LOG_E("ql_sms_read: AT+CMGR failed");
        at_delete_resp(resp);
        return -1;
    }

    /*
     * Example response:
     * +CMGR: "REC READ","+3816...",,"24/04/15,12:30:00+08"
     * Hello message text
     */
    for (i = 0; i < resp->line_counts; i++)
    {
        const char *line = at_resp_get_line(resp, i + 1);

        /* Skip header line (+CMGR) */
        if ((line != NULL) && (strstr(line, "+CMGR:") == NULL))
        {
            strncpy(msg_buf, line, buf_len - 1);
            msg_buf[buf_len - 1] = '\0';

            at_delete_resp(resp);
            return 0;
        }
    }

    LOG_E("ql_sms_read: failed to parse SMS content");
    at_delete_resp(resp);
    return -1;
}

/**
 * Send SMS message
 *
 * This function sends an SMS message in TEXT mode by using the
 * AT+CMGS command. The destination phone number and message
 * content must be valid.
 *
 * @param client AT client object used to communicate with the module
 * @param phone_num destination phone number
 * @param text SMS text content
 *
 * @return
 *         0 : success
 *        -1 : invalid parameter or execution failed
 */
int ql_sms_send(at_client_t client, const char *phone_num, const char *text)
{
    at_response_t resp = NULL;
    size_t ret;

    if (client == NULL)
    {
        LOG_E("ql_sms_send: client is NULL");
        return -1;
    }

    if ((phone_num == NULL) || (text == NULL))
    {
        LOG_E("ql_sms_send: invalid parameter");
        return -1;
    }

    if ((strlen(phone_num) == 0U) || (strlen(text) == 0U))
    {
        LOG_E("ql_sms_send: empty phone number or text");
        return -1;
    }

    resp = at_create_resp_new(256, 0, 10000, NULL);
    if (resp == NULL)
    {
        LOG_E("ql_sms_send: no memory for response");
        return -1;
    }


    /* Send AT+CMGS="<phone_num>" */
    ret = at_client_obj_send(client, "AT+CMGS=\"", strlen("AT+CMGS=\""), false);
    if (ret != strlen("AT+CMGS=\""))
    {
        LOG_E("ql_sms_send: failed to send CMGS prefix");
        at_delete_resp(resp);
        return -1;
    }

    ret = at_client_obj_send(client, phone_num, strlen(phone_num), false);
    if (ret != strlen(phone_num))
    {
        LOG_E("ql_sms_send: failed to send phone number");
        at_delete_resp(resp);
        return -1;
    }

    ret = at_client_obj_send(client, "\"\r", strlen("\"\r"), false);
    if (ret != strlen("\"\r"))
    {
        LOG_E("ql_sms_send: failed to send CMGS suffix");
        at_delete_resp(resp);
        return -1;
    }

    /* Wait module to enter SMS input state and show '>' prompt */
    qosa_task_sleep_ms(500);

    /* Send SMS text */
    ret = at_client_obj_send(client, text, strlen(text), false);
    if (ret != strlen(text))
    {
        LOG_E("ql_sms_send: failed to send SMS text");
        at_delete_resp(resp);
        return -1;
    }

    /* Send Ctrl+Z to submit SMS */
    ret = at_client_obj_send(client, "\x1A", 1, false);
    if (ret != 1)
    {
        LOG_E("ql_sms_send: failed to send Ctrl+Z");
        at_delete_resp(resp);
        return -1;
    }

    /* Wait for final result: +CMGS / OK */
    qosa_task_sleep_ms(3000);

    at_delete_resp(resp);
    return 0;
}


/**
 * Delete SMS message
 *
 * This function deletes SMS messages using AT+CMGD.
 *
 * @param client AT client object
 * @param index SMS index
 * @param delflag delete mode
 *       0: delete the specified message
 *     1: delete all read messages
 *    2: delete all read and sent messages
 *  4: delete all messages
 *
 * @return
 *         0 : success
 *        -1 : error
 */
int ql_sms_del(at_client_t client, int index, int delflag)
{
    at_response_t resp = NULL;
    int ret;
    int err = -1;
    int i;

    if (client == NULL)
    {
        LOG_E("ql_sms_del: client is NULL");
        return -1;
    }

    resp = at_create_resp_new(128, 0, 3000, NULL);
    if (resp == NULL)
    {
        LOG_E("ql_sms_del: no memory");
        return -1;
    }

    /* delete ALL messages */
    if (delflag == 4)
    {
        ret = at_obj_exec_cmd(client, resp, "AT+CMGD=1,4");
    }
    else
    {
        ret = at_obj_exec_cmd(client, resp, "AT+CMGD=%d,%d", index, delflag);
    }

    if (ret < 0)
    {
        for (i = 0; i < resp->line_counts; i++)
        {
            const char *line = at_resp_get_line(resp, i + 1);
            if ((line != NULL) && strstr(line, "+CME ERROR:"))
            {
                sscanf(line, "+CME ERROR: %d", &err);
                break;
            }
        }

        LOG_E("AT+CMGD failed, err=%d", err);
        at_delete_resp(resp);
        return err;
    }

    at_delete_resp(resp);
    return 0;
}

/**
 * List SMS messages
 *
 * This function lists SMS messages using AT+CMGL based on mode.
 *
 * @param client AT client object
 * @param mode listing mode:
 *        QL_SMS_LIST_ALL
 *        QL_SMS_LIST_REC_UNREAD
 *        QL_SMS_LIST_REC_READ
 *        QL_SMS_LIST_STO_SENT
 *        QL_SMS_LIST_STO_UNSENT
 *
 * @return
 *         0 : success
 *        -1 : error
 */
int ql_sms_list(at_client_t client, ql_sms_list_mode_e mode)
{
    int ret = -1;
    char cmd[32] = {0};

    switch (mode)
    {
        case QL_SMS_LIST_ALL:
            strcpy(cmd, "AT+CMGL=\"ALL\"");
            break;
        case QL_SMS_LIST_REC_UNREAD:
            strcpy(cmd, "AT+CMGL=\"REC UNREAD\"");
            break;
        case QL_SMS_LIST_REC_READ:
            strcpy(cmd, "AT+CMGL=\"REC READ\"");
            break;
        case QL_SMS_LIST_STO_SENT:
            strcpy(cmd, "AT+CMGL=\"STO SENT\"");
            break;
        case QL_SMS_LIST_STO_UNSENT:
            strcpy(cmd, "AT+CMGL=\"STO UNSENT\"");
            break;
        default:
            return -1;
    }

    ret = at_obj_exec_cmd(client, NULL, "%s", cmd);

    if (ret != 0)
    {
        LOG_E("ql_sms_list failed");
        return -1;
    }

    return 0;
}
