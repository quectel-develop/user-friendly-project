#ifndef __QUECTEL_COMMON_DEF_H__
#define __QUECTEL_COMMON_DEF_H__

/**
 * @brief network error code
 */
typedef enum
{
    QL_NET_OK,
    QL_NET_ERR_NOINIT,
    QL_NET_ERR_CONNECT,
    QL_NET_ERR_SIM,
    QL_NET_ERR_REGISTER,
    QL_NET_ERR_ACTIVE,
    QL_NET_ERR_DEACTIVE,

    QL_NET_ERR_UNKNOWN
} QL_NET_ERR_CODE_E;

/**
 * @brief http client error code
 */
typedef enum
{
    QL_HTTP_OK,
    QL_HTTP_ERR_NOINIT,
    QL_HTTP_ERR_PARAM_INVALID,
    QL_HTTP_ERR_SSL_CONFIG,
    QL_HTTP_ERR_URL_ERROR,
    QL_HTTP_ERR_CONNECT,
    QL_HTTP_ERR_SET_URL,
    QL_HTTP_ERR_GET,
    QL_HTTP_ERR_POST,
    QL_HTTP_ERR_READ,
    QL_HTTP_ERR_POST_FILE,
    QL_HTTP_ERR_PUT,
    QL_HTTP_ERR_PUT_FILE,
    QL_HTTP_ERR_INPUT_BODY,

    
    QL_HTTP_ERR_UNKNOWN           = 701,  // Unknown/unclassified error
    QL_HTTP_ERR_TIMEOUT           = 702,  // Request timeout (server not responding)
    QL_HTTP_ERR_BUSY              = 703,  // HTTP module is busy processing another request
    QL_HTTP_ERR_UART_BUSY         = 704,  // UART communication interface is busy
    QL_HTTP_ERR_NO_REQUEST        = 705,  // No GET/POST request was initiated
    QL_HTTP_ERR_NETWORK_BUSY      = 706,  // Network is busy (e.g., ongoing data transfer)
    QL_HTTP_ERR_NETWORK_OPEN_FAIL = 707,  // Failed to establish network connection
    QL_HTTP_ERR_NETWORK_NO_CONFIG = 708,  // Network not configured (missing APN/PDP context)
    QL_HTTP_ERR_NETWORK_DEACT     = 709,  // Network is deactivated (PDP context not active)
    QL_HTTP_ERR_NETWORK_ERROR     = 710,  // Generic network-related error
    QL_HTTP_ERR_URL_INVALID       = 711,  // Malformed URL (syntax error)
    QL_HTTP_ERR_URL_EMPTY         = 712,  // Empty URL provided
    QL_HTTP_ERR_IP_INVALID        = 713,  // Invalid IP address
    QL_HTTP_ERR_DNS_FAIL          = 714,  // DNS resolution failed (hostname not found)
    QL_HTTP_ERR_SOCKET_CREATE     = 715,  // Failed to create a socket
    QL_HTTP_ERR_SOCKET_CONNECT    = 716,  // Failed to connect to the server
    QL_HTTP_ERR_SOCKET_READ       = 717   // Socket read error (data reception failed)
} QL_HTTP_ERR_CODE_E;

/**
 * @brief mqtt client error code
 */
typedef enum
{
    QL_MQTT_OK,
    QL_MQTT_ERR_NOINIT,
    QL_MQTT_ERR_OPEN,
    QL_MQTT_ERR_CONNECT,
    QL_MQTT_ERR_PUB,
    QL_MQTT_ERR_SUB,

    QL_MQTT_ERR_UNKNOWN
} QL_MQTT_ERR_CODE_E;

/**
 * @brief ftp client error code
 */
typedef enum
{
    QL_FTP_OK = 0,
    QL_FTP_ERR_NOINIT,
    QL_FTP_ERR_NOT_SUPPORT,
    QL_FTP_ERR_INVALID_PARAM,
    QL_FTP_ERR_SSL_CONFIG,
    QL_FTP_ERR_OPEN,
    QL_FTP_ERR_CWD,
    QL_FTP_ERR_PWD,
    QL_FTP_ERR_MKDIR,
    QL_FTP_ERR_RMDIR,
    QL_FTP_ERR_RENAME,
    QL_FTP_ERR_LIST,
    QL_FTP_ERR_NLIST,
    QL_FTP_ERR_FILE_SIZE,
    QL_FTP_ERR_UPLOAD,
    QL_FTP_ERR_DOWNLOAD,
    QL_FTP_ERR_FILE_DELETE,
    QL_FTP_ERR_MLSD,
    QL_FTP_ERR_MDTM,
    QL_FTP_ERR_STAT,
    QL_FTP_ERR_LOGOUT,
    QL_FTP_ERR_CONNECT,

    /* 400-series errors */
    QL_FTP_RAW_SERVICE_UNAVAILABLE_CLOSING      = 421,  // Service unavailable, closing control connection
    QL_FTP_RAW_OPEN_DATA_CONNECTION_FAILED      = 425,  // Failed to open data connection
    QL_FTP_RAW_CONNECTION_CLOSED_ABORTED        = 426,  // Connection closed, transfer aborted
    QL_FTP_RAW_FILE_ACTION_NOT_TAKEN            = 450,  // Requested file action not performed
    QL_FTP_RAW_ACTION_ABORTED_LOCAL_ERROR       = 451,  // Action aborted due to local processing error
    QL_FTP_RAW_INSUFFICIENT_SYSTEM_STORAGE      = 452,  // Insufficient system storage space

    /* 500-series errors */
    QL_FTP_RAW_SYNTAX_ERROR_UNRECOGNIZED        = 500,  // Syntax error, command unrecognized
    QL_FTP_RAW_SYNTAX_ERROR_IN_PARAMETERS       = 501,  // Syntax error in parameters/arguments
    QL_FTP_RAW_COMMAND_NOT_IMPLEMENTED          = 502,  // Command not implemented
    QL_FTP_RAW_BAD_COMMAND_SEQUENCE             = 503,  // Invalid command sequence
    QL_FTP_RAW_COMMAND_PARAM_NOT_IMPLEMENTED    = 504,  // Command parameter not implemented
    QL_FTP_RAW_NOT_LOGGED_IN                    = 530,  // Not logged in (authentication required)
    QL_FTP_RAW_NEED_STORAGE_ACCOUNT             = 532,  // Need account for file storage
    QL_FTP_RAW_FILE_UNAVAILABLE                 = 550,  // Requested file unavailable
    QL_FTP_RAW_ABORTED_PAGE_TYPE_UNKNOWN        = 551,  // Action aborted: unknown page type
    QL_FTP_RAW_EXCEEDED_STORAGE_ALLOCATION      = 552,  // Exceeded storage allocation
    QL_FTP_RAW_FILENAME_NOT_ALLOWED             = 553,  // Filename not allowed

    QL_FTP_UNKNOWN_ERROR = 601,
    QL_FTP_SERVER_UNAVAILABLE,
    QL_FTP_SERVER_BUSY,
    QL_FTP_DNS_PARSE_FAILED,
    QL_FTP_NETWORK_ERROR,
    QL_FTP_CONTROL_CONN_CLOSED,
    QL_FTP_DATA_CONN_CLOSED,
    QL_FTP_SOCKET_CLOSED_BY_PEER,
    QL_FTP_TIMEOUT,
    QL_FTP_INVALID_PARAM,
    QL_FTP_FILE_OPEN_FAILED,
    QL_FTP_FILE_POS_INVALID,
    QL_FTP_FILE_ERROR,
    QL_FTP_SERVICE_UNAVAILABLE,
    QL_FTP_OPEN_DATA_CONN_FAILED,
    QL_FTP_CONN_CLOSED_ABORTED,
    QL_FTP_FILE_ACTION_NOT_TAKEN,
    QL_FTP_ACTION_ABORTED_LOCAL_ERROR,
    QL_FTP_INSUFFICIENT_STORAGE,
    QL_FTP_SYNTAX_ERROR_UNRECOGNIZED,
    QL_FTP_SYNTAX_ERROR_IN_PARAMS,
    QL_FTP_CMD_NOT_IMPLEMENTED,
    QL_FTP_BAD_CMD_SEQUENCE,
    QL_FTP_CMD_PARAM_NOT_IMPLEMENTED,
    QL_FTP_NOT_LOGGED_IN,
    QL_FTP_NEED_STORAGE_ACCOUNT,
    QL_FTP_ACTION_NOT_TAKEN,
    QL_FTP_ABORTED_PAGE_TYPE_UNKNOWN,
    QL_FTP_FILE_ACTION_ABORTED,
    QL_FTP_INVALID_FILENAME,
    QL_FTP_SSL_AUTH_FAILED
} QL_FTP_ERR_CODE_E;

/**
 * @brief socket error code
 */
typedef enum
{
    QL_SOCKET_OK =0,
    QL_SOCKET_ERR_UNKNOWN            = 550,  ///< Unknown/unclassified error
    QL_SOCKET_ERR_OPERATION_BLOCKED  = 551,  ///< Operation blocked by current state
    QL_SOCKET_ERR_INVALID_PARAMS     = 552,  ///< Invalid parameters provided
    QL_SOCKET_ERR_MEMORY_NOT_ENOUGH  = 553,  ///< Insufficient system memory
    QL_SOCKET_ERR_CREATE_FAIL        = 554,  ///< Failed to create socket
    QL_SOCKET_ERR_OP_NOT_SUPPORTED   = 555,  ///< Operation not supported
    QL_SOCKET_ERR_BIND_FAIL          = 556,  ///< Failed to bind socket
    QL_SOCKET_ERR_LISTEN_FAIL        = 557,  ///< Failed to listen on socket
    QL_SOCKET_ERR_WRITE_FAIL         = 558,  ///< Socket write operation failed
    QL_SOCKET_ERR_READ_FAIL          = 559,  ///< Socket read operation failed
    QL_SOCKET_ERR_ACCEPT_FAIL        = 560,  ///< Failed to accept incoming connection
    QL_SOCKET_ERR_PDP_OPEN_FAIL      = 561,  ///< Failed to activate PDP context
    QL_SOCKET_ERR_PDP_CLOSE_FAIL     = 562,  ///< Failed to deactivate PDP context
    QL_SOCKET_ERR_FD_USED            = 563,  ///< Socket identifier already in use
    QL_SOCKET_ERR_DNS_BUSY           = 564,  ///< DNS resolver busy
    QL_SOCKET_ERR_DNS_PARSE_FAIL     = 565,  ///< DNS resolution failed
    QL_SOCKET_ERR_CONNECT_FAIL       = 566,  ///< Failed to establish connection
    QL_SOCKET_ERR_CLOSED             = 567,  ///< Socket already closed
    QL_SOCKET_ERR_OPERATION_BUSY     = 568,  ///< Resource busy (try again later)
    QL_SOCKET_ERR_OPERATION_TIMEOUT  = 569,  ///< Operation timed out
    QL_SOCKET_ERR_PDP_BROKEN_DOWN    = 570,  ///< PDP context terminated unexpectedly
    QL_SOCKET_ERR_CANCEL_SENDING     = 571,  ///< Data transmission canceled
    QL_SOCKET_ERR_OP_NOT_ALLOWED     = 572,  ///< Operation not permitted
    QL_SOCKET_ERR_APN_NOT_CONFIGURED = 573,  ///< APN configuration missing
    QL_SOCKET_ERR_PORT_BUSY          = 574   ///< Network port unavailable
} QL_SOCKET_ERR_CODE_E;

#endif 