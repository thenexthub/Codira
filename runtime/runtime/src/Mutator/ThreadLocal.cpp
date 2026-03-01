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


#include "ThreadLocal.h"

#include "Common/Runtime.h"
#include "schedule.h"
#include "Base/Globals.h"

namespace MapleRuntime {
RwLock ThreadLocal::tlEnableLock;
MRT_EXPORT thread_local uint64_t threadLocalData[sizeof(ThreadLocalData) / sizeof(uint64_t)];
thread_local CleanThreadLocalData cleaner;

ThreadLocalData* ThreadLocal::GetThreadLocalData()
{
    return reinterpret_cast<ThreadLocalData*>(threadLocalData);
}

CleanThreadLocalData::~CleanThreadLocalData()
{
    if (!ThreadLocal::TryGetRdLock()) {
        return;
    }

    ThreadLocalData* local = ThreadLocal::GetThreadLocalData();
    if (Runtime::CurrentRef() == nullptr ||
        local->isCODEProcessor || local->foreignCODEThread == nullptr) {
        ThreadLocal::UnlockRdLock();
        return;
    }

    CODEForeignThreadExit(reinterpret_cast<CODEThreadHandle>(local->foreignCODEThread));
    ThreadLocal::UnlockRdLock();
}

extern "C" void MCC_CheckThreadLocalDataOffset()
{
    static_assert(offsetof(ThreadLocalData, buffer) == 0,
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
    static_assert(offsetof(ThreadLocalData, mutator) == sizeof(void*),
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
    static_assert(offsetof(ThreadLocalData, codethread) == sizeof(void*) * 2,
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
    static_assert(offsetof(ThreadLocalData, schedule) == sizeof(void*) * 3,
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
    static_assert(offsetof(ThreadLocalData, preemptFlag) == sizeof(void*) * 4,
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
    static_assert(offsetof(ThreadLocalData, protectAddr) == sizeof(void*) * 5,
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
    static_assert(offsetof(ThreadLocalData, safepointState) == sizeof(void*) * 6,
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
#if defined(__arm__)
    static_assert(offsetof(ThreadLocalData, tid) == sizeof(void*) * 6 + sizeof(uint64_t),
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
    static_assert(offsetof(ThreadLocalData, foreignCODEThread) == sizeof(void*) * 6 + sizeof(uint64_t) * 2,
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
    static_assert(sizeof(ThreadLocalData) == sizeof(void*) * 10 + sizeof(uint64_t) * 2,
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
#else    
    static_assert(offsetof(ThreadLocalData, tid) == sizeof(void*) * 7,
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
    static_assert(offsetof(ThreadLocalData, foreignCODEThread) == sizeof(void*) * 8,
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
    static_assert(sizeof(ThreadLocalData) == sizeof(void*) * 11,
                  "need to modify the offset of this value in llvm-project and codethread at the same time");
#endif
}

#ifdef __APPLE__
extern "C" MRT_EXPORT void MRT_CheckThreadLocalDataOffset();
__asm__(
    ".global _MRT_CheckThreadLocalDataOffset\n\t.set _MRT_CheckThreadLocalDataOffset, _MCC_CheckThreadLocalDataOffset");
extern "C" MRT_EXPORT void CODE_MCC_CheckThreadLocalDataOffset();
__asm__(
    ".global _CODE_MCC_CheckThreadLocalDataOffset\n\t.set _CODE_MCC_CheckThreadLocalDataOffset, "
    "_MCC_CheckThreadLocalDataOffset");
#else
extern "C" MRT_EXPORT void MRT_CheckThreadLocalDataOffset() __attribute__((alias("MCC_CheckThreadLocalDataOffset")));
extern "C" MRT_EXPORT void CODE_MCC_CheckThreadLocalDataOffset() __attribute__((alias("MCC_CheckThreadLocalDataOffset")));
#endif
} // namespace MapleRuntime
