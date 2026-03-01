/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stddef.h>
#include <stdbool.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "securec.h"
#include "api.h"

struct ExceptionDataS {
    const char* message;      // this is always allocated using malloc
    const char* constMessage; // this is never allocated
};

void X509ExceptionClear(ExceptionData* exception, DynMsg* dynMsg)
{
    DYN_ERR_clear_error(dynMsg);
    if (exception != NULL) {
        if (exception->message != NULL) {
            DYN_CRYPTO_free((void*)exception->message, dynMsg);
            exception->message = NULL;
        }
        exception->constMessage = NULL;
    }
}

static void X509HandleErrorPutFromStack(BIO* buffer, DynMsg* dynMsg)
{
    if (buffer == NULL) {
        return;
    }
    char codeBuffer[16];
    int maxErrors = 100;
    for (int count = 0; count < maxErrors; ++count) {
        unsigned long error = DYN_ERR_get_error(dynMsg);
        if (dynMsg && dynMsg->found == false) {
            return;
        }
        if (error == 0) {
            break;
        }
        const char* message = DYN_ERR_reason_error_string(error, dynMsg);
        if (message != NULL) {
            if (count > 0) {
                DYN_BIO_puts(buffer, ", ", dynMsg);
            }
            DYN_BIO_puts(buffer, message, dynMsg);

// here we need this AND because we are compiling against openssl 3.x but running with openssl 1.x
// ERR_GET_REASON from openssl3 is unable to remove the func code assuming it to be always zero
// so we have no choice other than do it ourselves.
#define ERR_REASON_MASK_COMPAT 0xfff
            int reason = (int)((unsigned int)ERR_GET_REASON(error) & ERR_REASON_MASK_COMPAT);
            if (dynMsg && dynMsg->found == false) {
                return;
            }
            if (snprintf_s(codeBuffer, sizeof(codeBuffer), sizeof(codeBuffer) - 1, " (%d)", reason) > 0) {
                DYN_BIO_write(buffer, codeBuffer, (int)strlen(codeBuffer), dynMsg);
            }
        }
    }
}

static const char* X509HandleErrorBuildString(BIO* buffer, DynMsg* dynMsg)
{
    if (buffer == NULL) {
        return NULL;
    }
    char zero[] = {0};
    (void)DYN_BIO_write(buffer, zero, 1, dynMsg);
    if (dynMsg && dynMsg->found == false) {
        return NULL;
    }
    BUF_MEM* ptr = NULL;
    DYN_BIO_get_mem_ptr(buffer, &ptr, dynMsg);
    if (ptr == NULL || ptr->data == NULL || ptr->length == 0) {
        return NULL;
    }
    return (const char*)DYN_OPENSSL_strndup(ptr->data, ptr->length, dynMsg);
}

static void AppendErrorMessage(ExceptionData* exception, BIO* buffer, DynMsg* dynMsg)
{
    if (exception == NULL || buffer == NULL) {
        return;
    }
    if (exception->constMessage != NULL) {
        DYN_BIO_puts(buffer, ", ", dynMsg);
        DYN_BIO_puts(buffer, exception->constMessage, dynMsg);
        if (dynMsg && dynMsg->found == false) {
            return;
        }
        exception->constMessage = NULL;
    }
    if (exception->message != NULL) {
        const char* existingMessage = exception->message;
        DYN_BIO_puts(buffer, ", ", dynMsg);
        DYN_BIO_puts(buffer, existingMessage, dynMsg);
        DYN_CRYPTO_free((void*)existingMessage, dynMsg);
        if (dynMsg && dynMsg->found == false) {
            return;
        }
        exception->message = NULL;
    }
    exception->message = X509HandleErrorBuildString(buffer, dynMsg);
}

void X509HandleError(ExceptionData* exception, const char* fallback, DynMsg* dynMsg)
{
    if (exception == NULL) {
        return;
    }
    if (DYN_ERR_peek_error(dynMsg) == 0) {
        if (dynMsg && dynMsg->found == false) {
            return;
        }
        exception->constMessage = fallback;
        return;
    }
    BIO* buffer = DYN_BIO_new_mem(dynMsg);
    if (buffer == NULL) {
        exception->constMessage = fallback;
        return;
    }
    DYN_BIO_puts(buffer, fallback, dynMsg);
    DYN_BIO_puts(buffer, ": ", dynMsg);
    if (dynMsg && dynMsg->found == false) {
        return;
    }
    X509HandleErrorPutFromStack(buffer, dynMsg);
    if (dynMsg && dynMsg->found == false) {
        return;
    }
    AppendErrorMessage(exception, buffer, dynMsg);
    if (dynMsg && dynMsg->found == false) {
        return;
    }
    if (exception->message == NULL) {
        exception->constMessage = fallback;
    }
    DYN_BIO_free(buffer, dynMsg);
}

static void FormatFailedAssertionError(ExceptionData* exception, const char* description, DynMsg* dynMsg)
{
    BIO* buffer = DYN_BIO_new_mem(dynMsg);
    if (buffer == NULL) {
        exception->constMessage = description;
        return;
    }
    DYN_BIO_puts(buffer, "Predicate failed: ", dynMsg);
    DYN_BIO_puts(buffer, description, dynMsg);
    AppendErrorMessage(exception, buffer, dynMsg);
    DYN_BIO_free(buffer, dynMsg);
}

bool X509CheckOrFillException(ExceptionData* exception, bool condition, const char* description, DynMsg* dynMsg)
{
    if (condition) {
        return true;
    }
    if (exception != NULL && description != NULL) {
        FormatFailedAssertionError(exception, description, dynMsg);
    }
    return false;
}

static void X509FormatNullAssertionError(ExceptionData* exception, const char* name, DynMsg* dynMsg)
{
    BIO* buffer = DYN_BIO_new_mem(dynMsg);
    if (buffer == NULL) {
        exception->constMessage = name;
        return;
    }

    DYN_BIO_puts(buffer, name, dynMsg);
    DYN_BIO_puts(buffer, " shouldn't be NULL", dynMsg);
    AppendErrorMessage(exception, buffer, dynMsg);
    DYN_BIO_free(buffer, dynMsg);
}

bool X509CheckNotNull(ExceptionData* exception, const void* candidate, const char* name, DynMsg* dynMsg)
{
    if (candidate != NULL) {
        return true;
    }

    if (exception != NULL && name != NULL) {
        X509FormatNullAssertionError(exception, name, dynMsg);
    }

    return false;
}
