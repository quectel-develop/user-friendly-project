#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_PSM__
#include <at.h>
#include "ql_net.h"
#include "ql_psm.h"
#include "qosa_log.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#include "windows.h"
#elif __linux__
#else
// #include "cmsis_os2.h"
#endif

/*
 * @brief Convert seconds to TAU format
 * @param[in] seconds
 * @return TAU byte value
*/
static uint8_t ql_convert_seconds_to_TAU_format(uint32_t seconds)
{
    uint8_t unit_bits = 0;
    uint8_t timer_value = 0;

    if (seconds <= 62)
    {
        unit_bits = 0x03; timer_value = (seconds + 1) / 2;
    }
    else if (seconds <= 930)
    {
        unit_bits = 0x04; timer_value = (seconds + 15) / 30;
    }
    else if (seconds <= 1860)
    {
        unit_bits = 0x05; timer_value = (seconds + 30) / 60;
    }
    else if (seconds <= 18600)
    {
        unit_bits = 0x00; timer_value = (seconds + 300) / 600;
    }
    else if (seconds <= 111600)
    {
        unit_bits = 0x01; timer_value = (seconds + 1800) / 3600;
    }
    else if (seconds <= 1116000)
    {
        unit_bits = 0x02; timer_value = (seconds + 18000) / 36000;
    }
    else
    {
        unit_bits = 0x02; timer_value = 31;
    }

    return (unit_bits << 5) | (timer_value & 0x1F);
}

/**
 * @brief Convert seconds to Active time format
 * @param seconds
 * @return Active byte value
 */
static uint8_t ql_convert_seconds_to_Active_format(uint32_t seconds)
{
    // deactivated
    if (seconds == 0) {
        return 0xE0; // 11100000 (bits 6-8: 111)
    }

    uint8_t unit_bits = 0;
    uint8_t timer_value = 0;

    if (seconds <= 62)
    {
        unit_bits = 0x00; timer_value = (seconds + 1) / 2;
    }
    else if (seconds <= 1860)
    {
        unit_bits = 0x01; timer_value = (seconds + 30) / 60;
    }
    else if (seconds <= 11160)
    {
        unit_bits = 0x02; timer_value = (seconds + 180) / 360;
    }
    else
    {
        unit_bits = 0x02; timer_value = 31; // Max
    }

    return (unit_bits << 5) | (timer_value & 0x1F);
}

/**
 * @brief Convert a byte to binary string representation
 * @param byte_value Input byte value to convert
 * @param output_str Output buffer for binary string (must be at least 9 bytes)
 */
static void ql_convert_byte_to_binary_str(uint8_t byte_value, char *output_str)
{
    for (int i = 7; i >= 0; i--)
	{
        output_str[7 - i] = ((byte_value >> i) & 0x01) ? '1' : '0';
    }
    output_str[8] = '\0';
}

/**
 * @brief Convert TAU binary string to seconds
 * @param binary_str 8-character binary string
 * @return Corresponding time in seconds
 */
uint32_t convert_TAU_binary_to_seconds(const char *binary_str)
{
    uint8_t byte_value = (uint8_t)strtol(binary_str, NULL, 2);
    uint8_t unit_bits = (byte_value & 0xE0) >> 5;
    uint8_t timer_value = byte_value & 0x1F;

    switch (unit_bits)
	{
	case 0x03: // 011: 2 second units
		return timer_value * 2;
	case 0x04: // 100: 30 second units
		return timer_value * 30;
	case 0x05: // 101: 1 minute units
		return timer_value * 60;
	case 0x00: // 000: 10 minute units
		return timer_value * 600;
	case 0x01: // 001: 1 hour units
		return timer_value * 3600;
	case 0x02: // 010: 10 hour units
		return timer_value * 36000;
	default:
		return 0;
    }
}

/**
 * @brief Convert Active time binary string to seconds
 * @param binary_str 8-character binary string
 * @return Corresponding time in seconds
 */
uint32_t convert_Active_binary_to_seconds(const char *binary_str)
{
    uint8_t byte_value = (uint8_t)strtol(binary_str, NULL, 2);

    uint8_t unit_bits = (byte_value & 0xE0) >> 5;
    uint8_t timer_value = byte_value & 0x1F;

    // Check if deactivated
    if (unit_bits == 0x07) // 111
	{
        return 0;
    }

    switch (unit_bits)
	{
        case 0x00: // 000: 2seconds units
            return timer_value * 2;
        case 0x01: // 001: 1 minute units
            return timer_value * 60;
        case 0x02: // 010: 6 minutes units
            return timer_value * 360; // 6 × 60 = 360 seconds
        default:
            return 0;
    }
}

int ql_psm_settings_write(at_client_t client, ql_psm_setting_s settings)
{
	if (settings.Requested_Periodic_TAU < 0|| settings.Requested_Active_Time < 0 )
	{
		LOG_E("Invalid PSM settings.");
		return -1;
	}
	char TAU[9] = {0};
	char Active[9] = {0};
	uint8_t value = ql_convert_seconds_to_TAU_format(settings.Requested_Periodic_TAU);
	ql_convert_byte_to_binary_str(value, TAU);
	value = ql_convert_seconds_to_Active_format(settings.Requested_Active_Time);
	ql_convert_byte_to_binary_str(value, Active);
	at_response_t resp = at_create_resp_new(256, 0, (3000), NULL);
	int ret = at_obj_exec_cmd(client, resp, "AT+CPSMS=%d,,,\"%s\",\"%s\"", settings.Mode, TAU, Active);
	at_delete_resp(resp);
	return (ret == 0) ? 0 : -1;
}

int ql_psm_settings_read(at_client_t client, ql_psm_setting_s *settings)
{
	at_response_t resp = at_create_resp_new(256, 0, (3000), NULL);
	if (at_obj_exec_cmd(client, resp, "AT+CPSMS?") < 0)
	{
		at_delete_resp(resp);
		return -1;
	}
	for (int i = 0; i < resp->line_counts; i++)
	{
		const char *line = at_resp_get_line(resp, i + 1);
		int tmp = 0;
		char TAU[9] = {0};
		char Active[9] = {0};
		LOG_I("%s", line);
        if (sscanf(line, "+CPSMS: %d,,,\"%[^\"]\",\"%[^\"]\"",&tmp, TAU, Active) == 3 ||
            sscanf(line, "+CPSMS: %d,%*[^,],%*[^,],\"%[^\"]\",\"%[^\"]\"",&tmp, TAU, Active) == 3)
		{
			settings->Mode = (bool)tmp;
			settings->Requested_Periodic_TAU = convert_TAU_binary_to_seconds(TAU);
			settings->Requested_Active_Time = convert_Active_binary_to_seconds(Active);
			at_delete_resp(resp);
			return 0;
		}
	}
	at_delete_resp(resp);
	return -1;
}

#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_PSM__ */

