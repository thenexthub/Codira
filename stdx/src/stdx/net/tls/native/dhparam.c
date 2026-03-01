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

static EVP_PKEY* LoadDHParam(const void* pem, size_t length, ExceptionData* exception, DynMsg* dynMsg)
{
    NOT_NULL_OR_RETURN(exception, pem, NULL, dynMsg);

    BIO* mem = InitBioWithPem(pem, length, exception, dynMsg);
    if (mem == NULL) {
        return NULL;
    }

    EVP_PKEY* param = (EVP_PKEY*)DYN_PEM_read_bio_Parameters(mem, NULL, dynMsg);
    if (dynMsg->found == false) {
        HandleError(exception, "Can not load openssl library or function PEM_read_bio_Parameters.", dynMsg);
    }
    if (param == NULL) {
        HandleError(exception, "Failed to create DH Parameter: PEM_read_bio_Parameters() failed", dynMsg);
    }

    DYN_BIO_vfree(mem, dynMsg);

    return param;
}

extern int CODE_TLS_DYN_SetDHParam(
    SSL_CTX* ctx, const void* keyPem, size_t length, ExceptionData* exception, DynMsg* dynMsg)
{
    EXCEPTION_OR_RETURN(exception, 0, dynMsg);
    NOT_NULL_OR_RETURN(exception, ctx, 0, dynMsg);

    if (keyPem == NULL || length == 0) {
        if (DYN_SSL_CTX_set_dh_auto(ctx, 1, dynMsg) == 0) {
            if (dynMsg->found == false) {
                HandleError(exception, "Can not load openssl library or function SSL_CTX_set_dh_auto.", dynMsg);
                return 0;
            }
            HandleError(exception, "Failed to apply default dh parameter: SSL_CTX_set_dh_auto() failed", dynMsg);
            return 0;
        }
        return 1;
    }

    EVP_PKEY* param = LoadDHParam(keyPem, length, exception, dynMsg);
    if (!param) {
        return 0;
    }

    if (DYN_SSL_CTX_set0_tmp_dh_pkey(ctx, param, dynMsg) == 0) {
        HandleError(exception, "Failed to apply dh parameter: SSL_CTX_set0_tmp_dh_pkey() failed", dynMsg);
        DYN_EVP_PKEY_free(param, dynMsg);
        return 0;
    }

    return 1;
}
