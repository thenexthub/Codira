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

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-macro-usage,
//             cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//             readability-else-after-return, readability-duplicate-include,
//             misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//             google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//             modernize-use-auto, llvm-namespace-comment,
//             cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//             readability-implicit-bool-conversion)

#ifndef COMMON_INTERFACES_THREAD_MUTATOR_BASE_H
#define COMMON_INTERFACES_THREAD_MUTATOR_BASE_H

#include <atomic>
#include <mutex>
#include <pthread.h>
#include "common_interfaces/base/common.h"
namespace common {
class Mutator;
class ThreadHolder;

// GCPhase describes phases for stw/concurrent gc.
enum GCPhase : uint8_t {
    GC_PHASE_UNDEF = 0,
    GC_PHASE_IDLE = 1,
    GC_PHASE_START = 8,

    // only gc phase after GC_PHASE_START ( enum value > GC_PHASE_START) needs barrier.
    GC_PHASE_ENUM = 9,
    GC_PHASE_MARK = 10,
    GC_PHASE_REMARK_SATB = 11,
    GC_PHASE_FINAL_MARK = 12,
    GC_PHASE_POST_MARK = 13,
    GC_PHASE_PRECOPY = 14,
    GC_PHASE_COPY = 15,
    GC_PHASE_FIX = 16,
};

class PUBLIC_API MutatorBase {
public:
    // flag which indicates the reason why mutator should suspend. flag is set by some external thread.
    enum SuspensionType : uint32_t {
        SUSPENSION_FOR_GC_PHASE = 1 << 0,
        SUSPENSION_FOR_STW = 1 << 1,
        SUSPENSION_FOR_EXIT = 1 << 2,
        SUSPENSION_FOR_CPU_PROFILE = 1 << 3,
        SUSPENSION_FOR_PENDING_CALLBACK = 1 << 4,
        SUSPENSION_FOR_RUNNING_CALLBACK = 1 << 5,
        /**
         * The below ones are not actually suspension request, it's just some callbacks need to process
         * at the beginning of transfering to RUNNING
         * So This is equivalent to:
         * ````    __attribute__((always_inline)) inline void DoLeaveSaferegion()
         * ````    {
         * ````        SetInSaferegion(SAFE_REGION_FALSE);
         * ````        if (UNLIKELY_CC(HasAnySuspensionRequest())) {
         * ````            HandleSuspensionRequest();
         * ````        }
         * ------>     if (UNLIKELY_CC(HasOtherCallback())) {
         * ------>         ProcessCallback();
         * ------>     }
         * ````    }
         * But this will make `DoLeaveSaferegion` cost more, so we just merge it into suspension request,
         * and do some extra process at the end of `HandleSuspensionRequest`
        */
        SUSPENSION_FOR_FINALIZE = 1U << 31,
        CALLBACKS_TO_PROCESS = SUSPENSION_FOR_FINALIZE,
    };

    enum GCPhaseTransitionState : uint32_t {
        NO_TRANSITION,
        NEED_TRANSITION,
        IN_TRANSITION,
        FINISH_TRANSITION,
    };

    enum CpuProfileState : uint32_t {
        NO_CPUPROFILE,
        NEED_CPUPROFILE,
        IN_CPUPROFILING,
        FINISH_CPUPROFILE,
    };

    // Indicate whether mutator is in saferegion
    enum SaferegionState : uint32_t {
        SAFE_REGION_TRUE = 0x17161514,
        SAFE_REGION_FALSE = 0x03020100,
    };

    // // Indicate whether mutator need SafepointPolling
    // enum class SafepointPollingState : uint64_t {
    //     SAFEPOINT_POLLING_FALSE = 0,
    //     SAFEPOINT_POLLING_TRUE = 1,
    // };

    // Called when a mutator starts and finishes, respectively.
    void Init()
    {
        observerCnt_ = 0;
        mutatorPhase_.store(GCPhase::GC_PHASE_IDLE);
    }

    // Sets saferegion state of this mutator.
    __attribute__((always_inline)) inline void SetInSaferegion(SaferegionState state)
    {
        // assure sequential execution of setting insaferegion state and checking suspended state.
        inSaferegion_.store(state, std::memory_order_seq_cst);
    }

    // Returns true if this mutator is in saferegion, otherwise false.
    __attribute__((always_inline)) inline bool InSaferegion() const
    {
        return inSaferegion_.load(std::memory_order_seq_cst) != SAFE_REGION_FALSE;
    }

    inline void IncObserver()
    {
        observerCnt_.fetch_add(1);
    }

    inline void DecObserver()
    {
        observerCnt_.fetch_sub(1);
    }

    // Return true indicate there are some observer is visitting this mutator
    inline bool HasObserver()
    {
        return observerCnt_.load() != 0;
    }

    inline size_t GetObserverCount() const
    {
        return observerCnt_.load();
    }

    // Force current mutator enter saferegion, internal use only.
    __attribute__((always_inline)) inline void DoEnterSaferegion();
    // Force current mutator leave saferegion, internal use only.
    __attribute__((always_inline)) inline void DoLeaveSaferegion()
    {
        SetInSaferegion(SAFE_REGION_FALSE);
        // go slow path if the mutator should suspend
        if (UNLIKELY_CC(HasAnySuspensionRequest())) {
            HandleSuspensionRequest();
        }
    }

    // If current mutator is not in saferegion, enter and return true
    // If current mutator has been in saferegion, return false
    __attribute__((always_inline)) inline bool EnterSaferegion(bool updateUnwindContext) noexcept;
    // If current mutator is in saferegion, leave and return true
    // If current mutator has left saferegion, return false
    __attribute__((always_inline)) inline bool LeaveSaferegion() noexcept;

    // Called if current mutator should do corresponding task by suspensionFlag value
    __attribute__((visibility ("default"))) void HandleSuspensionRequest();
    // Called if current mutator should handle stw request
    void SuspendForStw();

    // temporary impl to clean GC callback, and need to refact to flip function
    __attribute__((visibility ("default"))) void HandleJSGCCallback();

    static uint32_t ConstructSuspensionFlag(uint32_t flag, uint32_t clearFlag, uint32_t setFlag)
    {
        return (flag & ~clearFlag) | setFlag;
    }

    __attribute__((always_inline)) inline bool FinishedTransition() const
    {
        return transitionState_ == FINISH_TRANSITION;
    }

    __attribute__((always_inline)) inline bool FinishedCpuProfile() const
    {
        return cpuProfileState_.load(std::memory_order_acquire) == FINISH_CPUPROFILE;
    }

    __attribute__((always_inline)) inline void SetCpuProfileState(CpuProfileState state)
    {
        cpuProfileState_.store(state, std::memory_order_relaxed);
    }

    __attribute__((always_inline)) inline void SetSuspensionFlag(SuspensionType flag)
    {
        if (flag == SUSPENSION_FOR_GC_PHASE) {
            transitionState_.store(NEED_TRANSITION, std::memory_order_relaxed);
        } else if (flag == SUSPENSION_FOR_CPU_PROFILE) {
            cpuProfileState_.store(NEED_CPUPROFILE, std::memory_order_relaxed);
        }
        suspensionFlag_.fetch_or(flag, std::memory_order_seq_cst);
    }

    __attribute__((always_inline)) inline void ClearSuspensionFlag(SuspensionType flag)
    {
        suspensionFlag_.fetch_and(~flag, std::memory_order_seq_cst);
    }

    __attribute__((always_inline)) inline uint32_t GetSuspensionFlag() const
    {
        return suspensionFlag_.load(std::memory_order_acquire);
    }

    __attribute__((always_inline)) inline bool HasSuspensionRequest(SuspensionType flag) const
    {
        return (suspensionFlag_.load(std::memory_order_acquire) & flag) != 0;
    }

    // Check whether current mutator needs to be suspended for GC or other request
    __attribute__((always_inline)) inline bool HasAnySuspensionRequest() const
    {
        return (suspensionFlag_.load(std::memory_order_acquire) != 0);
    }

    // Check whether current mutator needs to be suspended for GC or other request, see comments in `SuspensionType`
    __attribute__((always_inline)) inline bool HasAnySuspensionRequestExceptCallbacks() const
    {
        uint32_t flag = suspensionFlag_.load(std::memory_order_acquire);
        return (flag & ~CALLBACKS_TO_PROCESS) != 0;
    }

    __attribute__((always_inline)) inline bool CASSetSuspensionFlag(uint32_t oldFlag, uint32_t newFlag)
    {
        return suspensionFlag_.compare_exchange_strong(oldFlag, newFlag);
    }

    __attribute__((always_inline)) inline void SetFinalizeRequest()
    {
        SetSuspensionFlag(SUSPENSION_FOR_FINALIZE);
    }

    __attribute__((always_inline)) inline void ClearFinalizeRequest()
    {
        ClearSuspensionFlag(SUSPENSION_FOR_FINALIZE);
    }

    void SetSafepointActive(bool value)
    {
        safepointActive_ = static_cast<uint32_t>(value);
    }

    // Spin wait phase transition finished when GC is tranverting this mutator's phase
    __attribute__((always_inline)) inline void WaitForPhaseTransition() const
    {
        GCPhaseTransitionState state = transitionState_.load(std::memory_order_acquire);
        while (state != FINISH_TRANSITION) {
            if (state != IN_TRANSITION) {
                return;
            }
            // Give up CPU to avoid overloading
            (void)sched_yield();
            state = transitionState_.load(std::memory_order_acquire);
        }
    }

    __attribute__((always_inline)) inline void WaitForCpuProfiling() const
    {
        while (cpuProfileState_.load(std::memory_order_acquire) != FINISH_CPUPROFILE) {
            // Give up CPU to avoid overloading
            (void)sched_yield();
        }
    }

    inline void GcPhaseEnum(GCPhase newPhase);
    inline void GCPhasePreForward(GCPhase newPhase);
    inline void HandleGCPhase(GCPhase newPhase);

    inline void HandleCpuProfile();

    void TransitionToGCPhaseExclusive(GCPhase newPhase);

    void TransitionToCpuProfileExclusive();

    // Ensure that mutator phase is changed only once by mutator itself or GC
    bool TransitionGCPhase(bool bySelf);

    __attribute__((always_inline)) inline bool TransitionToCpuProfile(bool bySelf);

    __attribute__((always_inline)) inline void SetMutatorPhase(const GCPhase newPhase)
    {
        mutatorPhase_.store(newPhase, std::memory_order_release);
    }

    __attribute__((always_inline)) inline GCPhase GetMutatorPhase() const
    {
        return mutatorPhase_.load(std::memory_order_acquire);
    }

    bool GetSafepointActiveState() const
    {
        return safepointActive_;
    }

    void MutatorBaseLock()
    {
        mutatorBaseLock_.lock();
    }

    void MutatorBaseUnlock() {
        mutatorBaseLock_.unlock();
    }

    void RegisterJSThread(void *jsThread)
    {
        CHECK_CC(jsThread_ == nullptr);
        jsThread_ = jsThread;
    }

    void UnregisterJSThread()
    {
        jsThread_ = nullptr;
    }

    static constexpr size_t GetSafepointActiveOffset()
    {
        return MEMBER_OFFSET_CC(MutatorBase, safepointActive_);
    }

private:
    // Indicate whether execution thread should check safepoint suspension request
    uint32_t safepointActive_ = 0;
    // Indicate the current mutator phase and use which barrier in concurrent gc
    std::atomic<GCPhase> mutatorPhase_ = { GCPhase::GC_PHASE_UNDEF };
    // in saferegion, it will not access any managed objects and can be visitted by observer
    std::atomic<uint32_t> inSaferegion_ = { SAFE_REGION_TRUE };
    // Protect observerCnt
    std::mutex observeCntMutex_;
    // Increase when this mutator is observed by some observer
    std::atomic<size_t> observerCnt_ = { 0 };

    // If set implies this mutator should process suspension requests
    std::atomic<uint32_t> suspensionFlag_ = { 0 };
    // Indicate the state of mutator's phase transition
    std::atomic<GCPhaseTransitionState> transitionState_ = { NO_TRANSITION };

    std::mutex mutatorBaseLock_;

    std::atomic<CpuProfileState> cpuProfileState_ = { NO_CPUPROFILE };

    // This is stored for process `satbNode`, merge Mutator & MutatorBase & SatbNode
    void *mutator_ {nullptr};

    // used to synchronize cmc-gc phase to JSThread
    void *jsThread_ {nullptr};

    friend Mutator;
    friend ThreadHolder;
};
}  // namespace common
#endif  // COMMON_INTERFACES_THREAD_MUTATOR_BASE_H
// NOLINTEND(readability-identifier-naming, cppcoreguidelines-macro-usage,
//           cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//           readability-else-after-return, readability-duplicate-include,
//           misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//           google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//           modernize-use-auto, llvm-namespace-comment,
//           cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//           readability-implicit-bool-conversion)
