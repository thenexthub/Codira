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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_JOB_H_
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_JOB_H_

#include "runtime/include/object_header.h"
#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_sync_primitives.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets {

class EtsCoroutine;

namespace test {
class EtsJobTest;
}  // namespace test

class EtsJob : public ObjectHeader {
public:
    static constexpr EtsInt STATE_RUNNING = 0;
    static constexpr EtsInt STATE_FINISHED = 1;
    static constexpr EtsInt STATE_FAILED = 2;

    EtsJob() = delete;
    EtsJob(EtsJob &) = delete;
    ~EtsJob() = default;

    NO_COPY_SEMANTIC(EtsJob);
    NO_MOVE_SEMANTIC(EtsJob);

    PANDA_PUBLIC_API static EtsJob *Create(EtsCoroutine *etsCoroutine = EtsCoroutine::GetCurrent());

    static void EtsJobFinish(EtsJob *job, EtsObject *value);

    static void EtsJobFail(EtsJob *job, EtsObject *error);

    static EtsJob *FromCoreType(ObjectHeader *job)
    {
        return reinterpret_cast<EtsJob *>(job);
    }

    ObjectHeader *GetCoreType() const
    {
        return static_cast<ObjectHeader *>(const_cast<EtsJob *>(this));
    }

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    static EtsJob *FromEtsObject(EtsObject *job)
    {
        return reinterpret_cast<EtsJob *>(job);
    }

    EtsMutex *GetMutex(EtsCoroutine *coro)
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsJob, mutex_));
        return EtsMutex::FromCoreType(obj);
    }

    void SetMutex(EtsCoroutine *coro, EtsMutex *mutex)
    {
        ASSERT(mutex != nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsJob, mutex_), mutex->GetCoreType());
    }

    EtsEvent *GetEvent(EtsCoroutine *coro)
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsJob, event_));
        return EtsEvent::FromCoreType(obj);
    }

    void SetEvent(EtsCoroutine *coro, EtsEvent *event)
    {
        ASSERT(event != nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsJob, event_), event->GetCoreType());
    }

    EtsObject *GetValue(EtsCoroutine *coro)
    {
        return EtsObject::FromCoreType(ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsJob, value_)));
    }

    /// Allows to get exclusive access to the job state
    void Lock()
    {
        auto *mutex = GetMutex(EtsCoroutine::GetCurrent());
        ASSERT(mutex != nullptr);
        mutex->Lock();
    }

    void Unlock()
    {
        auto *mutex = GetMutex(EtsCoroutine::GetCurrent());
        ASSERT(mutex != nullptr);
        ASSERT(mutex->IsHeld());
        mutex->Unlock();
    }

    bool IsLocked()
    {
        auto *mutex = GetMutex(EtsCoroutine::GetCurrent());
        ASSERT(mutex != nullptr);
        return mutex->IsHeld();
    }

    /// Blocks current coroutine until job is resolved/rejected
    void Wait()
    {
        auto *event = GetEvent(EtsCoroutine::GetCurrent());
        ASSERT(event != nullptr);
        event->Wait();
    }

    bool IsRunning() const
    {
        return state_ == STATE_RUNNING;
    }

    bool IsFinished() const
    {
        return state_ == STATE_FINISHED;
    }

    bool IsFailed() const
    {
        return state_ == STATE_FAILED;
    }

    void Finish(EtsCoroutine *coro, EtsObject *value)
    {
        ASSERT(state_ == STATE_RUNNING);
        auto coreValue = (value == nullptr) ? nullptr : value->GetCoreType();
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsJob, value_), coreValue);
        state_ = STATE_FINISHED;
        // Unblock awaitee coros
        GetEvent(coro)->Fire();
    }

    void Fail(EtsCoroutine *coro, EtsObject *error)
    {
        ASSERT(error != nullptr);
        ASSERT(state_ == STATE_RUNNING);
        ASSERT(error != nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsJob, value_), error->GetCoreType());
        state_ = STATE_FAILED;

        if (Runtime::GetOptions().IsListUnhandledOnExitJobs(plugins::LangToRuntimeType(panda_file::SourceLang::ETS))) {
            coro->GetPandaVM()->GetUnhandledObjectManager()->AddFailedJob(this);
        }

        // Unblock awaitee coros
        GetEvent(coro)->Fire();
    }

private:
    ObjectPointer<EtsObject> value_;  // the completion value of the Job
    ObjectPointer<EtsMutex> mutex_;
    ObjectPointer<EtsEvent> event_;
    uint32_t state_;  // the Job state

    friend class test::EtsJobTest;
};

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_JOB_H_
