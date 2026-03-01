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

#ifndef COMMON_COMPONENTS_MUTATOR_MUTATOR_H
#define COMMON_COMPONENTS_MUTATOR_MUTATOR_H

#include <climits>

#include "common_components/heap/allocator/allocator.h"
#include "common_components/heap/collector/gc_infos.h"
#include "common_components/mutator/thread_local.h"
#include "common_components/mutator/satb_buffer.h"
#include "common_interfaces/thread/mutator_base.h"
#include "common_interfaces/thread/thread_holder.h"

namespace common {
class Mutator;
using FlipFunction = std::function<void(Mutator&)>;
class Mutator {
public:
    using SuspensionType = MutatorBase::SuspensionType;
    using GCPhaseTransitionState = MutatorBase::GCPhaseTransitionState;
    using CpuProfileState = MutatorBase::CpuProfileState;
    using SaferegionState = MutatorBase::SaferegionState;

    // Called when a mutator starts and finishes, respectively.
    void Init()
    {
        mutatorBase_.Init();
        mutatorBase_.mutator_ = reinterpret_cast<void*>(this);
        holder_ = new ThreadHolder(&mutatorBase_);
    }

    ~Mutator()
    {
        tid_ = 0;
        if (satbNode_ != nullptr) {
            SatbBuffer::Instance().RetireNode(satbNode_);
            satbNode_ = nullptr;
        }
        delete holder_;
    }

    static Mutator* NewMutator()
    {
        Mutator* mutator = new (std::nothrow) Mutator();
        LOGF_CHECK(mutator != nullptr) << "new Mutator failed";
        mutator->Init();
        return mutator;
    }

    void ResetMutator()
    {
        rawObject_.object = nullptr;
        if (satbNode_ != nullptr) {
            SatbBuffer::Instance().RetireNode(satbNode_);
            satbNode_ = nullptr;
        }
    }

    static Mutator* GetMutator() noexcept;

    void InitTid()
    {
        tid_ = ThreadLocal::GetThreadLocalData()->tid;
        if (tid_ == 0) {
            tid_ = static_cast<uint32_t>(GetTid());
            ThreadLocal::GetThreadLocalData()->tid = tid_;
        }
    }
    void SetArkthreadPtr(void* threadPtr) { this->thread_ = threadPtr;}
    void SetEcmaVMPtr(void* ecmaVMPtr) { this->ecmavm_ = ecmaVMPtr;}
    uint32_t GetTid() const { return tid_; }
    void* GetArkthreadPtr() const {return thread_;}
    void* GetEcmaVMPtr() const {return ecmavm_;}

    __attribute__((always_inline)) inline void SetSafepointActive(bool state)
    {
        mutatorBase_.SetSafepointActive(state);
    }

    // Sets saferegion state of this mutator.
    __attribute__((always_inline)) inline void SetInSaferegion(SaferegionState state)
    {
        mutatorBase_.SetInSaferegion(state);
    }

    // Returns true if this mutator is in saferegion, otherwise false.
    __attribute__((always_inline)) inline bool InSaferegion() const
    {
        return mutatorBase_.InSaferegion();
    }

    // This interface can only be invoked in current thread environment.
    __attribute__((always_inline)) void UpdateUnwindContext()
    {
        // void* ip = __builtin_return_address(0)
        // void* fa = __builtin_frame_address(0)
        // uwContext.frameInfo.mFrame.SetIP(static_cast<uint32_t*>(ip))
        // uwContext.frameInfo.mFrame.SetFA(static_cast<FrameAddress*>(fa)->callerFrameAddress)
    }

    // Force current mutator enter saferegion, internal use only.
    __attribute__((always_inline)) inline void DoEnterSaferegion();
    // Force current mutator leave saferegion, internal use only.
    __attribute__((always_inline)) inline void DoLeaveSaferegion()
    {
        mutatorBase_.DoLeaveSaferegion();
    }

    // If current mutator is not in saferegion, enter and return true
    // If current mutator has been in saferegion, return false
    __attribute__((always_inline)) inline bool EnterSaferegion(bool updateUnwindContext) noexcept;
    // If current mutator is in saferegion, leave and return true
    // If current mutator has left saferegion, return false
    __attribute__((always_inline)) inline bool LeaveSaferegion() noexcept;

    __attribute__((always_inline)) inline bool FinishedTransition() const
    {
        return mutatorBase_.FinishedTransition();
    }

    __attribute__((always_inline)) inline bool FinishedCpuProfile() const
    {
        return mutatorBase_.FinishedCpuProfile();
    }

    __attribute__((always_inline)) inline void SetCpuProfileState(CpuProfileState state)
    {
        mutatorBase_.SetCpuProfileState(state);
    }

    __attribute__((always_inline)) inline void SetSuspensionFlag(SuspensionType flag)
    {
        mutatorBase_.SetSuspensionFlag(flag);
    }

    __attribute__((always_inline)) inline void ClearSuspensionFlag(SuspensionType flag)
    {
        mutatorBase_.ClearSuspensionFlag(flag);
    }

    __attribute__((always_inline)) inline uint32_t GetSuspensionFlag() const
    {
        return mutatorBase_.GetSuspensionFlag();
    }

    __attribute__((always_inline)) inline bool HasSuspensionRequest(SuspensionType flag) const
    {
        return mutatorBase_.HasSuspensionRequest(flag);
    }

    void SetFinalizeRequest()
    {
        mutatorBase_.SetFinalizeRequest();
    }

    // Ensure that mutator phase is changed only once by mutator itself or GC
    __attribute__((always_inline)) inline bool TransitionGCPhase(bool bySelf);

    __attribute__((always_inline)) inline bool TransitionToCpuProfile(bool bySelf);

    __attribute__((always_inline)) inline void SetMutatorPhase(const GCPhase newPhase)
    {
        mutatorBase_.SetMutatorPhase(newPhase);
    }

    __attribute__((always_inline)) inline GCPhase GetMutatorPhase() const
    {
        return mutatorBase_.GetMutatorPhase();
    }

    void VisitMutatorRoots(const RootVisitor& visitor)
    {
        LOG_COMMON(FATAL) << "Unresolved fatal";
        UNREACHABLE_CC();
    }

    void DumpMutator() const
    {
        LOG_COMMON(ERROR) << "mutator " << this << ": inSaferegion " <<
            mutatorBase_.inSaferegion_.load(std::memory_order_relaxed) << ", tid " << tid_ <<
            ", observerCnt " << mutatorBase_.observerCnt_.load() << ", gc phase: " <<
            mutatorBase_.mutatorPhase_.load() << ", suspension request "<< mutatorBase_.suspensionFlag_.load();
    }

    // Init after fork.
    void InitAfterFork()
    {
        // tid changed after fork, so we re-initialize it.
        InitTid();
    }

    void SetFlipFunction(FlipFunction *flipFunction)
    {
        flipFunction_ = flipFunction;
    }

    bool TryRunFlipFunction()
    {
        while (true) {
            uint32_t oldFlag = mutatorBase_.GetSuspensionFlag();
            if ((oldFlag & SuspensionType::SUSPENSION_FOR_PENDING_CALLBACK) != 0) {
                uint32_t newFlag =
                    MutatorBase::ConstructSuspensionFlag(oldFlag, SuspensionType::SUSPENSION_FOR_PENDING_CALLBACK,
                                                         SuspensionType::SUSPENSION_FOR_RUNNING_CALLBACK);
                if (mutatorBase_.CASSetSuspensionFlag(oldFlag, newFlag)) {
                    DCHECK_CC(flipFunction_);
                    (*flipFunction_)(*this);
                    flipFunction_ = nullptr;
                    std::unique_lock<std::mutex> lock(flipFunctionMtx_);
                    mutatorBase_.ClearSuspensionFlag(SuspensionType::SUSPENSION_FOR_RUNNING_CALLBACK);
                    flipFunctionCV_.notify_all();
                    return true;
                }
            } else {
                return false;
            }
        }
    }

    void WaitFlipFunctionFinish()
    {
        while (true) {
            std::unique_lock<std::mutex> lock(flipFunctionMtx_);
            if (HasSuspensionRequest(SuspensionType::SUSPENSION_FOR_RUNNING_CALLBACK)) {
                flipFunctionCV_.wait(lock);
            } else {
                return;
            }
        }
    }

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    void PushFrameInfoForMarking(const GCInfoNode& frameGCInfo) { gcInfos_.PushFrameInfoForMarking(frameGCInfo); }

    void PushFrameInfoForMarking(const GCInfoNode&& frameGCInfo) { gcInfos_.PushFrameInfoForMarking(frameGCInfo); }

    void PushFrameInfoForFix(const GCInfoNodeForFix& frameGCInfo) { gcInfos_.PushFrameInfoForFix(frameGCInfo); }

    void PushFrameInfoForFix(const GCInfoNodeForFix&& frameGCInfo) { gcInfos_.PushFrameInfoForFix(frameGCInfo); }

    void DumpGCInfos() const
    {
        DLOG(ENUM, "dump mutator gc info thread id: %d", tid);
        gcInfos_.DumpGCInfos();
    }
#endif

    NO_INLINE_CC void RememberObjectInSatbBuffer(const BaseObject* obj) { RememberObjectImpl(obj); }

    const SatbBuffer::TreapNode* GetSatbBufferNode() const { return satbNode_; }

    void ClearSatbBufferNode()
    {
        if (satbNode_ == nullptr) {
            return;
        }
        satbNode_->Clear();
    }

#ifdef _WIN64
    uint32_t GetStackGrowFrameSize() { return stackGrowFrameSize_; }
    void SetStackGrowFrameSize(uint32_t sgfs) { stackGrowFrameSize_ = sgfs; }
#endif

    void PushRawObject(BaseObject* obj) { rawObject_.object = obj; }

    BaseObject* PopRawObject()
    {
        BaseObject* obj = rawObject_.object;
        rawObject_.object = nullptr;
        return obj;
    }
    void MutatorLock()
    {
        mutatorBase_.MutatorBaseLock();
    }

    void MutatorUnlock()
    {
        mutatorBase_.MutatorBaseUnlock();
    }

    // temporary impl
    void *GetMutatorBasePtr()
    {
        return &mutatorBase_;
    }

    ThreadHolder *GetThreadHolder()
    {
        return holder_;
    }

protected:
    // for exception ref
    void VisitRawObjects(const RootVisitor& func);
    void CreateCurrentGCInfo();

private:
    void RememberObjectImpl(const BaseObject* obj)
    {
        if (LIKELY_CC(Heap::IsHeapAddress(obj))) {
            if (SatbBuffer::ShouldEnqueue(obj)) {
                SatbBuffer::Instance().EnsureGoodNode(satbNode_);
                satbNode_->Push(obj);
            }
        }
    }
    // Indicate the current mutator phase and use which barrier in concurrent gc
    // ATTENTION: THE LAYOUT FOR GCPHASE MUST NOT BE CHANGED!
    // currently store struct instead of pointer to keep GCPHASE offset to this is 0
    MutatorBase mutatorBase_ {};
    // thread id
    uint32_t tid_ = 0;
    // thread ptr
    void* thread_;
    // ecmavm
    void* ecmavm_ = nullptr;
#ifndef __WIN64
    void* unuse_ = nullptr; // reusable placeholder
#endif // __WIN64

#ifdef _WIN64
    uint32_t stackGrowFrameSize_ = 0;
#endif

    ObjectRef rawObject_{ nullptr };
    std::mutex flipFunctionMtx_;
    std::condition_variable flipFunctionCV_;
    FlipFunction* flipFunction_ {nullptr};
    SatbBuffer::TreapNode* satbNode_ = nullptr;
#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    GCInfos gcInfos_;
#endif

    ThreadHolder *holder_;

    friend MutatorBase;
    friend ThreadHolder;
};

// This function is mainly used to initialize the context of mutator.
// Ensured that updated fa is the caller layer of the managed function to be called.
void PreRunManagedCode(Mutator* mutator, int layers, ThreadLocalData* threadData);

ThreadLocalData *GetThreadLocalData();
} // namespace common

#endif // COMMON_COMPONENTS_MUTATOR_MUTATOR_H
