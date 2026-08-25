/**
 * @file ql_gnss.h 
 * @brief quectel GNSS interface definitions
 */

#ifndef __QL_GNSS_H__
#define __QL_GNSS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "at.h"
#include "ql_common_def.h"


// 定位信息结构体
typedef struct {
    char utc_time[12];      // hhmmss.sss
    float latitude;         // 根据mode格式不同
    float longitude;        // 根据mode格式不同
    float hdop;             // 水平精度因子
    float altitude;         // 海拔高度(m)
    int fix_mode;           // 定位模式
    float cog;              // 对地航向 ddd.mm
    float speed_kmh;        // 速度(km/h)
    float speed_knot;       // 速度(节)
    char date[7];           // ddmmyy
    int satellite_count;    // 卫星数量
} ql_gnss_location_s;

typedef struct ql_gnss
{
    at_client_t client;
} ql_gnss_s;

typedef ql_gnss_s* ql_gnss_t;

/**
 * @brief Initialize the GNSS instance
 * @param client AT client handle used for underlying communication
 * @return ql_gnss_t Handle to creat GNSS instance
 */
ql_gnss_t ql_gnss_init(at_client_t client);

/*
 * @brief Start GNSS
 * @param handle net client handle returned by ql_gnss_init()
 * @return QL_GNSS_ERR_CODE_E Error code
*/
QL_GNSS_ERR_CODE_E ql_gnss_start(ql_gnss_t handle);


/**
 * @brief Get location
 * @param loc Returns the location information
 * @return Error code
 */
QL_GNSS_ERR_CODE_E ql_gnss_get_location(ql_gnss_t handle, ql_gnss_location_s *loc);



/*
 * @brief Detach from network
 * @param handle net client handle returned by ql_gnss_init()
 * @return void
*/
void ql_gnss_stop(ql_gnss_t handle);

/*
 * @brief Deinitialize the Network instance
 * @param handle net client handle returned by ql_gnss_init()
 * @return void
*/
void ql_gnss_deinit(ql_gnss_t handle);

#ifdef __cplusplus
}
#endif

#endif // __QL_GNSS_H__