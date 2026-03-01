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
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_sync_primitives.h"
#include "runtime/include/thread_scopes.h"

#include <atomic>

namespace ark::ets {

/*static*/
EtsMutex *EtsMutex::Create(EtsCoroutine *coro)
{
    EtsHandleScope scope(coro);
    auto *klass = PlatformTypes(coro)->coreMutex;
    auto hMutex = EtsHandle<EtsMutex>(coro, EtsMutex::FromEtsObject(EtsObject::Create(coro, klass)));
    auto *waitersList = EtsWaitersList::Create(coro);
    ASSERT(hMutex.GetPtr() != nullptr);
    hMutex->SetWaitersList(coro, waitersList);
    return hMutex.GetPtr();
}

ALWAYS_INLINE inline static void BackOff(uint32_t i)
{
    volatile uint32_t x;  // Volatile to make sure loop is not optimized out.
    const uint32_t spinCount = 10 * i;
    for (uint32_t spin = 0; spin < spinCount; spin++) {
        x = x + 1;
    }
}

static bool TrySpinLockFor(std::atomic<uint32_t> &waiters, uint32_t expected, uint32_t desired)
{
    static constexpr uint32_t maxBackOff = 3;  // NOLINT(readability-identifier-naming)
    static constexpr uint32_t maxIter = 3;     // NOLINT(readability-identifier-naming)
    uint32_t exp;
    for (uint32_t i = 1; i <= maxIter; ++i) {
        exp = expected;
        if (waiters.compare_exchange_weak(exp, desired, std::memory_order_acq_rel, std::memory_order_relaxed)) {
            return true;
        }
        BackOff(std::min<uint32_t>(i, maxBackOff));
    }
    return false;
}

void EtsMutex::Lock()
{
    if (TrySpinLockFor(waiters_, 0, 1)) {
        return;
    }
    // Atomic with acq_rel order reason: sync Lock/Unlock in other threads
    if (waiters_.fetch_add(1, std::memory_order_acq_rel) == 0) {
        return;
    }
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *coroManager = coro->GetCoroutineManager();
    auto awaitee = EtsWaitersList::Node(coroManager);
    SuspendCoroutine(&awaitee);
}

void EtsMutex::Unlock()
{
    // Atomic with acq_rel order reason: sync Lock/Unlock in other threads
    if (waiters_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        return;
    }
    ResumeCoroutine();
}

bool EtsMutex::IsHeld()
{
    // Atomic with relaxed order reason: sync is not needed here
    // because it is expected that method is not called concurrently with Lock/Unlock
    return waiters_.load(std::memory_order_relaxed) != 0;
}

/*static*/
EtsEvent *EtsEvent::Create(EtsCoroutine *coro)
{
    EtsHandleScope scope(coro);
    auto *klass = PlatformTypes(coro)->coreEvent;
    auto hEvent = EtsHandle<EtsEvent>(coro, EtsEvent::FromEtsObject(EtsObject::Create(coro, klass)));
    auto *waitersList = EtsWaitersList::Create(coro);
    ASSERT(hEvent.GetPtr() != nullptr);
    hEvent->SetWaitersList(coro, waitersList);
    return hEvent.GetPtr();
}

void EtsEvent::Wait()
{
    // Atomic with acq_rel order reason: sync Wait/Fire in other threads
    auto state = state_.fetch_add(ONE_WAITER, std::memory_order_acq_rel);
    if (IsFireState(state)) {
        return;
    }
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *coroManager = coro->GetCoroutineManager();
    auto awaitee = EtsWaitersList::Node(coroManager);
    SuspendCoroutine(&awaitee);
}

void EtsEvent::Fire()
{
    // Atomic with acq_rel order reason: sync Wait/Fire in other threads
    auto state = state_.exchange(FIRE_STATE, std::memory_order_acq_rel);
    if (IsFireState(state)) {
        return;
    }
    for (auto waiters = GetNumberOfWaiters(state); waiters > 0; --waiters) {
        ResumeCoroutine();
    }
}

/* static */
EtsCondVar *EtsCondVar::Create(EtsCoroutine *coro)
{
    EtsHandleScope scope(coro);
    auto *klass = PlatformTypes(coro)->coreCondVar;
    auto hCondVar = EtsHandle<EtsCondVar>(coro, EtsCondVar::FromEtsObject(EtsObject::Create(klass)));
    auto *waitersList = EtsWaitersList::Create(coro);
    ASSERT(hCondVar.GetPtr() != nullptr);
    hCondVar->SetWaitersList(coro, waitersList);
    return hCondVar.GetPtr();
}

void EtsCondVar::Wait(EtsHandle<EtsMutex> &mutex)
{
    ASSERT(mutex->IsHeld());
    waiters_++;
    mutex->Unlock();
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *coroManager = coro->GetCoroutineManager();
    auto awaitee = EtsWaitersList::Node(coroManager);
    SuspendCoroutine(&awaitee);
    mutex->Lock();
}

void EtsCondVar::NotifyOne([[maybe_unused]] EtsMutex *mutex)
{
    ASSERT(mutex->IsHeld());
    if (waiters_ != 0) {
        ResumeCoroutine();
        waiters_--;
    }
}

void EtsCondVar::NotifyAll([[maybe_unused]] EtsMutex *mutex)
{
    ASSERT(mutex->IsHeld());
    while (waiters_ != 0) {
        ResumeCoroutine();
        waiters_--;
    }
}

/* static */
EtsQueueSpinlock *EtsQueueSpinlock::Create(EtsCoroutine *coro)
{
    auto *klass = PlatformTypes(coro)->coreQueueSpinlock;
    return EtsQueueSpinlock::FromEtsObject(EtsObject::Create(klass));
}

void EtsQueueSpinlock::Acquire(Guard *waiter)
{
    // Atomic with acq_rel order reason: to guarantee happens-before for critical sections
    auto *oldTail = tail_.exchange(waiter, std::memory_order_acq_rel);
    if (oldTail == nullptr) {
        return;
    }
    // Atomic with release order reason: to guarantee happens-before with waiter constructor
    oldTail->next_.store(waiter, std::memory_order_release);
    auto spinWait = SpinWait();
    ScopedNativeCodeThread nativeCode(EtsCoroutine::GetCurrent());
    // Atomic with acquire order reason: to guarantee happens-before for critical sections
    while (!waiter->isOwner_.load(std::memory_order_acquire)) {
        spinWait();
    }
}

void EtsQueueSpinlock::Release(Guard *owner)
{
    auto *head = owner;
    // Atomic with release order reason: to guarantee happens-before for critical sections
    if (tail_.compare_exchange_strong(head, nullptr, std::memory_order_release, std::memory_order_relaxed)) {
        return;
    }
    // Atomic with acquire order reason: to guarantee happens-before with next constructor
    Guard *next = owner->next_.load(std::memory_order_acquire);
    auto spinWait = SpinWait();
    while (next == nullptr) {
        spinWait();
        // Atomic with acquire order reason: to guarantee happens-before with next constructor
        next = owner->next_.load(std::memory_order_acquire);
    }
    // Atomic with release order reason: to guarantee happens-before for critical sections
    next->isOwner_.store(true, std::memory_order_release);
}

bool EtsQueueSpinlock::IsHeld() const
{
    // Atomic with relaxed order reason: sync is not needed here
    // because it is expected that method is not called concurrently with Acquire/Release
    return tail_.load(std::memory_order_relaxed) != nullptr;
}

EtsQueueSpinlock::Guard::Guard(EtsHandle<EtsQueueSpinlock> &spinlock) : spinlock_(spinlock)
{
    ASSERT(spinlock_.GetPtr() != nullptr);
    spinlock_->Acquire(this);
}

EtsQueueSpinlock::Guard::~Guard()
{
    ASSERT(spinlock_.GetPtr() != nullptr);
    spinlock_->Release(this);
}

void EtsRWLock::ReadLock()
{
    // Atomic with relaxed order reason: sync is not needed here because CAS has acq_rel order
    uint64_t oldState = state_.load(std::memory_order_relaxed);
    uint64_t newState = 0;
    do {
        newState = State::IncReaders(oldState) | (State::HasWriteLock(oldState) ? 0 : State::READ_LOCK_STATE);
        // Atomic with acq_rel order reason: sync Lock/Unlock in other threads
    } while (!state_.compare_exchange_weak(oldState, newState, std::memory_order_acq_rel, std::memory_order_relaxed));

    if (!State::HasReadLock(newState)) {
        auto *coro = EtsCoroutine::GetCurrent();
        auto *coroManager = coro->GetCoroutineManager();
        auto readAwaitee = EtsWaitersList::Node(coroManager);
        EtsSyncPrimitive<EtsRWLock>::SuspendCoroutine(GetReaders(coro), &readAwaitee);
    }
}

void EtsRWLock::WriteLock()
{
    // Atomic with relaxed order reason: sync is not needed here because CAS has acq_rel order
    uint64_t oldState = state_.load(std::memory_order_relaxed);
    uint64_t newState = 0;
    do {
        newState = State::IncWriters(oldState) | (State::HasReadLock(oldState) ? 0 : State::WRITE_LOCK_STATE);
        // Atomic with acq_rel order reason: sync Lock/Unlock in other threads
    } while (!state_.compare_exchange_weak(oldState, newState, std::memory_order_acq_rel, std::memory_order_relaxed));

    if (!State::HasWriteLock(newState) || State::HasWriteLock(oldState)) {
        auto *coro = EtsCoroutine::GetCurrent();
        auto *coroManager = coro->GetCoroutineManager();
        auto writeAwaitee = EtsWaitersList::Node(coroManager);
        EtsSyncPrimitive<EtsRWLock>::SuspendCoroutine(GetWriters(coro), &writeAwaitee);
    }
}

void EtsRWLock::Unlock()
{
    ASSERT(!State::IsUnlocked(GetState()));
    // Atomic with relaxed order reason: sync is not needed here because CAS has acq_rel order
    uint64_t oldState = state_.load(std::memory_order_relaxed);
    uint64_t newState = 0;
    do {
        newState = State::HasReadLock(oldState) ? State::DecReadersClearState(oldState)
                                                : State::DecWritersClearState(oldState);
        newState |= State::HasReaders(newState)   ? State::READ_LOCK_STATE
                    : State::HasWriters(newState) ? State::WRITE_LOCK_STATE
                                                  : State::UNLOCKED_STATE;
        // Atomic with acq_rel order reason: sync Lock/Unlock in other threads
    } while (!state_.compare_exchange_weak(oldState, newState, std::memory_order_acq_rel, std::memory_order_relaxed));

    if (newState == State::UNLOCKED_STATE || (State::HasReadLock(oldState) && State::HasReadLock(newState))) {
        return;
    }
    if (State::HasWriteLock(newState)) {
        EtsSyncPrimitive<EtsRWLock>::ResumeCoroutine(GetWriters(EtsCoroutine::GetCurrent()));
        return;
    }
    for (auto readers = State::GetReaders(newState); readers != 0; --readers) {
        EtsSyncPrimitive<EtsRWLock>::ResumeCoroutine(GetReaders(EtsCoroutine::GetCurrent()));
    }
}

uint64_t EtsRWLock::GetState() const
{
    // Atomic with relaxed order reason: sync is not needed here
    // because it is expected that method is not called concurrently with Lock/Unlock
    return state_.load(std::memory_order_relaxed);
}

bool EtsRWLock::IsHeld() const
{
    return !State::IsUnlocked(GetState());
}

}  // namespace ark::ets
