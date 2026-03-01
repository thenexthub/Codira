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
#include "common_components/mutator/mutator_manager.h"

#include <thread>

#include "common_components/base/time_utils.h"
#include "common_components/heap/collector/finalizer_processor.h"
#include "common_components/heap/collector/marking_collector.h"
#include "common_components/heap/heap.h"
#include "common_components/mutator/mutator.inline.h"

namespace common {
bool g_enableGCTimeoutCheck = true;

bool IsRuntimeThread()
{
    if (static_cast<int>(ThreadLocal::GetThreadType()) >= static_cast<int>(ThreadType::GC_THREAD)) {
        return true;
    }
    return false;
}

bool IsGcThread()
{
    if (static_cast<int>(ThreadLocal::GetThreadType()) == static_cast<int>(ThreadType::GC_THREAD)) {
        return true;
    }
    return false;
}

void MutatorManager::BindMutator(Mutator& mutator) const
{
    ThreadLocalData* tlData = ThreadLocal::GetThreadLocalData();
    if (UNLIKELY_CC(tlData->buffer == nullptr)) {
        (void)AllocationBuffer::GetOrCreateAllocBuffer();
    }
    mutator.SetSafepointActive(false);
    tlData->mutator = &mutator;
}

void MutatorManager::UnbindMutator(Mutator& mutator) const
{
    ThreadLocalData* tlData = ThreadLocal::GetThreadLocalData();
    tlData->mutator = nullptr;
    tlData->buffer = nullptr;
}

bool MutatorManager::BindMutatorOnly(Mutator *mutator) const
{
    // watch dog thread may call this function and copy barrier may occur, so bind mutator here.
    common::ThreadLocalData* tlData = common::ThreadLocal::GetThreadLocalData();
    DCHECK_CC(tlData != nullptr);
    if (tlData->mutator == nullptr) {
        tlData->mutator = mutator;
        return true;
    }
    return false;
}

void MutatorManager::UnbindMutatorOnly() const
{
    ThreadLocalData* tlData = ThreadLocal::GetThreadLocalData();
    tlData->mutator = nullptr;
}

Mutator* MutatorManager::CreateMutator()
{
    Mutator* mutator = ThreadLocal::GetMutator();
    ASSERT_LOGF(mutator != nullptr, "Mutator already exists");
    MutatorManagementRLock();
    mutator->Init();
    mutator->InitTid();
    BindMutator(*mutator);
    mutator->SetMutatorPhase(Heap::GetHeap().GetGCPhase());
    MutatorManagementRUnlock();
    return mutator;
}

void MutatorManager::TransitMutatorToExit()
{
    Mutator* mutator = Mutator::GetMutator();
    LOGF_CHECK(mutator != nullptr) << "Mutator has not initialized or has been fini: " << mutator;
    ASSERT_LOGF(!mutator->InSaferegion(), "Mutator to be fini should not be in saferegion");
    // Enter saferegion to avoid blocking gc stw
    mutator->MutatorLock();
    mutator->ResetMutator();
    mutator->MutatorUnlock();
    (void)mutator->EnterSaferegion(false);
    UnbindMutator(*mutator);
}

void MutatorManager::DestroyExpiredMutators()
{
    expiringMutatorListLock_.lock();
    ExpiredMutatorList workList;
    workList.swap(expiringMutators_);
    expiringMutatorListLock_.unlock();
    for (auto it = workList.begin(); it != workList.end(); ++it) {
        Mutator* expiringMutator = *it;
        delete expiringMutator;
    }
}

void MutatorManager::DestroyMutator(Mutator* mutator)
{
    if (TryAcquireMutatorManagementRLock()) {
        delete mutator; // call ~Mutator() under mutatorListLock
        MutatorManagementRUnlock();
    } else {
        expiringMutatorListLock_.lock();
        expiringMutators_.push_back(mutator);
        expiringMutatorListLock_.unlock();
    }
}

Mutator* MutatorManager::CreateRuntimeMutator(ThreadType threadType)
{
    // Because TSAN tool can't identify the RwLock implemented by ourselves,
    // we use a global instance fpMutatorInstance instead of an instance created on
    // heap in order to prevent false positives.
    static Mutator fpMutatorInstance;
    Mutator* mutator = nullptr;
    if (threadType == ThreadType::FP_THREAD) {
        mutator = &fpMutatorInstance;
    } else {
        mutator = new (std::nothrow) Mutator();
    }
    LOGF_CHECK(mutator != nullptr) << "create mutator out of native memory";
    MutatorManagementRLock();
    mutator->Init();
    mutator->InitTid();
    MutatorManager::Instance().BindMutator(*mutator);
    mutator->SetMutatorPhase(Heap::GetHeap().GetGCPhase());
    ThreadLocal::SetMutator(mutator);
    ThreadLocal::SetThreadType(threadType);
    ThreadLocal::SetProcessorFlag(true);
    ThreadLocalData* threadData = GetThreadLocalData();
    PreRunManagedCode(mutator, 2, threadData); // 2 layers
    // only running mutator can enter saferegion.
    MutatorManagementRUnlock();
    return mutator;
}

void MutatorManager::DestroyRuntimeMutator(ThreadType threadType)
{
    Mutator* mutator = ThreadLocal::GetMutator();
    LOGF_CHECK(mutator != nullptr) << "Fini UpdateThreads with null mutator";

    MutatorManagementRLock();
    (void)mutator->LeaveSaferegion();
    // fp mutator is a static instance, we can't delete it, we reset the mutator to avoid invalid memory
    // access when static instance destruction.
    if (threadType != ThreadType::FP_THREAD) {
        delete mutator;
    } else {
        mutator->ResetMutator();
    }
    ThreadLocal::SetAllocBuffer(nullptr);
    ThreadLocal::SetMutator(nullptr);
    ThreadLocal::SetProcessorFlag(false);
    MutatorManagementRUnlock();
}

void MutatorManager::Init()
{
#if defined(__linux__) || defined(PANDA_TARGET_OHOS) || defined(__APPLE__)
    safepointPageManager_ = new (std::nothrow) SafepointPageManager();
    LOGF_CHECK(safepointPageManager_ != nullptr) << "new safepointPageManager failed";
    safepointPageManager_->Init();
#endif
    SetSuspensionMutatorCount(0);
}

MutatorManager& MutatorManager::Instance() noexcept
{
    return BaseRuntime::GetInstance()->GetMutatorManager();
}

void MutatorManager::AcquireMutatorManagementWLock()
{
    uint64_t start = TimeUtil::NanoSeconds();
    bool acquired = TryAcquireMutatorManagementWLock();
    while (!acquired) {
        TimeUtil::SleepForNano(WAIT_LOCK_INTERVAL);
        acquired = TryAcquireMutatorManagementWLock();
        uint64_t now = TimeUtil::NanoSeconds();
        if (!acquired && ((now - start) / SECOND_TO_NANO_SECOND > WAIT_LOCK_TIMEOUT)) {
            LOG_COMMON(FATAL) << "Wait mutator list lock timeout";
            UNREACHABLE_CC();
        }
    }
}

bool MutatorManager::AcquireMutatorManagementWLockForCpuProfile()
{
    uint64_t start = TimeUtil::NanoSeconds();
    bool acquired = TryAcquireMutatorManagementWLock();
    while (!acquired) {
        TimeUtil::SleepForNano(WAIT_LOCK_INTERVAL);
        acquired = TryAcquireMutatorManagementWLock();
        uint64_t now = TimeUtil::NanoSeconds();
        if (!acquired && ((now - start) / SECOND_TO_NANO_SECOND > WAIT_LOCK_TIMEOUT)) {
            LOG_COMMON(FATAL) << "Wait mutator list lock timeout";
            UNREACHABLE_CC();
        }
    }
    return acquired;
}

// Visit all mutators, hold mutatorListLock firstly
void MutatorManager::VisitAllMutators(MutatorVisitor func, bool ignoreFinalizer)
{
    {
        std::lock_guard<std::mutex> guard(allMutatorListLock_);
        for (auto mutator : allMutatorList_) {
            func(*mutator);
        }
    }
    if (!ignoreFinalizer) {
        Mutator* mutator = Heap::GetHeap().GetFinalizerProcessor().GetMutator();
        if (mutator != nullptr) {
            func(*mutator);
        }
    }
}

void MutatorManager::StopTheWorld(bool syncGCPhase, GCPhase phase)
{
#ifndef NDEBUG
    bool saferegionEntered = false;
    // Ensure an active mutator entered saferegion before STW (aka. stop all other mutators).
    if (!IsGcThread()) {
        Mutator* mutator = Mutator::GetMutator();
        if (mutator != nullptr) {
            saferegionEntered = mutator->EnterSaferegion(true);
        }
    }
#endif
    // Block if another thread is holding the stwMutex.
    // Prevent multi-thread doing STW concurrently.
    stwMutex_.lock();
    stwTriggered_.store(true);

    AcquireMutatorManagementWLock();

#ifndef NDEBUG
    // If current mutator saferegion state changed,
    // we should restore it after the mutator called StartTheWorld().
    saferegionStateChanged_ = saferegionEntered;
#endif

    size_t mutatorCount = GetMutatorCount();
    if (UNLIKELY_CC(mutatorCount == 0)) {
        worldStopped_.store(true, std::memory_order_release);
        if (syncGCPhase) { TransitionAllMutatorsToGCPhase(phase); }
        return;
    }
    // set mutatorCount as countOfMutatorsToStop.
    SetSuspensionMutatorCount(static_cast<uint32_t>(mutatorCount));
    DemandSuspensionForStw();
    WaitUntilAllStopped();

    // the world is stopped.
    worldStopped_.store(true, std::memory_order_release);
    if (syncGCPhase) { TransitionAllMutatorsToGCPhase(phase); }
}

void MutatorManager::StartTheWorld() noexcept
{
#ifndef NDEBUG
    bool shouldLeaveSaferegion = saferegionStateChanged_;
#endif
    stwTriggered_.store(false);
    worldStopped_.store(false, std::memory_order_release);

    CancelSuspensionAfterStw();
    SetSuspensionMutatorCount(0);

    // wakeup all mutators which blocking on countOfMutatorsToStop futex.
#if defined(_WIN64) || defined(__APPLE__)
    WakeAllMutators();
#else
    (void)Futex(GetStwFutexWord(), FUTEX_WAKE, INT_MAX);
#endif

    MutatorManagementWUnlock();

    // Release stwMutex to allow other thread call STW.
    stwMutex_.unlock();
#ifndef NDEBUG
    // Restore saferegion state if the state is changed when mutator calls StopTheWorld().
    if (!IsGcThread()) {
        Mutator* mutator = Mutator::GetMutator();
        if (mutator != nullptr && shouldLeaveSaferegion) {
            (void)mutator->LeaveSaferegion();
        }
    }
#endif
}

void MutatorManager::WaitUntilAllStopped()
{
    uint64_t beginTime = TimeUtil::MilliSeconds();
    std::list<Mutator*> unstoppedMutators;
    auto func = [&unstoppedMutators](Mutator& mutator) {
        if ((!mutator.InSaferegion())) {
            unstoppedMutators.emplace_back(&mutator);
        }
    };
    VisitAllMutators(func);

    size_t remainMutatorsSize = unstoppedMutators.size();
    if (remainMutatorsSize == 0) {
        return;
    }

    // Synchronize operation to ensure that all mutators complete phase transition
    // Use unstoppedMutators to avoid traversing the entire mutatorList
    int timeoutTimes = 0;
    while (true) {
        for (auto it = unstoppedMutators.begin(); it != unstoppedMutators.end();) {
            Mutator* mutator = *it;
            if (mutator->InSaferegion()) {
                // current it(mutator) is finished by GC
                it = unstoppedMutators.erase(it);
            } else {
                ++it; // skip current round & check it next round
            }
        }

        if (unstoppedMutators.size() == 0) {
            return;
        }

        if (UNLIKELY_CC(common::g_enableGCTimeoutCheck && TimeUtil::MilliSeconds() - beginTime >
            (((remainMutatorsSize / STW_TIMEOUTS_THREADS_BASE_COUNT) * STW_TIMEOUTS_BASE_MS) + STW_TIMEOUTS_BASE_MS))) {
            timeoutTimes++;
            beginTime = TimeUtil::MilliSeconds();
            DumpMutators(timeoutTimes);
        }

        (void)sched_yield();
    }
}

void MutatorManager::EnsurePhaseTransition(GCPhase phase, std::list<Mutator*> &undoneMutators)
{
    // Traverse through undoneMutators to select mutators that have not yet completed transition
    // 1. ignore mutators which have completed transition
    // 2. gc compete phase transition with mutators which are in saferegion
    // 3. fill mutators which are running state in undoneMutators
    while (undoneMutators.size() > 0) {
        for (auto it = undoneMutators.begin(); it != undoneMutators.end();) {
            Mutator* mutator = *it;
            if (mutator->GetMutatorPhase() == phase && mutator->FinishedTransition()) {
                it = undoneMutators.erase(it);
                continue;
            }
            if (mutator->InSaferegion() && mutator->TransitionGCPhase(false)) {
                it = undoneMutators.erase(it);
                continue;
            }
            ++it;
        }
    }
}

void MutatorManager::TransitionAllMutatorsToGCPhase(GCPhase phase)
{
    // Try to occupy mutatorListLock prevent some mutators from exiting
    bool worldStopped = WorldStopped();
    if (!worldStopped) {
        AcquireMutatorManagementWLock();
    }

    GCPhase prevPhase = Heap::GetHeap().GetGCPhase();
    // Set global gc phase in the scope of mutatorlist lock
    Heap::GetHeap().InstallBarrier(phase);
    Heap::GetHeap().SetGCPhase(phase);

    VLOG(DEBUG, "transition gc phase: %s(%u) -> %s(%u)",
         Collector::GetGCPhaseName(prevPhase), prevPhase, Collector::GetGCPhaseName(phase), phase);

    std::list<Mutator*> undoneMutators;
    // Broadcast mutator phase transition signal to all mutators
    VisitAllMutators([&undoneMutators, phase](Mutator& mutator) {
        mutator.SetSuspensionFlag(Mutator::SuspensionType::SUSPENSION_FOR_GC_PHASE);
        mutator.SetSafepointActive(true);
        undoneMutators.push_back(&mutator);
    });
    EnsurePhaseTransition(phase, undoneMutators);
    if (!worldStopped) {
        MutatorManagementWUnlock();
    }
}

void MutatorManager::EnsureCpuProfileFinish(std::list<Mutator*> &undoneMutators)
{
    while (undoneMutators.size() > 0) {
        for (auto it = undoneMutators.begin(); it != undoneMutators.end();) {
            Mutator* mutator = *it;
            if (mutator->FinishedCpuProfile()) {
                it = undoneMutators.erase(it);
                continue;
            }
            if (mutator->InSaferegion() && mutator->TransitionToCpuProfile(false)) {
                it = undoneMutators.erase(it);
                continue;
            }
            ++it;
        }
    }
}

void MutatorManager::DumpMutators(uint32_t timeoutTimes)
{
    constexpr size_t bufferSize = 4096;
    char buf[bufferSize];
    int index = 0;
    size_t visitedCount = 0;
    size_t visitedSaferegion = 0;
    int firstNotStoppedTid = -1;
    index += sprintf_s(buf, sizeof(buf), "not stopped: ");
    LOGF_CHECK(index != -1) << "Dump mutators state failed";
    size_t mutatorCount = 0;
    VisitAllMutators([&](const Mutator& mut) {
        mutatorCount++;
#ifndef NDEBUG
        mut.DumpMutator();
#endif
        if (!mut.InSaferegion()) {
            if (firstNotStoppedTid == -1) {
                firstNotStoppedTid = static_cast<int>(mut.GetTid());
            }
            int ret = sprintf_s(buf + index, sizeof(buf) - index, "%u ", mut.GetTid());
            LOGF_CHECK(ret != -1) << "Dump mutators state failed";
            index += ret;
        } else {
            ++visitedSaferegion;
        }
        ++visitedCount;
    });
    LOG_COMMON(ERROR) << "MutatorList size: " << mutatorCount;

    LOGF_CHECK(sprintf_s(buf + index, sizeof(buf) - index, ", total: %u, visited: %zu/%zu",
        GetSuspensionMutatorCount(), visitedSaferegion, visitedCount) != -1) <<
            "Dump mutators state failed";
    LOGF_CHECK(timeoutTimes <= MAX_TIMEOUT_TIMES) << "Waiting mutators entering saferegion timeout status info:" << buf;
    LOG_COMMON(ERROR) << "STW status info: " << buf;
}

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
void MutatorManager::DumpForDebug()
{
    size_t count = 0;
    auto func = [&count](Mutator& mutator) {
        mutator.DumpMutator();
        count++;
    };
    VisitAllMutators(func);
    LOG_COMMON(INFO) << "MutatorList size : " << count;
}

void MutatorManager::DumpAllGcInfos()
{
    auto func = [](Mutator& mutator) { mutator.DumpGCInfos(); };
    VisitAllMutators(func);
}
#endif

} // namespace common
