/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <securec.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#define OPENSSLPATH "libcrypto-3-x64.dll"
#define OPENSSLPATHSSL "libssl-3-x64.dll"
#elif defined(__APPLE__)
#include <dlfcn.h>
#define OPENSSLPATH "libssl.3.dylib"
#elif defined(__ohos__)
#include <dlfcn.h>
#define OPENSSLPATH "libssl_openssl.z.so"
#else
#include <dlfcn.h>
#define OPENSSLPATH "libssl.so"
#endif
#include "opensslSymbols.h"
void* g_singletonHandle = NULL;
void* g_singletonHandleSsl = NULL;

DynMsg* MallocDynMsg(void)
{
    DynMsg* dynMsg = (DynMsg*)malloc(sizeof(DynMsg));
    if (dynMsg == NULL) {
        return NULL;
    }
    dynMsg->found = true;
    dynMsg->funcName = NULL;
    return dynMsg;
}

void FreeDynMsg(DynMsg* dynMsgPtr)
{
    if (dynMsgPtr == NULL) {
        return;
    }
    free((void*)dynMsgPtr);
}

static void* FindFunction(const char* name)
{
    void* func = NULL;
    if (g_singletonHandle == NULL) {
        return NULL;
    }
#ifdef _WIN32
    func = GetProcAddress(g_singletonHandle, name);
    if (func == NULL && g_singletonHandleSsl != NULL) {
        func = GetProcAddress(g_singletonHandleSsl, name);
    }
    if (func == NULL) {
        return NULL;
    }
#else
    func = dlsym(g_singletonHandle, name);
    if (func == NULL) {
        return NULL;
    }
#endif
    return func;
}

__attribute__((constructor)) void Singleton(void)
{
#ifdef _WIN32
    g_singletonHandle = LoadLibraryA(OPENSSLPATH);
    g_singletonHandleSsl = LoadLibraryA(OPENSSLPATHSSL);
#else
    g_singletonHandle = dlopen(OPENSSLPATH, RTLD_LAZY | RTLD_GLOBAL);
#endif
}

__attribute__((destructor)) void CloseSymbolTable(void)
{
#ifdef _WIN32
    if (g_singletonHandle != NULL) {
        (void)FreeLibrary(g_singletonHandle);
    }
    if (g_singletonHandleSsl != NULL) {
        (void)FreeLibrary(g_singletonHandleSsl);
    }
#else
    if (g_singletonHandle != NULL) {
        (void)dlclose(g_singletonHandle);
    }
#endif
}

/**============= Api =============*/

#define CHECKFUNCTION(dynMsg, index, name, errCode)                                                                    \
    if (func##index == NULL) {                                                                                         \
        (dynMsg)->found = false;                                                                                       \
        (dynMsg)->funcName = name;                                                                                     \
        return errCode;                                                                                                \
    }

#define FINDFUNCTIONI(dynMsg, index, name, errCode)                                                                    \
    static SSLFunc##index func##index = NULL;                                                                          \
    if (func##index == NULL) {                                                                                         \
        func##index = (SSLFunc##index)(FindFunction(#name));                                                           \
    }                                                                                                                  \
    CHECKFUNCTION(dynMsg, index, #name, errCode)

#define FINDFUNCTION(dynMsg, name, errCode) FINDFUNCTIONI(dynMsg, , name, errCode)

char* DYN_OPENSSL_strdup(const char* str, DynMsg* dynMsg)
{
    typedef char* (*SSLFunc)(const char* str, const char* file, int line);
    FINDFUNCTION(dynMsg, CRYPTO_strdup, NULL)
    return func(str, OPENSSL_FILE, OPENSSL_LINE);
}
char* DYN_OPENSSL_strndup(const char* str, size_t s, DynMsg* dynMsg)
{
    typedef char* (*SSLFunc)(const char* str, size_t s, const char* file, int line);
    FINDFUNCTION(dynMsg, CRYPTO_strndup, NULL)
    return func(str, s, OPENSSL_FILE, OPENSSL_LINE);
}

void* DYN_OPENSSL_memdup(void* str, size_t s, DynMsg* dynMsg)
{
    typedef void* (*SSLFunc)(const void* str, size_t s, const char* file, int line);
    FINDFUNCTION(dynMsg, CRYPTO_memdup, NULL)
    return func(str, s, OPENSSL_FILE, OPENSSL_LINE);
}
void DYN_CRYPTO_free(void* str, DynMsg* dynMsg)
{
    typedef void (*SSLFunc)(void*, const char*, int);
    FINDFUNCTION(dynMsg, CRYPTO_free, )
    return func(str, OPENSSL_FILE, OPENSSL_LINE);
}

void CODE_TLS_DYN_CRYPTO_free(void* str, DynMsg* dynMsg)
{
    typedef void (*SSLFunc)(void*, const char*, int);
    FINDFUNCTION(dynMsg, CRYPTO_free, )
    return func(str, "", 0);
}

void* DYN_OPENSSL_secure_malloc(size_t num, DynMsg* dynMsg)
{
    typedef void* (*SSLFunc)(size_t, const char*, int);
    FINDFUNCTION(dynMsg, CRYPTO_secure_malloc, NULL)
    return func(num, OPENSSL_FILE, OPENSSL_LINE);
}

void* DYN_OPENSSL_zalloc(size_t num, DynMsg* dynMsg)
{
    typedef void* (*SSLFunc)(size_t, const char*, int);
    FINDFUNCTION(dynMsg, CRYPTO_secure_zalloc, NULL)
    return func(num, OPENSSL_FILE, OPENSSL_LINE);
}

int DYN_BN_num_bytes(const BIGNUM* bn, DynMsg* dynMsg)
{
    typedef int (*SSLFunc)(const BIGNUM*);
    FINDFUNCTION(dynMsg, BN_num_bits, 0)
    return (func(bn) + 7) / 8; // Convert bits to bytes, rounding up
}

void DYN_OPENSSL_secure_free(void* ptr, DynMsg* dynMsg)
{
    typedef void (*SSLFunc)(void*, const char*, int);
    FINDFUNCTION(dynMsg, CRYPTO_secure_free, )
    return func(ptr, OPENSSL_FILE, OPENSSL_LINE);
}

int DYN_SSL_CTX_set_min_proto_version(SSL_CTX* ctx, int version, DynMsg* dynMsg)
{
    typedef int (*SSLFunc)(SSL_CTX*, int, long, void*);
    FINDFUNCTION(dynMsg, SSL_CTX_ctrl, -1)
    return func(ctx, SSL_CTRL_SET_MIN_PROTO_VERSION, version, NULL);
}

int DYN_SSL_CTX_set_max_proto_version(SSL_CTX* ctx, int version, DynMsg* dynMsg)
{
    typedef int (*SSLFunc)(SSL_CTX*, int, long, void*);
    FINDFUNCTION(dynMsg, SSL_CTX_ctrl, -1)
    return func(ctx, SSL_CTRL_SET_MAX_PROTO_VERSION, version, NULL);
}

long DYN_SSL_CTX_set_dh_auto(SSL_CTX* ctx, int onoff, DynMsg* dynMsg)
{
    typedef int (*SSLFunc)(SSL_CTX*, int, long, void*);
    FINDFUNCTION(dynMsg, SSL_CTX_ctrl, 0)
    return func(ctx, SSL_CTRL_SET_DH_AUTO, onoff, NULL);
}

int DYN_SSL_CTX_add0_chain_cert(SSL_CTX* ctx, X509* x509, DynMsg* dynMsg)
{
    typedef int (*SSLFunc)(SSL_CTX*, int, long, void*);
    FINDFUNCTION(dynMsg, SSL_CTX_ctrl, 0)
    return func(ctx, SSL_CTRL_CHAIN_CERT, 0, (char*)(x509));
}

long DYN_SSL_CTX_set_mode(SSL_CTX* ctx, long mode, DynMsg* dynMsg)
{
    typedef int (*SSLFunc)(SSL_CTX*, int, long, void*);
    FINDFUNCTION(dynMsg, SSL_CTX_ctrl, 0)
    return func((ctx), SSL_CTRL_MODE, (mode), NULL);
}

long DYN_SSL_CTX_set1_sigalgs_list(SSL_CTX* ctx, const char* str, DynMsg* dynMsg)
{
    typedef int (*SSLFunc)(SSL_CTX*, int, long, void*);
    FINDFUNCTION(dynMsg, SSL_CTX_ctrl, 0)
    return func(ctx, SSL_CTRL_SET_SIGALGS_LIST, 0, (char*)(str));
}

long DYN_SSL_CTX_set_session_cache_mode(SSL_CTX* ctx, long mode, DynMsg* dynMsg)
{
    typedef int (*SSLFunc)(SSL_CTX*, int, long, void*);
    FINDFUNCTION(dynMsg, SSL_CTX_ctrl, -1)
    return func(ctx, SSL_CTRL_SET_SESS_CACHE_MODE, mode, NULL);
}

size_t DYN_BIO_pending(BIO* b, DynMsg* dynMsg)
{
    typedef size_t (*SSLFunc)(BIO*, int, long, void*);
    FINDFUNCTION(dynMsg, BIO_ctrl, 0)
    return func(b, BIO_CTRL_PENDING, 0, NULL);
}

int DYN_BIO_eof(BIO* b, DynMsg* dynMsg)
{
    typedef int (*SSLFunc)(BIO*, int, long, void*);
    FINDFUNCTION(dynMsg, BIO_ctrl, 0)
    return func(b, BIO_CTRL_EOF, 0, NULL);
}

void DYN_BIO_clear_retry_flags(BIO* b, DynMsg* dynMsg)
{
    typedef void (*SSLFunc)(BIO*, int);
    FINDFUNCTION(dynMsg, BIO_clear_flags, )
    return func(b, (BIO_FLAGS_RWS | BIO_FLAGS_SHOULD_RETRY));
}

long DYN_BIO_get_mem_ptr(BIO* b, BUF_MEM** pp, DynMsg* dynMsg)
{
    typedef int (*SSLFunc)(BIO*, int, long, void*);
    FINDFUNCTION(dynMsg, BIO_ctrl, 0)
    return func(b, BIO_C_GET_BUF_MEM_PTR, 0, (void*)pp);
}

int DYN_SSL_set_tlsext_host_name(SSL* ssl, const char* name, DynMsg* dynMsg)
{
    typedef int (*SSLFunc)(SSL*, int, long, void*);
    FINDFUNCTION(dynMsg, SSL_ctrl, -1)
    return func(ssl, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, (void*)name);
}

long DYN_SSL_CTX_set_tlsext_servername_callback(SSL_CTX* ctx, int (*cb)(void* s, int* al, void* arg), DynMsg* dynMsg)
{
    typedef long (*SSLFunc)(SSL_CTX*, int, void(*fp));
    FINDFUNCTION(dynMsg, SSL_CTX_callback_ctrl, -1)
    return func(ctx, SSL_CTRL_SET_TLSEXT_SERVERNAME_CB, (void (*)(void))cb);
}

BIO* DYN_BIO_new_mem(DynMsg* dynMsg)
{
    typedef BIO* (*SSLFunc1)(const BIO_METHOD*);
    FINDFUNCTIONI(dynMsg, 1, BIO_new, NULL)
    typedef BIO_METHOD* (*SSLFunc2)(void);
    FINDFUNCTIONI(dynMsg, 2, BIO_s_mem, NULL)
    return func1(func2());
}

#define DEFINEFUNCTION0(name, errCode, type0)                                                                          \
    type0 DYN_##name(DynMsg* dynMsg)                                                                                   \
    {                                                                                                                  \
        typedef type0 (*SSLFunc)(void);                                                                                \
        FINDFUNCTION(dynMsg, name, errCode)                                                                            \
        return func();                                                                                                 \
    }

#define DEFINEFUNCTION1(name, errCode, type0, type1)                                                                   \
    type0 DYN_##name(type1 arg1, DynMsg* dynMsg)                                                                       \
    {                                                                                                                  \
        typedef type0 (*SSLFunc)(type1);                                                                               \
        FINDFUNCTION(dynMsg, name, errCode)                                                                            \
        return func(arg1);                                                                                             \
    }

#define DEFINEFUNCTION2(name, errCode, type0, type1, type2)                                                            \
    type0 DYN_##name(type1 arg1, type2 arg2, DynMsg* dynMsg)                                                           \
    {                                                                                                                  \
        typedef type0 (*SSLFunc)(type1, type2);                                                                        \
        FINDFUNCTION(dynMsg, name, errCode)                                                                            \
        return func(arg1, arg2);                                                                                       \
    }
#define DEFINEFUNCTIONCB2(name, errCode, type0, type1, type2)                                                          \
    type0 DYN_##name(type1, type2, DynMsg* dynMsg)                                                                     \
    {                                                                                                                  \
        typedef type0 (*SSLFunc)(type1, type2);                                                                        \
        FINDFUNCTION(dynMsg, name, errCode)                                                                            \
        return func(arg1, arg2);                                                                                       \
    }
#define DEFINEFUNCTION3(name, errCode, type0, type1, type2, type3)                                                     \
    type0 DYN_##name(type1 arg1, type2 arg2, type3 arg3, DynMsg* dynMsg)                                               \
    {                                                                                                                  \
        typedef type0 (*SSLFunc)(type1, type2, type3);                                                                 \
        FINDFUNCTION(dynMsg, name, errCode)                                                                            \
        return func(arg1, arg2, arg3);                                                                                 \
    }
#define DEFINEFUNCTIONCB3(name, errCode, type0, type1, type2, type3)                                                   \
    type0 DYN_##name(type1, type2, type3, DynMsg* dynMsg)                                                              \
    {                                                                                                                  \
        typedef type0 (*SSLFunc)(type1, type2, type3);                                                                 \
        FINDFUNCTION(dynMsg, name, errCode)                                                                            \
        return func(arg1, arg2, arg3);                                                                                 \
    }
#define DEFINEFUNCTION4(name, errCode, type0, type1, type2, type3, type4)                                              \
    type0 DYN_##name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, DynMsg* dynMsg)                                   \
    {                                                                                                                  \
        typedef type0 (*SSLFunc)(type1, type2, type3, type4);                                                          \
        FINDFUNCTION(dynMsg, name, errCode)                                                                            \
        return func(arg1, arg2, arg3, arg4);                                                                           \
    }
#define DEFINEFUNCTION5(name, errCode, type0, type1, type2, type3, type4, type5)                                       \
    type0 DYN_##name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, DynMsg* dynMsg)                       \
    {                                                                                                                  \
        typedef type0 (*SSLFunc)(type1, type2, type3, type4, type5);                                                   \
        FINDFUNCTION(dynMsg, name, errCode)                                                                            \
        return func(arg1, arg2, arg3, arg4, arg5);                                                                     \
    }
#define DEFINEFUNCTION6(name, errCode, type0, type1, type2, type3, type4, type5, type6)                                \
    type0 DYN_##name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, DynMsg* dynMsg)           \
    {                                                                                                                  \
        typedef type0 (*SSLFunc)(type1, type2, type3, type4, type5, type6);                                            \
        FINDFUNCTION(dynMsg, name, errCode)                                                                            \
        return func(arg1, arg2, arg3, arg4, arg5, arg6);                                                               \
    }
#define DEFINEFUNCTION7(name, errCode, type0, type1, type2, type3, type4, type5, type6, type7)                         \
    type0 DYN_##name(                                                                                                  \
        type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, DynMsg* dynMsg)            \
    {                                                                                                                  \
        typedef type0 (*SSLFunc)(type1, type2, type3, type4, type5, type6, type7);                                     \
        FINDFUNCTION(dynMsg, name, errCode)                                                                            \
        return func(arg1, arg2, arg3, arg4, arg5, arg6, arg7);                                                         \
    }
#define DEFINEFUNCTION8(name, errCode, type0, type1, type2, type3, type4, type5, type6, type7, type8)                  \
    type0 DYN_##name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, type8 arg8,   \
        DynMsg* dynMsg)                                                                                                \
    {                                                                                                                  \
        typedef type0 (*SSLFunc)(type1, type2, type3, type4, type5, type6, type7, type8);                              \
        FINDFUNCTION(dynMsg, name, errCode)                                                                            \
        return func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);                                                   \
    }
#include "defineFunction.inc"
#undef DEFINEFUNCTION0
#undef DEFINEFUNCTION1
#undef DEFINEFUNCTION2
#undef DEFINEFUNCTION3
#undef DEFINEFUNCTION4
#undef DEFINEFUNCTION5
#undef DEFINEFUNCTION6
#undef DEFINEFUNCTION7
#undef DEFINEFUNCTION8
#undef DEFINEFUNCTIONCB2
#undef DEFINEFUNCTIONCB3

bool LoadDynFuncForAlpnCallback(DynMsg* dynMsg)
{
    typedef SSL_CTX* (*SSLFunc0)(const SSL*);
    FINDFUNCTIONI(dynMsg, 0, SSL_get_SSL_CTX, false)

    typedef void* (*SSLFunc1)(const SSL_CTX*, int);
    FINDFUNCTIONI(dynMsg, 1, SSL_CTX_get_ex_data, false)

    typedef int (*SSLFunc2)(int, long, void*, CRYPTO_EX_new*, CRYPTO_EX_dup*, CRYPTO_EX_free*);
    FINDFUNCTIONI(dynMsg, 2, CRYPTO_get_ex_new_index, false)

    typedef int (*SSLFunc3)(
        unsigned char**, unsigned char*, const unsigned char*, unsigned int, const unsigned char*, unsigned int);
    FINDFUNCTIONI(dynMsg, 3, SSL_select_next_proto, false)

    return true;
}

bool LoadFuncForNewSessionCallback(DynMsg* dynMsg)
{
    typedef const unsigned char* (*SSLFunc1)(const SSL_SESSION*, unsigned int*);
    FINDFUNCTIONI(dynMsg, 1, SSL_SESSION_get_id, false)
    typedef int (*SSLFunc2)(SSL_SESSION*, unsigned char**);
    FINDFUNCTIONI(dynMsg, 2, i2d_SSL_SESSION, false)
    typedef SSL_SESSION* (*SSLFunc3)(SSL_SESSION**, const unsigned char**, long);
    FINDFUNCTIONI(dynMsg, 3, d2i_SSL_SESSION, false)
    typedef void (*SSLFunc4)(void*, const char*, int);
    FINDFUNCTIONI(dynMsg, 4, CRYPTO_free, false)
    typedef const unsigned char* (*SSLFunc5)(void*, const char*, int);
    FINDFUNCTIONI(dynMsg, 5, SSL_SESSION_get0_id_context, false)
    typedef int (*SSLFunc6)(SSL_SESSION*, const unsigned char*, unsigned int);
    FINDFUNCTIONI(dynMsg, 6, SSL_SESSION_set1_id_context, false)
    typedef void (*SSLFunc7)(SSL_SESSION*);
    FINDFUNCTIONI(dynMsg, 7, SSL_SESSION_free, false)

    return true;
}

bool LoadDynFuncForCreateMethod(DynMsg* dynMsg)
{
    typedef void (*SSLFunc1)(BIO*, void*);
    FINDFUNCTIONI(dynMsg, 1, BIO_set_data, false)
    typedef void (*SSLFunc2)(BIO*, int);
    FINDFUNCTIONI(dynMsg, 2, BIO_set_init, false)
    typedef void (*SSLFunc3)(BIO*, int);
    FINDFUNCTIONI(dynMsg, 3, BIO_set_flags, false)
    typedef void (*SSLFunc4)(BIO*, int);
    FINDFUNCTIONI(dynMsg, 4, BIO_set_retry_reason, false)
    typedef void* (*SSLFunc5)(BIO*);
    FINDFUNCTIONI(dynMsg, 5, BIO_get_data, false)
    typedef void (*SSLFunc6)(BIO*, int);
    FINDFUNCTIONI(dynMsg, 6, BIO_clear_flags, false)
    return true;
}

bool LoadDynFuncCertVerifyCallback(DynMsg* dynMsg)
{
    typedef X509* (*SSLFunc1)(const X509_STORE_CTX*);
    FINDFUNCTIONI(dynMsg, 1, X509_STORE_CTX_get0_cert, false)
    typedef STACK_OF(X509) * (*SSLFunc2)(const X509_STORE_CTX* ctx);
    FINDFUNCTIONI(dynMsg, 2, X509_STORE_CTX_get0_untrusted, false)
    typedef STACK_OF(X509) * (*SSLFunc3)(void);
    FINDFUNCTIONI(dynMsg, 3, OPENSSL_sk_new_null, false)
    typedef STACK_OF(X509) * (*SSLFunc4)(STACK_OF(X509)*);
    FINDFUNCTIONI(dynMsg, 4, X509_chain_up_ref, false)
    typedef int (*SSLFunc5)(STACK_OF(X509)*, X509*, int);
    FINDFUNCTIONI(dynMsg, 5, OPENSSL_sk_insert, false)
    typedef int (*SSLFunc6)(X509*);
    FINDFUNCTIONI(dynMsg, 6, X509_up_ref, false)
    typedef void (*SSLFunc7)(X509_STORE_CTX*, STACK_OF(X509)*);
    FINDFUNCTIONI(dynMsg, 7, X509_STORE_CTX_set0_verified_chain, false)
    typedef void (*SSLFunc8)(X509_STORE_CTX*, int);
    FINDFUNCTIONI(dynMsg, 8, X509_STORE_CTX_set_error, false)

    return true;
}

bool LoadDynFuncForCustomVerifyCallback(DynMsg* dynMsg)
{
    typedef SSL* (*SSLFunc1)(const X509_STORE_CTX*, int);
    FINDFUNCTIONI(dynMsg, 1, X509_STORE_CTX_get_ex_data, false)
    typedef int (*SSLFunc2)(void);
    FINDFUNCTIONI(dynMsg, 2, SSL_get_ex_data_X509_STORE_CTX_idx, false)
    typedef SSL_CTX* (*SSLFunc3)(const SSL*);
    FINDFUNCTIONI(dynMsg, 3, SSL_get_SSL_CTX, false)
    typedef STACK_OF(X509)* (*SSLFunc4)(const X509_STORE_CTX* ctx);
    FINDFUNCTIONI(dynMsg, 4, X509_STORE_CTX_get0_untrusted, false)
    typedef int (*SSLFunc5)(void*);
    FINDFUNCTIONI(dynMsg, 5, OPENSSL_sk_num, false)
    typedef void* (*SSLFunc6)(void*, int);
    FINDFUNCTIONI(dynMsg, 6, OPENSSL_sk_value, false)
    typedef void (*SSLFunc7)(void*, const char*, int);
    FINDFUNCTIONI(dynMsg, 7, CRYPTO_free, false)

    return true;
}

bool LoadDynForInfoCallback(DynMsg* dynMsg)
{
    typedef void* (*SSLFunc1)(const SSL*, int);
    FINDFUNCTIONI(dynMsg, 1, SSL_get_ex_data, false)
    typedef const char* (*SSLFunc2)(int);
    FINDFUNCTIONI(dynMsg, 2, SSL_alert_desc_string_long, false)
    typedef const char* (*SSLFunc3)(int);
    FINDFUNCTIONI(dynMsg, 3, SSL_alert_type_string, false)

    return true;
}

void DYN_BIO_set_retry_read(BIO* a, DynMsg* dynMsg)
{
    DYN_BIO_set_flags(a, (BIO_FLAGS_READ | BIO_FLAGS_SHOULD_RETRY), dynMsg);
}

void DYN_BIO_set_retry_write(BIO* a, DynMsg* dynMsg)
{
    DYN_BIO_set_flags(a, (BIO_FLAGS_WRITE | BIO_FLAGS_SHOULD_RETRY), dynMsg);
}

void DynPopFree(void* extlist, char* funcName, DynMsg* dynMsg)
{
    typedef void (*SSLFunc0)(void*);
    SSLFunc0 func0 = NULL;
    func0 = (SSLFunc0)(FindFunction(funcName));
    CHECKFUNCTION(dynMsg, 0, funcName,)
    DYN_OPENSSL_sk_pop_free(extlist, func0, dynMsg);
}
