/*
 * Copyright (c) 2025, FAE
 * @file ql_ftp.h
 * @brief Quectel ftp interface definitions
 * Date           Author           Notes
 * 2025-7-18      Wells         first version
 */
#ifndef __QUECTEL_FTP_H__
#define __QUECTEL_FTP_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include "ff.h"
#include "at.h"
#include "ql_ssl.h"
#include "qosa_system.h"
#include "ql_common_def.h"

/**
 * FTP(S) file type
 */
typedef enum
{
    QL_FTP_FILE_TYPE_BINARY = 0,       // binary
    QL_FTP_FILEL_TYPE_ASCII = 1        // ascii
} QL_FTP_FILE_TYPE_E;

/**
 * FTP(S) ssl type
 */
typedef enum
{
    QL_FTP_SSL_TYPE_CLIENT = 0,       // FTP client
    QL_FTP_SSL_TYPE_IMPLICIT = 1,     // FTPS Implicit Encryption
    QL_FTP_SSL_TYPE_EXPLICIT = 2      // FTPS Explicit Encryption
} QL_FTP_SSL_TYPE_E;

/**
 * FTP(S) data address mode
 * in LPWA module, FTP_DATA_ADDR_SMART_FALLBACK is not supported, default FTP_DATA_ADDR_PORT;
 * in LTE Standard Module, default is QL_FTP_DATA_ADDR_PASV
 */
typedef enum
{
    QL_FTP_DATA_ADDR_PASV = 0,          // Use server dispatched address
    QL_FTP_DATA_ADDR_PORT = 1,          // Use FTP(S) control session address
    QL_FTP_DATA_ADDR_SMART_FALLBACK = 2 // Attempt PORT first, auto-fallback to PASV on failure
} QL_FTP_DATA_ADDRESS_MODE_E;


/*
    Not all of the following options are supported by the module.
    Please configure according to your requirements by referring to the documentation.
*/
typedef enum
{
    QL_FTP_OPT_ACCOUNT,             // username,password, do not set this, auto set inside
    QL_FTP_OPT_FILE_TYPE,           // QL_FTP_FILE_TYPE_E, default QL_FTP_FILE_TYPE_BINARY
    QL_FTP_OPT_CONTEXT_ID,          //  range 1~16, default:1
    QL_FTP_OPT_TRANMODE,            //  must be 1, do not set this, auto set inside
    QL_FTP_OPT_RSP_TIMEOUT,         // range 20~180, default:90s
    QL_FTP_OPT_SSL_TYPE,            // QL_FTP_SSL_TYPE_E, default QL_FTP_SSL_TYPE_CLIENT
    QL_FTP_OPT_SSL_CTXID,           // range 0~5, default 0
    QL_FTP_OPT_SSL_DATA_ADDR,       // QL_FTP_DATA_ADDRESS_MODE_E
    QL_FTP_OPT_REST_ENABLE,         // default true. false: Disable FTP REST command;true: Enable FTP REST command.
    QL_FTP_OPT_WRITE_FUNCTION,      // Callback for handling DOWNLOADED data received from FTP server
    QL_FTP_OPT_WRITE_DATA,          // void*, user-defined context passed to the write callback
    QL_FTP_OPT_READ_FUNCTION,       // Callback for providing data to be UPLOADED to FTP server  
    QL_FTP_OPT_READ_DATA,           // void*, user-defined context passed to the read callback
    QL_FTP_OPT_PROGRESS_FUNCTION,   // Callback function for tracking upload/download progress  
    QL_FTP_OPT_PROGRESS_DATA,       // void*, user-defined context passed to the progress callback
    QL_FTP_OPT_UNKNOWN
} QL_FTP_OPTION_E;

typedef enum
{
    QL_FTP_USR_DOWNLOAD_START = 0,
    QL_FTP_USR_DOWNLOAD_DATA,
    QL_FTP_USR_DOWNLOAD_END

} QL_FTP_USR_DOWNLOAD_EVENT_E;

typedef void (*ftp_user_write_callback)(QL_FTP_USR_DOWNLOAD_EVENT_E, const char *, size_t, void *);
typedef size_t (*ftp_user_read_callback)(char *, size_t, void *);
typedef void (*ftp_user_progress_callback)(int, int, void *);

typedef struct ql_ftp_file_info
{
    char type;                            // 'd'=dir, '-'=file, 'l'=link
    char permissions[10];                 // permission "rwxr-xr-x"）
    int links;                            // hard link count
    char owner[32];                       // owner
    char group[32];                       // group
    long size;                            // size（byte）
    char date[32];                        // modify data (eg: "Jul 21 09:25")
    char name[255];                       // name
    struct ql_ftp_file_info *next;   // list next
} ql_ftp_file_info_s;

typedef struct ql_ftp
{
    at_client_t client;
    char *host;
    u16_t port;
    QL_FTP_ERR_CODE_E err;
    QL_FTP_ERR_CODE_E protocol_err;
    int stat;
    int timeout;
    osa_sem_t sem;
    char *pwd;
    char *modify_time;
    FIL file;
    size_t file_size;
    ftp_user_write_callback usr_write_cb;
    ftp_user_read_callback  usr_read_cb;
    ftp_user_progress_callback usr_progress_cb;
    void* user_write_data;
    void* user_read_data;
    void* user_progress_data;
    ql_ftp_file_info_s *file_list;
    ql_SSL_Config ssl;
} ql_ftp_s;


typedef ql_ftp_s* ql_ftp_t;

/**
 * @brief Initialize the FTP client instance
 * @param host FTP server host name or IP address
 * @param port FTP server port
 * @param client AT client handle used for underlying communication
 * @return ql_ftp_t Handle to the created FTP client instance
 */
ql_ftp_t ql_ftp_init(const char* host, u16_t port, at_client_t client);

/**
 * @brief Set FTP client options
 *
 * @param handle FTP client handle returned by ql_ftp_init()
 * @param option Option type to set (see QL_FTP_OPTION_E enum)
 * @param ... Variable arguments depending on the option being set
 * @return true if option was set successfully, false otherwise
 */
bool ql_ftp_setopt(ql_ftp_t handle, QL_FTP_OPTION_E option, ...);

/**
 * @brief Set SSL configuration for an FTP client
 * @param handle FTP client handle returned by ql_http_init()
 * @param config SSL configuration
 * @return true if SSL configuration was set successfully, false otherwise
 */
bool ql_ftp_set_ssl(ql_ftp_t handle, ql_SSL_Config config);
/**
 * @brief ftp login
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param username Username, if no need, set NULL, max bytes 255
 * @param password Password, if no need, set NULL, max bytes 255
 * @return QL_FTP_ERR_CODE_E Error code indicating request status
 */
QL_FTP_ERR_CODE_E ql_ftp_login(ql_ftp_t handle, const char* username, const char* password);

///////////////////////////////////////////////////////////////////////////////
                    /* begin operate dir*/
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief ftp set cwd
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param path Path, max bytes 255
 * @return QL_FTP_ERR_CODE_E Error code indicating request status
 */
QL_FTP_ERR_CODE_E ql_ftp_cwd(ql_ftp_t handle, const char* path);

/*
 * @brief ftp get cwd
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param path save the current working directory
 * @return QL_FTP_ERR_CODE_E Error code indicating request status
*/
QL_FTP_ERR_CODE_E ql_ftp_pwd(ql_ftp_t handle, char* path);

/*
 * @brief ftp make directory
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param path Path, max bytes 255
 * @return QL_FTP_ERR_CODE_E Error code indicating request status
 */
QL_FTP_ERR_CODE_E ql_ftp_mkdir(ql_ftp_t handle, const char* path);

/*
 * @brief ftp remove directory
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param path Path, max bytes 255
 * @return QL_FTP_ERR_CODE_E Error code indicating request status
*/
QL_FTP_ERR_CODE_E ql_ftp_rmdir(ql_ftp_t handle, const char* path);

/*
 * @brief ftp rename file or directory
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param old_path Old path, max bytes 255
 * @param new_path New path, max bytes 255
*/
QL_FTP_ERR_CODE_E ql_ftp_rename(ql_ftp_t handle, const char* old_name, const char* new_name);
/*
 * @brief ftp list directory contained type,permission,links,owner,group,size,date,name
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param path Path, max bytes 255
 * @param[out] head Pointer to the linked list head of directory entries.
 *                  On success, stores the list of affected files/directories.
 *                  Caller must free the list using ql_ftp_list_free().
 * @return QL_FTP_ERR_CODE_E Error code indicating request status
*/
QL_FTP_ERR_CODE_E ql_ftp_list(ql_ftp_t handle, const char *path, ql_ftp_file_info_s **head);

/*
 * @brief ftp list directory contain name
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param path Path, max bytes 255
 * @param[out] head Pointer to the linked list head of directory entries.
 *                  On success, stores the list of affected files/directories.
 *                  Caller must free the list using ql_ftp_list_free().
 * @return QL_FTP_ERR_CODE_E Error code indicating request status
*/
QL_FTP_ERR_CODE_E ql_ftp_nlist(ql_ftp_t handle, const char *path, ql_ftp_file_info_s **head);

/*
 * @brief Retrieve a machine-readable directory listing from FTP server using MLSD command
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param path Path, max bytes 255
 * @param[out] Pointer to receive structured of directory entries.
 * @return QL_FTP_ERR_CODE_E Error code indicating request status
*/
QL_FTP_ERR_CODE_E ql_ftp_mlsd(ql_ftp_t handle, const char *path, ql_ftp_file_info_s *info);
///////////////////////////////////////////////////////////////////////////////
                    /* end operate dir*/
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
                    /* begin operate file*/
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief  get file size
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param  remote_file_name: remote file name
 * @param  file_size: file size
 * @return QL_FTP_ERR_CODE_E Error code indicating request status
 */
QL_FTP_ERR_CODE_E ql_ftp_file_size(ql_ftp_t handle, const char *remote_file_name, size_t *file_size);

/*
* @brief:  ftp upload file
* @param handle FTP client handle  returned by ql_ftp_init()
* @param:  local_file_name: local file name. if setting write callback, the local_file_name is unused, send data through callback.
* @param:  remote_file_name: remote file name
* @return QL_FTP_ERR_CODE_E Error code indicating request status
*/
QL_FTP_ERR_CODE_E ql_ftp_upload(ql_ftp_t handle, const char *local_file_name, const char *remote_file_name);

/*
 * @brief:  ftp download file
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param:  remote_file_name: remote file name
 * @param:  local_file_name: local file name. if setting read callback, the local_file_name is unused, receive data through callback.  
 * @return QL_FTP_ERR_CODE_E Error code indicating request status
*/
QL_FTP_ERR_CODE_E ql_ftp_download(ql_ftp_t handle, const char *remote_file_name, const char *local_file_name);

/*
 * @brief:  ftp delete file
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param:  remote_file_name: remote file name
 * @return QL_FTP_ERR_CODE_E Error code indicating request status
*/
QL_FTP_ERR_CODE_E ql_ftp_file_delete(ql_ftp_t handle, const char *remote_file_name);

/*
 * @brief:  ftp get file modify time
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @param:  remote_file_name: remote file name
 * @param[out] time : file modify time(YYYYMMDDHHMMSS or YYYYMMDDHHMMSS.NNN)
*/
QL_FTP_ERR_CODE_E ql_ftp_file_get_modify_time(ql_ftp_t handle, const char *remote_file_name, char *time);
///////////////////////////////////////////////////////////////////////////////
                    /* end operate file*/
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief ftp logout
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @return QL_FTP_ERR_CODE_E Error code indicating request status
*/
QL_FTP_ERR_CODE_E ql_ftp_logout(ql_ftp_t handle);

/*
 * @brief ftp uninit
 * @param handle FTP client handle  returned by ql_ftp_init()
*/
void ql_ftp_uninit(ql_ftp_t handle);
/*
 * @brief Get protocol error code
 * @param handle FTP client handle  returned by ql_ftp_init()
 * @return QL_FTP_ERR_CODE_E protocol error code
 */
QL_FTP_ERR_CODE_E ql_get_protocol_err(ql_ftp_t handle);

void ql_ftp_list_free(ql_ftp_file_info_s *head);
#ifdef __cplusplus
}
#endif

#endif // __QL_FTP_H__