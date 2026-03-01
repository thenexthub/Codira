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


#include "MutatorManager.h"

#include <thread>
#include "Base/TimeUtils.h"
#include "Common/Runtime.h"
#include "Concurrency/ConcurrencyModel.h"
#include "Heap/Collector/FinalizerProcessor.h"
#include "Heap/Collector/TracingCollector.h"
#include "Heap/Heap.h"
#include "Mutator.inline.h"
#include "schedule.h"
#include "CpuProfiler/CpuProfiler.h"

namespace MapleRuntime {
extern "C" uintptr_t MRT_GetSafepointProtectedPage()
{
    return static_cast<uintptr_t>(true);
}

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

extern "C" void HandleSafepoint(ThreadLocalData* tlData)
{
    Mutator* mutator = tlData->mutator;
    // Current mutator enter saferegion
    mutator->DoEnterSaferegion();
    // Current mutator block before leaving saferegion
    mutator->DoLeaveSaferegion();
    DLOG(SIGNAL, "HandleSafepoint, thread restarted.");
}

#if defined (__arm__)
extern "C" void HandleSafepointForArm(ThreadLocalData* tlData)
{
    if (tlData->safepointState == 0) {
        return;
    }
    Mutator* mutator = tlData->mutator;
    // Current mutator enter saferegion
    mutator->DoEnterSaferegion();
    // Current mutator block before leaving saferegion
    mutator->DoLeaveSaferegion();
    DLOG(SIGNAL, "HandleSafepoint, thread restarted.");
}
#endif

void MutatorManager::BindMutator(Mutator& mutator) const
{
    ThreadLocalData* tlData = ThreadLocal::GetThreadLocalData();
    if (UNLIKELY(tlData->buffer == nullptr)) {
        (void)AllocBuffer::GetOrCreateAllocBuffer();
    }
    mutator.SetSafepointStatePtr(&tlData->safepointState);
    mutator.SetSafepointActive(false);
    tlData->mutator = &mutator;
}

void MutatorManager::UnbindMutator(Mutator& mutator) const
{
    ThreadLocalData* tlData = ThreadLocal::GetThreadLocalData();
    MRT_ASSERT(tlData->mutator == &mutator, "mutator in ThreadLocalData doesn't match in codethread");
    tlData->mutator = nullptr;
    mutator.SetSafepointStatePtr(nullptr);
}

Mutator* MutatorManager::CreateMutator()
{
    Mutator* mutator = ConcurrencyModel::GetMutator();
    if (mutator == nullptr) {
        mutator = new (std::nothrow) Mutator();
        CHECK_DETAIL(mutator != nullptr, "new Mutator failed");
        MutatorManagementRLock();
        mutator->Init();
        mutator->InitTid();
        BindMutator(*mutator);
        mutator->SetMutatorPhase(Heap::GetHeap().GetGCPhase());
        ConcurrencyModel::SetMutator(mutator);
    } else {
        MutatorManagementRLock();
        mutator->Init();
        mutator->InitTid();
        BindMutator(*mutator);
        mutator->SetMutatorPhase(Heap::GetHeap().GetGCPhase());
    }
    MutatorManagementRUnlock();
    return mutator;
}

void MutatorManager::TransitMutatorToExit()
{
    Mutator* mutator = Mutator::GetMutator();
    CHECK_DETAIL(mutator != nullptr, "Mutator has not initialized or has been fini: %p", mutator);
    // Enter saferegion to avoid blocking gc stw
    mutator->MutatorLock();
    mutator->ResetMutator();
    mutator->MutatorUnlock();
    (void)mutator->EnterSaferegion(false);
    UnbindMutator(*mutator);
}

void MutatorManager::DestroyExpiredMutators()
{
    expiringMutatorListLock.lock();
    ExpiredMutatorList workList;
    workList.swap(expiringMutators);
    expiringMutatorListLock.unlock();
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
        expiringMutatorListLock.lock();
        expiringMutators.push_back(mutator);
        expiringMutatorListLock.unlock();
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
    CHECK_DETAIL(mutator != nullptr, "create mutator out of native memory");
    MutatorManagementRLock();
    mutator->Init();
    mutator->InitTid();
    mutator->InitProtectStackAddr();
    mutator->SetManagedContext(false);
    MutatorManager::Instance().BindMutator(*mutator);
    mutator->SetMutatorPhase(Heap::GetHeap().GetGCPhase());
    ThreadLocal::SetMutator(mutator);
    ThreadLocal::SetThreadType(threadType);
    ThreadLocal::SetCODEProcessorFlag(true);
    ThreadLocalData* threadData = reinterpret_cast<ThreadLocalData*>(MRT_GetThreadLocalData());
    MRT_PreRunManagedCode(mutator, 2, threadData); // 2 layers
    // only running mutator can enter saferegion.
    MutatorManagementRUnlock();
    return mutator;
}

void MutatorManager::DestroyRuntimeMutator(ThreadType threadType)
{
    Mutator* mutator = ThreadLocal::GetMutator();
    CHECK_DETAIL(mutator != nullptr, "Fini UpdateThreads with null mutator");

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
    ThreadLocal::SetCODEProcessorFlag(false);
    MutatorManagementRUnlock();
}

void MutatorManager::Init()
{
#if defined(__linux__) || defined(hongmeng) || defined(__APPLE__)
    safepointPageManager = new (std::nothrow) SafepointPageManager();
    CHECK_DETAIL(safepointPageManager != nullptr, "new safepointPageManager failed");
    safepointPageManager->Init();
#endif
}

MutatorManager& MutatorManager::Instance() noexcept { return Runtime::Current().GetMutatorManager(); }

void MutatorManager::AcquireMutatorManagementWLock()
{
    uint64_t start = TimeUtil::NanoSeconds();
    bool acquired = TryAcquireMutatorManagementWLock();
    while (!acquired) {
        TimeUtil::SleepForNano(WAIT_LOCK_INTERVAL);
        acquired = TryAcquireMutatorManagementWLock();
        uint64_t now = TimeUtil::NanoSeconds();
        if (!acquired && ((now - start) / SECOND_TO_NANO_SECOND > WAIT_LOCK_TIMEOUT)) {
            LOG(RTLOG_FATAL, "Wait mutator list lock timeout");
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
            LOG(RTLOG_FATAL, "Wait mutator list lock timeout");
        }
        if (!CpuProfiler::GetInstance().GetGenerator().GetIsStart()) {
            break;
        }
    }
    return acquired;
}

// Visit all mutators, hold mutatorListLock firstly
void MutatorManager::VisitAllMutators(MutatorVisitor func)
{
    ScheduleAllCODEThreadVisitMutator(VisitMuatorHelper, &func);
    Mutator* mutator = Heap::GetHeap().GetFinalizerProcessor().GetMutator();
    if (mutator != nullptr) {
        func(*mutator);
    }
}

void MutatorManager::StopTheWorld(bool syncGCPhase, GCPhase phase)
{
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    bool saferegionEntered = false;
    // Ensure an active mutator entered saferegion before STW (aka. stop all other mutators).
    if (!IsGcThread()) {
        Mutator* mutator = Mutator::GetMutator();
        if (mutator != nullptr) {
            saferegionEntered = mutator->EnterSaferegion(true);
        }
    }
#endif
    // Block if another thread is holding the syncMutex.
    // Prevent multi-thread doing STW concurrently.
    syncMutex.lock();
    syncTriggered.store(true);

    AcquireMutatorManagementWLock();

#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    // If current mutator saferegion state changed,
    // we should restore it after the mutator called StartTheWorld().
    saferegionStateChanged = saferegionEntered;
#endif

    size_t mutatorCount = GetMutatorCount();
    if (UNLIKELY(mutatorCount == 0)) {
        worldStopped.store(true, std::memory_order_release);
        if (syncGCPhase) { TransitionAllMutatorsToGCPhase(phase); }
        return;
    }
    // set mutatorCount as countOfMutatorsToStop.
    SetSuspensionMutatorCount(static_cast<uint32_t>(mutatorCount));
    DemandSuspensionForSync();
    WaitUntilAllMutatorStopped();

    // the world is stopped.
    worldStopped.store(true, std::memory_order_release);
    if (syncGCPhase) { TransitionAllMutatorsToGCPhase(phase); }
}

void MutatorManager::StartTheWorld() noexcept
{
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    bool shouldLeaveSaferegion = saferegionStateChanged;
#endif
    syncTriggered.store(false);
    worldStopped.store(false, std::memory_order_release);

    CancelSuspensionAfterSync();
    SetSuspensionMutatorCount(0);

    // wakeup all mutators which blocking on countOfMutatorsToStop futex.
#if defined(_WIN64) || defined(__APPLE__)
    WakeAllMutators();
#else
    (void)MapleRuntime::Futex(GetSyncFutexWord(), FUTEX_WAKE, INT_MAX);
#endif

    MutatorManagementWUnlock();

    // Release syncMutex to allow other thread call STW.
    syncMutex.unlock();
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    // Restore saferegion state if the state is changed when mutator calls StopTheWorld().
    if (!IsGcThread()) {
        Mutator* mutator = Mutator::GetMutator();
        if (mutator != nullptr && shouldLeaveSaferegion) {
            (void)mutator->LeaveSaferegion();
        }
    }
#endif
}

void MutatorManager::StartLightSync(bool syncGCPhase, GCPhase phase)
{
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    bool saferegionEntered = false;
    // Ensure an active mutator entered saferegion before stw.
    if (!IsGcThread()) {
        Mutator* mutator = Mutator::GetMutator();
        if (mutator != nullptr) {
            saferegionEntered = mutator->EnterSaferegion(true);
        }
    }
#endif
    // Block if another thread is holding the syncMutex.
    // Prevent multi-thread doing lsync concurrently.
    syncMutex.lock();
    syncTriggered.store(true);

    AcquireMutatorManagementWLock();

#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    // If current mutator saferegion state changed,
    // we should restore it after the mutator called StopLightSync().
    saferegionStateChanged = saferegionEntered;
#endif

    size_t mutatorCount = GetMutatorCount();
    if (UNLIKELY(mutatorCount == 0)) {
        worldStopped.store(true, std::memory_order_release);
    } else {
        // set mutatorCount as countOfMutatorsToStop.
        SetSuspensionMutatorCount(static_cast<uint32_t>(mutatorCount));
        DemandSuspensionForSync();
        WaitUntilAllMutatorStopped();
        worldStopped.store(true, std::memory_order_release);
    }

    DLOG(GCPHASE, "transition gc: %s(%u) -> %s(%u)",
         Collector::GetGCPhaseName(Heap::GetHeap().GetGCPhase()), Heap::GetHeap().GetGCPhase(),
         Collector::GetGCPhaseName(phase), phase);

    // Set global gc phase in the scope of mutatorlist lock
    Heap::GetHeap().InstallBarrier(phase);
    Heap::GetHeap().SetGCPhase(phase);
    lightSyncGCPhase = phase;
    undoneLightSyncMutators.clear();
    // Broadcast mutator phase transition signal to all mutators
    VisitAllMutators([this](Mutator& mutator) {
        mutator.SetSuspensionFlag(Mutator::SuspensionType::SUSPENSION_FOR_GC_PHASE);
        mutator.SetSafepointActive(true);
        this->undoneLightSyncMutators.push_back(&mutator);
    });
}

void MutatorManager::StopLightSync() noexcept
{
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    bool shouldLeaveSaferegion = saferegionStateChanged;
#endif

    syncTriggered.store(false);
    worldStopped.store(false, std::memory_order_release);

    CancelSuspensionAfterSync();
    SetSuspensionMutatorCount(0);

    // wakeup all mutators which blocking on countOfMutatorsToStop futex.
#if defined(_WIN64) || defined(__APPLE__)
    WakeAllMutators();
#else
    (void)MapleRuntime::Futex(GetSyncFutexWord(), FUTEX_WAKE, INT_MAX);
#endif
    EnsurePhaseTransition(lightSyncGCPhase, undoneLightSyncMutators);
    MutatorManagementWUnlock();
    // Release syncMutex to allow other thread call lsync.
    syncMutex.unlock();
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    // Restore saferegion state if the state is changed when mutator calls StartLightSync().
    if (!IsGcThread()) {
        Mutator* mutator = Mutator::GetMutator();
        if (mutator != nullptr && shouldLeaveSaferegion) {
            (void)mutator->LeaveSaferegion();
        }
    }
#endif
}

void MutatorManager::WaitUntilAllMutatorStopped()
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

        if (UNLIKELY(TimeUtil::MilliSeconds() - beginTime >
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

    DLOG(GCPHASE, "transition gc: %s(%u) -> %s(%u)",
         Collector::GetGCPhaseName(Heap::GetHeap().GetGCPhase()), Heap::GetHeap().GetGCPhase(),
         Collector::GetGCPhaseName(phase), phase);

    // Set global gc phase in the scope of mutatorlist lock
    Heap::GetHeap().InstallBarrier(phase);
    Heap::GetHeap().SetGCPhase(phase);

    std::list<Mutator*> undoneMutators;
    // Broadcast mutator phase transition signal to all mutators
    VisitAllMutators([&undoneMutators](Mutator& mutator) {
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
            if (!CpuProfiler::GetInstance().GetGenerator().GetIsStart()) {
                mutator->ClearSuspensionFlag(Mutator::SUSPENSION_FOR_CPU_PROFILE);
                mutator->SetCpuProfileState(Mutator::FINISH_CPUPROFILE);
                it = undoneMutators.erase(it);
                continue;
            }
            ++it;
        }
    }
}

void MutatorManager::TransitionAllMutatorsToCpuProfile()
{
    bool worldStopped = WorldStopped();
    if (!worldStopped) {
        if (!AcquireMutatorManagementWLockForCpuProfile()) {
            return;
        }
    }
    std::list<Mutator*> undoneMutators;
    VisitAllMutators([&undoneMutators](Mutator& mutator) {
        if (mutator.GetTid() != Heap::GetHeap().GetFinalizerProcessor().GetTid() &&
            mutator.GetCodethreadPtr() == MutatorManager::Instance().GetMainThreadHandle()) {
            mutator.SetSuspensionFlag(Mutator::SuspensionType::SUSPENSION_FOR_CPU_PROFILE);
            mutator.SetSafepointActive(true);
            undoneMutators.push_back(&mutator);
        }
    });
    EnsureCpuProfileFinish(undoneMutators);
    if (!worldStopped) {
        MutatorManagementWUnlock();
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
    CHECK_DETAIL(index != -1, "Dump mutators state failed");
    size_t mutatorCount = 0;
    VisitAllMutators([&](const Mutator& mut) {
        mutatorCount++;
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
        mut.DumpMutator();
#endif
        if (!mut.InSaferegion()) {
            if (firstNotStoppedTid == -1) {
                firstNotStoppedTid = static_cast<int>(mut.GetTid());
            }
            int ret = sprintf_s(buf + index, sizeof(buf) - index, "%u ", mut.GetTid());
            CHECK_DETAIL(ret != -1, "Dump mutators state failed");
            index += ret;
        } else {
            ++visitedSaferegion;
        }
        ++visitedCount;
    });
    LOG(RTLOG_ERROR, "MutatorList size : %zu", mutatorCount);

    CHECK_DETAIL(sprintf_s(buf + index, sizeof(buf) - index, ", total: %u, visited: %zu/%zu",
                           GetSuspensionMutatorCount(), visitedSaferegion, visitedCount) != -1,
                 "Dump mutators state failed");
    CHECK_DETAIL(timeoutTimes <= MAX_TIMEOUT_TIMES, "Waiting mutators entering saferegion timeout status info:%s", buf);
    LOG(RTLOG_ERROR, "STW status info:%s", buf);
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
    LOG(RTLOG_INFO, "MutatorList size : %zu", count);
}

void MutatorManager::DumpAllGcInfos()
{
    auto func = [](Mutator& mutator) { mutator.DumpGCInfos(); };
    VisitAllMutators(func);
}
#endif

extern "C" void MRT_FlushGCInfo()
{
#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    // MutatorManager::Instance().DumpAllGcInfos();
    Mutator::GetMutator()->DumpGCInfos();
#endif
}

#ifdef __APPLE__
extern "C" MRT_EXPORT void CODE_MRT_FlushGCInfo();
__asm__(".global _CODE_MRT_FlushGCInfo\n\t.set _CODE_MRT_FlushGCInfo, _MRT_FlushGCInfo");
#else
extern "C" MRT_EXPORT void CODE_MRT_FlushGCInfo() __attribute__((alias("MRT_FlushGCInfo")));
#endif
} // namespace MapleRuntime
