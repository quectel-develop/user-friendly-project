#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SSL__
#include <at.h>
#include "ql_ssl.h"
#include "ql_net.h"
#include "debug_service.h"
#include "broadcast_service.h"
#include "at_socket_device.h"
#include "ql_fs.h"
#include "qosa_log.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#include "windows.h"
#elif __linux__
#else
#include "cmsis_os.h"
#include "ff.h"
#endif

// Function to send the AT+QSSLCFG command
int ql_sslcfg_set(ql_sslcfg_type type, u8_t ssl_ctx_id, ...)
{
    char cmd[128];
    va_list args;
    va_start(args, ssl_ctx_id);

    // Construct AT command based on the enum type
    switch (type)
    {
    case QL_SSLCFG_SSLVERSION:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"sslversion\",%d,%d", ssl_ctx_id, va_arg(args, int));
        break;
    case QL_SSLCFG_CIPHERSUITE:

        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"ciphersuite\",%d,0x%04x", ssl_ctx_id, va_arg(args, int));
        break;
    case QL_SSLCFG_CACERT:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"cacert\",%d,\"%s\"", ssl_ctx_id, va_arg(args, const char *));
        break;
    case QL_SSLCFG_CLIENTCERT:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"clientcert\",%d,\"%s\"", ssl_ctx_id, va_arg(args, const char *));
        break;
    case QL_SSLCFG_CLIENTKEY:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"clientkey\",%d,\"%s\"", ssl_ctx_id, va_arg(args, const char *));
        break;
    case QL_SSLCFG_SECLEVEL:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"seclevel\",%d,%d", ssl_ctx_id, va_arg(args, int));
        break;
    case QL_SSLCFG_SESSION:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"session\",%d,%d", ssl_ctx_id, va_arg(args, int));
        break;
    case QL_SSLCFG_SNI:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"sni\",%d,\"%s\"", ssl_ctx_id, va_arg(args, const char *));
        break;
    case QL_SSLCFG_CHECKHOST:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"checkhost\",%d,%d", ssl_ctx_id, va_arg(args, int));
        break;
    case QL_SSLCFG_IGNORELOCALTIME:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"ignorelocaltime\",%d,%d", ssl_ctx_id, va_arg(args, int));
        break;
    case QL_SSLCFG_NEGOTIATETIME:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"negotiatetime\",%d,%d", ssl_ctx_id, va_arg(args, int));
        break;
    case QL_SSLCFG_RENEGOTIATION:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"renegotiation\",%d,%d", ssl_ctx_id, va_arg(args, int));
        break;
    case QL_SSLCFG_DTLS:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"dtls\",%d,%d", ssl_ctx_id, va_arg(args, int));
        break;
    case QL_SSLCFG_DTLSVERSION:
        snprintf(cmd, sizeof(cmd), "AT+QSSLCFG=\"dtlsversion\",%d,%d", ssl_ctx_id, va_arg(args, int));
        break;
    default:
        va_end(args);
        LOG_E("Unsupported configuration type\n");
        return -1;
    }
    va_end(args);

    // Create response object
    at_response_t resp = at_create_resp(128, 0, 500);
    if (resp == NULL)
    {
        LOG_E("Unable to create response object.\n");
        return -1;
    }

    // Send the AT command
    if (at_exec_cmd(resp, cmd) < 0)
    {
        LOG_E("AT command execution failed.\n");
        at_delete_resp(resp);
        return -1;
    }

    // Delete the response object
    at_delete_resp(resp);
    return 0;
}

int ql_processFile(const char *fileName)
{
    FILINFO fileInfo;
    FRESULT result;
    char fullPath[30];

    result = f_stat(fileName, &fileInfo);
    if (result == QOSA_OK)
    {
        LOG_I("[%s] Found in SD Card. File size: %lu bytes", fileName, fileInfo.fsize);

        snprintf(fullPath, sizeof(fullPath), "0:%s", fileName);
        ql_file_put_ex(fullPath, fileName, fileInfo.fsize);
        return 0;
    }
    else if (result == FR_NO_FILE)
    {

        LOG_E("File does not exist.\n");
        return -1;
    }
    else
    {

        LOG_E("Error occurred: %d\n", result);
        return -1;
    }
}

// Function to configure SSL for FTP
int configure_ssl(ql_SSL_Config *config)
{
    if (config->sslenble)
    {
        // SSL is enabled, configure SSL settings using ql_sslcfg_set function
        // Example:
        LOG_V("config->ciphersuite  0x%x\n",config->ciphersuite);
        ql_sslcfg_set(QL_SSLCFG_CIPHERSUITE, config->sslctxid, config->ciphersuite);
        ql_sslcfg_set(QL_SSLCFG_SECLEVEL, config->sslctxid, config->seclevel);
        ql_sslcfg_set(QL_SSLCFG_SSLVERSION, config->sslctxid, config->sslversion);
        //    ql_file_del();
        if (config->seclevel == 1)
        {
            // File_Moudle_Info fileList[1];
            // if (ql_module_list_get("ca.pem", fileList, sizeof(fileList) / sizeof(fileList[0]), 1) != 0)
            {
                if (ql_processFile(config->cacert_path) != 0)
                {
                    LOG_E("[ca.pem] uploaded to UFS Fail.");
                    return -1;
                }
                LOG_I("[UFS:ca.pem] upload done.");
            }
            ql_sslcfg_set(QL_SSLCFG_CACERT, config->sslctxid, config->cacert_path);
        }
        else if (config->seclevel == 2)
        {
            // File_Moudle_Info fileList[1];
            // if (ql_module_list_get("ca.pem", fileList, sizeof(fileList) / sizeof(fileList[0]), 1) != 0)
            {
                if (ql_processFile(config->cacert_path) != 0)
                {
                    LOG_E("[ca.pem] uploaded to UFS Fail.");
                    return -1;
                }
                LOG_I("[UFS:ca.pem] upload done.");
            }
            // if (ql_module_list_get("user.pem", fileList, sizeof(fileList) / sizeof(fileList[0]), 1) != 0)
            {
                if (ql_processFile(config->clientcert_path) != 0)
                {
                    LOG_E("[user.pem] uploaded to UFS Fail.");
                    return -1;
                }
                LOG_I("[UFS:user.pem] upload done.");
            }
            // if (ql_module_list_get("user_key.pem", fileList, sizeof(fileList) / sizeof(fileList[0]), 1) != 0)
            {
                if (ql_processFile(config->clientkey_path) != 0)
                {
                    LOG_E("[user_key.pem] uploaded to UFS Fail.");
                    return -1;
                }
                LOG_I("[UFS:user_key.pem] upload done.");
            }

            ql_sslcfg_set(QL_SSLCFG_CACERT, config->sslctxid, config->cacert_path);
            ql_sslcfg_set(QL_SSLCFG_CLIENTCERT, config->sslctxid, config->clientcert_path);
            ql_sslcfg_set(QL_SSLCFG_CLIENTKEY, config->sslctxid, config->clientkey_path);
            ql_sslcfg_set(QL_SSLCFG_IGNORELOCALTIME, config->sslctxid, 1);
        }
        // Add more SSL configurations as needed
    }
    return 0; // Return success
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SSL__ */

