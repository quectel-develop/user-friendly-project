#include "QuectelConfig.h"

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SMS__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli_sms.h"
#include "qosa_log.h"

/*===========================================================================
 * Help
 *===========================================================================*/
void cli_sms_get_help(void)
{
    LOG_I("Usage:");
    LOG_I("  sms format <0|1>");
    LOG_I("    0 : PDU mode");
    LOG_I("    1 : TEXT mode");

    LOG_I("  sms char <GSM|IRA|UCS2>");
    LOG_I("    Set SMS character set");
    LOG_I("    Example: sms char GSM");

    LOG_I("  sms textmd <fo> <vp> <pid> <dcs>");
    LOG_I("    Set SMS text mode parameters");
    LOG_I("    Example: sms textmd 17 167 0 0");

    LOG_I("  sms send <number> <text>");
    LOG_I("    Send SMS message");
    LOG_I("    Example: sms send +381643352257 Hello from STM32");

    LOG_I("  sms read <index>");
    LOG_I("    Read SMS from storage");
    LOG_I("    Example: sms read 1");

    LOG_I("  sms delete <index|all>");
    LOG_I("    Delete SMS message");
    LOG_I("    Example: sms delete 1");
    LOG_I("    Example: sms delete all");

    LOG_I("  sms storage");
    LOG_I("    Show SMS storage usage");

    LOG_I("  sms storage set <SM|ME>");
    LOG_I("    Set SMS storage location");
    LOG_I("    Example: sms storage set ME");

    LOG_I("sms list [all|unread|read|sent|unsent]");
    LOG_I("  List SMS messages");

    LOG_I("sms auto send <format> <charset> <fo> <vp> <pid> <dcs> <number> <text>");
    LOG_I("  Example: sms auto send 1 gsm 17 167 0 0 (number) Hello world");

    LOG_I("sms auto read <format> <charset> <fo> <vp> <pid> <dcs> <storage> <index|pending>");
    LOG_I("  Example: sms auto read 1 gsm 17 167 0 0 ME pending");
    LOG_I("  Example: sms auto read 1 gsm 17 167 0 0 ME 7");

    LOG_I("sms auto config <format> <charset> <fo> <vp> <pid> <dcs> <storage>");
    LOG_I("  Example: sms auto config 1 gsm 17 167 0 0 ME");
}

/*===========================================================================
 * Subcommands
 *===========================================================================*/
static int cli_sms_format(int argc, char **argv)
{
    int ret;

    if ((argc < 1) || (argv == NULL) || (argv[0] == NULL))
    {
        LOG_E("Usage: sms format <0|1>");
        return -1;
    }

    if (strcmp(argv[0], "0") == 0 || strcmp(argv[0], "PDU") == 0 || strcmp(argv[0], "pdu") == 0)
    {
        ret = ql_sms_set_format(at_client_get_first(), QL_SMS_MODE_PDU);
        if (ret != 0)
        {
            LOG_E("Failed to set SMS format to PDU, ret=%d", ret);
            return -1;
        }

        LOG_I("SMS format set to PDU mode");
        return 0;
    }
    else if (strcmp(argv[0], "1" ) == 0 || strcmp(argv[0], "TEXT") == 0 || strcmp(argv[0], "text") == 0)
    {
        ret = ql_sms_set_format(at_client_get_first(), QL_SMS_MODE_TEXT);
        if (ret != 0)
        {
            LOG_E("Failed to set SMS format to TEXT, ret=%d", ret);
            return -1;
        }

        LOG_I("SMS format set to TEXT mode");
        return 0;
    }

    LOG_E("Invalid sms format parameter: %s", argv[0]);
    cli_sms_get_help();
    return -1;
}

static int cli_sms_set_char(int argc, char **argv)
{
    ql_sms_charset_e charset;
    int ret;

    if ((argc < 1) || (argv == NULL) || (argv[0] == NULL))
    {
        LOG_E("Usage: sms char <GSM|IRA|UCS2>");
        return -1;
    }

    if (strcmp(argv[0], "GSM") == 0 || strcmp(argv[0], "gsm") == 0)
    {
        charset = QL_SMS_CHARSET_GSM;
    }
    else if (strcmp(argv[0], "IRA") == 0 || strcmp(argv[0], "ira") == 0)
    {
        charset = QL_SMS_CHARSET_IRA;
    }
    else if (strcmp(argv[0], "UCS2") == 0 || strcmp(argv[0], "ucs2") == 0)
    {
        charset = QL_SMS_CHARSET_UCS2;
    }
    else
    {
        LOG_E("Invalid charset: %s", argv[0]);
        LOG_E("Use GSM, IRA or UCS2");
        return -1;
    }

    ret = ql_sms_set_char(at_client_get_first(), charset);
    if (ret != 0)
    {
        LOG_E("Failed to set SMS character set, ret=%d", ret);
        return -1;
    }

    LOG_I("SMS character set set to %s", argv[0]);
    return 0;
}

static int cli_sms_set_textmd(int argc, char **argv)
{
    int fo, vp, pid, dcs;
    int ret;

    if ((argc < 4) ||
        (argv == NULL) ||
        (argv[0] == NULL) ||
        (argv[1] == NULL) ||
        (argv[2] == NULL) ||
        (argv[3] == NULL))
    {
        LOG_E("Usage: sms textmd <fo> <vp> <pid> <dcs>");
        LOG_E("Example: sms textmd 17 167 0 0");
        return -1;
    }

    fo  = atoi(argv[0]);
    vp  = atoi(argv[1]);
    pid = atoi(argv[2]);
    dcs = atoi(argv[3]);

    ret = ql_sms_set_textmd(at_client_get_first(), fo, vp, pid, dcs);
    if (ret != 0)
    {
        LOG_E("Failed to set SMS text mode parameters, ret=%d", ret);
        return -1;
    }

    LOG_I("SMS text mode set: %d %d %d %d", fo, vp, pid, dcs);
    return 0;
}

static int cli_sms_send(int argc, char **argv)
{
    const char *phone_num;
    char msg[160] = {0};
    size_t used_len = 0;
    int ret;
    int i;

    if ((argc < 2) || (argv == NULL) || (argv[0] == NULL) || (argv[1] == NULL))
    {
        LOG_E("Usage: sms send <number> <text>");
        return -1;
    }

    phone_num = argv[0];

    for (i = 1; i < argc; i++)
    {
        size_t part_len  = strlen(argv[i]);
        size_t extra_len = (i < (argc - 1)) ? 1 : 0;

        if ((used_len + part_len + extra_len) >= sizeof(msg))
        {
            LOG_E("SMS message too long");
            return -1;
        }

        memcpy(&msg[used_len], argv[i], part_len);
        used_len += part_len;

        if (i < (argc - 1))
        {
            msg[used_len++] = ' ';
        }
    }

    msg[used_len] = '\0';

    ret = ql_sms_send(at_client_get_first(), phone_num, msg);
    if (ret != 0)
    {
        LOG_E("Failed to send SMS, ret=%d", ret);
        return -1;
    }

    LOG_I("SMS sent successfully to %s", phone_num);
    return 0;
}

static int cli_sms_read(int argc, char **argv)
{
    int index;
    int ret;
    char msg[256] = {0};

    if ((argc < 1) || (argv == NULL) || (argv[0] == NULL))
    {
        LOG_E("Usage: sms read <index>");
        return -1;
    }

    index = atoi(argv[0]);
    if (index < 0)
    {
        LOG_E("Invalid SMS index");
        return -1;
    }

    ret = ql_sms_read(at_client_get_first(), index, msg, sizeof(msg));
    if (ret != 0)
    {
        LOG_E("Failed to read SMS, ret=%d", ret);
        return -1;
    }

    LOG_I("SMS[%d]: %s", index, msg);
    return 0;
}

static int cli_sms_delete(int argc, char **argv)
{
    int ret;
    int index;

    if ((argc < 1) || (argv == NULL) || (argv[0] == NULL))
    {
        LOG_E("Usage: sms delete <index|all>");
        return -1;
    }

    if (strcmp(argv[0], "all") == 0)
    {
        ret = ql_sms_del(at_client_get_first(), 0, 4);
        if (ret != 0)
        {
            LOG_E("Failed to delete all SMS, ret=%d", ret);
            return -1;
        }

        LOG_I("All SMS deleted successfully");
        return 0;
    }

    index = atoi(argv[0]);
    if (index < 0)
    {
        LOG_E("Invalid SMS index");
        return -1;
    }

    ret = ql_sms_del(at_client_get_first(), index, 0);
    if (ret != 0)
    {
        LOG_E("Failed to delete SMS[%d], ret=%d", index, ret);
        return -1;
    }

    LOG_I("SMS[%d] deleted successfully", index);
    return 0;
}

static int cli_sms_storage(int argc, char **argv)
{
    int used = 0;
    int total = 0;
    int ret;

    if ((argc >= 2) && (argv != NULL) && (argv[0] != NULL) && (argv[1] != NULL))
    {
        ql_sms_storage_e storage;

        if (strcmp(argv[0], "set") != 0)
        {
            LOG_E("Usage: sms storage");
            LOG_E("Usage: sms storage set <SM|ME>");
            return -1;
        }

        if (strcmp(argv[1], "SM") == 0 || strcmp(argv[1], "sm") == 0)
        {
            storage = QL_SMS_STORAGE_SM;
        }
        else if (strcmp(argv[1], "ME") == 0 || strcmp(argv[1], "me") == 0)
        {
            storage = QL_SMS_STORAGE_ME;
        }
        else
        {
            LOG_E("Invalid storage. Use SM or ME");
            return -1;
        }

        ret = ql_sms_set_storage(at_client_get_first(), storage);
        if (ret != 0)
        {
            LOG_E("Failed to set SMS storage, ret=%d", ret);
            return -1;
        }

        LOG_I("SMS storage set successfully: %s", argv[1]);
        return 0;
    }

    ret = ql_sms_get_storage(at_client_get_first(), &used, &total);
    if (ret != 0)
    {
        LOG_E("Failed to get SMS storage, ret=%d", ret);
        return -1;
    }

    LOG_I("SMS storage: %d / %d", used, total);
    return 0;
}

int cli_sms_list(int argc, char **argv)
{
    ql_sms_list_mode_e mode = QL_SMS_LIST_ALL;

    if (argc >= 1)
    {
        if (strcmp(argv[0], "unread") == 0)
            mode = QL_SMS_LIST_REC_UNREAD;
        else if (strcmp(argv[0], "read") == 0)
            mode = QL_SMS_LIST_REC_READ;
        else if (strcmp(argv[0], "sent") == 0)
            mode = QL_SMS_LIST_STO_SENT;
        else if (strcmp(argv[0], "unsent") == 0)
            mode = QL_SMS_LIST_STO_UNSENT;
    }

    return ql_sms_list(at_client_get_first(), mode);
}



static int cli_sms_auto_send(int argc, char **argv)
{
    if (argc < 8)
    {
        LOG_E("Usage:");
        LOG_E("sms auto send <format> <charset> <fo> <vp> <pid> <dcs> <number> <text>");
        return -1;
    }

    at_client_t client = at_client_get_first();

    int format = atoi(argv[0]);
    char *charset_str = argv[1];
    int fo  = atoi(argv[2]);
    int vp  = atoi(argv[3]);
    int pid = atoi(argv[4]);
    int dcs = atoi(argv[5]);
    char *number = argv[6];

    /* merge message */
    char msg[160] = {0};
    for (int i = 7; i < argc; i++)
    {
        strcat(msg, argv[i]);
        if (i != argc - 1)
            strcat(msg, " ");
    }

    /* format */
    ql_sms_set_format(client, format);

    /* charset */
    if (strcasecmp(charset_str, "gsm") == 0)
        ql_sms_set_char(client, QL_SMS_CHARSET_GSM);
    else if (strcasecmp(charset_str, "ira") == 0)
        ql_sms_set_char(client, QL_SMS_CHARSET_IRA);
    else if (strcasecmp(charset_str, "ucs2") == 0)
        ql_sms_set_char(client, QL_SMS_CHARSET_UCS2);
    else
    {
        LOG_E("Invalid charset");
        return -1;
    }

    /* text mode params */
    ql_sms_set_textmd(client, fo, vp, pid, dcs);

    /* send */
    return ql_sms_send(client, number, msg);
}


static int cli_sms_auto_read(int argc, char **argv)
{
    at_client_t client;
    ql_sms_charset_e charset;
    ql_sms_storage_e storage;
    int format;
    int fo, vp, pid, dcs;
    int index;
    int ret;
    char msg[256] = {0};

    if ((argc < 8) || (argv == NULL))
    {
        LOG_E("Usage:");
        LOG_E("sms auto read <format> <charset> <fo> <vp> <pid> <dcs> <storage> <index|pending>");
        return -1;
    }

    client = at_client_get_first();
    if (client == NULL)
    {
        LOG_E("AT client not ready");
        return -1;
    }

    format = atoi(argv[0]);
    fo  = atoi(argv[2]);
    vp  = atoi(argv[3]);
    pid = atoi(argv[4]);
    dcs = atoi(argv[5]);

    if (strcasecmp(argv[1], "gsm") == 0)
    {
        charset = QL_SMS_CHARSET_GSM;
    }
    else if (strcasecmp(argv[1], "ira") == 0)
    {
        charset = QL_SMS_CHARSET_IRA;
    }
    else if (strcasecmp(argv[1], "ucs2") == 0)
    {
        charset = QL_SMS_CHARSET_UCS2;
    }
    else
    {
        LOG_E("Invalid charset. Use gsm, ira or ucs2");
        return -1;
    }

    if (strcasecmp(argv[6], "SM") == 0)
    {
        storage = QL_SMS_STORAGE_SM;
    }
    else if (strcasecmp(argv[6], "ME") == 0)
    {
        storage = QL_SMS_STORAGE_ME;
    }
    else
    {
        LOG_E("Invalid storage. Use SM or ME");
        return -1;
    }

    if (strcasecmp(argv[7], "pending") == 0)
    {
        index = ql_sms_get_pending_index();
        if (index < 0)
        {
            LOG_E("No pending SMS");
            return -1;
        }
    }
    else
    {
        index = atoi(argv[7]);
        if (index < 0)
        {
            LOG_E("Invalid SMS index");
            return -1;
        }
    }

    ret = ql_sms_set_format(client, (ql_sms_mode_e)format);
    if (ret != 0)
    {
        LOG_E("auto read failed at ql_sms_set_format, ret=%d", ret);
        return -1;
    }

    ret = ql_sms_set_char(client, charset);
    if (ret != 0)
    {
        LOG_E("auto read failed at ql_sms_set_char, ret=%d", ret);
        return -1;
    }

    ret = ql_sms_set_textmd(client, fo, vp, pid, dcs);
    if (ret != 0)
    {
        LOG_E("auto read failed at ql_sms_set_textmd, ret=%d", ret);
        return -1;
    }

    ret = ql_sms_set_storage(client, storage);
    if (ret != 0)
    {
        LOG_E("auto read failed at ql_sms_set_storage, ret=%d", ret);
        return -1;
    }

    ret = ql_sms_read(client, index, msg, sizeof(msg));
    if (ret != 0)
    {
        LOG_E("auto read failed at ql_sms_read, ret=%d", ret);
        return -1;
    }

    LOG_I("AUTO SMS[%d]: %s", index, msg);

    if (strcasecmp(argv[7], "pending") == 0)
    {
        ql_sms_clear_pending();
    }

    return 0;
}



static int cli_sms_auto_config(int argc, char **argv)
{
    at_client_t client;
    ql_sms_charset_e charset;
    ql_sms_storage_e storage;
    int format;
    int fo, vp, pid, dcs;
    int ret;

    if ((argc < 7) || (argv == NULL))
    {
        LOG_E("Usage:");
        LOG_E("sms auto config <format> <charset> <fo> <vp> <pid> <dcs> <storage>");
        return -1;
    }

    client = at_client_get_first();
    if (client == NULL)
    {
        LOG_E("AT client not ready");
        return -1;
    }

    format = atoi(argv[0]);
    fo  = atoi(argv[2]);
    vp  = atoi(argv[3]);
    pid = atoi(argv[4]);
    dcs = atoi(argv[5]);

    if (strcasecmp(argv[1], "gsm") == 0)
    {
        charset = QL_SMS_CHARSET_GSM;
    }
    else if (strcasecmp(argv[1], "ira") == 0)
    {
        charset = QL_SMS_CHARSET_IRA;
    }
    else if (strcasecmp(argv[1], "ucs2") == 0)
    {
        charset = QL_SMS_CHARSET_UCS2;
    }
    else
    {
        LOG_E("Invalid charset. Use gsm, ira or ucs2");
        return -1;
    }

    if (strcasecmp(argv[6], "SM") == 0)
    {
        storage = QL_SMS_STORAGE_SM;
    }
    else if (strcasecmp(argv[6], "ME") == 0)
    {
        storage = QL_SMS_STORAGE_ME;
    }
    else
    {
        LOG_E("Invalid storage. Use SM or ME");
        return -1;
    }

    ret = ql_sms_set_format(client, (ql_sms_mode_e)format);
    if (ret != 0)
    {
        LOG_E("auto config failed at ql_sms_set_format, ret=%d", ret);
        return -1;
    }

    ret = ql_sms_set_char(client, charset);
    if (ret != 0)
    {
        LOG_E("auto config failed at ql_sms_set_char, ret=%d", ret);
        return -1;
    }

    ret = ql_sms_set_textmd(client, fo, vp, pid, dcs);
    if (ret != 0)
    {
        LOG_E("auto config failed at ql_sms_set_textmd, ret=%d", ret);
        return -1;
    }

    ret = ql_sms_set_storage(client, storage);
    if (ret != 0)
    {
        LOG_E("auto config failed at ql_sms_set_storage, ret=%d", ret);
        return -1;
    }

    LOG_I("SMS auto config success");
    return 0;
}

/*===========================================================================
 * Main CLI Entry
 *===========================================================================*/
int cli_sms_test(s32_t argc, char *argv[])
{
    /* AUTO SMS READ (SAFE CONTEXT) */
    // int idx = ql_sms_get_pending_index();

    // if (idx >= 0)
    // {
    //     char msg[256] = {0};

    //     if (ql_sms_read(at_client_get_first(), idx, msg, sizeof(msg)) == 0)
    //     {
    //         LOG_I(">>> AUTO SMS[%d]: %s", idx, msg);
    //     }
    //     else
    //     {
    //         LOG_E(">>> FAILED TO READ SMS");
    //     }

    //     ql_sms_clear_pending();
    // }
    
    if ((argc < 2) || (argv == NULL))
    {
        cli_sms_get_help();
        return -1;
    }

    if (strcmp(argv[1], "format") == 0)
    {
        return cli_sms_format(argc - 2, &argv[2]);
    }
    else if (strcmp(argv[1], "char") == 0)
    {
        return cli_sms_set_char(argc - 2, &argv[2]);
    }
    else if (strcmp(argv[1], "textmd") == 0)
    {
        return cli_sms_set_textmd(argc - 2, &argv[2]);
    }
    else if (strcmp(argv[1], "send") == 0)
    {
        return cli_sms_send(argc - 2, &argv[2]);
    }
    else if (strcmp(argv[1], "read") == 0)
    {
        return cli_sms_read(argc - 2, &argv[2]);
    }
    else if (strcmp(argv[1], "delete") == 0)
    {
        return cli_sms_delete(argc - 2, &argv[2]);
    }
    else if (strcmp(argv[1], "storage") == 0)
    {
        return cli_sms_storage(argc - 2, &argv[2]);
    }
    else if (strcmp(argv[1], "list") == 0)
    {
        return cli_sms_list(argc - 2, &argv[2]);
    }
    else if (strcmp(argv[1], "auto") == 0)
    {
        if ((argc >= 3) && (strcmp(argv[2], "send") == 0))
        {
            return cli_sms_auto_send(argc - 3, &argv[3]);
        }
        else if ((argc >= 3) && (strcmp(argv[2], "read") == 0))
        {
            return cli_sms_auto_read(argc - 3, &argv[3]);
        }
        else if ((argc >= 3) && (strcmp(argv[2], "config") == 0))
        {
            return cli_sms_auto_config(argc - 3, &argv[3]);
        }

        LOG_E("Usage: sms auto send <format> <charset> <fo> <vp> <pid> <dcs> <number> <text>");
        LOG_E("Usage: sms auto read <format> <charset> <fo> <vp> <pid> <dcs> <storage> <index|pending>");
        LOG_E("Usage: sms auto config <format> <charset> <fo> <vp> <pid> <dcs> <storage>");
        return -1;
    }
    

    LOG_E("Unknown SMS command: %s", argv[1]);
    cli_sms_get_help();
    return -1;
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SMS__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */