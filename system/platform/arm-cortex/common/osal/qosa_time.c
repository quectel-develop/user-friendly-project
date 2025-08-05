#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "qosa_def.h"
#include "qosa_time.h"

static time_t s_ntp_synced_time = 0;

time_t time(time_t *t)
{
    if (t != NULL)
    {
        *t = s_ntp_synced_time + (osKernelGetTickCount() / configTICK_RATE_HZ);
    }
    return s_ntp_synced_time + (osKernelGetTickCount() / configTICK_RATE_HZ);
}

void update_ntp_time(time_t new_time)
{
    s_ntp_synced_time = new_time - (osKernelGetTickCount() / (1000 / portTICK_PERIOD_MS));
}