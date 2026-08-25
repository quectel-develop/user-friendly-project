/*
 * Copyright (c) 2025, FAE
 * @file ql_ota.h 
 * @brief Quectel ota interface definitions
 * Date          Author           Notes
 * 2025-9-9      Wells         first version
 */

 #ifndef __QL_FOTA_H__
 #define __QL_FOTA_H__ 
#include <stdbool.h>
#include "ql_ssl.h"

typedef enum
{
	QL_FOTA_DWNLD_MOD_HTTP,
	QL_FOTA_DWNLD_MOD_FTP
} QL_FOTA_MODE_E;

typedef enum
{
	QL_FOTA_FAIL = -1,
	QL_FOTA_SUCCEED = 0,
	QL_FOTA_INPROGRESS = 1,
	QL_FOTA_SETFLAG = 2
} QL_FOTA_STATUS_E;

typedef enum
{
    QL_FOTA_DOWNLOAD_SD_CARD = 0,
    QL_FOTA_DOWNLOAD_FLASH
    
} QL_FOTA_DOWNLOAD_LOCATION_E;

typedef void (*fota_progress_callback)(QL_FOTA_STATUS_E status, int progress, void *userData);

typedef struct ql_fota_http_config
{
    bool async;
    int content_id;
	char url[128];
    char header[128];
    QL_FOTA_DOWNLOAD_LOCATION_E location;
    char save_path[128];
    fota_progress_callback cb;
    void *user_data;
    ql_SSL_Config ssl_cfg;
} ql_fota_http_config_s;

typedef struct ql_fota_ftp_config
{
    bool async;
    int content_id;
	char address[64];
    int port;
	char username[32];
	char password[32];
    char work_dir[64];
    char file_name[64];
    QL_FOTA_DOWNLOAD_LOCATION_E location;
    char save_path[128];
    fota_progress_callback cb;
    void *user_data;
    ql_SSL_Config ssl_cfg;
} ql_fota_ftp_config_s;

typedef union ql_fota_config
{
    ql_fota_http_config_s http_config;
	ql_fota_ftp_config_s ftp_config;
} ql_fota_config_u;


int ql_fota_firmware_download(QL_FOTA_MODE_E mode, ql_fota_config_u config);

 #endif