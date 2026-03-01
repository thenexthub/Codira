/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include "socket_buffer.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include "securec.h"

#ifndef MAX_MALLOC_SIZE
#define MAX_MALLOC_SIZE 2147483647 /* max 2 GB */
#endif

#ifdef __arm__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Watomic-alignment"
#endif

typedef struct SockBuffer {
    int32_t rBufSize;
    int32_t wBufSize;
    char* rBuf;
    char* wBuf;
    atomic_llong handle;
    atomic_int count;
} SocketBuffer;

extern void* CODE_SOCKET_MallocWithInit(size_t size)
{
    if (size == 0 || size > MAX_MALLOC_SIZE) {
        return NULL;
    }
    return calloc(1, size);
}

extern SocketBuffer* CODE_SOCKET_BufferInit(long long handle, int32_t rBufSize, int32_t wBufSize)
{
    if (handle == -1 || rBufSize <= 0 || wBufSize <= 0) {
        return NULL;
    }
    SocketBuffer* sockBuf = (SocketBuffer*)CODE_SOCKET_MallocWithInit(sizeof(SocketBuffer));
    if (sockBuf == NULL) {
        return NULL;
    }
    sockBuf->rBuf = (char*)CODE_SOCKET_MallocWithInit((size_t)rBufSize);
    if (sockBuf->rBuf == NULL) {
        free(sockBuf);
        return NULL;
    }
    sockBuf->wBuf = (char*)CODE_SOCKET_MallocWithInit((size_t)wBufSize);
    if (sockBuf->wBuf == NULL) {
        free(sockBuf->rBuf);
        free(sockBuf);
        return NULL;
    }
    atomic_init(&sockBuf->handle, handle);
    atomic_init(&sockBuf->count, 1);
    sockBuf->rBufSize = rBufSize;
    sockBuf->wBufSize = wBufSize;
    return sockBuf;
}

static void CODE_SocketFreeWrapperBuffer(SocketBuffer* sockBuf)
{
    char* itemR = sockBuf->rBuf;
    sockBuf->rBuf = NULL;
    sockBuf->rBufSize = 0;
    char* itemW = sockBuf->wBuf;
    sockBuf->wBuf = NULL;
    sockBuf->wBufSize = 0;
    free(itemR);
    free(itemW);
    free(sockBuf);
}

static bool CODE_SocketIncreaseRef(SocketBuffer* sockBuf)
{
    int item = atomic_fetch_add(&sockBuf->count, 1);
    long long handle = atomic_load(&sockBuf->handle);
    if (handle == -1 || item == 0) {
        atomic_fetch_sub(&sockBuf->count, 1);
        return true;
    }
    return false;
}

static bool CODE_SocketDecreaseRef(SocketBuffer* sockBuf)
{
    int item = atomic_fetch_sub(&sockBuf->count, 1);
    if (item == 1) {
        atomic_store(&sockBuf->handle, -1);
        CODE_SocketFreeWrapperBuffer(sockBuf);
        return true;
    }
    return false;
}

extern int32_t CODE_SOCKET_BufferRead(
    SocketBuffer* sockBuf, size_t bufOff, int32_t readSize, int64_t timeout, int32_t flags)
{
    if (CODE_SocketIncreaseRef(sockBuf)) {
        return 0;
    }
    long long handle = atomic_load(&sockBuf->handle);
    int32_t maxReadSize = readSize;
    if (sockBuf->rBufSize < maxReadSize) {
        maxReadSize = sockBuf->rBufSize;
    }
    int32_t recvLen = 0;
    if (timeout < 0) {
        recvLen = CODE_MRT_SockRecv(handle, (const char*)sockBuf->rBuf + bufOff, (unsigned int)maxReadSize, flags);
    } else {
        recvLen = CODE_MRT_SockRecvTimeout(
            handle, (const char*)sockBuf->rBuf + bufOff, (unsigned int)maxReadSize, flags, (uint64_t)timeout);
    }
    if (CODE_SocketDecreaseRef(sockBuf)) {
        return 0; // This affects subsequent operations. Therefore, return 0 directly.
    }
    return recvLen;
}

extern int32_t CODE_SOCKET_BufferRecvFrom(
    SocketBuffer* sockBuf, size_t bufOff, int32_t readSize, int64_t timeout, struct SockAddr* addr, int32_t flags)
{
    if (CODE_SocketIncreaseRef(sockBuf)) {
        return 0;
    }
    long long handle = atomic_load(&sockBuf->handle);
    int32_t maxReadSize = readSize;
    if (sockBuf->rBufSize < maxReadSize) {
        maxReadSize = sockBuf->rBufSize;
    }
    int32_t recvLen = 0;

    recvLen = CODE_MRT_SockRecvfromTimeout(
        handle, (void*)sockBuf->rBuf + bufOff, (unsigned int)maxReadSize, flags, addr, (uint64_t)timeout);

    if (CODE_SocketDecreaseRef(sockBuf)) {
        return 0; // This affects subsequent operations. Therefore, return 0 directly.
    }
    return recvLen;
}

extern int32_t CODE_SOCKET_BufferWrite(
    SocketBuffer* sockBuf, size_t bufOff, int32_t writeSize, int64_t timeout, int32_t flags)
{
    if (CODE_SocketIncreaseRef(sockBuf)) {
        return 0;
    }
    long long handle = atomic_load(&sockBuf->handle);
    int32_t sendLen = 0;
    if (timeout < 0) {
        sendLen = CODE_MRT_SockSend(handle, (const char*)sockBuf->wBuf + bufOff, (unsigned int)writeSize, flags);
    } else {
        sendLen = CODE_MRT_SockSendTimeout(
            handle, (const char*)sockBuf->wBuf + bufOff, (unsigned int)writeSize, flags, (uint64_t)timeout);
    }
    (void)CODE_SocketDecreaseRef(sockBuf); // The value 0 is not returned because subsequent operations are not affected.
    return sendLen;
}

extern int32_t CODE_SOCKET_BufferSendto(SocketBuffer* sockBuf, size_t bufOff, int32_t writeSize, int64_t timeout,
    const struct SockAddr* addr, int32_t flags)
{
    if (CODE_SocketIncreaseRef(sockBuf)) {
        return 0;
    }
    long long handle = atomic_load(&sockBuf->handle);
    int32_t sendLen =
        CODE_MRT_SockSendto(handle, (const char*)sockBuf->wBuf + bufOff, (unsigned int)writeSize, flags, addr);

    (void)CODE_SocketDecreaseRef(sockBuf); // The value 0 is not returned because subsequent operations are not affected.
    return sendLen;
}

extern int32_t CODE_SOCKET_BufferRCopy(SocketBuffer* sockBuf, const char* arrBuf, int64_t bufLen, int32_t copyLen)
{
    if (CODE_SocketIncreaseRef(sockBuf) || bufLen <= 0 || copyLen <= 0) {
        return 0;
    }
    int32_t ret = memcpy_s((void*)arrBuf, (size_t)bufLen, sockBuf->rBuf, (size_t)copyLen);
    (void)CODE_SocketDecreaseRef(sockBuf); // The value 0 is not returned because subsequent operations are not affected.
    if (ret == EOK) {
        return copyLen;
    }
    return -1;
}

extern int32_t CODE_SOCKET_BufferWCopy(SocketBuffer* sockBuf, const char* arrBuf, int64_t bufLen, int32_t copyLen)
{
    if (CODE_SocketIncreaseRef(sockBuf) || bufLen <= 0 || copyLen <= 0) {
        return 0;
    }
    int32_t ret = memcpy_s((void*)sockBuf->wBuf, (size_t)bufLen, arrBuf, (size_t)copyLen);
    if (CODE_SocketDecreaseRef(sockBuf)) {
        return 0; // This affects subsequent operations. Therefore, return 0 directly.
    }
    if (ret == 0) {
        return copyLen;
    }
    return -1;
}

/**
 * Does close socket and derecare refcount causing memory removal if necessary.
 * The knownHandle different from -1 makes this function only work
 * if the handle is the same (sockBuf.handle = knownHandle)
 * when knownHandle = -1, this function does always do work.
 * After invoking this function, sockBuf.handle is set to -1 (unless knownHandle doesn't match).
 * @return 0 on success, -1 if handle doesn't match
 */
extern int32_t CODE_SOCKET_BufferClose(SocketBuffer* sockBuf, long long knownHandle)
{
    long long handle;
    if (knownHandle == -1) {
        // we always take handle from sockBuf
        handle = atomic_exchange(&sockBuf->handle, -1);
    } else if (!atomic_compare_exchange_strong(&sockBuf->handle, &knownHandle, -1)) {
        return -1; // handle doesn't match so we abort
    } else {
        // handle matches and we got it swapped
        handle = knownHandle;
    }
    if (handle == -1) {
        return 0;
    }
    int32_t ret = CODE_MRT_SockClose(handle);
    (void)CODE_SocketDecreaseRef(sockBuf); // The value 0 is not returned because subsequent operations are not affected.
    return ret;
}

#ifdef __arm__
#pragma GCC diagnostic pop
#endif
