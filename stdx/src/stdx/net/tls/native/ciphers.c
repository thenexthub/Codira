/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>
#include "securec.h"
#include "api.h"
#include "opensslSymbols.h"

extern int CODE_TLS_DYN_SetCipherList(SSL_CTX* ctx, const char* str, DynMsg* dynMsg)
{
    if (ctx == NULL || str == NULL) {
        return 0;
    }

    return DYN_SSL_CTX_set_cipher_list(ctx, str, dynMsg);
}

extern int CODE_TLS_DYN_SetCipherSuites(SSL_CTX* ctx, const char* str, DynMsg* dynMsg)
{
    if (ctx == NULL || str == NULL) {
        return 0;
    }

    return DYN_SSL_CTX_set_ciphersuites(ctx, str, dynMsg);
}

static const TlsCipherSuite* GetCipherSuite(const SSL_CIPHER* cipher, DynMsg* dynMsg)
{
    if (cipher == NULL) {
        return NULL;
    }

    TlsCipherSuite* cipherSuite = (TlsCipherSuite*)malloc((size_t)sizeof(TlsCipherSuite));
    if (cipherSuite == NULL) {
        return NULL;
    }

    const char* name = DYN_SSL_CIPHER_get_name(cipher, dynMsg);
    cipherSuite->name = DYN_OPENSSL_strdup(name, dynMsg);
    if (cipherSuite->name == NULL) {
        free(cipherSuite);
        return NULL;
    }

    return cipherSuite;
}

extern const TlsCipherSuite* CODE_TLS_DYN_GetCipherSuite(SSL* ssl, DynMsg* dynMsg)
{
    if (ssl == NULL) {
        return NULL;
    }

    const SSL_CIPHER* cipher = (const SSL_CIPHER*)DYN_SSL_get_current_cipher(ssl, dynMsg);

    return GetCipherSuite(cipher, dynMsg);
}

extern const TlsCipherSuite** CODE_TLS_DYN_GetAllCipherSuites(DynMsg* dynMsg)
{
    const SSL_METHOD* method = (const SSL_METHOD*)DYN_TLS_client_method(dynMsg);
    SSL_CTX* ctx = (SSL_CTX*)DYN_SSL_CTX_new(method, dynMsg);
    if (ctx == NULL) {
        return NULL;
    }

    STACK_OF(SSL_CIPHER)* ciphers = (STACK_OF(SSL_CIPHER)*)DYN_SSL_CTX_get_ciphers(ctx, dynMsg);
    if (ciphers == NULL) {
        return NULL;
    }

    int ciphersCount = DYN_OPENSSL_sk_num((void*)ciphers, dynMsg);
    size_t initialSize = (size_t)(ciphersCount + 1) * sizeof(TlsCipherSuite*);
    if (initialSize == 0) {
        DYN_SSL_CTX_free(ctx, dynMsg);
        return NULL;
    }
    const TlsCipherSuite** cipherSuites = malloc(initialSize);
    if (cipherSuites == NULL) {
        DYN_SSL_CTX_free(ctx, dynMsg);
        return NULL;
    }

    // parse suites
    int parsedSuites = 0;
    for (int i = 0; i < ciphersCount; i++) {
        const SSL_CIPHER* cipher = (const SSL_CIPHER*)DYN_OPENSSL_sk_value((void*)ciphers, i, dynMsg);
        if (cipher != NULL) {
            const TlsCipherSuite* cipherSuite = GetCipherSuite(cipher, dynMsg);
            if (cipherSuite != NULL) {
                cipherSuites[parsedSuites] = cipherSuite;
                parsedSuites++;
            }
        }
    }

    cipherSuites[parsedSuites] = NULL;

    DYN_SSL_CTX_free(ctx, dynMsg);

    return cipherSuites;
}
