#include "qosa_log.h"
#include "ql_voice_manager.h"

static bool s_global_voice_init = false;

static void ql_dsci(struct at_client *client, const char *data, size_t size, void* arg)
{
    ql_voice_manager_t handle = (ql_voice_manager_t)arg;
    if (NULL == handle ||  NULL == handle->voice_notice)
        return;
    ql_voice_info_s info = {0};
    int ret = sscanf(data, "^DSCI: %d,%d,%d,%*d,%[^,]", &info.id, &info.dir, &info.state, info.number);
    if (ret == 4)
    {
        handle->voice_notice(info, handle->user_data);
    }
}

static const struct at_urc s_voice_urc_table[] =
{
    {"^DSCI:",    "\r\n",                 ql_dsci}
};

static void ql_voice_enable_urc_DSCI(ql_voice_manager_t handle)
{
    if (NULL == handle)
    {
        return;
    }
    at_response_t resp = NULL;
    resp = at_create_resp_new(128, 0, (15000), handle);
    at_obj_exec_cmd(handle->client, resp, "AT^DSCI=1"); 
}

ql_voice_manager_t ql_voice_manager_create(at_client_t client, voice_notice_callback cb, void *user_data)
{
    ql_voice_manager_t handle = (ql_voice_manager_t)malloc(sizeof(ql_voice_manager_s));
    if (NULL == handle)
    {
        LOG_E("no memory for AT client response object.");
        return NULL;
    }
    handle->client = client;
    handle->state = QL_VOICE_STATE_IDLE;
    handle->voice_notice = cb;
    handle->user_data = user_data;
    if (!s_global_voice_init)
    {
        at_set_urc_table(s_voice_urc_table, sizeof(s_voice_urc_table) / sizeof(s_voice_urc_table[0]));
        s_global_voice_init = true;
    }
    ql_voice_enable_urc_DSCI(handle);
    return handle;
}

QL_VOICE_MANAGER_ERR_CODE_E ql_voice_dial(ql_voice_manager_t handle, const char* phone_number)
{
    if (NULL == handle)
    {
        return QL_VOICE_MANAGER_ERR_NOINIT;
    }
    at_response_t resp = NULL;
    resp = at_create_resp_new(128, 0, (15000), handle);
    if (at_obj_exec_cmd(handle->client, resp, "ATD%s;", phone_number) < 0)
    {
        at_delete_resp(resp);
        return QL_VOICE_MANAGER_ERR_DIAL;
    }

    at_delete_resp(resp);
    return QL_VOICE_MANAGER_OK;
}

QL_VOICE_MANAGER_ERR_CODE_E ql_voice_answer(ql_voice_manager_t handle)
{
    if (NULL == handle)
    {
        return QL_VOICE_MANAGER_ERR_NOINIT;
    }
    at_response_t resp = NULL;
    resp = at_create_resp_new(128, 0, (15000), handle);
    if (at_obj_exec_cmd(handle->client, resp, "ATA") < 0)
    {
        at_delete_resp(resp);
        return QL_VOICE_MANAGER_ERR_ANSWER;
    }

    at_delete_resp(resp);
    return QL_VOICE_MANAGER_OK; 
}

static QL_VOICE_MANAGER_ERR_CODE_E ql_voice_chld(ql_voice_manager_t handle, int n)
{
    if (NULL == handle)
    {
        return QL_VOICE_MANAGER_ERR_NOINIT;
    }
    at_response_t resp = NULL;
    resp = at_create_resp_new(128, 0, (15000), handle);
    if (at_obj_exec_cmd(handle->client, resp, "AT+CHLD=%d", n) < 0)
    {
        at_delete_resp(resp);
        return QL_VOICE_MANAGER_ERR_ANSWER;
    }

    at_delete_resp(resp);
    return QL_VOICE_MANAGER_OK; 
}

QL_VOICE_MANAGER_ERR_CODE_E ql_voice_hold_and_answer(ql_voice_manager_t handle)
{
    return ql_voice_chld(handle, 2);
}

QL_VOICE_MANAGER_ERR_CODE_E ql_voice_release_and_answer(ql_voice_manager_t handle)
{
    return ql_voice_chld(handle, 1);
}

void ql_voice_hangup(ql_voice_manager_t handle)
{
    if (NULL == handle)
    {
        return;
    }
    at_response_t resp = NULL;
    resp = at_create_resp_new(128, 0, (15000), handle);
    at_obj_exec_cmd(handle->client, resp, "ATH"); 
}

void ql_voice_manager_destroy(ql_voice_manager_t handle)
{
    if (handle != NULL)
    {
        free(handle);
    }
}