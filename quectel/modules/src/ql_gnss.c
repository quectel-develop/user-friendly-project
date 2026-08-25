#include "qosa_log.h"
#include "ql_gnss.h"

ql_gnss_t ql_gnss_init(at_client_t client)
{
    ql_gnss_t handle = (ql_gnss_t)malloc(sizeof(ql_gnss_s));
    if (NULL == handle)
    {
        LOG_E("no memory for AT client response object.");
        return NULL;
    }
    handle->client = client;
    return handle;
}

QL_GNSS_ERR_CODE_E ql_gnss_start(ql_gnss_t handle)
{
    at_response_t resp = NULL;
    resp = at_create_resp_new(128, 0, (3000), NULL);
    if (at_obj_exec_cmd(handle->client, resp, "AT+QGPS=1") < 0)
    {
        int err = QL_GNSS_ERR_UNKNOWN;
        for (int i = 0; i < resp->line_counts; i++)
        {
            const char *line = at_resp_get_line(resp, i + 1);

            if (strstr(line, "+CME ERROR:") != NULL)
            {
                sscanf(line, "+CME ERROR: %d", &err);
                break;
            }
        }
        at_delete_resp(resp);
        return (QL_GNSS_ERR_CODE_E)err;
    }

    at_delete_resp(resp);
    return QL_GNSS_SUCCESS;
}

static bool ql_parse_qgpsloc(const char *response, ql_gnss_location_s *loc)
{
    if (!response || !loc)
        return false;

    char lat_str[16] = {0};
    char lon_str[16] = {0};
    char hdop_str[16] = {0};
    char alt_str[16] = {0};
    char cog_str[16] = {0};
    char spd_kmh_str[16] = {0};
    char spd_knot_str[16] = {0};

    int ret = sscanf(response,
        "+QGPSLOC: %11[^,],%15[^,],%15[^,],%15[^,],%15[^,],%d,%15[^,],%15[^,],%15[^,],%6[^,],%d",
        loc->utc_time,
        lat_str,
        lon_str,
        hdop_str,
        alt_str,
        &loc->fix_mode,
        cog_str,
        spd_kmh_str,
        spd_knot_str,
        loc->date,
        &loc->satellite_count);

    if (ret != 11)
        return false;

    loc->latitude   = strtof(lat_str, NULL);
    loc->longitude  = strtof(lon_str, NULL);
    loc->hdop       = strtof(hdop_str, NULL);
    loc->altitude   = strtof(alt_str, NULL);
    loc->cog        = strtof(cog_str, NULL);
    loc->speed_kmh  = strtof(spd_kmh_str, NULL);
    loc->speed_knot = strtof(spd_knot_str, NULL);
    LOG_I("latitude: %f, longitude: %f", loc->latitude, loc->longitude);
    return true;
}
QL_GNSS_ERR_CODE_E ql_gnss_get_location(ql_gnss_t handle, ql_gnss_location_s *loc)
{
    if (NULL == handle)
        return QL_GNSS_NOT_INIT;
    at_response_t resp = NULL;
    resp = at_create_resp_new(128, 0, (3000), NULL);
    if (at_obj_exec_cmd(handle->client, resp, "AT+QGPSLOC=2") < 0)
    {
        int err = QL_GNSS_ERR_UNKNOWN;
        for (int i = 0; i < resp->line_counts; i++)
        {
            const char *line = at_resp_get_line(resp, i + 1);

            if (strstr(line, "+CME ERROR:") != NULL)
            {
                sscanf(line, "+CME ERROR: %d", &err);
                break;
            }
        }
        at_delete_resp(resp);
        return (QL_GNSS_ERR_CODE_E)err;
    }
    for (int i = 0; i < resp->line_counts; i++)
    {
        const char *line = at_resp_get_line(resp, i + 1);
        if (ql_parse_qgpsloc(line, loc))
        {

            at_delete_resp(resp);
            return QL_GNSS_SUCCESS;
        }
    }
    at_delete_resp(resp);
    return QL_GNSS_ERR_UNKNOWN;
}

void ql_gnss_stop(ql_gnss_t handle)
{
    if (NULL == handle)
        return;
    at_response_t query_resp = NULL;
    query_resp = at_create_resp(128, 0, (3000));
    at_obj_exec_cmd(handle->client, query_resp, "AT+QGPSEND");
    at_delete_resp(query_resp);
}

void ql_gnss_deinit(ql_gnss_t handle)
{
    if (handle != NULL)
    {
        free(handle);
    }
}