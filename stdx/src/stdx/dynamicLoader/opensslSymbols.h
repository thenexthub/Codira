/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef HEADFILE_SSL
#define HEADFILE_SSL
#include <openssl/bio.h>
#include <openssl/dh.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <stdbool.h>
#include <string.h>

#ifndef OPENSSL_VERSION
#define OPENSSL_VERSION 0
#endif

#ifndef SSL_CTRL_SET_DH_AUTO
#define SSL_CTRL_SET_DH_AUTO 118
#endif

#ifndef SSL_CTRL_SET_MIN_PROTO_VERSION
#define SSL_CTRL_SET_MIN_PROTO_VERSION 123
#endif

#ifndef SSL_CTRL_SET_MAX_PROTO_VERSION
#define SSL_CTRL_SET_MAX_PROTO_VERSION 124
#endif

#ifndef OPENSSL_FILE
#ifdef OPENSSL_NO_FILENAMES
#define OPENSSL_FILE ""
#define OPENSSL_LINE 0
#else
#define OPENSSL_FILE __FILE__
#define OPENSSL_LINE __LINE__
#endif
#endif

typedef struct DynMsg {
    bool found;
    char* funcName;
} DynMsg;

DynMsg* MallocDynMsg(void);
void FreeDynMsg(DynMsg* dynMsgPtr);

char* DYN_OPENSSL_strdup(const char* str, DynMsg* dynMsg);
char* DYN_OPENSSL_strndup(const char* str, size_t s, DynMsg* dynMsg);
void* DYN_OPENSSL_memdup(void* str, size_t s, DynMsg* dynMsg);
void DYN_CRYPTO_free(void* str, DynMsg* dynMsg);
void CODE_TLS_DYN_CRYPTO_free(void* str, DynMsg* dynMsg);
void* DYN_OPENSSL_secure_malloc(size_t num, DynMsg* dynMsg);
void DYN_OPENSSL_secure_free(void* ptr, DynMsg* dynMsg);
void* DYN_OPENSSL_zalloc(size_t num, DynMsg* dynMsg);

int DYN_SSL_CTX_set_min_proto_version(SSL_CTX* ctx, int version, DynMsg* dynMsg);
int DYN_SSL_CTX_set_max_proto_version(SSL_CTX* ctx, int version, DynMsg* dynMsg);
long DYN_SSL_CTX_set_dh_auto(SSL_CTX* ctx, int onoff, DynMsg* dynMsg);
int DYN_SSL_CTX_add0_chain_cert(SSL_CTX* ctx, X509* x509, DynMsg* dynMsg);
long DYN_SSL_CTX_set_mode(SSL_CTX* ctx, long mode, DynMsg* dynMsg);
long DYN_SSL_CTX_set1_sigalgs_list(SSL_CTX* ctx, const char* str, DynMsg* dynMsg);
long DYN_SSL_CTX_set_session_cache_mode(SSL_CTX* ctx, long mode, DynMsg* dynMsg);

size_t DYN_BIO_pending(BIO* b, DynMsg* dynMsg);
int DYN_BIO_eof(BIO* b, DynMsg* dynMsg);
void DYN_BIO_clear_retry_flags(BIO* b, DynMsg* dynMsg);
long DYN_BIO_get_mem_ptr(BIO* b, BUF_MEM** pp, DynMsg* dynMsg);

int DYN_SSL_set_tlsext_host_name(SSL* ssl, const char* name, DynMsg* dynMsg);
long DYN_SSL_CTX_set_tlsext_servername_callback(SSL_CTX* ctx, int (*cb)(void* s, int* al, void* arg), DynMsg* dynMsg);

void DYN_BIO_set_retry_read(BIO* a, DynMsg* dynMsg);
void DYN_BIO_set_retry_write(BIO* a, DynMsg* dynMsg);
BIO* DYN_BIO_new_mem(DynMsg* dynMsg);

bool LoadDynFuncForAlpnCallback(DynMsg* dynMsg);
bool LoadFuncForNewSessionCallback(DynMsg* dynMsg);
bool LoadDynFuncForCreateMethod(DynMsg* dynMsg);
bool LoadDynFuncCertVerifyCallback(DynMsg* dynMsg);
bool LoadDynFuncForCustomVerifyCallback(DynMsg* dynMsg);
bool LoadDynForInfoCallback(DynMsg* dynMsg);

void DynPopFree(void* extlist, char* funcName, DynMsg* dynMsg);

#define DECLAREFUNCTION0(name, type0) type0 DYN_##name(DynMsg* dynMsg);
#define DECLAREFUNCTION1(name, type0, type1) type0 DYN_##name(type1 arg1, DynMsg* dynMsg);
#define DECLAREFUNCTION2(name, type0, type1, type2) type0 DYN_##name(type1 arg1, type2 arg2, DynMsg* dynMsg);
#define DECLAREFUNCTION3(name, type0, type1, type2, type3)                                                             \
    type0 DYN_##name(type1 arg1, type2 arg2, type3 arg3, DynMsg* dynMsg);
#define DECLAREFUNCTION4(name, type0, type1, type2, type3, type4)                                                      \
    type0 DYN_##name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, DynMsg* dynMsg);
#define DECLAREFUNCTION5(name, type0, type1, type2, type3, type4, type5)                                               \
    type0 DYN_##name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, DynMsg* dynMsg);
#define DECLAREFUNCTION6(name, type0, type1, type2, type3, type4, type5, type6)                                        \
    type0 DYN_##name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, DynMsg* dynMsg);
#define DECLAREFUNCTION7(name, type0, type1, type2, type3, type4, type5, type6, type7)                                 \
    type0 DYN_##name(                                                                                                  \
        type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, DynMsg* dynMsg);
#define DECLAREFUNCTION8(name, type0, type1, type2, type3, type4, type5, type6, type7, type8)                          \
    type0 DYN_##name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, type8 arg8,   \
        DynMsg* dynMsg);
#define DECLAREFUNCTIONCB2(name, type0, type1, type2) type0 DYN_##name(type1, type2, DynMsg* dynMsg);
#define DECLAREFUNCTIONCB3(name, type0, type1, type2, type3) type0 DYN_##name(type1, type2, type3, DynMsg* dynMsg);
#include "declareFunction.inc"
#undef DECLAREFUNCTION0
#undef DECLAREFUNCTION1
#undef DECLAREFUNCTION2
#undef DECLAREFUNCTION3
#undef DECLAREFUNCTION4
#undef DECLAREFUNCTION5
#undef DECLAREFUNCTION6
#undef DECLAREFUNCTION7
#undef DECLAREFUNCTION8

int DYN_BN_num_bytes(const BIGNUM* bn, DynMsg* dynMsg);

#endif
