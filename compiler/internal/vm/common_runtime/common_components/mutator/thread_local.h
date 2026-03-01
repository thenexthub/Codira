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

#ifndef COMMON_COMPONENTS_MUTATOR_THREAD_LOCAL_H
#define COMMON_COMPONENTS_MUTATOR_THREAD_LOCAL_H

#include <cstdint>

namespace common {
class AllocationBuffer;
class Mutator;

enum class ThreadType { ARK_PROCESSOR = 0, GC_THREAD, FP_THREAD, HOT_UPDATE_THREAD };

// Backend and ArkThread will use external tls var through offset calculation, so external tls
// must in the first place, followed by the internal tls.
struct ThreadLocalData {
    // External thread local var.
    AllocationBuffer* buffer;
    Mutator* mutator {nullptr};
    uint8_t* thread;
    uint8_t* schedule;
    uint8_t* preemptFlag;
    uint8_t* protectAddr;
    uint64_t tid;
    void* foreignThread;
    // Internal thread local var.
    ThreadType threadType;
    bool isArkProcessor;
};

class ThreadLocal { // merge this to ThreadLocalData.
public:
    static ThreadLocalData* GetThreadLocalData();

    static void SetMutator(Mutator* newMutator) { GetThreadLocalData()->mutator = newMutator; }

    static Mutator* GetMutator() { return GetThreadLocalData()->mutator; }

    static AllocationBuffer* GetAllocBuffer() { return GetThreadLocalData()->buffer; }
    static void ClearAllocBufferRegion();

    static void SetAllocBuffer(AllocationBuffer* buffer) { GetThreadLocalData()->buffer = buffer; }

    static uint8_t* GetPreemptFlag() { return GetThreadLocalData()->preemptFlag; }

    static void SetProtectAddr(uint8_t* addr) { GetThreadLocalData()->protectAddr = addr; }

    static void SetThreadType(ThreadType type) { GetThreadLocalData()->threadType = type; }

    static ThreadType GetThreadType() { return GetThreadLocalData()->threadType; }

    static void SetProcessorFlag(bool flag) { GetThreadLocalData()->isArkProcessor = flag; }

    static bool IsArkProcessor() { return GetThreadLocalData()->isArkProcessor; }

    static void SetForeignThread(void* thread)
    {
        GetThreadLocalData()->foreignThread = thread;
    }
    
    static void* GetForeignThread()
    {
        return GetThreadLocalData()->foreignThread;
    }

    static void SetArkThread(void* thread)
    {
        GetThreadLocalData()->thread = reinterpret_cast<uint8_t*>(thread);
    }

    static void SetSchedule(void* schedule)
    {
        GetThreadLocalData()->schedule = reinterpret_cast<uint8_t*>(schedule);
    }
};
} // namespace common

#endif // COMMON_COMPONENTS_MUTATOR_THREAD_LOCAL_H
