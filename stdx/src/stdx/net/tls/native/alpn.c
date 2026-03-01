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
#include "opensslSymbols.h"
#include "securec.h"

/* The server saves the structure of the user setting alpn protocol*/
struct AlpnArg {
    unsigned char* protos;
    unsigned int protosLen;
};

#define CODETLS_PROTOS_LEN_MAX (INT_MAX)

extern int CODE_TLS_DYN_SetClientAlpnProtocols(
    SSL_CTX* ctx, const unsigned char* protos, unsigned int protosLen, DynMsg* dynMsg)
{
    if (ctx == NULL || protos == NULL || protosLen == 0) {
        return -1;
    }

    /* This interface returns 0 for success and non-zero for failure. */
    if (DYN_SSL_CTX_set_alpn_protos(ctx, protos, protosLen, dynMsg) != 0) {
        return -1;
    }

    return 1;
}

/* When ctx is released, the callback function will be called to release ex_data */
static void AlpnFreeCallback(void* parent, void* ptr, CRYPTO_EX_DATA* ad, int idx, long argl, void* argp)
{
    struct AlpnArg* data;
    (void)parent;
    (void)ad;
    (void)idx;
    (void)argl;
    (void)argp;

    if (ptr != NULL) {
        data = (struct AlpnArg*)ptr;
        free(data->protos);
        free(data);
    }
}

static int AlpnGetIndex(DynMsg* dynMsg)
{
    /* The server saves the index of alpn's exdata, which must be initialized to -1 */
    static int g_alpnIndex = -1;
    static pthread_mutex_t g_alpnIndexLock = PTHREAD_MUTEX_INITIALIZER;

    if (g_alpnIndex == -1) {
        pthread_mutex_lock(&g_alpnIndexLock);
        if (g_alpnIndex == -1) {
            g_alpnIndex =
                DYN_CRYPTO_get_ex_new_index(CRYPTO_EX_INDEX_SSL_CTX, 0, NULL, NULL, NULL, AlpnFreeCallback, dynMsg);
        }
        pthread_mutex_unlock(&g_alpnIndexLock);
    }

    return g_alpnIndex;
}

static struct AlpnArg* ServerAlpnProtosDataInit(const unsigned char* protos, unsigned int protosLen)
{
    struct AlpnArg* data;
    int ret;

    if (protosLen >= CODETLS_PROTOS_LEN_MAX) {
        return NULL;
    }

    data = malloc(sizeof(struct AlpnArg));
    if (data == NULL) {
        return NULL;
    }

    data->protos = malloc(protosLen);
    if (data->protos == NULL) {
        free(data);
        return NULL;
    }

    ret = memcpy_s(data->protos, protosLen, protos, protosLen);
    if (ret != 0) {
        free(data->protos);
        free(data);
        return NULL;
    }

    data->protosLen = protosLen;
    return data;
}

/* The server calls this callback function to select the alpn protocol according to the client request. */
static int AlpnSelectCallback(
    SSL* ssl, const unsigned char** out, unsigned char* outlen, const unsigned char* in, unsigned int inlen, void* arg)
{
    unsigned char* res = NULL;
    unsigned char reslen = 0;
    SSL_CTX* ctx = (SSL_CTX*)DYN_SSL_get_SSL_CTX(ssl, NULL);
    int ret;
    (void)arg;

    int alpnIndex = AlpnGetIndex(NULL);
    if (alpnIndex == -1) {
        return SSL_TLSEXT_ERR_NOACK;
    }

    struct AlpnArg* data = (struct AlpnArg*)DYN_SSL_CTX_get_ex_data(ctx, alpnIndex, NULL);
    if (data == NULL) {
        /* Server not set alpn, continue handshake */
        return SSL_TLSEXT_ERR_NOACK;
    }

    ret = DYN_SSL_select_next_proto(&res, &reslen, data->protos, data->protosLen, in, inlen, NULL);
    if (ret == OPENSSL_NPN_NEGOTIATED) {
        *out = res;
        *outlen = reslen;
        return SSL_TLSEXT_ERR_OK;
    } else {
        /* The server set up alpn, but did not select success, terminating the handshake */
        return SSL_TLSEXT_ERR_ALERT_FATAL;
    }
}

/* Before setting new data, call this interface to clear the old data. */
static int AlpnClear(SSL_CTX* ctx, int index, DynMsg* dynMsg)
{
    struct AlpnArg* data = DYN_SSL_CTX_get_ex_data(ctx, index, dynMsg);
    if (data != NULL) {
        free(data->protos);
        free(data);
    }

    return DYN_SSL_CTX_set_ex_data(ctx, index, NULL, dynMsg);
}

extern int CODE_TLS_DYN_SetServerAlpnProtos(
    SSL_CTX* ctx, const unsigned char* protos, unsigned int protosLen, DynMsg* dynMsg)
{
    struct AlpnArg* data;
    int ret;

    if (ctx == NULL || protos == NULL || protosLen == 0) {
        return -1;
    }

    /* During the existence of ctx, it is necessary to ensure that the memory pointed to by protos is always valid, so
     * copy and save the memory */
    data = ServerAlpnProtosDataInit(protos, protosLen);
    if (data == NULL) {
        return -1;
    }

    /* Save the data in exdata of ctx. When ctx is released, tls_alpn_free_callback will be called automatically to
     * release the data. */
    int alpnIndex = AlpnGetIndex(dynMsg);
    if (alpnIndex == -1) {
        free(data->protos);
        free(data);
        return -1;
    }

    ret = AlpnClear(ctx, alpnIndex, dynMsg);
    if (ret <= 0) {
        free(data->protos);
        free(data);
        return ret;
    }
    ret = DYN_SSL_CTX_set_ex_data(ctx, alpnIndex, data, dynMsg);
    if (ret <= 0) {
        free(data->protos);
        free(data);
        return ret;
    }

    // OPENSSL is loaded dynamically. First check whether the OPENSSL method called in the AlpnSelectCallback callback
    // function can be found.
    if (!LoadDynFuncForAlpnCallback(dynMsg)) {
        return -1;
    }

    DYN_SSL_CTX_set_alpn_select_cb(ctx, AlpnSelectCallback, NULL, dynMsg);
    return 1;
}

extern void CODE_TLS_DYN_GetAlpnSelected(
    const SSL* stream, const unsigned char** proto, unsigned int* len, DynMsg* dynMsg)
{
    if (stream == NULL || proto == NULL || len == NULL) {
        if (proto != NULL) {
            *proto = NULL;
        }
        if (len != NULL) {
            *len = 0;
        }
        return;
    }

    DYN_SSL_get0_alpn_selected(stream, proto, len, dynMsg);
}
