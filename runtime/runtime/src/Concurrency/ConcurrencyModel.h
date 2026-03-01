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


#ifndef MRT_CONCURRENCY_MODEL_H
#define MRT_CONCURRENCY_MODEL_H

#include <cstdint>
#include <list>
#include <mutex>
#include <sys/types.h>
#include <unistd.h>

#include "Mutator/Mutator.h"
#include "RuntimeConfig.h"
#include "CodeScheduler.h"

namespace MapleRuntime {
/**
 * Data structure to save arguments when creating a thread.
 * There are three types of thread to be created.
 * 1. create the main thread;
 * 2. create a thread with a future;
 * 3. create a thread without a future,
 *    where a closure structure `{void *fnGeneric, void *fnInst, captured_var1, ..., captured_varN}` will be saved.
 */
using LWTData = struct {
    void* execute; // Execute function (from Codira) of the thread;
                   // It may be `Future.execute` or `executeClosure`
    void* fn;  // Function pointer of closure when create a thread without a future object
    void* obj; // Pointer of a Codira object;
               // future or env of closure
    void* threadObject;   // Codira class Thread in std/core
};
struct ConcurrencyTask; // Task depends on the implementation of ConcurrencyModel

// ConcurrencyModel is an abstraction for runtime to implement concurrency.
class ConcurrencyModel {
public:
    // Get thread-local mutator according to concurrency model
    static Mutator* GetMutator();
    static void SetMutator(Mutator* mutator);

    ConcurrencyModel() = default;
    virtual ~ConcurrencyModel() = default;

    virtual void Init(const ConcurrencyParam, ScheduleType) {}
    virtual void Fini() {}
    virtual void VisitGCRoots(RootVisitor* visitorHandle) = 0;
    virtual void* GetThreadScheduler() const
    {
        std::abort();
    }
    virtual size_t GetReservedStackSize() const { std::abort(); }
    virtual bool GetStackGuardCheckFlag() const { std::abort(); }
    virtual uint32_t GetProcessorNum() const { return 1; };

protected:
    void* scheduler{ nullptr };
    size_t reservedStackSize{ 0 };
    bool stackGuardCheck{ false };
};
} // namespace MapleRuntime
#endif // MRT_CONCURRENCY_MODEL_H
