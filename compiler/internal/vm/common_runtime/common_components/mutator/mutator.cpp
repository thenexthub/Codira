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

#include <cstdint>
#include <stack>
#include <unistd.h>

#include "common_components/common_runtime/hooks.h"
#include "common_components/common/type_def.h"
#if defined(_WIN64)
#define NOGDI
#include <windows.h>
#endif
#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/collector/marking_collector.h"
#include "common_components/common/scoped_object_access.h"
#include "common_components/mutator/mutator_manager.h"

namespace common {
ThreadLocalData *GetThreadLocalData()
{
    uintptr_t tlDataAddr = reinterpret_cast<uintptr_t>(ThreadLocal::GetThreadLocalData());
#if defined(__aarch64__)
    if (Heap::GetHeap().IsGcStarted()) {
        // Since the TBI(top bit ignore) feature in Aarch64,
        // set gc phase to high 8-bit of ThreadLocalData Address for gc barrier fast path.
        // 56: make gcphase value shift left 56 bit to set the high 8-bit
        tlDataAddr = tlDataAddr | (static_cast<uint64_t>(Heap::GetHeap().GetGCPhase()) << 56);
    }
#endif
    return reinterpret_cast<ThreadLocalData *>(tlDataAddr);
}

// Ensure that mutator phase is changed only once by mutator itself or GC
bool MutatorBase::TransitionGCPhase(bool bySelf)
{
    do {
        GCPhaseTransitionState state = transitionState_.load();
        // If this mutator phase transition has finished, just return
        if (state == FINISH_TRANSITION) {
            bool result = mutatorPhase_.load() == Heap::GetHeap().GetGCPhase();
            if (!bySelf && !result) { // why check bySelf?
                LOG_COMMON(FATAL) << "Unresolved fatal";
                UNREACHABLE_CC();
            }
            return result;
        }

        // If this mutator is executing phase transition by other thread, mutator should wait but GC just return
        if (state == IN_TRANSITION) {
            if (bySelf) {
                WaitForPhaseTransition();
                return true;
            } else {
                return false;
            }
        }

        if (!bySelf && state == NO_TRANSITION) {
            return true;
        }

        // Current thread set atomic variable to ensure atomicity of phase transition
        CHECK_CC(state == NEED_TRANSITION);
        if (transitionState_.compare_exchange_weak(state, IN_TRANSITION)) {
            TransitionToGCPhaseExclusive(Heap::GetHeap().GetGCPhase());
            transitionState_.store(FINISH_TRANSITION, std::memory_order_release);
            return true;
        }
    } while (true);
}

void MutatorBase::HandleSuspensionRequest()
{
    for (;;) {
        SetInSaferegion(SAFE_REGION_TRUE);
        if (HasSuspensionRequest(SUSPENSION_FOR_STW)) {
            SuspendForStw();
            if (HasSuspensionRequest(SUSPENSION_FOR_GC_PHASE)) {
                TransitionGCPhase(true);
            } else if (HasSuspensionRequest(SUSPENSION_FOR_CPU_PROFILE)) {
                TransitionToCpuProfile(true);
            }
        } else if (HasSuspensionRequest(SUSPENSION_FOR_GC_PHASE)) {
            TransitionGCPhase(true);
        } else if (HasSuspensionRequest(SUSPENSION_FOR_CPU_PROFILE)) {
            TransitionToCpuProfile(true);
        } else if (HasSuspensionRequest(SUSPENSION_FOR_EXIT)) {
            while (true) {
                sleep(INT_MAX);
            }
        } else if (HasSuspensionRequest(SUSPENSION_FOR_PENDING_CALLBACK)) {
            reinterpret_cast<Mutator*>(mutator_)->TryRunFlipFunction();
        } else if (HasSuspensionRequest(SUSPENSION_FOR_RUNNING_CALLBACK)) {
            reinterpret_cast<Mutator*>(mutator_)->WaitFlipFunctionFinish();
        }
        SetInSaferegion(SAFE_REGION_FALSE);
        // Leave saferegion if current mutator has no suspend request, otherwise try again
        if (LIKELY_CC(!HasAnySuspensionRequestExceptCallbacks() && !HasObserver())) {
            if (HasSuspensionRequest(SUSPENSION_FOR_FINALIZE)) {
                ClearFinalizeRequest();
                HandleJSGCCallback();
            }
            return;
        }
    }
}

void MutatorBase::HandleJSGCCallback()
{
    if (mutator_ != nullptr) {
        void *vm = reinterpret_cast<Mutator*>(mutator_)->GetEcmaVMPtr();
        if (vm != nullptr) {
            JSGCCallback(vm);
        }
    }
}

void MutatorBase::SuspendForStw()
{
    ClearSuspensionFlag(SUSPENSION_FOR_STW);
    // wait until StartTheWorld
    int curCount = static_cast<int>(MutatorManager::Instance().GetStwFutexWordValue());
    // Avoid losing wake-ups
    if (curCount > 0) {
#if defined(_WIN64) || defined(__APPLE__)
        MutatorManager::Instance().MutatorWait();
#else
        int* countAddr = MutatorManager::Instance().GetStwFutexWord();
        // FUTEX_WAIT may fail when gc thread wakes up all threads before the current thread reaches this position.
        // But it is not important because there won't be data race between the current thread and the gc thread,
        // and it also won't be frozen since gc thread also modifies the value at countAddr before its waking option.
        (void)Futex(countAddr, FUTEX_WAIT, curCount);
#endif
    }
    SetInSaferegion(SAFE_REGION_FALSE);
    if (MutatorManager::Instance().StwTriggered()) {
        // entering this branch means a second request has been broadcasted, we need to reset this flag to avoid
        // missing the request. And this must be after the behaviour that set saferegion state to false, because
        // we need to make sure that the mutator can always perceive the gc request when the mutator is not in
        // safe region.
        SetSuspensionFlag(SUSPENSION_FOR_STW);
    }
}

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
void Mutator::CreateCurrentGCInfo() { gcInfos_.CreateCurrentGCInfo(); }
#endif


void Mutator::VisitRawObjects(const RootVisitor& func)
{
    if (rawObject_.object != nullptr) {
        func(rawObject_);
    }
}

Mutator* Mutator::GetMutator() noexcept
{
    return ThreadLocal::GetMutator();
}

inline void CheckAndPush(BaseObject* obj, std::set<BaseObject*>& rootSet, std::stack<BaseObject*>& rootStack)
{
    auto search = rootSet.find(obj);
    if (search == rootSet.end()) {
        rootSet.insert(obj);
        if (obj->IsValidObject() && obj->HasRefField()) {
            rootStack.push(obj);
        }
    }
}

inline void MutatorBase::GcPhaseEnum(GCPhase newPhase)
{
}

// comment all
inline void MutatorBase::GCPhasePreForward(GCPhase newPhase)
{
}

inline void MutatorBase::HandleGCPhase(GCPhase newPhase)
{
    if (newPhase == GCPhase::GC_PHASE_POST_MARK) {
        std::lock_guard<std::mutex> lg(mutatorBaseLock_);
        Mutator *actMutator = reinterpret_cast<Mutator*>(mutator_);
        if (actMutator->satbNode_ != nullptr) {
            DCHECK_CC(actMutator->satbNode_->IsEmpty());
            SatbBuffer::Instance().RetireNode(actMutator->satbNode_);
            actMutator->satbNode_ = nullptr;
        }
    } else if (newPhase == GCPhase::GC_PHASE_ENUM) {
        GcPhaseEnum(newPhase);
    } else if (newPhase == GCPhase::GC_PHASE_PRECOPY) {
        GCPhasePreForward(newPhase);
    } else if (newPhase == GCPhase::GC_PHASE_REMARK_SATB || newPhase == GCPhase::GC_PHASE_FINAL_MARK) {
        std::lock_guard<std::mutex> lg(mutatorBaseLock_);
        Mutator *actMutator = reinterpret_cast<Mutator*>(mutator_);
        if (actMutator->satbNode_ != nullptr) {
            SatbBuffer::Instance().RetireNode(actMutator->satbNode_);
            actMutator->satbNode_ = nullptr;
        }
    }
}

void MutatorBase::TransitionToGCPhaseExclusive(GCPhase newPhase)
{
    HandleGCPhase(newPhase);
    SetSafepointActive(false);
    mutatorPhase_.store(newPhase, std::memory_order_relaxed); // handshake between mutator & mainGC thread
    if (jsThread_ != nullptr) {
        // non-atomic, should update JSThread local gc state before SuspensionFlag store,
        // and SuspensionFlag load when transfer to running will guarantee the visibility of
        // the JSThread local gc state
        SynchronizeGCPhaseToJSThread(jsThread_, newPhase);
    }
    // Clear mutator's suspend request after phase transition
    ClearSuspensionFlag(SUSPENSION_FOR_GC_PHASE); // atomic seq-cst
}

inline void MutatorBase::HandleCpuProfile()
{
    LOG_COMMON(FATAL) << "Unresolved fatal";
    UNREACHABLE_CC();
}

void MutatorBase::TransitionToCpuProfileExclusive()
{
    HandleCpuProfile();
    SetSafepointActive(false);
    ClearSuspensionFlag(SUSPENSION_FOR_CPU_PROFILE);
}

void PreRunManagedCode(Mutator* mutator, int layers, ThreadLocalData* threadData)
{
    if (UNLIKELY_CC(MutatorManager::Instance().StwTriggered())) {
        mutator->SetSuspensionFlag(Mutator::SuspensionType::SUSPENSION_FOR_STW);
        mutator->EnterSaferegion(false);
    }
    mutator->LeaveSaferegion();
    mutator->SetMutatorPhase(Heap::GetHeap().GetGCPhase());
}

} // namespace common
