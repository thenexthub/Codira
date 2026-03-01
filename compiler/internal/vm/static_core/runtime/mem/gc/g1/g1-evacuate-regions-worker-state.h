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
#ifndef PANDA_RUNTIME_MEM_GC_G1_G1_EVACUATE_REGIONS_WORKER_STATE_H
#define PANDA_RUNTIME_MEM_GC_G1_G1_EVACUATE_REGIONS_WORKER_STATE_H

#include "runtime/mem/lock_config_helper.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/mark_word.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/gc/g1/g1-object-pointer-handlers.h"
#include "runtime/mem/gc/g1/gc_evacuate_regions_task_stack.h"

namespace ark::mem {
template <typename LanguageConfig>
class G1GC;

template <MTModeT MODE>
class ObjectAllocatorG1;

class CardTable;

/// Data and methods to evacuate live objects by one worker.
template <typename LanguageConfig>
class G1EvacuateRegionsWorkerState {
public:
    using Ref = typename ObjectReference<LanguageConfig::LANG_TYPE>::Type;

    G1EvacuateRegionsWorkerState(G1GC<LanguageConfig> *gc, GCEvacuateRegionsTaskStack<Ref> *refStack);
    ~G1EvacuateRegionsWorkerState();

    NO_COPY_SEMANTIC(G1EvacuateRegionsWorkerState);
    NO_MOVE_SEMANTIC(G1EvacuateRegionsWorkerState);

    void ProcessRef(Ref p);

    void PushToQueue(Ref p)
    {
        refStack_->PushToStack(p);
    }

    /// Evacuate single live object, traverse its fields and enqueue found references to the queue_
    ObjectHeader *Evacuate(ObjectHeader *obj, MarkWord markWord);

    G1GC<LanguageConfig> *GetGC()
    {
        return gc_;
    }

    size_t GetCopiedBytesYoung() const
    {
        return copiedBytesYoung_;
    }

    size_t GetCopiedObjectsYoung() const
    {
        return copiedObjectsYoung_;
    }

    size_t GetCopiedBytesOld() const
    {
        return copiedBytesOld_;
    }

    size_t GetCopiedObjectsOld() const
    {
        return copiedObjectsOld_;
    }

    size_t GetLiveBytes() const
    {
        return liveBytes_;
    }

    Region *GetRegionTo()
    {
        return regionTo_;
    }

    bool IsSameRegion(void *ref, void *obj) const
    {
        return ark::mem::IsSameRegion(ref, obj, regionSizeBits_);
    }

    void EnqueueCard(void *p)
    {
        auto *card = cardTable_->GetCardPtr(ToUintPtr(p));
        if (card != latestCard_) {
            cardQueue_.insert(card);
            latestCard_ = card;
        }
    }

    template <typename Visitor>
    void VisitCards(Visitor visitor)
    {
        for (auto *card : cardQueue_) {
            visitor(card);
        }
    }

    void EvacuateLiveObjects();
    void PrintObjectEvents(const CollectionSet &collectionSet);
    void EvacuateNonHeapRoots();
    size_t ScanRemset(const RemSet<> &remset);

private:
    void IterateRefsInMemRange(const MemRange &memRange, Region *region);

    Region *GetNextRegion()
    {
        auto *region = objectAllocator_->template PopFromOldRegionQueue<true>();
        if (region != nullptr) {
            return region;
        }
        return CreateNewRegion();
    }

    Region *CreateNewRegion()
    {
        return objectAllocator_->template CreateAndSetUpNewOldRegion<true>();
    }

    ObjectHeader *SetForwardAddress(ObjectHeader *src, ObjectHeader *dst, MarkWord markWord);

    G1GC<LanguageConfig> *gc_;
    ObjectAllocatorG1<LanguageConfig::MT_MODE> *objectAllocator_;
    CardTable *cardTable_;
    size_t regionSizeBits_;
    GCAdaptiveStack<Ref> *refStack_;
    EvacuationObjectPointerHandler<LanguageConfig> evacuationObjectPointerHandler_;
    EvacuationObjectPointerHandler<LanguageConfig, true> evacuationObjectPointerWithUpdRemsetHandler_;

    PandaSet<CardTable::CardPtr> cardQueue_;
    CardTable::CardPtr latestCard_ {nullptr};

    size_t copiedBytesYoung_ {0};
    size_t copiedObjectsYoung_ {0};
    size_t copiedBytesOld_ {0};
    size_t copiedObjectsOld_ {0};
    size_t liveBytes_ {0};
    Region *regionTo_ {nullptr};
};
}  // namespace ark::mem
#endif
