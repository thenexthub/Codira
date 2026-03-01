/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_RUNTIME_MEM_GC_G1_REF_UPDATER_H
#define PANDA_RUNTIME_MEM_GC_G1_REF_UPDATER_H

#include "runtime/include/object_header.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "runtime/mem/region_allocator.h"
#include "runtime/mem/gc/card_table-inl.h"

namespace ark::mem {

template <class LanguageConfig>
class BaseRefUpdater {
public:
    bool operator()(ObjectHeader *object, ObjectHeader *ref, uint32_t offset,
                    [[maybe_unused]] bool isVolatile = false) const
    {
        auto *forwarded = UpdateRefToMovedObject(object, ref, offset);
        Process(object, offset, forwarded);
        return true;
    }

    DEFAULT_COPY_SEMANTIC(BaseRefUpdater);
    DEFAULT_MOVE_SEMANTIC(BaseRefUpdater);
    virtual ~BaseRefUpdater() = default;

protected:
    explicit BaseRefUpdater(uint32_t regionSizeBits) : regionSizeBits_(regionSizeBits) {}

    virtual void Process(ObjectHeader *object, size_t offset, ObjectHeader *ref) const = 0;

    bool IsSameRegion(ObjectHeader *o1, ObjectHeader *o2) const
    {
        return ark::mem::IsSameRegion(o1, o2, regionSizeBits_);
    }

private:
    ObjectHeader *UpdateRefToMovedObject(ObjectHeader *object, ObjectHeader *ref, uint32_t offset) const;

    uint32_t regionSizeBits_;
};

template <class LanguageConfig, bool NEED_LOCK = false>
class UpdateRemsetRefUpdater : public BaseRefUpdater<LanguageConfig> {
public:
    explicit UpdateRemsetRefUpdater(uint32_t regionSizeBits) : BaseRefUpdater<LanguageConfig>(regionSizeBits) {}

protected:
    void Process(ObjectHeader *object, size_t offset, ObjectHeader *ref) const override;
};

template <class LanguageConfig>
class EnqueueRemsetRefUpdater : public BaseRefUpdater<LanguageConfig> {
public:
    EnqueueRemsetRefUpdater(CardTable *cardTable, GCG1BarrierSet::ThreadLocalCardQueues *updatedRefsQueue,
                            uint32_t regionSizeBits)
        : BaseRefUpdater<LanguageConfig>(regionSizeBits), cardTable_(cardTable), updatedRefsQueue_(updatedRefsQueue)
    {
    }

protected:
    void Process(ObjectHeader *object, size_t offset, ObjectHeader *ref) const override
    {
        if (!this->IsSameRegion(object, ref)) {
            auto *card = cardTable_->GetCardPtr(ToUintPtr(object) + offset);
            auto cardStatus = card->GetStatus();
            if (!CardTable::Card::IsYoung(cardStatus) && !CardTable::Card::IsMarked(cardStatus)) {
                card->Mark();
                updatedRefsQueue_->push_back(card);
            }
        }
    }

private:
    CardTable *cardTable_;
    GCG1BarrierSet::ThreadLocalCardQueues *updatedRefsQueue_;
};
}  // namespace ark::mem

#endif
