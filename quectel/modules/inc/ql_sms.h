#include "QuectelConfig.h"

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SMS__
/*
 * Copyright (c) 2024, FAE
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author           Notes
 * 2026-4-7       Danijel       first version
 */
#ifndef __QL_SMS_H__
#define __QL_SMS_H__

#include <stdbool.h>
#include "at.h"

/**
 * @brief SMS mode selection
 *
 * Defines SMS working mode:
 * - PDU: raw binary format
 * - TEXT: human-readable format
 */
typedef enum
{
    QL_SMS_MODE_PDU  = 0,   /*!< PDU mode */
    QL_SMS_MODE_TEXT = 1    /*!< TEXT mode */
} ql_sms_mode_e;

/**
 * @brief SMS character set selection
 *
 * Defines the character set used in TEXT mode:
 * - GSM: GSM 7-bit default alphabet
 * - IRA: ISO 8859-1 (Latin) character set
 * - UCS2: UCS2 character set
 */
typedef enum
{
    QL_SMS_CHARSET_GSM,
    QL_SMS_CHARSET_IRA,
    QL_SMS_CHARSET_UCS2
} ql_sms_charset_e;


/**
 * @brief SMS storage location
 *
 * Defines where SMS messages are stored
 */
typedef enum
{
    QL_SMS_STORAGE_SM = 0, /*!< SIM card storage */
    QL_SMS_STORAGE_ME     /*!< Module internal memory */
} ql_sms_storage_e;

/**
 * @brief SMS list filter mode
 *
 * Used when listing messages from storage
 */
typedef enum
{
    QL_SMS_LIST_ALL = 0,        /*!< List all messages */
    QL_SMS_LIST_REC_UNREAD,    /*!< Received unread messages */
    QL_SMS_LIST_REC_READ,      /*!< Received read messages */
    QL_SMS_LIST_STO_SENT,      /*!< Stored sent messages */
    QL_SMS_LIST_STO_UNSENT     /*!< Stored unsent messages */
} ql_sms_list_mode_e;



/**
 * Configure SMS message format
 *
 * This function sets the SMS message format of the module by using the
 * AT+CMGF command. The SMS mode can be configured as PDU mode or TEXT mode.
 *
 * @param client AT client object used to communicate with the module
 * @param mode SMS format mode
 *        - QL_SMS_MODE_PDU  : PDU mode
 *        - QL_SMS_MODE_TEXT : TEXT mode
 *
 * @return
 *         0 : success
 *        -1 : invalid parameter or execution failed
 *        >0 : CME error code returned by the module
 */
int ql_sms_set_format(at_client_t client, ql_sms_mode_e mode);


/**
 * Set TE character set for SMS
 *
 * This function configures the character set used in TEXT mode
 * via AT+CSCS command.
 *
 * @param client AT client object used to communicate with the module
 * @param charset character set string (e.g. "GSM", "IRA", "UCS2")
 *
 * @return
 *         0 : success
 *        -1 : invalid parameter or execution failed
 *        >0 : CME error code returned by the module
 */
int ql_sms_set_char(at_client_t client, ql_sms_charset_e charset);


/**
 * Set SMS text mode parameters
 *
 * This function configures SMS parameters in TEXT mode
 * using AT+CSMP command.
 *
 * @param client AT client object
 * @param fo first octet
 * @param vp validity period
 * @param pid protocol identifier
 * @param dcs data coding scheme
 *
 * @return
 *         0 : success
 *        -1 : error
 */
int ql_sms_set_textmd(at_client_t client, int fo, int vp, int pid, int dcs);

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
int ql_sms_set_storage(at_client_t client, ql_sms_storage_e storage);

/**
 * Get SMS storage information
 *
 * This function queries the SMS storage status using AT+CPMS?.
 *
 * @param client AT client object
 * @param used pointer to store used messages count
 * @param total pointer to store total capacity
 *
 * @return
 *         0 : success
 *        -1 : error
 */
int ql_sms_get_storage(at_client_t client, int *used, int *total);

/**
 * Send SMS message
 *
 * This function sends an SMS message in TEXT mode by using the AT+CMGS
 * command. The destination phone number and message content must be valid.
 *
 * @param client AT client object used to communicate with the module
 * @param phone_num destination phone number
 * @param text SMS text content
 *
 * @return
 *         0 : success
 *        -1 : invalid parameter or execution failed
 *        >0 : CME error code returned by the module
 */
int ql_sms_send(at_client_t client, const char *phone_num, const char *text);

/**
 * Read SMS message
 *
 * This function reads an SMS message from storage using AT+CMGR.
 *
 * @param client AT client object
 * @param index SMS index to read
 * @param msg_buf buffer to store SMS content
 * @param buf_len buffer size
 *
 * @return
 *         0 : success
 *        -1 : error
 */
int ql_sms_read(at_client_t client, int index, char *msg_buf, int buf_len);


/**
 * Delete SMS message
 *
 * This function deletes SMS messages using AT+CMGD command.
 *
 * @param client AT client object
 * @param index SMS index
 * @param delflag delete mode
 *     - 0: delete the specified message
 *     - 1: delete all read messages
 *     - 2: delete all read and sent messages
 *     - 4: delete all messages  
 *
 * @return
 *         0 : success
 *        -1 : error
 */
int ql_sms_del(at_client_t client, int index, int delflag);

/**
 * @brief List SMS messages
 *
 * @param client AT client handle
 * @param mode Filter mode (all, unread, etc.)
 *
 * @return
 *         0 : success
 *        -1 : failed
 */
int ql_sms_list(at_client_t client, ql_sms_list_mode_e mode);



/**
 * @brief SMS URC handler
 *
 * Handles unsolicited result codes such as +CMTI
 *
 * @param line Received AT line
 */
void ql_sms_urc_handler(const char *line);


/**
 * @brief Get pending SMS index
 *
 * Used after +CMTI notification
 *
 * @return index >= 0 if available, otherwise -1
 */
int ql_sms_get_pending_index(void);


/**
 * @brief Clear pending SMS index
 */
void ql_sms_clear_pending(void);




#endif /* __QL_SMS_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SMS__ */