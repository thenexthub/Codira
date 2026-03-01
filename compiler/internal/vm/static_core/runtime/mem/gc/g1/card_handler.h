/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_GC_G1_CARD_HANDLER_H
#define PANDA_RUNTIME_MEM_GC_G1_CARD_HANDLER_H

#include "runtime/include/object_header.h"
#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/mem/gc/g1/g1-object-pointer-handlers.h"
#include "runtime/mem/object-references-iterator-inl.h"

namespace ark::mem {
class Region;

template <typename Handler, typename LanguageConfig>
class CardHandler {
public:
    explicit CardHandler(CardTable *cardTable, size_t regionSizeBits, const std::atomic_bool &deferCards,
                         const Handler &handler)
        : cardTable_(cardTable), regionSizeBits_(regionSizeBits), deferCards_(deferCards), handler_(handler)
    {
    }

    bool Handle(CardTable::CardPtr cardPtr);

private:
    CardTable *cardTable_;
    size_t regionSizeBits_;
    const std::atomic_bool &deferCards_;
    const Handler &handler_;
};

template <typename Handler, typename LanguageConfig>
class RegionRemsetBuilder {
public:
    RegionRemsetBuilder(Region *fromRegion, void *startAddress, void *endAddress, size_t regionSizeBits,
                        const std::atomic_bool &deferCards, const Handler &handler, bool *result)
        : objectPointerHandler_(fromRegion, regionSizeBits, deferCards, handler),
          startAddress_(startAddress),
          endAddress_(endAddress),
          result_(result)
    {
    }

    bool operator()(void *mem) const
    {
        auto *obj = static_cast<ObjectHeader *>(mem);
        if (obj->ClassAddr<BaseClass>() != nullptr) {
            // Class may be null when we are visiting a card and at the same time a new non-movable
            // object is allocated in the memory region covered by the card.
            *result_ = ObjectIterator<LanguageConfig::LANG_TYPE>::template Iterate<true>(obj, &objectPointerHandler_,
                                                                                         startAddress_, endAddress_);
            return *result_;
        }
        return true;
    }

private:
    RemsetObjectPointerHandler<Handler, LanguageConfig> objectPointerHandler_;
    void *startAddress_;
    void *endAddress_;
    bool *result_;
};

template <typename Handler, typename LanguageConfig>
bool CardHandler<Handler, LanguageConfig>::Handle(CardTable::CardPtr cardPtr)
{
    bool result = true;
    auto *startAddress = ToVoidPtr(cardTable_->GetCardStartAddress(cardPtr));
    LOG(DEBUG, GC) << "HandleCard card: " << cardTable_->GetMemoryRange(cardPtr);
    ASSERT_PRINT(IsHeapSpace(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(startAddress)),
                 "Invalid space type for the " << startAddress);
    auto *region = AddrToRegion(startAddress);
    ASSERT(region != nullptr);
    ASSERT_PRINT(region->GetLiveBitmap() != nullptr, "Region " << region << " GetLiveBitmap() == nullptr");
    auto *endAddress = ToVoidPtr(cardTable_->GetCardEndAddress(cardPtr));
    RegionRemsetBuilder<Handler, LanguageConfig> remsetBuilder(region, startAddress, endAddress, regionSizeBits_,
                                                               deferCards_, handler_, &result);
    if (region->HasFlag(RegionFlag::IS_LARGE_OBJECT)) {
        region->GetLiveBitmap()->CallForMarkedChunkInHumongousRegion<true>(ToVoidPtr(region->Begin()), remsetBuilder);
    } else {
        region->GetLiveBitmap()->IterateOverMarkedChunkInRangeInterruptible<true>(startAddress, endAddress,
                                                                                  remsetBuilder);
    }
    return result;
}
}  // namespace ark::mem

#endif
