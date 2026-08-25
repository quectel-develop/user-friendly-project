
#include "ql_voice_manager.h"
#include "qosa_log.h"

static ql_voice_manager_t s_handle = NULL;

void cli_voice_notice_callback(ql_voice_info_s info, void *self)
{
    switch (info.state)
    {
    case QL_VOICE_STATE_HOLD:
        LOG_I("Holding %s...", info.number);
        break;
    case QL_VOICE_STATE_ORIGINAL:
        LOG_I("Calling %s...", info.number);
        break;
    case QL_VOICE_STATE_CONNECT:
        LOG_I("Connected with %s", info.number);
        break;
    case QL_VOICE_STATE_INCOMING:
        LOG_I("Incoming call from %s - Please answer or reject", info.number);
        break;
    case QL_VOICE_STATE_WAITING:
        LOG_I("New incoming call[%s] during active call", info.number);
        break;
    case QL_VOICE_STATE_END:
        LOG_I("Call with %s ended", info.number);
        break;
    case QL_VOICE_STATE_ALERTING:
        LOG_I("Waiting for %s to answer...", info.number);
        break;
    default:
        break;
    }
}
static int cli_voice_start()
{
    if (s_handle != NULL)
    {
        LOG_W("voice testing already started");
        return 0;
    }
    s_handle = ql_voice_manager_create(at_client_get_first(), cli_voice_notice_callback, NULL);
    if (s_handle != NULL)
    {
        LOG_I("voice testing start success");
        return 0;
    }
    LOG_E("voice testing start failed");
    return -1;
}
static int cli_voice_answer()
{
    if (NULL == s_handle)
    {
        LOG_W("voice testing not started");
        return -1;
    }
    QL_VOICE_MANAGER_ERR_CODE_E ret = ql_voice_answer(s_handle);
    if (ret == QL_VOICE_MANAGER_OK)
    {
        LOG_I("answer success");
        return 0;
    }
    LOG_E("answer failed");
    return -1;
}

static int cli_voice_hold_and_answer()
{
    if (NULL == s_handle)
    {
        LOG_W("voice testing not started");
        return -1;
    }
    QL_VOICE_MANAGER_ERR_CODE_E ret = ql_voice_hold_and_answer(s_handle);
    if (ret == QL_VOICE_MANAGER_OK)
    {
        LOG_I("hold and answer success");
        return 0;
    }
    LOG_E("hold and answer failed");
    return -1;
}

static int cli_voice_release_and_answer()
{
    if (NULL == s_handle)
    {
        LOG_W("voice testing not started");
        return -1;
    }
    QL_VOICE_MANAGER_ERR_CODE_E ret = ql_voice_release_and_answer(s_handle);
    if (ret == QL_VOICE_MANAGER_OK)
    {
        LOG_I("release and answer success");
        return 0;
    }
    LOG_E("release and answer failed");
    return -1;
}
static int cli_voice_call(const char *phone_number)
{
    if (NULL == s_handle)
    {
        LOG_W("voice testing not started");
        return -1;
    }
    QL_VOICE_MANAGER_ERR_CODE_E ret = ql_voice_dial(s_handle, phone_number);
    if (ret == QL_VOICE_MANAGER_OK)
    {
        LOG_I("voice dial success");
        return 0;
    }
    LOG_E("voice dial failed");
    return -1;
}
static void cli_voice_hangup()
{
    if (NULL == s_handle)
    {
        LOG_W("voice testing not started");
        return;
    }
    ql_voice_hangup(s_handle);
    LOG_I("voice hangup success");
}

static void cli_voice_stop()
{
    if (NULL == s_handle)
    {
        LOG_W("voice testing not started");
        return;
    }
    ql_voice_manager_destroy(s_handle);
    s_handle = NULL;
    LOG_I("voice stop success");
}
void cli_voice_manager_get_help(void)
{
    LOG_I("voice test_type [phone_number]");
    LOG_I("test_type:");
    LOG_I("     0     : start voice test");
    LOG_I("     1     : answer an incoming call");
    LOG_I("     2     : hold and answer an incoming call");
    LOG_I("     3     : release and answer an incoming call");
    LOG_I("     4     : call a phone number (requires phone_number)");
    LOG_I("             example: voice 4 13800138000");
    LOG_I("     5     : hang up the current active call");
    LOG_I("     6     : stop voice test");
    LOG_I("");
    LOG_I("phone_number:");
    LOG_I("     required only for test_type=4");
}
int cli_voice_manager_test(s32_t argc, char *argv[])
{
    if (argc < 2)
    {
        return -1;
    }
    if (atoi(argv[1]) == 0)
    {
        return cli_voice_start();
    }
    else if (atoi(argv[1]) == 1)
    {
        return cli_voice_answer();
    }
    else if (atoi(argv[1]) == 2)
    {
        return cli_voice_hold_and_answer();
    }
        else if (atoi(argv[1]) == 3)
    {
        return cli_voice_release_and_answer();
    }
    else if (atoi(argv[1]) == 4)
    {
        if (argc < 3)
        {
            LOG_E("phone_number is required");
            return -1;
        }
        cli_voice_call(argv[2]);
        return 0;
    }
    else if (atoi(argv[1]) == 5)
    {
        cli_voice_hangup();
        return 0;
    }
    else if (atoi(argv[1]) == 6)
    {
        cli_voice_stop();
        return 0;
    }
    return 0;
}

