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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MUTEX_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MUTEX_H

#include "libarkbase/mem/object_pointer.h"
#include "libarkbase/macros.h"
#include "runtime/include/thread_scopes.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_waiters_list.h"

#include <cstdint>

namespace ark::ets {

template <typename T>
class EtsHandle;

namespace test {
class EtsSyncPrimitivesTest;
}  // namespace test

/// @brief The base class for sync primitives. Should not be used directly.
template <typename T>
class EtsSyncPrimitive : public EtsObject {
public:
    static T *FromCoreType(ObjectHeader *syncPrimitive)
    {
        return reinterpret_cast<T *>(syncPrimitive);
    }

    static T *FromEtsObject(EtsObject *syncPrimitive)
    {
        return reinterpret_cast<T *>(syncPrimitive);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    EtsWaitersList *GetWaitersList(EtsCoroutine *coro)
    {
        return EtsWaitersList::FromCoreType(
            ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsSyncPrimitive, waitersList_)));
    }

    void SetWaitersList(EtsCoroutine *coro, EtsWaitersList *waitersList)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsSyncPrimitive, waitersList_),
                                  waitersList->GetCoreType());
    }

    /**
     * Blocks current coroutine. It can be used concurrently with other Suspend and Resume
     * @param awaitee - EtsWaitersList node that contains GenericEvent and probably other useful data
     * NOTE: `this` and all other raw ObjectHeaders may become invalid after this call due to GC
     */
    static ALWAYS_INLINE void SuspendCoroutine(EtsWaitersList *waiters, EtsWaitersList::Node *awaitee)
    {
        auto *coro = EtsCoroutine::GetCurrent();
        ASSERT(coro != nullptr);
        auto *coroManager = coro->GetCoroutineManager();
        auto &event = awaitee->GetEvent();
        // Need to lock event before PushBack
        // to avoid use-after-free in CoroutineEvent::Happen method
        event.Lock();
        waiters->PushBack(awaitee);
        ScopedNativeCodeThread nativeCode(coro);
        coroManager->Await(&event);
    }

    /**
     * Unblocks suspended coroutine. It can be used concurrently with Suspend,
     * BUT not with other Resume (PopFront is not thread-safety)
     */
    static ALWAYS_INLINE void ResumeCoroutine(EtsWaitersList *waiters)
    {
        auto *awaitee = waiters->PopFront();
        awaitee->GetEvent().Happen();
    }

    void SuspendCoroutine(EtsWaitersList::Node *awaitee)
    {
        SuspendCoroutine(GetWaitersList(EtsCoroutine::GetCurrent()), awaitee);
    }

    void ResumeCoroutine()
    {
        ResumeCoroutine(GetWaitersList(EtsCoroutine::GetCurrent()));
    }

private:
    ObjectPointer<EtsWaitersList> waitersList_;

    friend class test::EtsSyncPrimitivesTest;
};

/// @brief Coroutine mutex. This allows to get exclusive access to the critical section.
class EtsMutex : public EtsSyncPrimitive<EtsMutex> {
public:
    template <typename T>
    class LockHolder {
    public:
        explicit LockHolder(EtsHandle<T> &hLock) : hLock_(hLock)
        {
            ASSERT(hLock_.GetPtr() != nullptr);
            hLock_->Lock();
        }

        NO_COPY_SEMANTIC(LockHolder);
        NO_MOVE_SEMANTIC(LockHolder);

        ~LockHolder()
        {
            ASSERT(hLock_.GetPtr() != nullptr);
            hLock_->Unlock();
        }

    private:
        EtsHandle<T> &hLock_;
    };

    /**
     * Method gives exclusive access to critical section is case of successful mutex lock.
     * Otherwise, blocks current coroutine until other coroutine will unlock the mutex.
     */
    void Lock();

    /**
     * Calling the method means exiting the critical section.
     * If there are blocked coroutines, unblocks one of them.
     */
    void Unlock();

    /// This method should be used to make sure that mutex is locked by current coroutine.
    bool IsHeld();

    static EtsMutex *Create(EtsCoroutine *coro);

private:
    std::atomic<uint32_t> waiters_;

    friend class test::EtsSyncPrimitivesTest;
};

/// @brief Coroutine one-shot event. This allows to block current coroutine until event is fired
class EtsEvent : public EtsSyncPrimitive<EtsEvent> {
public:
    /**
     * Blocks current coroutine until another coroutine will fire the same event.
     * It can be used concurrently with other wait/fire.
     * It has no effect in case fire has already been caused, but guarantees happens-before.
     */
    void Wait();

    /**
     * Unblocks all coroutines that are waiting the same event.
     * Can be used concurrently with other wait/fire but multiply calls have no effect.
     */
    void Fire();

    static EtsEvent *Create(EtsCoroutine *coro);

private:
    static constexpr uint32_t STATE_BIT = 1U;
    static constexpr uint32_t FIRE_STATE = 1U;
    static constexpr uint32_t ONE_WAITER = 2U;

    static bool IsFireState(uint32_t state)
    {
        return (state & STATE_BIT) == FIRE_STATE;
    }

    static uint32_t GetNumberOfWaiters(uint32_t state)
    {
        return state >> STATE_BIT;
    }

    std::atomic<uint32_t> state_;

    friend class test::EtsSyncPrimitivesTest;
};

/// Coroutine conditional variable
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class EtsCondVar : public EtsSyncPrimitive<EtsCondVar> {
public:
    /**
     * precondition: mutex is locked
     * 1. Unlocks mutex
     * 2. Blocks current coroutine
     * 3. Locks mutex
     * This method is thread-safe in relation to other methods of the CondVar
     */
    void Wait(EtsHandle<EtsMutex> &mutex);

    /**
     * precondition: mutex is locked
     * Unblocks ONE suspended coroutine associated with this CondVar, if it exists.
     * This method is thread-safe in relation to other methods of the CondVar
     */
    void NotifyOne([[maybe_unused]] EtsMutex *mutex);

    /**
     * precondition: mutex is locked
     * Unblocks ALL suspended coroutine associated with this CondVar.
     * This method is thread-safe in relation to other methods of the CondVar
     */
    void NotifyAll([[maybe_unused]] EtsMutex *mutex);

    static EtsCondVar *Create(EtsCoroutine *coro);

private:
    EtsInt waiters_;

    friend class test::EtsSyncPrimitivesTest;
};

class EtsQueueSpinlock : public EtsObject {
public:
    class Guard {
    public:
        explicit Guard(EtsHandle<EtsQueueSpinlock> &spinlock);
        NO_COPY_SEMANTIC(Guard);
        NO_MOVE_SEMANTIC(Guard);
        ~Guard();

    private:
        friend class EtsQueueSpinlock;

        EtsHandle<EtsQueueSpinlock> &spinlock_;
        std::atomic<bool> isOwner_ {false};
        std::atomic<Guard *> next_ {nullptr};
    };

    static EtsQueueSpinlock *FromCoreType(ObjectHeader *syncPrimitive)
    {
        return reinterpret_cast<EtsQueueSpinlock *>(syncPrimitive);
    }

    static EtsQueueSpinlock *FromEtsObject(EtsObject *syncPrimitive)
    {
        return reinterpret_cast<EtsQueueSpinlock *>(syncPrimitive);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    static EtsQueueSpinlock *Create(EtsCoroutine *coro);

    /// This method should be used to make sure that spinlock is acquired by current coroutine.
    bool IsHeld() const;

private:
    class SpinWait {
    public:
        void operator()() {}
    };

    /// Acquires lock and allows to get exclusive access to the critical seciton
    void Acquire(Guard *waiter);

    /// Releases lock and unblocks waiter
    void Release(Guard *owner);

    alignas(alignof(EtsLong)) std::atomic<Guard *> tail_;

    friend class test::EtsSyncPrimitivesTest;
};

class EtsRWLock : public EtsObject {
public:
    /// Acquires lock and allows to get shared with other readers access to the critical seciton
    void ReadLock();

    /// Acquires lock and allows to get exclusive access to the critical seciton
    void WriteLock();

    /**
     * Releases lock and unblocks waiters
     * NOTE: readers have higher priority for unblocking than writers
     */
    void Unlock();

    static EtsRWLock *FromCoreType(ObjectHeader *rwLock)
    {
        return reinterpret_cast<EtsRWLock *>(rwLock);
    }

    static EtsRWLock *FromEtsObject(EtsObject *rwLock)
    {
        return reinterpret_cast<EtsRWLock *>(rwLock);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    /// This method should not be used concurrently with Lock/Unlock
    uint64_t GetState() const;

    /// This method should be used to make sure that the lock is acquired by current coroutine
    bool IsHeld() const;

    class State;

private:
    EtsWaitersList *GetReaders(EtsCoroutine *coro)
    {
        return EtsWaitersList::FromCoreType(ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsRWLock, readers_)));
    }

    EtsWaitersList *GetWriters(EtsCoroutine *coro)
    {
        return EtsWaitersList::FromCoreType(ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsRWLock, writers_)));
    }

    ObjectPointer<EtsObject> rLock_;
    ObjectPointer<EtsObject> wLock_;
    ObjectPointer<EtsWaitersList> readers_;
    ObjectPointer<EtsWaitersList> writers_;

    /*
     * 63          32          1       0
     * ---------------------------------
     * |  writers  |  readers  | state |
     * ---------------------------------
     *
     * state: 00 - unlocked
     * state: 01 - read locked
     * state: 10 - write locked
     */
    alignas(alignof(EtsLong)) std::atomic<uint64_t> state_;

    friend class test::EtsSyncPrimitivesTest;
};

class EtsRWLock::State {
public:
    static bool IsUnlocked(uint64_t state)
    {
        return state == UNLOCKED_STATE;
    }

    static bool HasReadLock(uint64_t state)
    {
        return (state & STATE_MASK) == READ_LOCK_STATE;
    }

    static bool HasWriteLock(uint64_t state)
    {
        return (state & STATE_MASK) == WRITE_LOCK_STATE;
    }

    static bool HasReaders(uint64_t state)
    {
        return (state & READERS_MASK) != 0;
    }

    static bool HasWriters(uint64_t state)
    {
        return (state & WRITERS_MASK) != 0;
    }

    static uint64_t GetReaders(uint64_t state)
    {
        return (state & READERS_MASK) >> STATE_BITS;
    }

    static uint64_t IncReaders(uint64_t state)
    {
        return state + ONE_READER;
    }

    static uint64_t IncWriters(uint64_t state)
    {
        return state + ONE_WRITER;
    }

    static uint64_t DecReadersClearState(uint64_t state)
    {
        return (state - ONE_READER) & ~STATE_MASK;
    }

    static uint64_t DecWritersClearState(uint64_t state)
    {
        return (state - ONE_WRITER) & ~STATE_MASK;
    }

    static constexpr uint64_t UNLOCKED_STATE = 0;
    static constexpr uint64_t READ_LOCK_STATE = 1;
    static constexpr uint64_t WRITE_LOCK_STATE = 1U << 1U;

private:
    static constexpr uint64_t STATE_MASK = 0x3;
    static constexpr uint64_t READERS_MASK = 0xFFFFFFFEULL << 1U;
    static constexpr uint64_t WRITERS_MASK = 0xFFFFFFFEULL << 32U;

    static constexpr uint64_t STATE_BITS = 2;
    static constexpr uint64_t READER_BITS = 31;

    static constexpr uint64_t ONE_READER = 1ULL << STATE_BITS;
    static constexpr uint64_t ONE_WRITER = 1ULL << (STATE_BITS + READER_BITS);
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MUTEX_H
