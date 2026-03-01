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
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include "securec.h"
#include "api.h"

extern bool DYN_CODECheckKeyType(void* pkey, int keyType, DynMsg* dynMsg)
{
    int id = DYN_EVP_PKEY_get_base_id((void*)pkey, dynMsg);
    if (keyType == 0) {
        return (id == EVP_PKEY_RSA || id == EVP_PKEY_EC || id == EVP_PKEY_DSA);
    } else {
        return (id == keyType);
    }
}
