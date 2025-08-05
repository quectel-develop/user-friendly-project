/*
 * Copyright (c) 2025, FAE
 * @file quectel_ftp.h 
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
#include "quectel_common_def.h"

/**
 * FTP(S) file type
 */
typedef enum
{
    QT_FTP_FILE_TYPE_BINARY = 0,       // binary
    QT_FTP_FILEL_TYPE_ASCII = 1        // ascii
} QtFtpFileType;

/**
 * FTP(S) ssl type
 */
typedef enum
{
    QT_FTP_SSL_TYPE_CLIENT = 0,       // FTP client
    QT_FTP_SSL_TYPE_IMPLICIT = 1,     // FTPS Implicit Encryption
    QT_FTP_SSL_TYPE_EXPLICIT = 2      // FTPS Explicit Encryption
} QtFtpSslType;

/**
 * FTP(S) data address mode
 * in LPWA module, FTP_DATA_ADDR_SMART_FALLBACK is not supported, default FTP_DATA_ADDR_PORT;
 * in LTE Standard Module, default is QT_FTP_DATA_ADDR_PASV
 */
typedef enum
{
    QT_FTP_DATA_ADDR_PASV = 0,          // Use server dispatched address
    QT_FTP_DATA_ADDR_PORT = 1,          // Use FTP(S) control session address
    QT_FTP_DATA_ADDR_SMART_FALLBACK = 2 // Attempt PORT first, auto-fallback to PASV on failure
} QtFtpDataAddressMode;


/*
    Not all of the following options are supported by the module.
    Please configure according to your requirements by referring to the documentation.
*/
typedef enum
{
    QT_FTP_OPT_ACCOUNT,             // username,password, do not set this, auto set inside
    QT_FTP_OPT_FILE_TYPE,           // QtFtpFileType, default QT_FTP_FILE_TYPE_BINARY
    QT_FTP_OPT_CONTEXT_ID,          //  range 1~16, default:1
    QT_FTP_OPT_TRANMODE,            //  must be 1, do not set this, auto set inside
    QT_FTP_OPT_RSP_TIMEOUT,         // range 20~180, default:90s
    QT_FTP_OPT_SSL_TYPE,            // QtFtpSslType, default QT_FTP_SSL_TYPE_CLIENT
    QT_FTP_OPT_SSL_CTXID,           // range 0~5, default 0
    QT_FTP_OPT_SSL_DATA_ADDR,       // QtFtpDataAddressMode
    QT_FTP_OPT_REST_ENABLE,         // default true. false: Disable FTP REST command;true: Enable FTP REST command.
    QT_FTP_OPT_UNKNOWN
} QtFtpOption;

typedef struct quectel_ftp_file_info
{
    char type;                            // 'd'=dir, '-'=file, 'l'=link
    char permissions[10];                 // permission "rwxr-xr-x"）
    int links;                            // hard link count
    char owner[32];                       // owner
    char group[32];                       // group
    long size;                            // size（byte）
    char date[32];                        // modify data（如 "Jul 21 09:25"）
    char name[255];                       // name
    struct quectel_ftp_file_info *next;   // list next
} quectel_ftp_file_info_s;

typedef struct quectel_ftp
{
    at_client_t client;
    char *host;
    u16_t port;
    QtFtpErrCode err;
    QtFtpErrCode protocol_err;
    int timeout;
    osa_sem_t sem;
    char *pwd;
    char *modify_time;
    FIL file;
    size_t file_size;
    quectel_ftp_file_info_s *file_list;
    ql_SSL_Config ssl;
} quectel_ftp_s;


typedef quectel_ftp_s* quectel_ftp_t;

/**
 * @brief Initialize the FTP client instance
 * @param host FTP server host name or IP address
 * @param port FTP server port
 * @param client AT client handle used for underlying communication
 * @return quectel_ftp_t Handle to the created FTP client instance
 */
quectel_ftp_t quectel_ftp_init(const char* host, u16_t port, at_client_t client);

/**
 * @brief Set FTP client options
 * 
 * @param handle FTP client handle returned by quectel_ftp_init()
 * @param option Option type to set (see QtFtpOption enum)
 * @param ... Variable arguments depending on the option being set
 * @return true if option was set successfully, false otherwise
 */
bool quectel_ftp_setopt(quectel_ftp_t handle, QtFtpOption option, ...);

/**
 * @brief Set SSL configuration for an FTP client
 * @param handle FTP client handle returned by quectel_http_init()
 * @param config SSL configuration
 */
void quectel_ftp_set_ssl(quectel_ftp_t handle, ql_SSL_Config config);
/**
 * @brief ftp login
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param username Username, if no need, set NULL, max bytes 255
 * @param password Password, if no need, set NULL, max bytes 255
 * @return QtFtpErrCode Error code indicating request status
 */
QtFtpErrCode quectel_ftp_login(quectel_ftp_t handle, const char* username, const char* password);

///////////////////////////////////////////////////////////////////////////////
                    /* begin operate dir*/ 
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief ftp set cwd
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param path Path, max bytes 255
 * @return QtFtpErrCode Error code indicating request status
 */
QtFtpErrCode quectel_ftp_cwd(quectel_ftp_t handle, const char* path);

/*
 * @brief ftp get cwd
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param path save the current working directory
 * @return QtFtpErrCode Error code indicating request status
*/
QtFtpErrCode quectel_ftp_pwd(quectel_ftp_t handle, char* path);

/*
 * @brief ftp make directory
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param path Path, max bytes 255
 * @return QtFtpErrCode Error code indicating request status
 */
QtFtpErrCode quectel_ftp_mkdir(quectel_ftp_t handle, const char* path);

/*
 * @brief ftp remove directory
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param path Path, max bytes 255
 * @return QtFtpErrCode Error code indicating request status
*/
QtFtpErrCode quectel_ftp_rmdir(quectel_ftp_t handle, const char* path);

/*
 * @brief ftp rename file or directory
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param old_path Old path, max bytes 255
 * @param new_path New path, max bytes 255
*/
QtFtpErrCode quectel_ftp_rename(quectel_ftp_t handle, const char* old_name, const char* new_name);
/*
 * @brief ftp list directory contained type,permission,links,owner,group,size,date,name
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param path Path, max bytes 255
 * @param[out] head Pointer to the linked list head of directory entries. 
 *                  On success, stores the list of affected files/directories.
 *                  Caller must free the list using quectel_ftp_list_free().
 * @return QtFtpErrCode Error code indicating request status
*/
QtFtpErrCode quectel_ftp_list(quectel_ftp_t handle, const char *path, quectel_ftp_file_info_s **head);

/*
 * @brief ftp list directory contain name
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param path Path, max bytes 255
 * @param[out] head Pointer to the linked list head of directory entries. 
 *                  On success, stores the list of affected files/directories.
 *                  Caller must free the list using quectel_ftp_list_free().
 * @return QtFtpErrCode Error code indicating request status
*/
QtFtpErrCode quectel_ftp_nlist(quectel_ftp_t handle, const char *path, quectel_ftp_file_info_s **head);

/*
 * @brief Retrieve a machine-readable directory listing from FTP server using MLSD command
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param path Path, max bytes 255
 * @param[out] Pointer to receive structured of directory entries.
 * @return QtFtpErrCode Error code indicating request status
*/
QtFtpErrCode quectel_ftp_mlsd(quectel_ftp_t handle, const char *path, quectel_ftp_file_info_s *info);
///////////////////////////////////////////////////////////////////////////////
                    /* end operate dir*/ 
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
                    /* begin operate file*/ 
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief  get file size
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param  remote_file_name: remote file name
 * @param  file_size: file size
 * @return QtFtpErrCode Error code indicating request status
 */
QtFtpErrCode quectel_ftp_file_size(quectel_ftp_t handle, const char *remote_file_name, size_t *file_size);

/*
* @brief:  ftp upload file
* @param handle FTP client handle  returned by quectel_ftp_init()
* @param:  local_file_name: local file name
* @param:  remote_file_name: remote file name
* @return QtFtpErrCode Error code indicating request status
*/
QtFtpErrCode quectel_ftp_upload(quectel_ftp_t handle, const char *local_file_name, const char *remote_file_name);

/*
 * @brief:  ftp download file
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param:  remote_file_name: remote file name
 * @param:  local_file_name: local file name
 * @return QtFtpErrCode Error code indicating request status
*/
QtFtpErrCode quectel_ftp_download(quectel_ftp_t handle, const char *remote_file_name, const char *local_file_name);

/*
 * @brief:  ftp delete file
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param:  remote_file_name: remote file name
 * @return QtFtpErrCode Error code indicating request status
*/
QtFtpErrCode quectel_ftp_file_delete(quectel_ftp_t handle, const char *remote_file_name);

/*
 * @brief:  ftp get file modify time
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @param:  remote_file_name: remote file name
 * @param[out] time : file modify time(YYYYMMDDHHMMSS or YYYYMMDDHHMMSS.NNN)
*/
QtFtpErrCode quectel_ftp_file_get_modify_time(quectel_ftp_t handle, const char *remote_file_name, char *time);
///////////////////////////////////////////////////////////////////////////////
                    /* end operate file*/ 
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief ftp logout
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @return QtFtpErrCode Error code indicating request status
*/
QtFtpErrCode quectel_ftp_logout(quectel_ftp_t handle);

/*
 * @brief ftp uninit
 * @param handle FTP client handle  returned by quectel_ftp_init()
*/
void quectel_ftp_uninit(quectel_ftp_t handle);
/*
 * @brief Get protocol error code
 * @param handle FTP client handle  returned by quectel_ftp_init()
 * @return QtFtpErrCode protocol error code
 */
QtFtpErrCode quectel_get_protocol_err(quectel_ftp_t handle);

void quectel_ftp_list_free(quectel_ftp_file_info_s *head);
#ifdef __cplusplus
}
#endif

#endif // __QUECTEL_FTP_H__