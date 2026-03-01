/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_RUNTIME_COROUTINES_COROUTINE_CONTEXT_H
#define PANDA_RUNTIME_COROUTINES_COROUTINE_CONTEXT_H

#include "runtime/coroutines/coroutine.h"

namespace ark {

/**
 * @brief Native context of a coroutine.
 *
 * Implementation dependent: may contain register state or OS thread pointer or something else.
 * Does not contain managed state (contains native state only).
 */
class CoroutineContext {
public:
    NO_COPY_SEMANTIC(CoroutineContext);
    NO_MOVE_SEMANTIC(CoroutineContext);

    CoroutineContext() = default;
    virtual ~CoroutineContext() = default;

    virtual void AttachToCoroutine(Coroutine *co)
    {
        co_ = co;
    }
    /// Get linked coroutine instance (contains managed portion of state)
    Coroutine *GetCoroutine() const
    {
        return co_;
    }
    // access to status might require implementation-dependent synchronization, so we have to store it
    // in the native context
    virtual Coroutine::Status GetStatus() const = 0;
    virtual void SetStatus(Coroutine::Status newStatus) = 0;

    virtual void Destroy() = 0;
    virtual void CleanUp() = 0;
    virtual void RequestSuspend(bool getsBlocked) = 0;
    virtual void RequestResume() = 0;
    virtual void RequestUnblock() = 0;

    /**
     * Writes coroutine stack parameters into the provided stack_addr, stack_size, guard_size variables
     * and returns true on successful operation
     */
    virtual bool RetrieveStackInfo(void *&stackAddr, size_t &stackSize, size_t &guardSize) = 0;

protected:
    static void UpdateId(os::thread::ThreadId id, Coroutine *co)
    {
        co->UpdateId(id);
    }

private:
    Coroutine *co_ = nullptr;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_COROUTINES_COROUTINE_CONTEXT_H
