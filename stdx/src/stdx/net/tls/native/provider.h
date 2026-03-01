/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef KEYLESS_H
#define KEYLESS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <openssl/x509.h>
#include <stdarg.h>
#include <pthread.h>
#include "api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KEYLESS_KEY_TYPE_RSA (1)
#define KEYLESS_KEY_TYPE_EC (2)

typedef struct KeylessKey
{
    int type;    /* 1=rsa 2=ec */
    char* keyId; /* allocated */

    /* For RSA */
    unsigned char* n;
    size_t nLen;
    unsigned char* e;
    size_t eLen;

    /* For EC */
    unsigned char* ecPoint;
    size_t ecPointLen; /* encoded P in uncompressed form */
    char* groupName;   /* e.g., P-256 */

    DynMsg dynMsg; /* snapshot of the dyn loader state captured at creation */
    int refCnt;
} KeylessKey;

typedef struct Pkcs1DigestDescriptor
{
    const char* name;
    const unsigned char* derPrefix;
    size_t derLen;
    size_t hashLen;
} Pkcs1DigestDescriptor;

/**
 * Remote signing callback
 * @param keyId The identifier of the key to use for signing.
 * @param alg The algorithm to use for signing (e.g., "rsa-pss-sha256", "ecdsa-secp256r1-sha256").
 * @param digest Pointer to the message digest to be signed.
 * @param size Length of the digest in bytes.
 * @param written Pointer to receive the length of the signature written.
 * @return Pointer to the signature buffer (owned by the callback) or NULL on failure.
 * @note The signature buffer must remain valid until the next invocation of this callback.
 */
typedef char* (*KeylessRemoteSignCb)(const char* keyId, const char* alg, const unsigned char* digest, const int64_t size, int64_t* written);

/**
 * Remote decryption callback
 * @param keyId The identifier of the key to use for decryption.
 * @param digest Pointer to the ciphertext to be decrypted.
 * @param size Length of the ciphertext in bytes.
 * @param written Pointer to receive the length of the plaintext written.
 * @return Pointer to the plaintext buffer (owned by the callback) or NULL on failure.
 * @note The plaintext buffer must remain valid until the next invocation of this callback.
 */
typedef char* (*KeylessRemoteDecryptCb)(const char* keyId, const unsigned char* digest, const int64_t size, int64_t* written);

enum
{
    KEYLESS_LOG_OFF = 0,
    KEYLESS_LOG_ON = 1,
};

enum
{
    KEYLESS_LOAD_SUCCESS = 0,
    KEYLESS_PROVIDER_ADD_FAILED = 1,
    KEYLESS_PROVIDER_LOAD_FAILED = 2,
};

extern int8_t LOG_LEVEL;
static inline void KeylessProviderLog(const char* fmt, ...)
{
    if (LOG_LEVEL != KEYLESS_LOG_ON) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

typedef struct KeylessAsymcipherCtx
{
    void* keyData;
    int type;    /* 1=rsa */
    int padMode; /* expected RSA padding mode (e.g., RSA_PKCS1_PADDING) */
    DynMsg dynMsg;
} KeylessAsymcipherCtx;

typedef struct KeylessCallbackEntry
{
    char* keyId;
    KeylessRemoteSignCb signCb;
    KeylessRemoteDecryptCb decryptCb;

    struct KeylessCallbackEntry* next;
} KeylessCallbackEntry;

typedef struct KeylessProviderCtx
{
    pthread_rwlock_t callbackLock;
    KeylessCallbackEntry* callbacks;
} KeylessProviderCtx;

__attribute__((visibility("hidden"))) size_t KeylessKeyGetSize(const void* keyData);
__attribute__((visibility("hidden"))) const unsigned char* KeylessKeyGetN(const void* keyData);
__attribute__((visibility("hidden"))) size_t KeylessKeyGetNLen(const void* keyData);

/* Accessors used internally by provider modules */
__attribute__((visibility("hidden"))) KeylessRemoteSignCb KeylessLookupSignCb(const char* keyId);
__attribute__((visibility("hidden"))) KeylessRemoteDecryptCb KeylessLookupDecryptCb(const char* keyId);

/* Opaque keydata accessors (implemented in keymgmt) */
__attribute__((visibility("hidden"))) int KeylessKeyGetType(const void* keyData);          /* 1=rsa 2=ec */
__attribute__((visibility("hidden"))) const char* KeylessKeyGetGroup(const void* keyData); /* NULL if not EC */
__attribute__((visibility("hidden"))) const char* KeylessKeyGetId(const void* keyData);
__attribute__((visibility("hidden"))) int KeylessKeyUpRef(void* keyData);
__attribute__((visibility("hidden"))) void KeylessKeyFreeExtern(void* keyData);
__attribute__((visibility("hidden"))) size_t KeylessKeyGetRsaNLen(const void* keyData);
__attribute__((visibility("hidden"))) int CertIssuerSerialSha256Hex(const X509* crt, char out_hex[65], size_t out_hex_len); /* 64(sha256 length)chars + NULL */
__attribute__((visibility("hidden"))) int8_t GetKeylessLogLevel();
__attribute__((visibility("hidden"))) int KeylessErrorLibInit(void);
__attribute__((visibility("hidden"))) unsigned int GetEcOrderBitsFromGroup(const char* groupName);

/* Thread-local dynamic message accessors */
__attribute__((visibility("hidden"))) void KeylessProviderSetThreadDynMsg(const DynMsg* dynMsg);
__attribute__((visibility("hidden"))) DynMsg* KeylessProviderGetThreadDynMsg(void);
__attribute__((visibility("hidden"))) void KeylessCopyDynMsg(DynMsg* dst, const DynMsg* src);
__attribute__((visibility("hidden"))) bool KeylessCheckDynMsg(const DynMsg* dynMsg, const char* context);

__attribute__((visibility("hidden"))) char* GetCertSha256Hex(const X509* crt);

#ifdef __cplusplus
}
#endif

#endif // KEYLESS_H
