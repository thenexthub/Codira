/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_PROMISE_H_
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_PROMISE_H_

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_sync_primitives.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "runtime/coroutines/coroutine_events.h"

namespace ark::ets {

class EtsCoroutine;

namespace test {
class EtsPromiseTest;
}  // namespace test

class EtsPromise : public ObjectHeader {
public:
    // temp
    static constexpr EtsInt STATE_PENDING = 0;
    static constexpr EtsInt STATE_LINKED = 1;
    static constexpr EtsInt STATE_RESOLVED = 2;
    static constexpr EtsInt STATE_REJECTED = 3;

    EtsPromise() = delete;
    ~EtsPromise() = delete;

    NO_COPY_SEMANTIC(EtsPromise);
    NO_MOVE_SEMANTIC(EtsPromise);

    PANDA_PUBLIC_API static EtsPromise *Create(EtsCoroutine *etsCoroutine = EtsCoroutine::GetCurrent());

    static EtsPromise *FromCoreType(ObjectHeader *promise)
    {
        return reinterpret_cast<EtsPromise *>(promise);
    }

    ObjectHeader *GetCoreType() const
    {
        return static_cast<ObjectHeader *>(const_cast<EtsPromise *>(this));
    }

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    static EtsPromise *FromEtsObject(EtsObject *promise)
    {
        return reinterpret_cast<EtsPromise *>(promise);
    }

    EtsObjectArray *GetCallbackQueue(EtsCoroutine *coro) const
    {
        return EtsObjectArray::FromCoreType(
            ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsPromise, callbackQueue_)));
    }

    void SetCallbackQueue(EtsCoroutine *coro, EtsObjectArray *callbackQueue)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, callbackQueue_), callbackQueue->GetCoreType());
    }

    EtsIntArray *GetWorkerDomainQueue(EtsCoroutine *coro) const
    {
        return reinterpret_cast<EtsIntArray *>(
            ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsPromise, workerDomainQueue_)));
    }

    void SetWorkerDomainQueue(EtsCoroutine *coro, EtsIntArray *workerDomainQueue)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, workerDomainQueue_),
                                  workerDomainQueue->GetCoreType());
    }

    EtsInt GetQueueSize() const
    {
        return queueSize_;
    }

    bool IsResolved() const
    {
        return (state_ == STATE_RESOLVED);
    }

    bool IsRejected() const
    {
        return (state_ == STATE_REJECTED);
    }

    bool IsPending() const
    {
        return (state_ == STATE_PENDING);
    }

    bool IsLinked() const
    {
        return (state_ == STATE_LINKED);
    }

    bool IsProxy() const
    {
        return GetLinkedPromise(EtsCoroutine::GetCurrent()) != nullptr;
    }

    EtsObject *GetInteropObject(EtsCoroutine *coro) const
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsPromise, interopObject_));
        return EtsObject::FromCoreType(obj);
    }

    void SetInteropObject(EtsCoroutine *coro, EtsObject *o)
    {
        ASSERT(o != nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, interopObject_), o->GetCoreType());
    }

    EtsObject *GetLinkedPromise(EtsCoroutine *coro) const
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsPromise, linkedPromise_));
        return EtsObject::FromCoreType(obj);
    }

    void SetLinkedPromise(EtsCoroutine *coro, EtsObject *p)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, linkedPromise_), p->GetCoreType());
    }

    static void CreateLink(EtsObject *source, EtsPromise *target)
    {
        ASSERT(target != nullptr);
        EtsCoroutine *currentCoro = EtsCoroutine::GetCurrent();
        auto *jobQueue = currentCoro->GetExternalIfaceTable()->GetJobQueue();
        if (jobQueue != nullptr) {
            ASSERT(target != nullptr);
            jobQueue->CreateLink(source, target->AsObject());
        }
    }

    EtsMutex *GetMutex(EtsCoroutine *coro) const
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsPromise, mutex_));
        return EtsMutex::FromCoreType(obj);
    }

    void SetMutex(EtsCoroutine *coro, EtsMutex *mutex)
    {
        ASSERT(mutex != nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, mutex_), mutex->GetCoreType());
    }

    EtsEvent *GetEvent(EtsCoroutine *coro) const
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsPromise, event_));
        return EtsEvent::FromCoreType(obj);
    }

    void SetEvent(EtsCoroutine *coro, EtsEvent *event)
    {
        ASSERT(event != nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, event_), event->GetCoreType());
    }

    void Resolve(EtsCoroutine *coro, EtsObject *value)
    {
        ASSERT(IsPending() || IsLinked());
        auto coreValue = (value == nullptr) ? nullptr : value->GetCoreType();
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, value_), coreValue);
        state_ = STATE_RESOLVED;
        OnPromiseCompletion(coro);
    }

    void Reject(EtsCoroutine *coro, EtsObject *error)
    {
        ASSERT(IsPending() || IsLinked());
        ASSERT(error != nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, value_), error->GetCoreType());
        state_ = STATE_REJECTED;
        OnPromiseCompletion(coro);
    }

    void SubmitCallback(EtsCoroutine *coro, EtsObject *callback, CoroutineWorkerDomain workerDomain)
    {
        ASSERT(IsLocked());
        ASSERT(queueSize_ < static_cast<int>(GetCallbackQueue(coro)->GetLength()));
        auto *cbQueue = GetCallbackQueue(coro);
        auto *workerDomainQueue = GetWorkerDomainQueue(coro);
        workerDomainQueue->Set(queueSize_, static_cast<int>(workerDomain));
        cbQueue->Set(queueSize_, callback);
        queueSize_++;
    }

    EtsObject *GetValue(EtsCoroutine *coro) const
    {
        return EtsObject::FromCoreType(ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsPromise, value_)));
    }

    uint32_t GetState() const
    {
        return state_;
    }

    static size_t ValueOffset()
    {
        return MEMBER_OFFSET(EtsPromise, value_);
    }

    /// Allows to get exclusive access to the promise state
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

    /// Blocks current coroutine until promise is resolved/rejected
    void Wait()
    {
        auto *event = GetEvent(EtsCoroutine::GetCurrent());
        ASSERT(event != nullptr);
        event->Wait();
    }

    void ChangeStateToPendingFromLinked()
    {
        ASSERT(IsLinked());
        state_ = STATE_PENDING;
    }

    // launch promise then/catch callback: void()
    static void LaunchCallback(EtsCoroutine *coro, EtsObject *callback, const CoroutineWorkerGroup::Id &groupId);

private:
    void OnPromiseCompletion(EtsCoroutine *coro);

    void ClearQueues(EtsCoroutine *coro)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, callbackQueue_), nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, workerDomainQueue_), nullptr);
        queueSize_ = 0;
    }

    ObjectPointer<EtsObject> value_;  // the completion value of the Promise
    ObjectPointer<EtsMutex> mutex_;
    ObjectPointer<EtsEvent> event_;
    ObjectPointer<EtsObjectArray>
        callbackQueue_;  // the queue of 'then and catch' calbacks which will be called when the Promise gets fulfilled
    ObjectPointer<EtsIntArray> workerDomainQueue_;  // the queue of callbacks' launch mode
    ObjectPointer<EtsObject> interopObject_;        // internal object used in js interop
    ObjectPointer<EtsObject> linkedPromise_;        // linked JS promise as JSValue (if exists)
    EtsInt queueSize_;
    uint32_t state_;  // the Promise's state

    friend class test::EtsPromiseTest;
};

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_PROMISE_H_
