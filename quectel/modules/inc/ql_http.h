/**
 * @file ql_http.h 
 * @brief Quectel http interface definitions
 */

#ifndef __QL_HTTP_H__
#define __QL_HTTP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdbool.h"
#include  "qosa_system.h"
#include "at.h"
#include "ql_ssl.h"
#include "ql_common_def.h"

// Content types
typedef enum
{
    QL_HTTP_CONTENT_URLENCODED = 0,
    QL_HTTP_CONTENT_PLAIN_TEXT = 1,
    QL_HTTP_CONTENT_OCTET_STREAM = 2,
    QL_HTTP_CONTENT_MULTIPART = 3,
    QL_HTTP_CONTENT_JSON = 4,
    QL_HTTP_CONTENT_JPEG = 5
} QL_HTTP_CONTENT_TYPE_E;

// file type
typedef enum
{
    QL_HTTP_FILE_BOTH_HEADERS_BODY = 0, // if request_header is false, file contains body; else file contains headers and body 
    QL_HTTP_FILE_HEADERS = 1, // request_header must be true
    QL_HTTP_FILE_BODY = 2 // request_header must be true
} QL_HTTP_FILE_TYPE_E;

/*
    Not all of the following options are supported by the module.
    Please configure according to your requirements by referring to the documentation.
*/
typedef enum
{
    QL_HTTP_OPT_CONTEXT_ID,          // range 1~16, default:1
    QL_HTTP_OPT_REQUEST_HEADER,      // bool, default false
    QL_HTTP_OPT_RESPONSE_HEADER,     // bool, default false
    QL_HTTP_OPT_SSL_CONTEXT_ID,      // range 0~5, default 0
    QL_HTTP_OPT_CONTENT_TYPE,        // QL_HTTP_CONTENT_TYPE_E http body data type, default QL_HTTP_CONTENT_URLENCODED
    QL_HTTP_OPT_CUSTOM_HEADER,       // const char*
    QL_HTTP_OPT_TIMEOUT,             // uint32_t (seconds)
    QL_HTTP_OPT_WAIT_TIME,           // uint32_t (seconds)
    QL_HTTP_OPT_FILE_TYPE,           // QL_HTTP_FILE_TYPE_E
    QL_HTTP_OPT_WRITE_FUNCTION,      // callback for handling received response data, type: http_user_write_callback
    QL_HTTP_OPT_WRITE_DATA,          // void*, user-defined data passed to write callback
    QL_HTTP_OPT_READ_FUNCTION,       // callback for providing data to be sent in request, type: http_user_read_callback
    QL_HTTP_OPT_READ_DATA,           // void*, user-defined data passed to read callback
    QL_HTTP_OPT_UNKNOWN
} QL_HTTP_OPTION_E;

// HTTP methods
typedef enum
{
    QL_HTTP_METHORD_GET,
    QL_HTTP_METHORD_POST,
    QL_HTTP_METHORD_POST_FILE,  // not implemented
    QL_HTTP_METHORD_PUT,
    QL_HTTP_METHORD_PUT_FILE,  // not implemented
} QL_HTTP_METHOD_E;

typedef enum
{
    QL_HTTP_USR_EVENT_START = 0,
    QL_HTTP_USR_EVENT_HEADER_DATA,
    QL_HTTP_USR_EVENT_BODY_DATA,
    QL_HTTP_USR_EVENT_END

} QL_HTTP_USR_EVENT_E;

typedef void (*http_user_write_callback)(QL_HTTP_USR_EVENT_E, const char *, size_t, void *);
typedef size_t (*http_user_read_callback)(char *, size_t, void *);
typedef struct ql_http
{
    at_client_t client;
    uint8_t context_id;
    uint8_t ssl_context_id;
    bool request_header;
    bool response_header;
    u32_t timeout;
    u32_t wait_time;
    QL_HTTP_FILE_TYPE_E file_type;
    osa_sem_t sem;
    int content_length;
    int err_code;
    int rsp_code;
    http_user_write_callback usr_write_cb;
    http_user_read_callback  usr_read_cb;
    void* user_write_data;
    void* user_read_data;
    char* data;
    int32_t content_left;
    ql_SSL_Config ssl;
} ql_http_s;


typedef ql_http_s* ql_http_t;

/**
 * @brief Initialize the HTTP client instance
 * 
 * @param client AT client handle used for underlying communication
 * @return ql_http_t Handle to the created HTTP client instance
 */
ql_http_t ql_http_init(at_client_t client);

/**
 * @brief Set HTTP client options
 * 
 * @param handle HTTP client handle returned by ql_http_init()
 * @param option Option type to set (see QL_HTTP_OPTION_E enum)
 * @param ... Variable arguments depending on the option being set
 * @return true if option was set successfully, false otherwise
 */
bool ql_http_setopt(ql_http_t handle, QL_HTTP_OPTION_E option, ...);

/**
 * @brief Set SSL configuration for an HTTP client
 * @param handle HTTP client handle returned by ql_http_init()
 * @param config SSL configuration
 */
void ql_http_set_ssl(ql_http_t handle, ql_SSL_Config config);

/**
 * @brief Send an HTTP request
 * 
 * @param handle HTTP client handle
 * @param url URL to send request to
 * @param method HTTP method (GET, POST, etc.)
 * @param data Request body data (can be NULL for methods like GET)
 * @param data_len Length of request body data
 * @return QL_HTTP_ERR_CODE_E Error code indicating request status
 */
QL_HTTP_ERR_CODE_E ql_http_request(ql_http_t handle, const char* url, QL_HTTP_METHOD_E method, const char* data, size_t data_len);

/**
 * @brief Receive HTTP response data
 * this function only can be called when user callback NULL
 * 
 * @param handle HTTP client handle
 * @param buf Buffer to store received data
 * @param size Size of the buffer
 * @return int Number of bytes actually received, or negative value for error
 */
int ql_http_recv(ql_http_t handle, char* buf, size_t size);

/**
 * @brief Deinitialize and cleanup the HTTP client instance
 * 
 * @param handle HTTP client handle to be deinitialized (returned by ql_http_init())
 * @note After calling this function, the handle becomes invalid and should not be used
 */
void ql_http_deinit(ql_http_t handle);

#ifdef __cplusplus
}
#endif

#endif // __QL_HTTP_H__