#ifndef QUECTEL_PROJECT_VERSION
#define QUECTEL_PROJECT_VERSION "This version generated automatically if use build.bat/build.sh"
#endif
#ifndef QUECTEL_BUILD_ENV
#define QUECTEL_BUILD_ENV "This version generated automatically if use build.bat/build.sh"
#endif

/**********************************************************
             Support Features Enable/Disable
**********************************************************/
#define __QUECTEL_UFP_FEATURE_SUPPORT_NETWORK__
#define __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET__
#define __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_SERVER__
#define __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_TCP_CLIENT__
#define __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_UDP_SERVER__
#define __QUECTEL_UFP_FEATURE_SUPPORT_SOCKET_UDP_CLIENT__

#define __QUECTEL_UFP_FEATURE_SUPPORT_PSM__
#define __QUECTEL_UFP_FEATURE_SUPPORT_TFCARD__
#define __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__  /* Must  enable __QUECTEL_UFP_FEATURE_SUPPORT_TFCARD__ */
#define __QUECTEL_UFP_FEATURE_SUPPORT_SSL__         /* Must  enable __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__*/

#define __QUECTEL_UFP_FEATURE_SUPPORT_FTP_S__
#define __QUECTEL_UFP_FEATURE_SUPPORT_HTTP_S__
#define __QUECTEL_UFP_FEATURE_SUPPORT_MQTT_S__

#define __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_PRINT__
#define __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_SAVE__  /* Must  enable __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_PRINT__  and  __QUECTEL_UFP_FEATURE_SUPPORT_TFCARD__*/
#define __QUECTEL_UFP_FEATURE_SUPPORT_DEBUG_SHELL__

/**********************************************************
                CLI Test Enable/Disable
**********************************************************/
#define __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
