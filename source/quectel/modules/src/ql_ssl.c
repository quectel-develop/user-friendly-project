#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_SSL__
#include <stdarg.h>
#include "ql_file.h"
#include "qosa_log.h"
#include "ql_ssl.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#include "windows.h"
#elif __linux__
#else
#include "ff.h"
#endif

// Function to send the AT+QSSLCFG command
static int ql_sslcfg_set(at_client_t client, ql_sslcfg_type type, u8_t ssl_ctx_id, ...)
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
        LOG_E("Unsupported configuration type");
        return -1;
    }
    va_end(args);

    // Create response object
    at_response_t resp = at_create_resp_new(128, 0, 500, NULL);
    if (resp == NULL)
    {
        LOG_E("Unable to create response object");
        return -1;
    }

    // Send the AT command
    if (at_obj_exec_cmd(client, resp, cmd) < 0)
    {
        LOG_E("AT command execution failed");
        at_delete_resp(resp);
        return -1;
    }

    // Delete the response object
    at_delete_resp(resp);
    return 0;
}

static int ql_processFile(at_client_t client, const char *local,  const char *remote, bool is_path)
{
    if (NULL == local || NULL == remote)
        return -1;
    if (is_path)
        return ql_file_upload(client, local, remote);
    QL_FILE file = ql_fopen(remote, QL_FILE_MODE_CREATE_OR_TRUNCATE);
    if (NULL == file)
        return -1;
    int ret = ql_fwrite(local, 1, strlen(local), file);
    if (ret < 0)
    {
        ql_fclose(file);
        return -1;
    }
    ql_fclose(file);
    return 0;
}

// Function to configure SSL for FTP
int configure_ssl(ql_SSL_Config *config)
{
    if (!config->sslenble)
        return 0;
    LOG_V("config->ciphersuite  0x%x",config->ciphersuite);
    ql_sslcfg_set(config->client, QL_SSLCFG_CIPHERSUITE, config->sslctxid, config->ciphersuite);
    ql_sslcfg_set(config->client, QL_SSLCFG_SECLEVEL, config->sslctxid, config->seclevel);
    ql_sslcfg_set(config->client, QL_SSLCFG_SSLVERSION, config->sslctxid, config->sslversion);
    if (0 == config->seclevel || config->seclevel > 2)
        return 0;
    if (ql_processFile(config->client, config->cacert_src, config->cacert_dst_path, config->src_is_path) != 0)
    {
        LOG_E("[ca.pem] uploaded to UFS Fail");
        return -1;
    }
    LOG_I("[UFS:ca.pem] upload done");
    ql_sslcfg_set(config->client, QL_SSLCFG_CACERT, config->sslctxid, config->cacert_dst_path);
    if (config->seclevel == 2)
    {
        if (ql_processFile(config->client, config->clientcert_src, config->clientcert_dst_path, config->src_is_path) != 0)
        {
            LOG_E("[user.pem] uploaded to UFS Fail");
            return -1;
        }
        LOG_I("[UFS:user.pem] upload done");
        ql_sslcfg_set(config->client, QL_SSLCFG_CLIENTCERT, config->sslctxid, config->clientcert_dst_path);
        if (ql_processFile(config->client, config->clientkey_src, config->clientkey_dst_path, config->src_is_path) != 0)
        {
            LOG_E("[user_key.pem] uploaded to UFS Fail");
            return -1;
        }
        LOG_I("[UFS:user_key.pem] upload done");

        ql_sslcfg_set(config->client, QL_SSLCFG_CLIENTKEY, config->sslctxid, config->clientkey_dst_path);
        ql_sslcfg_set(config->client, QL_SSLCFG_IGNORELOCALTIME, config->sslctxid, 1);
    }
    return 0; // Return success
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_SSL__ */

