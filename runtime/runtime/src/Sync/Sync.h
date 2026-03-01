/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */


#ifndef MRT_SYNC_H
#define MRT_SYNC_H

#include <atomic>

#include "CodeScheduler.h"
#include "Base/AtomicSpinLock.h"
#include "Common/BaseObject.h"
#include "sema.h"
#include "waitqueue.h"

namespace MapleRuntime {
#ifdef __cplusplus
extern "C" {
#endif
struct CODEFuture {
    void* klass;
#ifdef __arm__
    uint32_t padding;
    uint32_t data[4];
#else
    long long int data[4]; // 4: occupied by _thread(1)/result(1)/executeFn(2)
#endif
    // Reader/writer functions of `completeFlag` (i.e., MCC_FutureIsComplete/FutureSetComplete)
    // are used as callbacks of runtime functions, they will be executed with atomicity.
    // So the variable is not an atomic type.
    std::atomic<bool> completeFlag;
    std::atomic<int> isWaitQueueInit;
    Waitqueue wq;
    AtomicSpinLock spinLock;

    static constexpr size_t SYNC_OBJECT_SIZE = 168; // the size of future object with typeinfo header
};

struct CODEMutex {
    void* klass;
#ifdef __arm__
    uint32_t padding;
#endif
    // atomic int64_t whose size should comfort to `MRT_GetCurrentThreadID`
    std::atomic<int64_t> ownerThreadId;
    // `ownCount` is always accessed when the mutex is held, so it can be non-atomic.
    uint64_t ownCount;
    std::atomic<int64_t> state; // includes waiter couter, locked, starve, spin
    bool isSemaInit;
    Sema sema;
};

struct CODEMonitor {
    void* klass;
#ifdef __arm__
    uint32_t padding;
#endif
    CODEMutex* mutexPtr;
    bool isWaitQueueInit;
    Waitqueue wq;
};

struct CODEWaitQueue {
    void* klass;
#ifdef __arm__
    uint32_t padding;
#endif
    bool isWaitQueueInit;
    Waitqueue wq;
};

struct CODEMultiConditionMonitor {
    void* klass;
#ifdef __arm__
    uint32_t padding;
#endif
    CODEMutex* mutexPtr;
};

void ReleaseNativeResource(BaseObject* obj);

void MCC_FutureInit(void* ptr);
bool MCC_FutureIsComplete(void* ptr);
void MRT_FutureWait(const void* ptr, int64_t timeout);
void MCC_FutureNotifyAll(const void* ptr);
int MCC_MutexInit(void* ptr);
void MCC_MutexLock(void* ptr);
void MCC_MutexLockSlowPath(void* ptr);
bool MCC_MutexTryLock(void* ptr);
bool MCC_MutexCheckStatus(const void* ptr);
void MCC_MutexUnlock(const void* ptr);
int MCC_WaitQueueForMonitorInit(void* ptr);
int MCC_WaitQueueInit(void* ptr);
bool MCC_MonitorWait(const void* ptr, int64_t timeout);
void MCC_MonitorNotify(const void* ptr);
void MCC_MonitorNotifyAll(const void* ptr);
bool MCC_MultiConditionMonitorWait(const void* ptr, void* waitQueuePtr, int64_t timeout);
void MCC_MultiConditionMonitorNotify(const void* ptr, const void* waitQueuePtr);
void MCC_MultiConditionMonitorNotifyAll(const void* ptr, const void* waitQueuePtr);
bool MCC_IsThreadObjectInited();
void* MRT_GetCurrentCODEThreadObject();
void MCC_SetCurrentCODEThreadObject(void* ptr);
void MRT_SetCODEThreadName(void* handle, uint8_t* name, size_t len);
int64_t MRT_GetCODEThreadId(void* handle);
int64_t MRT_GetCODEThreadState(void* handle);
void* MRT_GetCurrentCODEThread();
void MRT_ThreadWait();
void MRT_ThreadResumeAndWait(void* handle);
void MRT_ThreadReady(void* handle);
#ifdef __cplusplus
};
#endif
} // namespace MapleRuntime

#endif // MRT_SYNC_H
