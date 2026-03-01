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


#ifndef COMMON_ALIAS_SYNC
#define COMMON_ALIAS_SYNC

MRT_EXPORT void CODE_MCC_FutureInit(void* ptr) __attribute__((alias("MCC_FutureInit")));
MRT_EXPORT bool CODE_MCC_FutureIsComplete(const void* ptr) __attribute__((alias("MCC_FutureIsComplete")));
MRT_EXPORT void CODE_MCC_FutureNotifyAll(void* ptr) __attribute__((alias("MCC_FutureNotifyAll")));
MRT_EXPORT int CODE_MCC_MutexInit(void* ptr) __attribute__((alias("MCC_MutexInit")));
MRT_EXPORT bool CODE_MCC_MutexCheckStatus(void* ptr) __attribute__((alias("MCC_MutexCheckStatus")));
MRT_EXPORT void CODE_MCC_MutexUnlock(const void* ptr) __attribute__((alias("MCC_MutexUnlock")));
MRT_EXPORT void CODE_MCC_MutexTryLock(const void* ptr, void* waitQueuePtr) __attribute__((alias("MCC_MutexTryLock")));
MRT_EXPORT int CODE_MCC_WaitQueueInit(void* ptr) __attribute__((alias("MCC_WaitQueueInit")));
MRT_EXPORT int CODE_MCC_WaitQueueForMonitorInit(void* ptr) __attribute__((alias("MCC_WaitQueueForMonitorInit")));
MRT_EXPORT void CODE_MCC_MonitorNotify(const void* ptr) __attribute__((alias("MCC_MonitorNotify")));
MRT_EXPORT void CODE_MCC_MonitorNotifyAll(const void* ptr) __attribute__((alias("MCC_MonitorNotifyAll")));
MRT_EXPORT void CODE_MCC_MultiConditionMonitorNotify(const void* ptr, void* waitQueuePtr)
    __attribute__((alias("MCC_MultiConditionMonitorNotify")));
MRT_EXPORT void CODE_MCC_MultiConditionMonitorNotifyAll(const void* ptr, void* waitQueuePtr)
    __attribute__((alias("MCC_MultiConditionMonitorNotifyAll")));
MRT_EXPORT bool CODE_MCC_IsThreadObjectInited() __attribute__((alias("MCC_IsThreadObjectInited")));
MRT_EXPORT void* CODE_MCC_GetCurrentCODEThreadObject() __attribute__((alias("MRT_GetCurrentCODEThreadObject")));
MRT_EXPORT void CODE_MCC_SetCurrentCODEThreadObject(void* ptr) __attribute__((alias("MCC_SetCurrentCODEThreadObject")));
MRT_EXPORT void CODE_MRT_SetCODEThreadName(void* handle, uint8_t* name, size_t len)
    __attribute__((alias("MRT_SetCODEThreadName")));
MRT_EXPORT int64_t CODE_MRT_GetCODEThreadId(void* handle) __attribute__((alias("MRT_GetCODEThreadId")));
MRT_EXPORT int64_t CODE_MRT_GetCODEThreadState(void* handle) __attribute__((alias("MRT_GetCODEThreadState")));
MRT_EXPORT void* CODE_MRT_GetCurrentCODEThread() __attribute__((alias("MRT_GetCurrentCODEThread")));

#endif
