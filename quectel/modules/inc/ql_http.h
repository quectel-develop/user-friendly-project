/**
 * @file quectel_http.h 
 * @brief Quectel http interface definitions
 */

#ifndef __QUECTEL_HTTP_H__
#define __QUECTEL_HTTP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdbool.h"
#include  "qosa_system.h"
#include "at.h"
#include "ql_ssl.h"
#include "quectel_common_def.h"

// Content types
typedef enum
{
    QT_HTTP_CONTENT_URLENCODED = 0,
    QT_HTTP_CONTENT_PLAIN_TEXT = 1,
    QT_HTTP_CONTENT_OCTET_STREAM = 2,
    QT_HTTP_CONTENT_MULTIPART = 3,
    QT_HTTP_CONTENT_JSON = 4,
    QT_HTTP_CONTENT_JPEG = 5
} QtHttpContentType;

// file type
typedef enum
{
    QT_HTTP_FILE_BOTH_HEADERS_BODY = 0, // if request_header is false, file contains body; else file contains headers and body 
    QT_HTTP_FILE_HEADERS = 1, // request_header must be true
    QT_HTTP_FILE_BODY = 2 // request_header must be true
} QtHttpFileType;

/*
    Not all of the following options are supported by the module.
    Please configure according to your requirements by referring to the documentation.
*/
typedef enum
{
    QT_HTTP_OPT_CONTEXT_ID,          // uint8_t (1-16)
    QT_HTTP_OPT_REQUEST_HEADER,      // bool
    QT_HTTP_OPT_RESPONSE_HEADER,     // bool
    QT_HTTP_OPT_SSL_CONTEXT_ID,      // uint8_t (0-5)
    QT_HTTP_OPT_CONTENT_TYPE,        // QT_HTTP_OPT_CONTEXT_ID
    QT_HTTP_OPT_AUTO_RESPONSE,       // bool
    QT_HTTP_OPT_CLOSE_INDICATION,    // bool
    QT_HTTP_OPT_WINDOW_SIZE,         // uint32_t
    QT_HTTP_OPT_CLOSE_WAIT_TIME,     // uint32_t (ms)
    QT_HTTP_OPT_AUTH,                // const char*
    QT_HTTP_OPT_CUSTOM_HEADER,       // const char*
    QT_HTTP_OPT_TIMEOUT,             // uint32_t (seconds)
    QT_HTTP_OPT_FILE_TYPE,           // QtHttpFileType
    QT_HTTP_OPT_VERBOSE,             // bool
    QT_HTTP_OPT_FOLLOW_REDIRECTS,    // bool
    QT_HTTP_OPT_MAX_REDIRECTS,       // uint8_t
    QT_HTTP_OPT_CA_INFO,             // const char* (path to CA cert)
    QT_HTTP_OPT_CLIENT_CERT,         // const char* (path to client cert)
    QT_HTTP_OPT_CLIENT_KEY,          // const char* (path to client key)
    QT_HTTP_OPT_WRITE_FUNCTION,      // callback for writing received data
    QT_HTTP_OPT_WRITE_DATA,          // void* user data for write callback

    QT_HTTP_OPT_UNKNOWN
} QtHttpOption;

// HTTP methods
typedef enum
{
    QT_HTTP_METHORD_GET,
    QT_HTTP_METHORD_POST,
    QT_HTTP_METHORD_POST_FILE,  // not implemented
    QT_HTTP_METHORD_PUT,
    QT_HTTP_METHORD_PUT_FILE,  // not implemented
} QtHttpMethod;

typedef enum
{
    QT_HTTP_EVENT_REQUEST_OK = 0,
    QT_HTTP_EVENT_REQUEST_FAIL,
    QT_HTTP_EVENT_READ,
    QT_HTTP_EVENT_READ_FAIL,
    QT_HTTP_EVENT_RECV_URC,
} QtHttpEvent;

typedef enum
{
    QT_HTTP_USR_EVENT_START = 0,
    QT_HTTP_USR_EVENT_DATA,
    QT_HTTP_USR_EVENT_END,

} QtHttpUsrEvent;

typedef void (*user_callback)(QtHttpUsrEvent, const char *, size_t, void *);
typedef struct quectel_http
{
    at_client_t client;
    uint8_t context_id;
    uint8_t ssl_context_id;
    bool request_header;
    bool response_header;
    bool auto_outrsp;
    bool close_ind;
    uint32_t window_size;
    uint32_t close_wait_time;
    QtHttpContentType content_type;
    u32_t timeout;
    QtHttpFileType file_type;
    QtHttpEvent event;
    osa_sem_t sem;
    int content_length;
    int err_code;
    int rsp_code;
    user_callback usr_cb;
    void* user_data;
    char* data;
    int32_t content_left;
    ql_SSL_Config ssl;
} quectel_http_s;


typedef quectel_http_s* quectel_http_t;

/**
 * @brief Initialize the HTTP client instance
 * 
 * @param client AT client handle used for underlying communication
 * @return quectel_http_t Handle to the created HTTP client instance
 */
quectel_http_t quectel_http_init(at_client_t client);

/**
 * @brief Set HTTP client options
 * 
 * @param handle HTTP client handle returned by quectel_http_init()
 * @param option Option type to set (see QtHttpOption enum)
 * @param ... Variable arguments depending on the option being set
 * @return true if option was set successfully, false otherwise
 */
bool quectel_http_setopt(quectel_http_t handle, QtHttpOption option, ...);

/**
 * @brief Set SSL configuration for an HTTP client
 * @param handle HTTP client handle returned by quectel_http_init()
 * @param config SSL configuration
 */
void quectel_http_set_ssl(quectel_http_t handle, ql_SSL_Config config);

/**
 * @brief Send an HTTP request
 * 
 * @param handle HTTP client handle
 * @param url URL to send request to
 * @param method HTTP method (GET, POST, etc.)
 * @param data Request body data (can be NULL for methods like GET)
 * @return QtHttpErrCode Error code indicating request status
 */
QtHttpErrCode quectel_http_request(quectel_http_t handle, const char* url, QtHttpMethod method, const char* data);

/**
 * @brief Receive HTTP response data
 * this function only can be called when user callback NULL
 * 
 * @param handle HTTP client handle
 * @param buf Buffer to store received data
 * @param size Size of the buffer
 * @return int Number of bytes actually received, or negative value for error
 */
int quectel_http_recv(quectel_http_t handle, char* buf, size_t size);

/**
 * @brief Deinitialize and cleanup the HTTP client instance
 * 
 * @param handle HTTP client handle to be deinitialized (returned by quectel_http_init())
 * @note After calling this function, the handle becomes invalid and should not be used
 */
void quectel_http_deinit(quectel_http_t handle);

#ifdef __cplusplus
}
#endif

#endif // __QUECTEL_HTTP_H__