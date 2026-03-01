/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include "securec.h"
#include "api.h"
#include "opensslSymbols.h"

typedef struct BioDataS {
    void* buffer;
    size_t length;
    size_t position;
    int eof;
} BioData;

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static atomic_uintptr_t g_methodPtr = 0;

static int BioCreate(BIO* bio)
{
    if (bio == NULL) {
        return 0; // 0 - fail for BIO_METHOD.create
    }

    BioData* data = malloc(sizeof(BioData));
    if (data == NULL) {
        return 0;
    }

    data->buffer = NULL;
    data->length = 0;
    data->position = 0;
    data->eof = 0;

    DYN_BIO_set_init(bio, 1, NULL);
    DYN_BIO_set_data(bio, data, NULL);
    DYN_BIO_set_flags(bio, 0, NULL);
    return 1;
}

static int BioDestroy(BIO* bio)
{
    if (bio == NULL) {
        return 1;
    }

    BioData* data = (BioData*)DYN_BIO_get_data(bio, NULL);
    if (data != NULL) {
        DYN_BIO_set_data(bio, NULL, NULL);
        data->buffer = NULL;
        free(data);
    }

    return 0;
}

static long BioCtl(BIO* bio, int cmd, long num, void* ptr)
{
    (void)num;
    (void)ptr;

    if (cmd == BIO_CTRL_EOF) {
        BioData* data = (BioData*)DYN_BIO_get_data(bio, NULL);
        if (data != NULL) {
            return (long)(data->length == data->position && data->eof != 0);
        }
    }
    if (cmd == BIO_CTRL_FLUSH) {
        return 1;
    }
    if (cmd == BIO_CTRL_PENDING) {
        BioData* data = (BioData*)DYN_BIO_get_data(bio, NULL);
        if (data != NULL) {
            return (long)(data->length - data->position);
        }
    }

    return 0;
}

static int BioRead(BIO* bio, char* resultBuffer, int length)
{
    // we allow resultBuffer = NULL | length == 0 intentionally as it's useful for EOF polling
    if (bio == NULL || length < 0) {
        return -1;
    }

    DYN_BIO_clear_retry_flags(bio, NULL);

    BioData* data = (BioData*)DYN_BIO_get_data(bio, NULL);
    if (data == NULL) {
        DYN_BIO_set_retry_reason(bio, BIO_R_NULL_PARAMETER, NULL);
        return -1;
    }

    size_t remaining = data->length - data->position;
    if (remaining == 0 || data->buffer == NULL) {
        if (data->eof != 0) {
            return 0;
        }

        DYN_BIO_set_retry_read(bio, NULL);
        return -1;
    }

    if (resultBuffer == NULL || length <= 0) {
        DYN_BIO_set_retry_read(bio, NULL);
        return -1;
    }

    size_t resultBufferSize = (size_t)length;
    size_t batchSize = resultBufferSize;

    if (batchSize > remaining) {
        batchSize = remaining;
    }

    char* buffer = (char*)data->buffer;
    buffer += data->position;

    errno_t copyResult = memcpy_s(resultBuffer, resultBufferSize, (const void*)buffer, batchSize);
    if (copyResult != EOK) {
        return -1;
    }
    data->position += batchSize;

    return (int)batchSize;
}

static int BioWrite(BIO* bio, const char* sourceBuffer, int length)
{
    if (bio == NULL) {
        return -1;
    }
    if (sourceBuffer == NULL && length == 0) {
        return 0;
    }
    if (sourceBuffer == NULL || length == 0) {
        return -1;
    }

    DYN_BIO_clear_retry_flags(bio, NULL);

    BioData* data = (BioData*)DYN_BIO_get_data(bio, NULL);
    if (data == NULL) {
        DYN_BIO_set_retry_reason(bio, BIO_R_INVALID_ARGUMENT, NULL);
        return -1;
    }

    const size_t remaining = data->length - data->position;
    if (remaining == 0 && length > 0) {
        DYN_BIO_set_retry_write(bio, NULL);
        return -1;
    }
    if (remaining == 0 || data->buffer == NULL || length == 0) {
        return 0;
    }

    size_t batchSize = (size_t)length;

    if (batchSize > remaining) {
        batchSize = remaining;
    }

    char* buffer = (char*)data->buffer;
    buffer += data->position;

    errno_t copyResult = memcpy_s((void*)buffer, remaining, (const void*)sourceBuffer, batchSize);
    if (copyResult != EOK) {
        return -1;
    }
    data->position += batchSize;

    return (int)batchSize;
}

static int BioPuts(BIO* bio, const char* text)
{
    return BioWrite(bio, text, (int)strlen(text));
}

static BIO_METHOD* CreateMethod(ExceptionData* exception, DynMsg* dynMsg)
{
    BIO_METHOD* m;
    int index = DYN_BIO_get_new_index(dynMsg);
    if (index == -1) {
        HandleError(exception, "BIO_get_new_index() failed", dynMsg);
        return NULL;
    }

    m = DYN_BIO_meth_new(index | BIO_TYPE_SOURCE_SINK, "code.tls.PinnedArray", dynMsg);
    if (m == NULL) {
        HandleError(exception, "BIO_meth_new() failed", dynMsg);
        return NULL;
    }

    int rc = 1;

    if (!LoadDynFuncForCreateMethod(dynMsg)) {
        return NULL;
    }

    rc &= DYN_BIO_meth_set_read(m, BioRead, dynMsg);
    rc &= DYN_BIO_meth_set_write(m, BioWrite, dynMsg);
    rc &= DYN_BIO_meth_set_puts(m, BioPuts, dynMsg);
    rc &= DYN_BIO_meth_set_ctrl(m, BioCtl, dynMsg);

    rc &= DYN_BIO_meth_set_create(m, BioCreate, dynMsg);
    rc &= DYN_BIO_meth_set_destroy(m, BioDestroy, dynMsg);
    if (rc != 1) {
        HandleError(exception, "BIO_meth_set_XXX() failed", dynMsg);
        DYN_BIO_meth_free(m, dynMsg);
        return NULL;
    }

    return m;
}

static BIO_METHOD* GetMethodSlowpath(ExceptionData* exception, DynMsg* dynMsg)
{
    pthread_mutex_lock(&g_mutex);

    BIO_METHOD* m = (BIO_METHOD*)atomic_load(&g_methodPtr);
    if (m == NULL) {
        m = CreateMethod(exception, dynMsg);
        atomic_store(&g_methodPtr, (uintptr_t)m);
    }

    pthread_mutex_unlock(&g_mutex);

    return m;
}

BIO_METHOD* CODE_TLS_BIO_GetMethod(ExceptionData* exception, DynMsg* dynMsg)
{
    BIO_METHOD* m = (BIO_METHOD*)atomic_load(&g_methodPtr);
    if (m != NULL) {
        return m;
    }

    return GetMethodSlowpath(exception, dynMsg);
}

int CODE_TLS_BIO_Map(BIO* bio, void* pointer, size_t length, int eof, ExceptionData* exception, DynMsg* dynMsg)
{
    NOT_NULL_OR_FAIL(exception, bio, dynMsg);
    CHECK_OR_FAIL(exception, pointer != NULL || length == 0, dynMsg);

    BioData* data = (BioData*)DYN_BIO_get_data(bio, dynMsg);
    if (data == NULL) {
        HandleError(exception, "BIO has no data", dynMsg);
        return CODETLS_FAIL;
    }

    data->buffer = pointer;
    data->position = 0;
    data->length = length;
    data->eof = eof;

    return CODETLS_OK;
}

int CODE_TLS_BIO_Unmap(BIO* bio, int eof, ExceptionData* exception, DynMsg* dynMsg)
{
    NOT_NULL_OR_FAIL(exception, bio, dynMsg);

    BioData* data = (BioData*)DYN_BIO_get_data(bio, dynMsg);
    if (data == NULL) {
        HandleError(exception, "BIO has no data", dynMsg);
        return CODETLS_FAIL;
    }

    int position = (int)data->position;

    data->buffer = NULL;
    data->position = 0;
    data->length = 0;
    data->eof = eof;

    return position;
}
