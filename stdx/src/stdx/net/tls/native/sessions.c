/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <string.h>
#include "securec.h"
#include "api.h"
#include "opensslSymbols.h"

#define MAX_SESSION_ID_LENGTH SSL_MAX_SSL_SESSION_ID_LENGTH

extern void CODE_TLS_DYN_DeleteSession(SSL_SESSION* session, DynMsg* dynMsg)
{
    if (session == NULL) {
        return;
    }

    DYN_SSL_SESSION_free(session, dynMsg);
}

static SSL_SESSION* CopySession(SSL_SESSION* session, DynMsg* dynMsg)
{
    unsigned char* p = NULL;
    int size = DYN_i2d_SSL_SESSION(session, &p, dynMsg);
    if (size <= 0) {
        return 0;
    }

    const unsigned char* p2 = p;
    SSL_SESSION* copy = DYN_d2i_SSL_SESSION(NULL, &p2, (long)size, dynMsg);
    DYN_CRYPTO_free(p, dynMsg);

    if (copy == NULL) {
        return 0;
    }

    unsigned int len = 0;
    const unsigned char* ctx = DYN_SSL_SESSION_get0_id_context(session, &len, dynMsg);
    if (ctx != NULL && len > 0) {
        DYN_SSL_SESSION_set1_id_context(copy, ctx, len, dynMsg);
    }

    return copy;
}

extern int CODE_TLS_DYN_SetSession(SSL* stream, SSL_SESSION* session, DynMsg* dynMsg)
{
    if (stream == NULL || session == NULL) {
        return 1;
    }

    if (DYN_SSL_SESSION_is_resumable(session, dynMsg) == 0) {
        SSL_SESSION* copy = CopySession(session, dynMsg);
        int result = CODE_TLS_DYN_SetSession(stream, copy, dynMsg);
        DYN_SSL_SESSION_free(copy, dynMsg);
        return result;
    }

    if (DYN_SSL_set_session(stream, session, dynMsg) == 0) {
        return 1;
    }

    return 0;
}

extern int CODE_TLS_DYN_AddSession(SSL_CTX* ctx, SSL_SESSION* session, DynMsg* dynMsg)
{
    if (ctx == NULL || session == NULL) {
        return 1;
    }

    if (DYN_SSL_SESSION_is_resumable(session, dynMsg) == 0) {
        SSL_SESSION* copy = CopySession(session, dynMsg);
        int result = CODE_TLS_DYN_AddSession(ctx, copy, dynMsg);
        DYN_SSL_SESSION_free(copy, dynMsg);
        return result;
    }

    if (DYN_SSL_CTX_add_session(ctx, session, dynMsg) == 0) {
        return 1;
    }

    return 0;
}

typedef void (*PutSessionFunction)(const SSL* ssl, const unsigned char* id, size_t idLength, SSL_SESSION* session);

typedef void (*RemoveSessionFunction)(SSL_CTX* ctx, const unsigned char* id, size_t idLength, SSL_SESSION* session);

typedef SSL_SESSION* (*FindSessionFunction)(SSL* ssl, const unsigned char* id, unsigned int idLength);

typedef void (*AssignSessionFunction)(const SSL* ssl, SSL_SESSION* session);

static PutSessionFunction g_putSession = 0;
static RemoveSessionFunction g_removeSession = 0;
static FindSessionFunction g_findSession = 0;
static AssignSessionFunction g_assignSession = 0;

extern void CODE_TLS_DYN_SetSessionCallback(
    PutSessionFunction put, RemoveSessionFunction remove, FindSessionFunction find, AssignSessionFunction assign)
{
    g_putSession = put;
    g_removeSession = remove;
    g_findSession = find;
    g_assignSession = assign;
}

extern void CODE_TLS_DYN_IncrementUse(SSL_SESSION* session, DynMsg* dynMsg)
{
    if (session != NULL) {
        DYN_SSL_SESSION_up_ref(session, dynMsg);
    }
}

int NewSessionCallback(SSL* ssl, SSL_SESSION* session) // (SSL *ssl, SSL_SESSION *session)
{
    PutSessionFunction putSession = g_putSession;
    if (session == NULL || putSession == NULL) {
        return 0;
    }

    unsigned int idLength = 0;
    const unsigned char* sessionId = DYN_SSL_SESSION_get_id(session, &idLength, NULL);
    if (sessionId == NULL || idLength == 0 || idLength > MAX_SESSION_ID_LENGTH) {
        return 0;
    }

    SSL_SESSION* copy = CopySession(session, NULL);
    if (copy != NULL) {
        putSession(ssl, sessionId, (size_t)idLength, copy);
        DYN_SSL_SESSION_free(copy, NULL);
    }

    return 0; // we do return 0 because we keep a copy in the cache so we keep refcounter same
}

void SessionReusedCallback(SSL* ssl, SSL_SESSION* session)
{
    AssignSessionFunction assignSession = g_assignSession;
    if (ssl == NULL || session == NULL || assignSession == NULL) {
        return;
    }

    assignSession(ssl, session);
}

static void RemoveSessionCallback(SSL_CTX* ctx, SSL_SESSION* session)
{
    RemoveSessionFunction removeSession = g_removeSession;
    if (session == NULL || removeSession == NULL) {
        return;
    }

    unsigned int idLength = 0;
    const unsigned char* sessionId = DYN_SSL_SESSION_get_id(session, &idLength, NULL);
    if (sessionId == NULL || idLength == 0 || idLength > MAX_SESSION_ID_LENGTH) {
        return;
    }

    removeSession(ctx, sessionId, (size_t)idLength, session);
}

static SSL_SESSION* GetSessionCallback(SSL* ssl, const unsigned char* data, int len, int* copy)
{
    FindSessionFunction findSession = g_findSession;
    if (data == NULL || len <= 0 || len > MAX_SESSION_ID_LENGTH || findSession == NULL) {
        return NULL;
    }

    SSL_SESSION* result = findSession(ssl, data, (size_t)len);
    if (result != NULL && copy != NULL) {
        // we always return an already incremented session
        // so it's important to turn it to zero
        // otherwise openssl would increment refcount once again
        *copy = 0;
    }

    if (result != NULL) {
        // the session has preincremented refcount that need to be decremented after applying it
        SSL_SESSION* sessionCopy = CopySession(result, NULL);
        DYN_SSL_SESSION_free(result, NULL); // this should be done after doing a copy
        result = sessionCopy;
    }

    return result;
}

extern int CODE_TLS_DYN_SetSessionIdContext(
    SSL_CTX* ctx, const unsigned char* sidCtx, unsigned int sidCtxLen, DynMsg* dynMsg)
{
    if (ctx == NULL || sidCtx == NULL) {
        return -1;
    }

    if (!LoadFuncForNewSessionCallback(dynMsg)) {
        return -1;
    }

    DYN_SSL_CTX_sess_set_new_cb(ctx, NewSessionCallback, dynMsg);
    DYN_SSL_CTX_sess_set_remove_cb(ctx, RemoveSessionCallback, dynMsg);
    DYN_SSL_CTX_sess_set_get_cb(ctx, GetSessionCallback, dynMsg);
    DYN_SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_SERVER | SSL_SESS_CACHE_NO_INTERNAL, dynMsg);

    return DYN_SSL_CTX_set_session_id_context(ctx, sidCtx, sidCtxLen, dynMsg);
}

extern void CODE_TLS_DYN_GetSessionId(
    const SSL_SESSION* session, const unsigned char** data, size_t* length, DynMsg* dynMsg)
{
    if (session == NULL || data == NULL || length == NULL) {
        return;
    }

    unsigned int returnedSize;
    const unsigned char* returnedData = DYN_SSL_SESSION_get_id(session, &returnedSize, dynMsg);

    if (returnedData == NULL || returnedSize == 0 || returnedSize > MAX_SESSION_ID_LENGTH) {
        *data = NULL;
        *length = 0;
        return;
    }

    *data = returnedData;
    *length = (size_t)returnedSize;
}
