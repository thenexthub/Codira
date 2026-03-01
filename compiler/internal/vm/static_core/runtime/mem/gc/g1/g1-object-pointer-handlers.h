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
#ifndef PANDA_RUNTIME_MEM_GC_G1_G1_OBJECT_POINTER_HANDLER_H
#define PANDA_RUNTIME_MEM_GC_G1_G1_OBJECT_POINTER_HANDLER_H

#include "libarkbase/mem/mem.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/object_accessor.h"
#include "runtime/mem/region_allocator.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/gc/g1/object_ref.h"

namespace ark::mem {

template <typename LanguageConfig>
class G1GC;

template <typename LanguageConfig>
class G1EvacuateRegionsWorkerState;

template <typename Handler, typename LanguageConfig>
class RemsetObjectPointerHandler {
public:
    RemsetObjectPointerHandler(Region *fromRegion, size_t regionSizeBits, const std::atomic_bool &deferCards,
                               const Handler &handler)
        : fromRemset_(fromRegion->GetRemSet()),
          regionSizeBits_(regionSizeBits),
          deferCards_(deferCards),
          handler_(handler)
    {
    }

    template <typename T>
    bool ProcessObjectPointer(ObjectHeader *fromObject, T *ref) const
    {
        ProcessObjectPointerInternal(fromObject, ref);
        // Atomic with relaxed order reason: memory order is not required
        return !deferCards_.load(std::memory_order_relaxed);
    }

private:
    template <typename T>
    void ProcessObjectPointerInternal(ObjectHeader *fromObject, T *ref) const
    {
        // fromRemset_ is not changed while handling one card
        ASSERT(AddrToRegion(ref)->GetRemSet() == fromRemset_);

        auto o = ObjectAccessor::LoadAtomic(ref);
        if (!ObjectAccessor::IsHeapObject<LanguageConfig::LANG_TYPE>(o)) {
            return;
        }

        auto *obj = ObjectAccessor::DecodeNotNull<LanguageConfig::LANG_TYPE>(o);
        if (ark::mem::IsSameRegion(ref, obj, regionSizeBits_)) {
            return;
        }

        ASSERT_PRINT(IsHeapSpace(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(obj)),
                     "Not suitable space for to_obj: " << obj);

        // don't need lock because only one thread changes remsets
        RemSet<>::AddRefWithAddr<false>(ref, obj);
        LOG(DEBUG, GC) << "fill rem set " << ref << " -> " << obj;
        handler_(fromObject, ToUintPtr(ref) - ToUintPtr(fromObject));
    }
    RemSetT *fromRemset_;
    size_t regionSizeBits_;
    const std::atomic_bool &deferCards_;
    const Handler &handler_;
};

template <class LanguageConfig, bool NEED_UPDATE_REMSET = false>
class EvacuationObjectPointerHandler {
public:
    using Ref = typename ObjectReference<LanguageConfig::LANG_TYPE>::Type;

    explicit EvacuationObjectPointerHandler(G1EvacuateRegionsWorkerState<LanguageConfig> *workerState)
        : gc_(workerState->GetGC()), workerState_(workerState)
    {
    }

    bool ProcessObjectPointer(ObjectHeader *fromObject, Ref ref) const
    {
        ProcessObjectPointerHelper(fromObject, ref);
        return true;
    }

private:
    void ProcessObjectPointerHelper([[maybe_unused]] ObjectHeader *fromObject, Ref ref) const
    {
        auto o = ObjectAccessor::Load(ref);
        if (!ObjectAccessor::IsHeapObject<LanguageConfig::LANG_TYPE>(o)) {
            return;
        }
        auto *obj = ObjectAccessor::DecodeNotNull<LanguageConfig::LANG_TYPE>(o);
        if (gc_->InGCSweepRange(obj)) {
            LOG(DEBUG, GC) << "Add object to stack: " << GetDebugInfoAboutObject(obj)
                           << " accessed from object: " << fromObject;
            workerState_->PushToQueue(ref);
        } else if (!workerState_->IsSameRegion(ref, obj)) {
            if constexpr (NEED_UPDATE_REMSET) {
                // Need to update Remset by current thread
                RemSet<>::AddRefWithAddr<false>(fromObject, ToUintPtr(ref) - ToUintPtr(fromObject), obj);
            } else {
                // Add a card so that UpdateRemsetWorker thread updates Remset in the future.
                workerState_->EnqueueCard(ref);
            }
        }
    }

    G1GC<LanguageConfig> *gc_;
    G1EvacuateRegionsWorkerState<LanguageConfig> *workerState_;
};
}  // namespace ark::mem

#endif
