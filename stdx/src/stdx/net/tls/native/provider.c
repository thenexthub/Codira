/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <openssl/core.h>
#include <openssl/core_names.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/provider.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "api.h"
#include "opensslSymbols.h"
#include "provider.h"

static KeylessProviderCtx* PROVIDER_CTX = NULL;

static __thread DynMsg THREAD_DYNMSG; // Auto be freed in thread exit
static __thread int THREAD_DYN_MSG_SET = 0;

__attribute__((visibility("hidden"))) void KeylessCopyDynMsg(DynMsg* dst, const DynMsg* src)
{
    if (!dst) {
        return;
    }
    if (src) {
        *dst = *src;
    } else {
        dst->found = true;
        dst->funcName = NULL;
    }
}

/**
 * @brief Set the thread-local DynMsg for OpenSSL dynamic symbol loading.
 * @param dynMsg Pointer to the DynMsg structure to set for the current thread. If NULL, resets to a default state indicating all symbols are found.
 */
__attribute__((visibility("hidden"))) void KeylessProviderSetThreadDynMsg(const DynMsg* dynMsg)
{
    if (dynMsg) {
        THREAD_DYNMSG = *dynMsg;
        THREAD_DYN_MSG_SET = 1;
    } else {
        THREAD_DYNMSG.found = true;
        THREAD_DYNMSG.funcName = NULL;
        THREAD_DYN_MSG_SET = 0;
    }
}

__attribute__((visibility("hidden"))) DynMsg* KeylessProviderGetThreadDynMsg(void)
{
    return THREAD_DYN_MSG_SET ? &THREAD_DYNMSG : NULL;
}

extern int8_t LOG_LEVEL;
__attribute__((visibility("hidden"))) bool KeylessCheckDynMsg(const DynMsg* dynMsg, const char* context)
{
    if (LOG_LEVEL == KEYLESS_LOG_OFF || !dynMsg || dynMsg->found) {
        return true;
    }

    const char* funcName = dynMsg->funcName ? dynMsg->funcName : "(unknown)";
    const char* phase = context ? context : "operation";
    KeylessProviderLog("[keyless] missing OpenSSL symbol %s during %s\n.", funcName, phase);
    return false;
}

static KeylessCallbackEntry* KeylessFindEntryLocked(KeylessProviderCtx* ctx, const char* keyId)
{
    if (!ctx || !keyId) {
        return NULL;
    }
    for (KeylessCallbackEntry* it = ctx->callbacks; it != NULL; it = it->next) {
        if (it->keyId && strcmp(it->keyId, keyId) == 0) {
            return it;
        }
    }
    return NULL;
}

static KeylessCallbackEntry* KeylessEnsureEntryLocked(KeylessProviderCtx* ctx, const char* keyId, DynMsg* dynMsg)
{
    KeylessCallbackEntry* entry = KeylessFindEntryLocked(ctx, keyId);
    if (entry) {
        return entry;
    }

    KeylessCallbackEntry* created = DYN_OPENSSL_zalloc(sizeof(*created), dynMsg);
    if (!created) {
        return NULL;
    }

    created->keyId = strdup(keyId);
    if (!created->keyId) {
        DYN_OPENSSL_secure_free(created, dynMsg);
        return NULL;
    }
    created->next = ctx->callbacks;
    ctx->callbacks = created;
    return created;
}

static void KeylessClearCallbacks(KeylessProviderCtx* ctx)
{
    if (!ctx) {
        return;
    }
    pthread_rwlock_wrlock(&ctx->callbackLock);
    KeylessCallbackEntry* it = ctx->callbacks;
    ctx->callbacks = NULL;
    pthread_rwlock_unlock(&ctx->callbackLock);

    while (it) {
        KeylessCallbackEntry* next = it->next;
        free(it->keyId);
        DYN_OPENSSL_secure_free(it, NULL);
        it = next;
    }
}

__attribute__((visibility("hidden"))) KeylessRemoteSignCb KeylessLookupSignCb(const char* keyId)
{
    if (!keyId) {
        return NULL;
    }

    KeylessProviderCtx* ctx = PROVIDER_CTX;
    if (!ctx) {
        return NULL;
    }

    pthread_rwlock_rdlock(&ctx->callbackLock);
    KeylessCallbackEntry* entry = KeylessFindEntryLocked(ctx, keyId);
    KeylessRemoteSignCb cb = entry ? entry->signCb : NULL;
    pthread_rwlock_unlock(&ctx->callbackLock);

    if (cb) {
        return cb;
    }
    return NULL;
}

__attribute__((visibility("hidden"))) KeylessRemoteDecryptCb KeylessLookupDecryptCb(const char* keyId)
{
    if (!keyId) {
        return NULL;
    }

    KeylessProviderCtx* ctx = PROVIDER_CTX;
    if (!ctx) {
        return NULL;
    }

    pthread_rwlock_rdlock(&ctx->callbackLock);
    KeylessCallbackEntry* entry = KeylessFindEntryLocked(ctx, keyId);
    KeylessRemoteDecryptCb cb = entry ? entry->decryptCb : NULL;
    pthread_rwlock_unlock(&ctx->callbackLock);
    if (cb) {
        return cb;
    }
    return NULL;
}

static void KeylessRegisterSignCallback(const char* keyId, KeylessRemoteSignCb cb, ExceptionData* exception, DynMsg* dynMsg)
{
    if (!keyId || !cb) {
        HandleError(exception, "Invalid parameters for sign callback registration", dynMsg);
        return;
    }

    KeylessProviderCtx* ctx = PROVIDER_CTX;
    if (!ctx) {
        HandleError(exception, "Provider context not ready for sign callback registration", dynMsg);
        return;
    }
    pthread_rwlock_wrlock(&ctx->callbackLock);
    KeylessCallbackEntry* entry = KeylessEnsureEntryLocked(ctx, keyId, dynMsg);
    if (entry) {
        entry->signCb = cb;
    }
    pthread_rwlock_unlock(&ctx->callbackLock);
}

static void KeylessRegisterDecryptCallback(const char* keyId, KeylessRemoteDecryptCb cb, ExceptionData* exception, DynMsg* dynMsg)
{
    if (!keyId || !cb) {
        HandleError(exception, "Invalid parameters for decrypt callback registration", dynMsg);
        return;
    }

    KeylessProviderCtx* ctx = PROVIDER_CTX;
    if (!ctx) {
        HandleError(exception, "Provider context not ready for decrypt callback registration", dynMsg);
        return;
    }

    pthread_rwlock_wrlock(&ctx->callbackLock);
    KeylessCallbackEntry* entry = KeylessEnsureEntryLocked(ctx, keyId, dynMsg);
    if (entry) {
        entry->decryptCb = cb;
    }
    pthread_rwlock_unlock(&ctx->callbackLock);
}

/**
 * Creates a new provider context.
 * @param libctx The library context (unused).
 * @param in The dispatch table from OpenSSL (unused).
 * @param handle The core handle (unused).
 * @return Pointer to the newly created provider context, or NULL on failure.
 * @note called by OpenSSL when the provider is loaded.
 */
static void* KeylessProviderNewctx(OSSL_LIB_CTX* libctx, const OSSL_DISPATCH* in, const OSSL_CORE_HANDLE* handle)
{
    (void)libctx;
    (void)in;
    (void)handle;
    DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
    KeylessProviderCtx* ctx = DYN_OPENSSL_zalloc(sizeof(*ctx), dynMsg);
    KeylessCheckDynMsg(dynMsg, "KeylessProviderNewctx");
    if (!ctx) {
        return NULL;
    }
    if (pthread_rwlock_init(&ctx->callbackLock, NULL) != 0) {
        DYN_OPENSSL_secure_free(ctx, dynMsg);
        KeylessCheckDynMsg(dynMsg, "KeylessProviderNewctx");
        return NULL;
    }
    ctx->callbacks = NULL;
    PROVIDER_CTX = ctx;
    return ctx;
}

/**
 * Frees the provider context.
 * @param vctx Pointer to the provider context to free.
 * @note called by OpenSSL when the provider is unloaded.
 */
static void KeylessProviderFreectx(void* vctx)
{
    KeylessProviderCtx* ctx = vctx;
    if (!ctx) {
        return;
    }
    KeylessClearCallbacks(ctx);
    pthread_rwlock_destroy(&ctx->callbackLock);
    if (PROVIDER_CTX == ctx) {
        PROVIDER_CTX = NULL;
    }
    DYN_OPENSSL_secure_free(ctx, NULL);
}

extern const OSSL_ALGORITHM keylessKeymgmtAlgorithms[];
extern const OSSL_ALGORITHM keylessSignatureAlgorithms[];
extern const OSSL_ALGORITHM keylessAsymcipherAlgorithms[];

/**
 * Query function for the keyless provider.
 * @param prov The provider instance (unused).
 * @param operationId The operation ID being queried.
 * @param noCache Pointer to an integer indicating whether caching is allowed (unused).
 * @return Pointer to the array of algorithms for the requested operation, or NULL if unsupported.
 */
static const OSSL_ALGORITHM* KeylessQuery(OSSL_PROVIDER* prov, int operationId, int* noCache)
{
    (void)prov;
    (void)noCache;
    switch (operationId) {
        case OSSL_OP_KEYMGMT:
            return keylessKeymgmtAlgorithms;
        case OSSL_OP_SIGNATURE:
            return keylessSignatureAlgorithms;
        case OSSL_OP_ASYM_CIPHER:
            return keylessAsymcipherAlgorithms;
        default:
            return NULL;
    }
}

/**
 * Teardown function for the keyless provider.
 * @param provctx Pointer to the provider context to free.
 * @note called by OpenSSL when the provider is unloaded.
 */
static void KeylessProviderTeardown(void* provctx)
{
    KeylessProviderFreectx(provctx);
}

/**
 * Dispatch table for the keyless provider, includes query and teardown functions.
 */
static const OSSL_DISPATCH keyless_dispatch_table[] = {
        {OSSL_FUNC_PROVIDER_QUERY_OPERATION, (void (*)(void))KeylessQuery}, {OSSL_FUNC_PROVIDER_TEARDOWN, (void (*)(void))KeylessProviderTeardown}, {0, NULL}};

/**
 * Initialization function for the keyless provider.
 * @param handle The core handle from OpenSSL.
 * @param in The dispatch table from OpenSSL.
 * @param out Pointer to receive the provider's dispatch table.
 * @param provctx Pointer to receive the provider context.
 * @return 1 on success, 0 on failure.
 * @note called by OpenSSL when the provider is loaded.
 */
static int KeylessProviderInit(const OSSL_CORE_HANDLE* handle, const OSSL_DISPATCH* in, const OSSL_DISPATCH** out, void** provctx)
{
    *out = keyless_dispatch_table;
    *provctx = KeylessProviderNewctx(NULL, in, handle);
    if (*provctx == NULL) {
        return 0;
    }
    return 1;
}

int8_t LOG_LEVEL = KEYLESS_LOG_OFF;

extern int8_t DYN_CODE_TLS_InitEmbeddedKeylessProvider(DynMsg* dynMsg)
{
    LOG_LEVEL = GetKeylessLogLevel();
    const char* customProviderName = "CODE_KEYLESS_PROVIDER";
    if (DYN_OSSL_PROVIDER_add_builtin(NULL, customProviderName, KeylessProviderInit, dynMsg) != 1) {
        KeylessProviderLog("[keyless] Failed to add builtin keyless provider\n");
        return KEYLESS_PROVIDER_ADD_FAILED;
    }

    OSSL_PROVIDER* p = DYN_OSSL_PROVIDER_load(NULL, customProviderName, dynMsg);
    if (!p) {
        KeylessProviderLog("[keyless] Failed to load keyless provider\n");
        return KEYLESS_PROVIDER_LOAD_FAILED;
    }
    (void)DYN_OSSL_PROVIDER_load(NULL, "default", dynMsg);
    return KEYLESS_LOAD_SUCCESS;
}

extern void DYN_CODE_TLS_RegisterKeylessSignCallback(const char* keyId, KeylessRemoteSignCb cb, ExceptionData* exception, DynMsg* dynMsg)
{
    KeylessProviderLog("[keyless] Register sign callback for keyId=%s\n", keyId);
    KeylessRegisterSignCallback(keyId, cb, exception, dynMsg);
}

extern void DYN_CODE_TLS_RegisterKeylessDecryptCallback(const char* keyId, KeylessRemoteDecryptCb cb, ExceptionData* exception, DynMsg* dynMsg)
{
    KeylessProviderLog("[keyless] Register decrypt callback for keyId=%s\n", keyId);
    KeylessRegisterDecryptCallback(keyId, cb, exception, dynMsg);
}
