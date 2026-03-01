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


#ifndef MRT_THREAD_LOCAL_H
#define MRT_THREAD_LOCAL_H

#include <cstdint>
#include "Base/RwLock.h"

namespace MapleRuntime {
class AllocBuffer;
class Mutator;

enum class ThreadType { CODE_PROCESSOR = 0, GC_THREAD, FP_THREAD, HOT_UPDATE_THREAD };

// Backend and CODEThread will use external tls var through offset calculation, so external tls
// must in the first place, followed by the internal tls.
struct ThreadLocalData {
    // External thread local var.
    AllocBuffer* buffer;
    Mutator* mutator;
    uint8_t* codethread;
    uint8_t* schedule;
    uint8_t* preemptFlag;
    uint8_t* protectAddr;
    uint64_t safepointState;
    uint64_t tid;
    void* foreignCODEThread;
    // Internal thread local var.
    ThreadType threadType;
    bool isCODEProcessor;
    void* threadCache;
};

struct CleanThreadLocalData {
    ~CleanThreadLocalData();
};

class ThreadLocal { // merge this to ThreadLocalData.
public:
    static ThreadLocalData* GetThreadLocalData();

    static void SetMutator(Mutator* newMutator) { GetThreadLocalData()->mutator = newMutator; }

    static Mutator* GetMutator() { return GetThreadLocalData()->mutator; }

    static AllocBuffer* GetAllocBuffer() { return GetThreadLocalData()->buffer; }

    static void SetAllocBuffer(AllocBuffer* buffer) { GetThreadLocalData()->buffer = buffer; }

    static uint8_t* GetPreemptFlag() { return GetThreadLocalData()->preemptFlag; }

    static void SetProtectAddr(uint8_t* addr) { GetThreadLocalData()->protectAddr = addr; }

    static void SetThreadType(ThreadType type) { GetThreadLocalData()->threadType = type; }

    static ThreadType GetThreadType() { return GetThreadLocalData()->threadType; }

    static void SetCODEProcessorFlag(bool flag) { GetThreadLocalData()->isCODEProcessor = flag; }

    static bool IsCODEProcessor() { return GetThreadLocalData()->isCODEProcessor; }

    static void SetForeignCODEThread(void* codethread)
    {
        GetThreadLocalData()->foreignCODEThread = codethread;
    }
    
    static void* GetForeignCODEThread()
    {
        return GetThreadLocalData()->foreignCODEThread;
    }

    static void SetCODEThread(void* codethread)
    {
        GetThreadLocalData()->codethread = reinterpret_cast<uint8_t*>(codethread);
    }

    static void SetSchedule(void* schedule)
    {
        GetThreadLocalData()->schedule = reinterpret_cast<uint8_t*>(schedule);
    }

    static void* GetThreadCache()
    {
        return GetThreadLocalData()->threadCache;
    }

    static void* SetThreadCache(void* threadCache)
    {
        return GetThreadLocalData()->threadCache = threadCache;
    }

    // When runtime is stop, we need to lock any operation which may access runtime.
    static void ThreadLocalFini()
    {
        tlEnableLock.LockWrite();
    }

    static bool TryGetRdLock()
    {
        return tlEnableLock.TryLockRead();
    }

    static void UnlockRdLock()
    {
        tlEnableLock.UnlockRead();
    }
private:
    static RwLock tlEnableLock;
};
} // namespace MapleRuntime
#endif // MRT_THREAD_LOCAL_H
