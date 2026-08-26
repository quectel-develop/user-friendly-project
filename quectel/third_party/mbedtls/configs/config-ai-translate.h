/**
 * \file config-ai-translate.h
 * \brief Minimal Mbed TLS configuration for AI translation (HMAC-SHA1 + Base64)
 *
 *  Only enables:
 *  - SHA-1 (for HMAC)
 *  - MD (message digest interface)
 *  - Base64
 *  - Platform (memory allocation)
 *  - Error strings (optional)
 *
 *  Disables all TLS, X.509, asymmetric crypto, and other hash algorithms.
 */

#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

/* System support (required by platform) */
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_CALLOC_MALLOC
#define MBEDTLS_PLATFORM_FREE_FREE

/* Core modules for your code */
#define MBEDTLS_SHA1_C          /* SHA-1 algorithm */
#define MBEDTLS_MD_C            /* Message Digest interface (needed for HMAC) */
#define MBEDTLS_BASE64_C        /* Base64 encoding */

/* Optional but recommended for debugging */
#define MBEDTLS_ERROR_C         /* Error code strings (for mbedtls_strerror) */

/* Disable everything else */
/* No ASN.1, no PEM, no X.509, no TLS, no RSA/ECC, no other hashes */

/* Silence some platform warnings */
#define MBEDTLS_HAVE_ASM        /* Optional, speeds up some operations */

#include "mbedtls/check_config.h"

#endif /* MBEDTLS_CONFIG_H */