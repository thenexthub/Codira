/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef CODE_API_H
#define CODE_API_H

#include <stdbool.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include "opensslSymbols.h"

#define CODE_EOF 0
#define CODE_FAIL (-1)
#define CODE_NEED_READ (-2)
#define CODE_NEED_WRITE (-3)
#define CODE_OK 1

struct StringArrayResult {
    char** buffer;
    size_t size;
};

struct ByteResult {
    uint8_t* buffer;
    size_t size;
};

struct ByteArrayResult {
    struct ByteResult* buffer;
    size_t size;
};

struct UInt16Result {
    uint16_t* buffer;
    size_t size;
};

#define X509_RESULT_SIZE sizeof(struct StringArrayResult)

typedef struct ExceptionDataS ExceptionData;

typedef struct CipherSuite {
    const char* name;
} CipherSuite;

// this should be syncronized with foreign struct in Codira code
typedef struct EncryptedKeyParams {
    char* password;
    const unsigned char* iv;
    size_t ivLength;
    const char* cipherName;
} EncryptedKeyParams;

void X509ExceptionClear(ExceptionData* exception, DynMsg* dynMsg);

bool X509CheckOrFillException(ExceptionData* exception, bool condition, const char* description, DynMsg* dynMsg);

bool X509CheckNotNull(ExceptionData* exception, const void* candidate, const char* name, DynMsg* dynMsg);

void X509HandleError(ExceptionData* exception, const char* fallback, DynMsg* dynMsg);

const char* X509DescribePrivateKey(EVP_PKEY* key, DynMsg* dynMsg);

#endif
