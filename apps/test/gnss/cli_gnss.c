
#include "ql_gnss.h"
#include "qosa_log.h"

static ql_gnss_t s_handle = NULL;

static int cli_start_gnss()
{
    if (s_handle != NULL)
    {
        LOG_W("gnss already start");
        return 0;
    }
    s_handle = ql_gnss_init(at_client_get_first());
    if (ql_gnss_start(s_handle) != QL_GNSS_SUCCESS)
    {
        ql_gnss_deinit(s_handle);
        LOG_E("gnss start failed");
        return -1;
    }
    LOG_I("gnss start success");
    return 0;
}

static int cli_get_location()
{
    if (NULL == s_handle)
    {
        LOG_W("gnss not start");
        return -1;
    }
    ql_gnss_location_s loc;
    if (ql_gnss_get_location(s_handle, &loc) != QL_GNSS_SUCCESS)
    {
        LOG_E("get location failed");
        return -1;
    }
    LOG_I("latitude = %f, longitude = %f, altitude = %f", loc.latitude, loc.longitude, loc.altitude);
    return 0;
}

static void cli_stop_gnss()
{
    if (NULL == s_handle)
    {
        LOG_W("gnss not start");
        return;
    }
    ql_gnss_stop(s_handle);
    ql_gnss_deinit(s_handle);
    s_handle = NULL;
    LOG_I("gnss start stop");
}
void cli_gnss_get_help(void)
{
    LOG_I("gnss test_type:");
    LOG_I("     0     : Start GNSS");
    LOG_I("     1     : Get Location");
    LOG_I("     2     : Stop GNSS");
}
int cli_gnss_test(s32_t argc, char *argv[])
{
    if (argc < 2)
    {
        return -1;
    }
    if (atoi(argv[1]) == 0)
    {
        return cli_start_gnss();
    }
    else if (atoi(argv[1]) == 1)
    {
        return cli_get_location();
    }
    else if (atoi(argv[1]) == 2)
    {
        cli_stop_gnss();
        return 0;
    }
    return 0;
}

