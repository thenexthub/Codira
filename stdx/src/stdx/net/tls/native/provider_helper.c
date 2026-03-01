/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <openssl/bn.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>
#include "provider.h"
#include "opensslSymbols.h"
#include "securec.h"
#include "api.h"

__attribute__((visibility("hidden"))) unsigned int GetEcOrderBitsFromGroup(const char* groupName)
{
    if (!groupName) {
        return 0;
    }
    if (!strcasecmp(groupName, "P-256") || !strcasecmp(groupName, "p256") || !strcasecmp(groupName, "prime256v1") || !strcasecmp(groupName, "secp256r1")) {
        return 256;
    }
    if (!strcasecmp(groupName, "P-384") || !strcasecmp(groupName, "p384") || !strcasecmp(groupName, "secp384r1")) {
        return 384;
    }
    if (!strcasecmp(groupName, "P-521") || !strcasecmp(groupName, "p521") || !strcasecmp(groupName, "secp521r1")) {
        return 521;
    }
    if (!strcasecmp(groupName, "secp256k1")) {
        return 256;
    }

    return 0;
}

__attribute__((visibility("hidden"))) inline int8_t GetKeylessLogLevel()
{
    static int8_t cached = -1;
    if (cached < 0) {
        const char* e = getenv("CODE_KEYLESS_DEBUG");
        cached = (e && *e && *e != '0') ? 1 : 0;
    }
    return cached;
}

__attribute__((visibility("hidden"))) size_t KeylessKeyGetSize(const void* keyData)
{
    const KeylessKey* k = keyData;
    if (!k) {
        return 0;
    }
    if (k->type == KEYLESS_KEY_TYPE_RSA) {
        return k->nLen;
    }
    /**
    * For EC keys, 80-byte upper bound, which is conservative enough for all supported curves
    * and keeps OpenSSL’s size probes satisfied.
    */
    if (k->type == KEYLESS_KEY_TYPE_EC) {
        return 80;
    }
    return 0;
}

__attribute__((visibility("hidden"))) const unsigned char* KeylessKeyGetN(const void* keyData)
{
    const KeylessKey* k = keyData;
    return (k && k->type == KEYLESS_KEY_TYPE_RSA) ? k->n : NULL;
}

__attribute__((visibility("hidden"))) size_t KeylessKeyGetNLen(const void* keyData)
{
    const KeylessKey* k = keyData;
    return (k && k->type == KEYLESS_KEY_TYPE_RSA) ? k->nLen : 0;
}

/**
 * Computes the SHA-256 hash of the issuer and serial number of the given X509 certificate,
 * and outputs the result as a hexadecimal string.
 *
 * @param crt      Pointer to the X509 certificate.
 * @param outHex  Output buffer to store the resulting SHA-256 hash in hexadecimal format.
 *                 Must be at least 65 bytes to accommodate the 64-character hash and null terminator.
 * @return         0 on success, non-zero on failure.
 */
__attribute__((visibility("hidden"))) int CertIssuerSerialSha256Hex(const X509* crt, char outHex[65], size_t outHexSize)
{
    if (!crt || !outHex || outHexSize != 65) { // 65: need 64 hex chars + NUL
        return 0;
    }

    DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
    const X509_NAME* issuer = DYN_X509_get_issuer_name((X509*)crt, dynMsg);
    const ASN1_INTEGER* serial = DYN_X509_get0_serialNumber((X509*)crt, dynMsg);
    if (!issuer || !serial) {
        KeylessCheckDynMsg(dynMsg, "X509_get_issuer_name or X509_get0_serialNumber");
        return 0;
    }

    unsigned char *issuerDer = NULL, *serialDer = NULL;
    int issuerLength = DYN_i2d_X509_NAME((X509_NAME*)issuer, &issuerDer, dynMsg);
    int serialLength = DYN_i2d_ASN1_INTEGER((ASN1_INTEGER*)serial, &serialDer, dynMsg);
    unsigned char dig[32]; // SHA-256 produces 32-byte digests
    int ok = 0;

    if (issuerLength <= 0 || serialLength <= 0) {
        goto cleanup;
    }

    SHA256_CTX ctx;
    if (!DYN_SHA256_Init(&ctx, dynMsg)) {
        goto cleanup;
    }
    if (!DYN_SHA256_Update(&ctx, issuerDer, (size_t)issuerLength, dynMsg)) {
        goto cleanup;
    }
    if (!DYN_SHA256_Update(&ctx, serialDer, (size_t)serialLength, dynMsg)) {
        goto cleanup;
    }
    if (!DYN_SHA256_Final(dig, &ctx, dynMsg)) {
        goto cleanup;
    }

    static const char* hex = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) { // 32 bytes in SHA-256 digest
        outHex[i * 2] = hex[(dig[i] >> 4) & 0xF];
        outHex[i * 2 + 1] = hex[dig[i] & 0xF];
    }
    outHex[64] = '\0'; // SHA-256 is 32 bytes -> 64 hex chars; index 64 is the NUL terminator
    ok = 1;

cleanup:
    DYN_OPENSSL_secure_free(issuerDer, dynMsg);
    DYN_OPENSSL_secure_free(serialDer, dynMsg);
    KeylessCheckDynMsg(dynMsg, "CertIssuerSerialSha256Hex");
    return ok;
}

/**
 * @brief Compute the SHA-256 fingerprint of an X.509 certificate and return it as a hexadecimal string.
 *
 * Computes the SHA-256 digest of the provided certificate and encodes it as a
 * null-terminated hexadecimal string without separators (64 hex characters).
 *
 * @param crt Pointer to an OpenSSL X509 certificate. Must not be NULL.
 *
 * @return A newly allocated C string containing the hex-encoded SHA-256 digest on success;
 *         NULL on failure (e.g., invalid input, digest computation error, or memory allocation failure).
 *
 * @note The caller owns the returned string and is responsible for freeing it with free().
 */
__attribute__((visibility("hidden"))) char* GetCertSha256Hex(const X509* crt)
{
    const size_t bufSize = 65; // 64 hex chars + null terminator
    char buf[bufSize];

    if (!CertIssuerSerialSha256Hex(crt, buf, bufSize)) {
        return NULL;
    }

    DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
    char* out = DYN_OPENSSL_secure_malloc(bufSize, dynMsg);
    KeylessCheckDynMsg(dynMsg, "GetCertSha256Hex");
    if (!out) {
        return NULL;
    }
    (void)memcpy_s(out, bufSize, buf, bufSize);
    return out;
}

extern char* CODE_TLS_GetCertId(const unsigned char* der, size_t len)
{
    const unsigned char* p = der;
    DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
    X509* cert = DYN_d2i_X509(NULL, &p, (long)len, dynMsg);
    KeylessCheckDynMsg(dynMsg, "CODE_TLS_GetCertId");
    if (!cert) {
        return NULL;
    }
    char* out = GetCertSha256Hex(cert);
    DYN_X509_free(cert, dynMsg);
    KeylessCheckDynMsg(dynMsg, "CODE_TLS_GetCertId");
    return out;
}
