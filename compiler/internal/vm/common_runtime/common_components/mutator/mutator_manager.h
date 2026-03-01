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

#ifndef COMMON_COMPONENTS_MUTATOR_MUTATOR_MANAGER_H
#define COMMON_COMPONENTS_MUTATOR_MUTATOR_MANAGER_H

#include <bitset>
#include <list>
#include <mutex>
#include <unordered_set>

#include "common_components/base/atomic_spin_lock.h"
#include "common_components/base/globals.h"
#include "common_components/base/rw_lock.h"
#include "common_components/common/page_allocator.h"
#include "common_components/mutator/mutator.h"
#if defined(__linux__) || defined(PANDA_TARGET_OHOS) || defined(__APPLE__)
#include "common_components/mutator/safepoint_page_manager.h"
#endif
#include "common_components/mutator/thread_local.h"
#include "common_components/log/log.h"

namespace common {
const uint64_t WAIT_LOCK_INTERVAL = 5000; // 5us
const uint64_t WAIT_LOCK_TIMEOUT = 30;    // seconds
const uint32_t MAX_TIMEOUT_TIMES = 1;
const int STW_TIMEOUTS_THREADS_BASE_COUNT = 100;
// STW wait base timeout in milliseconds, for every 100 threads, the time is increased by 240000ms.
const int STW_TIMEOUTS_BASE_MS = 240000;
const uint32_t LOCK_OWNER_NONE = 0;
const uint32_t LOCK_OWNER_GC = LOCK_OWNER_NONE + 1;
const uint32_t LOCK_OWNER_MUTATOR = LOCK_OWNER_GC + 1;

extern bool g_enableGCTimeoutCheck;

bool IsRuntimeThread();
bool IsGcThread();

using MutatorVisitor = std::function<void(Mutator&)>;

struct STWParam {
    const char* stwReason;
    uint64_t elapsedTimeNs = 0;

    uint64_t GetElapsedNs() const { return elapsedTimeNs; }
    uint64_t GetElapsedUs() const { return elapsedTimeNs / 1000; }
};

class MutatorManager {
public:
    MutatorManager() {}
    ~MutatorManager()
    {
#if defined(__linux__) || defined(PANDA_TARGET_OHOS) || defined(__APPLE__)
        if (safepointPageManager_ != nullptr) {
            delete safepointPageManager_;
            safepointPageManager_ = nullptr;
        }
#endif
    }

    MutatorManager(const MutatorManager&) = delete;
    MutatorManager(MutatorManager&&) = delete;
    MutatorManager& operator=(const MutatorManager&) = delete;
    MutatorManager& operator=(MutatorManager&&) = delete;

    static MutatorManager& Instance() noexcept;

    void Init();
    void Fini() { SatbBuffer::Instance().Fini(); }

    // Get the mutator list instance
    size_t GetMutatorCount()
    {
        size_t count = 0;
        auto func = [&count](Mutator&) { count++; };
        VisitAllMutators(func);
        return count;
    }

    void GetAllMutators(std::vector<Mutator*>& mutators)
    {
        auto func = [&mutators](Mutator& mutator) { mutators.emplace_back(&mutator); };
        VisitAllMutators(func);
    }

    bool TryAcquireMutatorManagementWLock()
    {
        return mutatorManagementRWLock_.TryLockWrite();
    }

    bool TryAcquireMutatorManagementRLock()
    {
        return mutatorManagementRWLock_.TryLockRead();
    }

    void AcquireMutatorManagementWLock();

    bool AcquireMutatorManagementWLockForCpuProfile();

    static void VisitMuatorHelper(void* argPtr, void* handle)
    {
        Mutator* mutator = reinterpret_cast<Mutator*>(argPtr);
        if (mutator != nullptr) {
            (*reinterpret_cast<MutatorVisitor*>(handle))(*mutator);
        }
    }

    // Visit all mutators, hold mutatorListLock firstly
    void VisitAllMutators(MutatorVisitor func, bool ignoreFinalizer = false);

    // Some functions about stw
    void StopTheWorld(bool syncGCPhase, GCPhase phase);
    void StartTheWorld() noexcept;
    void WaitUntilAllStopped();
    void DumpMutators(uint32_t timeoutTimes);
    void DemandSuspensionForStw()
    {
        VisitAllMutators([](Mutator& mutator) {
            mutator.SetSafepointActive(true);
            mutator.SetSuspensionFlag(MutatorBase::SuspensionType::SUSPENSION_FOR_STW);
        });
    }

    void CancelSuspensionAfterStw()
    {
        VisitAllMutators([](Mutator& mutator) {
            mutator.SetSafepointActive(false);
            mutator.ClearSuspensionFlag(MutatorBase::SuspensionType::SUSPENSION_FOR_STW);
        });
    }

    void BindMutator(Mutator& mutator) const;
    void UnbindMutator(Mutator& mutator) const;

    bool BindMutatorOnly(Mutator *mutator) const;
    void UnbindMutatorOnly() const;

    // Create and initialize the local mutator, then register to mutatorlist.
    Mutator* CreateMutator();

    // Delete the thread-local mutator and unregister from mutatorlist before a mutator exit
    void TransitMutatorToExit();

    void DestroyMutator(Mutator* mutator);

    Mutator* CreateRuntimeMutator(ThreadType threadType) __attribute__((noinline));
    void DestroyRuntimeMutator(ThreadType threadType);

    bool WorldStopped() const { return worldStopped_.load(std::memory_order_acquire); }

    bool StwTriggered() const { return stwTriggered_.load(); }

    void LockStopTheWorld() { stwMutex_.lock(); }

    void UnlockStopTheWorld() noexcept { stwMutex_.unlock(); }

    void EnsurePhaseTransition(GCPhase phase, std::list<Mutator*> &undoneMutators);
    void TransitionAllMutatorsToGCPhase(GCPhase phase);

    void EnsureCpuProfileFinish(std::list<Mutator*> &undoneMutators);

    template<class STWFunction>
    void FlipMutators(STWParam& param, STWFunction&& stwFunction, FlipFunction *flipFunction);
#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    void DumpForDebug();
    void DumpAllGcInfos();
#endif

    __attribute__((always_inline)) inline int* GetStwFutexWord()
    {
        return reinterpret_cast<int*>(&suspensionMutatorCount_);
    }

    __attribute__((always_inline)) inline int GetStwFutexWordValue() const
    {
        return GetSuspensionMutatorCount();
    }

    __attribute__((always_inline)) inline uint32_t GetSuspensionMutatorCount() const
    {
        return suspensionMutatorCount_.load(std::memory_order_acquire);
    }

    __attribute__((always_inline)) inline void SetSuspensionMutatorCount(uint32_t total)
    {
        suspensionMutatorCount_.store(total, std::memory_order_release);
    }

#if defined(_WIN64) || defined (__APPLE__)
    __attribute__((always_inline)) inline void MutatorWait()
    {
        std::unique_lock<std::mutex> mutatorsToStopLck(mutatorSuspensionMtx_);
        mutatorSuspensionCV_.wait(mutatorsToStopLck, [this]() {
            return GetSuspensionMutatorCount() == 0;
        });
    }

    __attribute__((always_inline)) inline void WakeAllMutators()
    {
        std::unique_lock<std::mutex> mutatorsToStopLck(mutatorSuspensionMtx_);
        mutatorSuspensionCV_.notify_all();
    }
#endif

#if not defined (_WIN64)
    const SafepointPageManager* GetSafepointPageManager() const { return safepointPageManager_; }
#endif

    void MutatorManagementRLock() { mutatorManagementRWLock_.LockRead(); }

    void MutatorManagementRUnlock() { mutatorManagementRWLock_.UnlockRead(); }

    void MutatorManagementWLock() { mutatorManagementRWLock_.LockWrite(); }

    void MutatorManagementWUnlock() { mutatorManagementRWLock_.UnlockWrite(); }

    void DestroyExpiredMutators();

    bool HasNativeMutator();

    std::mutex allMutatorListLock_;  /* Mutex guard for allMutatorList */
    // Manage all mutator during runtime
    /* Manage all active mutators during runtime */
    std::list<Mutator*, StdContainerAllocator<Mutator*, MUTATOR_LIST>> allMutatorList_;

private:
    using ExpiredMutatorList = std::list<Mutator*, StdContainerAllocator<Mutator*, MUTATOR_LIST>>;
    ExpiredMutatorList expiringMutators_;
    std::mutex expiringMutatorListLock_;

    // guard mutator set for stop-the-world
    RwLock mutatorManagementRWLock_;

    // count of mutators need to be suspended for stw.
    // this field is also used as futex wait/wakeup word for stw.
    std::atomic<uint32_t> suspensionMutatorCount_;

    // Ensure only one thread can doing STW.
    std::recursive_mutex stwMutex_;
#ifndef NDEBUG
    bool saferegionStateChanged_ = false;
#endif
    // Show current STW state
    std::atomic<bool> worldStopped_ = { false };
    std::atomic<bool> stwTriggered_ = { false };
#if defined(_WIN64) || defined (__APPLE__)
    std::condition_variable mutatorSuspensionCV_;
    std::mutex mutatorSuspensionMtx_;
#endif
#if not defined (_WIN64)
    SafepointPageManager* safepointPageManager_ = nullptr;
#endif
    Mutator* nativeMutator_ = nullptr;
};

// Scoped stop the world.
class ScopedStopTheWorld {
public:
    __attribute__((always_inline)) explicit ScopedStopTheWorld(STWParam& param,
                                                               bool syncGCPhase = false, GCPhase phase = GC_PHASE_IDLE)
        : stwParam_(param)
    {
        reason_ = param.stwReason;
        MutatorManager::Instance().StopTheWorld(syncGCPhase, phase);
        startTime_ = TimeUtil::NanoSeconds();
    }

    __attribute__((always_inline)) ~ScopedStopTheWorld()
    {
        uint64_t elapsedTimeNs = GetElapsedTime();
        stwParam_.elapsedTimeNs = elapsedTimeNs;
        VLOG(DEBUG, "%s stw time %zu us", reason_, elapsedTimeNs / 1000); // 1000:nsec per usec
        MutatorManager::Instance().StartTheWorld();
    }

    uint64_t GetElapsedTime() const { return TimeUtil::NanoSeconds() - startTime_; }

private:
    uint64_t startTime_ = 0;
    const char* reason_ = nullptr;
    STWParam& stwParam_;
};

// Scoped lock STW, this prevent other thread STW during the current scope.
class ScopedSTWLock {
public:
    __attribute__((always_inline)) explicit ScopedSTWLock() { MutatorManager::Instance().LockStopTheWorld(); }

    __attribute__((always_inline)) ~ScopedSTWLock() { MutatorManager::Instance().UnlockStopTheWorld(); }
};
} // namespace common

#endif  // COMMON_COMPONENTS_MUTATOR_MUTATOR_MANAGER_H
