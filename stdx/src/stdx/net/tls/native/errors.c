/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stddef.h>
#include <stdbool.h>
#include "securec.h"
#include "api.h"
#include "opensslSymbols.h"

struct ExceptionDataS {
    const char* message;      // this is always allocated using malloc
    const char* constMessage; // this is never allocated
};

void ExceptionClear(ExceptionData* exception, DynMsg* dynMsg)
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

static void HandleErrorPutFromStack(BIO* buffer, DynMsg* dynMsg)
{
    if (buffer == NULL) {
        return;
    }
    char codeBuffer[16];
    int maxErrors = 100;
    for (int count = 0; count < maxErrors; ++count) {
        unsigned long error = DYN_ERR_get_error(dynMsg);
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
            int reason = ERR_GET_REASON(error) & ERR_REASON_MASK_COMPAT;
            if (snprintf_s(codeBuffer, sizeof(codeBuffer), sizeof(codeBuffer) - 1, " (%d)", reason) > 0) {
                DYN_BIO_puts(buffer, codeBuffer, dynMsg);
            }
        }
    }
}

static const char* HandleErrorBuildString(BIO* buffer, DynMsg* dynMsg)
{
    if (buffer == NULL) {
        return NULL;
    }

    DYN_BIO_write(buffer, "\0", 1, dynMsg);

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
        exception->constMessage = NULL;
    }

    if (exception->message != NULL) {
        const char* existingMessage = exception->message;
        DYN_BIO_puts(buffer, ", ", dynMsg);
        DYN_BIO_puts(buffer, existingMessage, dynMsg);
        DYN_CRYPTO_free((void*)existingMessage, dynMsg);
        exception->message = NULL;
    }

    exception->message = HandleErrorBuildString(buffer, dynMsg);
}

void HandleError(ExceptionData* exception, const char* fallback, DynMsg* dynMsg)
{
    if (exception == NULL) {
        return;
    }

    if (DYN_ERR_peek_error(dynMsg) == 0) {
        exception->constMessage = fallback;
        return;
    }

    BIO* buffer = (BIO*)DYN_BIO_new(DYN_BIO_s_mem(dynMsg), dynMsg);
    if (buffer == NULL) {
        exception->constMessage = fallback;
        return;
    }

    DYN_BIO_puts(buffer, fallback, dynMsg);
    DYN_BIO_puts(buffer, ": ", dynMsg);

    HandleErrorPutFromStack(buffer, dynMsg);

    AppendErrorMessage(exception, buffer, dynMsg);

    if (exception->message == NULL) {
        exception->constMessage = fallback;
    }

    DYN_BIO_free(buffer, dynMsg);
}

void HandleAlertError(ExceptionData* exception, const char* description, const char* type, DynMsg* dynMsg)
{
    if (exception == NULL) {
        return;
    }

    BIO* buffer = (BIO*)DYN_BIO_new(DYN_BIO_s_mem(dynMsg), dynMsg);
    if (buffer == NULL) {
        return;
    }

    DYN_BIO_puts(buffer, "TLS alert", dynMsg);

    if (description != NULL) {
        DYN_BIO_puts(buffer, " ", dynMsg);
        DYN_BIO_puts(buffer, description, dynMsg);
    }
    if (type != NULL) {
        DYN_BIO_puts(buffer, "(", dynMsg);
        DYN_BIO_puts(buffer, type, dynMsg);
        DYN_BIO_puts(buffer, ")", dynMsg);
    }

    AppendErrorMessage(exception, buffer, dynMsg);

    DYN_BIO_free(buffer, dynMsg);
}

static void FormatFailedAssertionError(ExceptionData* exception, const char* description, DynMsg* dynMsg)
{
    BIO* buffer = DYN_BIO_new(DYN_BIO_s_mem(dynMsg), dynMsg);
    if (buffer == NULL) {
        exception->constMessage = description;
        return;
    }

    DYN_BIO_puts(buffer, "Predicate failed: ", dynMsg);
    DYN_BIO_puts(buffer, description, dynMsg);
    AppendErrorMessage(exception, buffer, dynMsg);

    DYN_BIO_free(buffer, dynMsg);
}

bool CheckOrFillException(ExceptionData* exception, bool condition, const char* description, DynMsg* dynMsg)
{
    if (condition) {
        return true;
    }

    if (exception != NULL && description != NULL) {
        FormatFailedAssertionError(exception, description, dynMsg);
    }

    return false;
}

static void FormatNullAssertionError(ExceptionData* exception, const char* name, DynMsg* dynMsg)
{
    BIO* buffer = DYN_BIO_new(DYN_BIO_s_mem(dynMsg), dynMsg);
    if (buffer == NULL) {
        exception->constMessage = name;
        return;
    }

    DYN_BIO_puts(buffer, name, dynMsg);
    DYN_BIO_puts(buffer, " shouldn't be NULL", dynMsg);
    AppendErrorMessage(exception, buffer, dynMsg);

    DYN_BIO_free(buffer, dynMsg);
}

bool CheckNotNull(ExceptionData* exception, const void* candidate, const char* name, DynMsg* dynMsg)
{
    if (candidate != NULL) {
        return true;
    }

    if (exception != NULL && name != NULL) {
        FormatNullAssertionError(exception, name, dynMsg);
    }

    return false;
}
