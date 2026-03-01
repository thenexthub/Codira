/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/pkcs12.h>
#include "securec.h"
#include "api.h"

static const char* BioToString(BIO* buffer, DynMsg* dynMsg)
{
    if (buffer == NULL) {
        return NULL;
    }

    BUF_MEM* ptr = NULL;
    const char* message = NULL;
    long rc = DYN_BIO_get_mem_ptr(buffer, &ptr, dynMsg);
    if (rc == 1 && ptr != NULL && ptr->data != NULL && ptr->length > 0) {
        message = (const char*)DYN_OPENSSL_strndup(ptr->data, ptr->length, dynMsg);
    }
    return message;
}

static void FormatDescription(BIO* buffer, const char* typeName, int bits, DynMsg* dynMsg)
{
    if (buffer == NULL || typeName == NULL) {
        return;
    }
    if (bits > 0) {
        char bitsBuffer[32] = {};
        if (snprintf_s(bitsBuffer, sizeof(bitsBuffer), sizeof(bitsBuffer) - 1, "%s %d bits", typeName, bits) > 0) {
            (void)DYN_BIO_write(buffer, bitsBuffer, (int)strlen(bitsBuffer), dynMsg);
            if (dynMsg && dynMsg->found == false) {
                return;
            }
        }
    } else {
        (void)DYN_BIO_puts(buffer, typeName, dynMsg);
        if (dynMsg && dynMsg->found == false) {
            return;
        }
    }
}

/**
 * Try to briefly describe the provided key: key kind and bits strength.
 *
 * @return allocated memory (should be freed) or NULL if faied
 */
const char* X509DescribePrivateKey(EVP_PKEY* key, DynMsg* dynMsg)
{
    BIO* buffer = DYN_BIO_new_mem(dynMsg);
    if (buffer == NULL) {
        DYN_EVP_PKEY_free(key, dynMsg);
        return NULL;
    }
    int bits = 0;
    int pkeyType = 0;
    const char* typeName = "PK";
    pkeyType = DYN_EVP_PKEY_get_base_id(key, dynMsg);
    switch (pkeyType) {
        case EVP_PKEY_EC:
            typeName = "EC";
            break;
        case EVP_PKEY_RSA:
            typeName = "RSA";
            break;
        case EVP_PKEY_DH:
            typeName = "DH";
            break;
        case EVP_PKEY_POLY1305:
            typeName = "POLY1305";
            break;
        default:
            break;
    }
    bits = DYN_EVP_PKEY_get_bits(key, dynMsg);
    if (dynMsg && dynMsg->found == false) {
        return NULL;
    }
    FormatDescription(buffer, typeName, bits, dynMsg);
    char zero[] = {0}; // because of codestyle checker we can't use string literal here
    (void)DYN_BIO_write(buffer, zero, 1, dynMsg);
    const char* message = BioToString(buffer, dynMsg);
    DYN_BIO_vfree(buffer, dynMsg);
    return message;
}
