#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SMS__

#ifndef __CLI_SMS_H__
#define __CLI_SMS_H__
#include "ql_sms.h"
#include "qosa_def.h"

/**
 * @brief Display SMS CLI help information
 *
 * This function prints all available SMS CLI commands and their usage.
 * It is typically called when the user enters an invalid command or requests help.
 */
void cli_sms_get_help(void);

/**
 * @brief Main SMS CLI entry function
 *
 * This function is the entry point for all SMS-related CLI commands.
 * It parses user input arguments and dispatches them to the corresponding
 * SMS operation (format, send, read, delete, list, etc.).
 *
 * Example CLI usage:
 * @code
 * sms format 1
 * sms send +381600000000 Hello
 * sms read 1
 * sms delete 1
 * sms list unread
 * @endcode
 *
 * @param argc argument count
 * @param argv argument vector (array of strings)
 *
 * @return
 *         0 : success
 *        -1 : failed or invalid command
 */
int cli_sms_test(s32_t argc, char *argv[]);


/****************** SMS API Wrappers for CLI ******************/    

#endif /* __CLI_SMS_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SMS__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */