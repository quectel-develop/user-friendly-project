#ifndef __QUECTEL_COMMON_DEF_H__
#define __QUECTEL_COMMON_DEF_H__

/**
 * @brief network error code
 */
typedef enum
{
    QT_NETWORK_OK,
    QT_NETWORK_ERR_NOINIT,
    QT_NETWORK_ERR_CONNECT,
    QT_NETWORK_ERR_CPIN,
    QT_NETWORK_ERR_ACTIVE,
    QT_NETWORK_ERR_DEACTIVE,

    QT_NETWORK_ERR_UNKNOWN
} QtNetworkErrCode;

/**
 * @brief socket error code
 */
typedef enum
{
    QT_SOCKET_OK,

    QT_SOCKET_ERR_UNKNOWN
} QtSocketStatus;

/**
 * @brief http client error code
 */
typedef enum
{
    QT_HTTP_OK,
    QT_HTTP_ERR_NOINIT,
    QT_HTTP_ERR_PARAM_INVALID,
    QT_HTTP_ERR_URL_ERROR,
    QT_HTTP_ERR_CONNECT,
    QT_HTTP_ERR_SET_URL,
    QT_HTTP_ERR_GET,
    QT_HTTP_ERR_POST,
    QT_HTTP_ERR_READ,
    QT_HTTP_ERR_POST_FILE,
    QT_HTTP_ERR_PUT,
    QT_HTTP_ERR_PUT_FILE,
    QT_HTTP_ERR_INPUT_BODY,

    
    QT_HTTP_ERR_UNKNOWN           = 701,  // Unknown/unclassified error
    QT_HTTP_ERR_TIMEOUT           = 702,  // Request timeout (server not responding)
    QT_HTTP_ERR_BUSY              = 703,  // HTTP module is busy processing another request
    QT_HTTP_ERR_UART_BUSY         = 704,  // UART communication interface is busy
    QT_HTTP_ERR_NO_REQUEST        = 705,  // No GET/POST request was initiated
    QT_HTTP_ERR_NETWORK_BUSY      = 706,  // Network is busy (e.g., ongoing data transfer)
    QT_HTTP_ERR_NETWORK_OPEN_FAIL = 707,  // Failed to establish network connection
    QT_HTTP_ERR_NETWORK_NO_CONFIG = 708,  // Network not configured (missing APN/PDP context)
    QT_HTTP_ERR_NETWORK_DEACT     = 709,  // Network is deactivated (PDP context not active)
    QT_HTTP_ERR_NETWORK_ERROR     = 710,  // Generic network-related error
    QT_HTTP_ERR_URL_INVALID       = 711,  // Malformed URL (syntax error)
    QT_HTTP_ERR_URL_EMPTY         = 712,  // Empty URL provided
    QT_HTTP_ERR_IP_INVALID        = 713,  // Invalid IP address
    QT_HTTP_ERR_DNS_FAIL          = 714,  // DNS resolution failed (hostname not found)
    QT_HTTP_ERR_SOCKET_CREATE     = 715,  // Failed to create a socket
    QT_HTTP_ERR_SOCKET_CONNECT    = 716,  // Failed to connect to the server
    QT_HTTP_ERR_SOCKET_READ       = 717   // Socket read error (data reception failed)
} QtHttpErrCode;

/**
 * @brief mqtt client error code
 */
typedef enum
{
    QT_MQTT_OK,
    QT_MQTT_ERR_NOINIT,
    QT_MQTT_ERR_OPEN,
    QT_MQTT_ERR_CONNECT,
    QT_MQTT_ERR_PUB,

    QT_MQTT_ERR_UNKNOWN
} QtMqttErrCode;

/**
 * @brief ftp client error code
 */
typedef enum
{
    QT_FTP_OK = 0,
    QT_FTP_ERR_NOINIT,
    QT_FTP_ERR_NOT_SUPPORT,
    QT_FTP_ERR_INVALID_PARAM,
    QT_FTP_ERR_OPEN,
    QT_FTP_ERR_CWD,
    QT_FTP_ERR_PWD,
    QT_FTP_ERR_MKDIR,
    QT_FTP_ERR_RMDIR,
    QT_FTP_ERR_RENAME,
    QT_FTP_ERR_LIST,
    QT_FTP_ERR_NLIST,
    QT_FTP_ERR_FILE_SIZE,
    QT_FTP_ERR_UPLOAD,
    QT_FTP_ERR_DOWNLOAD,
    QT_FTP_ERR_FILE_DELETE,
    QT_FTP_ERR_MLSD,
    QT_FTP_ERR_MDTM,
    QT_FTP_ERR_STAT,
    QT_FTP_ERR_LOGOUT,
    QT_FTP_ERR_CONNECT,

    /* 400-series errors */
    QT_FTP_RAW_SERVICE_UNAVAILABLE_CLOSING      = 421,  // Service unavailable, closing control connection
    QT_FTP_RAW_OPEN_DATA_CONNECTION_FAILED      = 425,  // Failed to open data connection
    QT_FTP_RAW_CONNECTION_CLOSED_ABORTED        = 426,  // Connection closed, transfer aborted
    QT_FTP_RAW_FILE_ACTION_NOT_TAKEN            = 450,  // Requested file action not performed
    QT_FTP_RAW_ACTION_ABORTED_LOCAL_ERROR       = 451,  // Action aborted due to local processing error
    QT_FTP_RAW_INSUFFICIENT_SYSTEM_STORAGE      = 452,  // Insufficient system storage space

    /* 500-series errors */
    QT_FTP_RAW_SYNTAX_ERROR_UNRECOGNIZED        = 500,  // Syntax error, command unrecognized
    QT_FTP_RAW_SYNTAX_ERROR_IN_PARAMETERS       = 501,  // Syntax error in parameters/arguments
    QT_FTP_RAW_COMMAND_NOT_IMPLEMENTED          = 502,  // Command not implemented
    QT_FTP_RAW_BAD_COMMAND_SEQUENCE             = 503,  // Invalid command sequence
    QT_FTP_RAW_COMMAND_PARAM_NOT_IMPLEMENTED    = 504,  // Command parameter not implemented
    QT_FTP_RAW_NOT_LOGGED_IN                    = 530,  // Not logged in (authentication required)
    QT_FTP_RAW_NEED_STORAGE_ACCOUNT             = 532,  // Need account for file storage
    QT_FTP_RAW_FILE_UNAVAILABLE                 = 550,  // Requested file unavailable
    QT_FTP_RAW_ABORTED_PAGE_TYPE_UNKNOWN        = 551,  // Action aborted: unknown page type
    QT_FTP_RAW_EXCEEDED_STORAGE_ALLOCATION      = 552,  // Exceeded storage allocation
    QT_FTP_RAW_FILENAME_NOT_ALLOWED             = 553,  // Filename not allowed

    QT_FTP_UNKNOWN_ERROR = 601,
    QT_FTP_SERVER_UNAVAILABLE,
    QT_FTP_SERVER_BUSY,
    QT_FTP_DNS_PARSE_FAILED,
    QT_FTP_NETWORK_ERROR,
    QT_FTP_CONTROL_CONN_CLOSED,
    QT_FTP_DATA_CONN_CLOSED,
    QT_FTP_SOCKET_CLOSED_BY_PEER,
    QT_FTP_TIMEOUT,
    QT_FTP_INVALID_PARAM,
    QT_FTP_FILE_OPEN_FAILED,
    QT_FTP_FILE_POS_INVALID,
    QT_FTP_FILE_ERROR,
    QT_FTP_SERVICE_UNAVAILABLE,
    QT_FTP_OPEN_DATA_CONN_FAILED,
    QT_FTP_CONN_CLOSED_ABORTED,
    QT_FTP_FILE_ACTION_NOT_TAKEN,
    QT_FTP_ACTION_ABORTED_LOCAL_ERROR,
    QT_FTP_INSUFFICIENT_STORAGE,
    QT_FTP_SYNTAX_ERROR_UNRECOGNIZED,
    QT_FTP_SYNTAX_ERROR_IN_PARAMS,
    QT_FTP_CMD_NOT_IMPLEMENTED,
    QT_FTP_BAD_CMD_SEQUENCE,
    QT_FTP_CMD_PARAM_NOT_IMPLEMENTED,
    QT_FTP_NOT_LOGGED_IN,
    QT_FTP_NEED_STORAGE_ACCOUNT,
    QT_FTP_ACTION_NOT_TAKEN,
    QT_FTP_ABORTED_PAGE_TYPE_UNKNOWN,
    QT_FTP_FILE_ACTION_ABORTED,
    QT_FTP_INVALID_FILENAME,
    QT_FTP_SSL_AUTH_FAILED
} QtFtpErrCode;

#endif 