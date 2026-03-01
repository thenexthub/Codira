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


#ifndef MAC_ALIAS_SYNC
#define MAC_ALIAS_SYNC

MRT_EXPORT void CODE_MCC_FutureInit(void* ptr);
__asm__(".global _CODE_MCC_FutureInit\n\t.set _CODE_MCC_FutureInit, _MCC_FutureInit");
MRT_EXPORT bool CODE_MCC_FutureIsComplete(const void* ptr);
__asm__(".global _CODE_MCC_FutureIsComplete\n\t.set _CODE_MCC_FutureIsComplete, _MCC_FutureIsComplete");
MRT_EXPORT void CODE_MCC_FutureNotifyAll(void* ptr);
__asm__(".global _CODE_MCC_FutureNotifyAll\n\t.set _CODE_MCC_FutureNotifyAll, _MCC_FutureNotifyAll");
MRT_EXPORT int CODE_MCC_MutexInit(void* ptr);
__asm__(".global _CODE_MCC_MutexInit\n\t.set _CODE_MCC_MutexInit, _MCC_MutexInit");
MRT_EXPORT bool CODE_MCC_MutexCheckStatus(void* ptr);
__asm__(".global _CODE_MCC_MutexCheckStatus\n\t.set _CODE_MCC_MutexCheckStatus, _MCC_MutexCheckStatus");
MRT_EXPORT void CODE_MCC_MutexUnlock(const void* ptr);
__asm__(".global _CODE_MCC_MutexUnlock\n\t.set _CODE_MCC_MutexUnlock, _MCC_MutexUnlock");
MRT_EXPORT void CODE_MCC_MutexTryLock(const void* ptr, void* waitQueuePtr);
__asm__(".global _CODE_MCC_MutexTryLock\n\t.set _CODE_MCC_MutexTryLock, _MCC_MutexTryLock");
MRT_EXPORT int CODE_MCC_WaitQueueInit(void* ptr);
__asm__(".global _CODE_MCC_WaitQueueInit\n\t.set _CODE_MCC_WaitQueueInit, _MCC_WaitQueueInit");
MRT_EXPORT int CODE_MCC_WaitQueueForMonitorInit(void* ptr);
__asm__(
    ".global _CODE_MCC_WaitQueueForMonitorInit\n\t.set _CODE_MCC_WaitQueueForMonitorInit, _MCC_WaitQueueForMonitorInit");
MRT_EXPORT void CODE_MCC_MonitorNotify(const void* ptr);
__asm__(".global _CODE_MCC_MonitorNotify\n\t.set _CODE_MCC_MonitorNotify, _MCC_MonitorNotify");
MRT_EXPORT void CODE_MCC_MonitorNotifyAll(const void* ptr);
__asm__(".global _CODE_MCC_MonitorNotifyAll\n\t.set _CODE_MCC_MonitorNotifyAll, _MCC_MonitorNotifyAll");
MRT_EXPORT void CODE_MCC_MultiConditionMonitorNotify(const void* ptr, void* waitQueuePtr);
__asm__(
    ".global _CODE_MCC_MultiConditionMonitorNotify\n\t.set _CODE_MCC_MultiConditionMonitorNotify, "
    "_MCC_MultiConditionMonitorNotify");
MRT_EXPORT void CODE_MCC_MultiConditionMonitorNotifyAll(const void* ptr, void* waitQueuePtr);
__asm__(
    ".global _CODE_MCC_MultiConditionMonitorNotifyAll\n\t.set _CODE_MCC_MultiConditionMonitorNotifyAll, "
    "_MCC_MultiConditionMonitorNotifyAll");
MRT_EXPORT bool CODE_MRT_HasThreadObjectInited();
__asm__(".global _CODE_MCC_IsThreadObjectInited\n\t.set _CODE_MCC_IsThreadObjectInited, "
    "_MCC_IsThreadObjectInited");
MRT_EXPORT void* CODE_MCC_GetCurrentCODEThreadObject();
__asm__(".global _CODE_MCC_GetCurrentCODEThreadObject\n\t.set _CODE_MCC_GetCurrentCODEThreadObject, "
    "_MRT_GetCurrentCODEThreadObject");
MRT_EXPORT void CODE_MCC_SetCurrentCODEThreadObject(void* ptr);
__asm__(".global _CODE_MCC_SetCurrentCODEThreadObject\n\t.set _CODE_MCC_SetCurrentCODEThreadObject, "
    "_MCC_SetCurrentCODEThreadObject");
MRT_EXPORT void CODE_MRT_SetCODEThreadName(void* handle, uint8_t* name, size_t len);
__asm__(".global _CODE_MRT_SetCODEThreadName\n\t.set _CODE_MRT_SetCODEThreadName, _MRT_SetCODEThreadName");
MRT_EXPORT int64_t CODE_MRT_GetCODEThreadId(void* handle);
__asm__(".global _CODE_MRT_GetCODEThreadId\n\t.set _CODE_MRT_GetCODEThreadId, _MRT_GetCODEThreadId");
MRT_EXPORT int64_t CODE_MRT_GetCODEThreadState(void* handle);
__asm__(".global _CODE_MRT_GetCODEThreadState\n\t.set _CODE_MRT_GetCODEThreadState, _MRT_GetCODEThreadState");
MRT_EXPORT void* CODE_MRT_GetCurrentCODEThread();
__asm__(".global _CODE_MRT_GetCurrentCODEThread\n\t.set _CODE_MRT_GetCurrentCODEThread, _MRT_GetCurrentCODEThread");

#endif
