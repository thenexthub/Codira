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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ITEM_POOL_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ITEM_POOL_H_

#include <atomic>

#include "libarkbase/macros.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/utils/math_helpers.h"

namespace ark::ets::interop::js::ets_proxy {

namespace testing {
class ItemsPoolTest;
}  // namespace testing

template <typename Item, uint32_t NR_INDEX_BITS>
class ItemsPool {
    union PaddedItem {
        Item item;
        PaddedItem *next;
        std::atomic<size_t> size;  // For the 0 index
        std::array<uint8_t, helpers::math::GetPowerOfTwoValue32(sizeof(Item))> aligned;

        PaddedItem()  // NOLINT(cppcoreguidelines-pro-type-member-init)
        {
            new (&item) Item();
        }
        ~PaddedItem()
        {
            item.~Item();
        }
        NO_COPY_SEMANTIC(PaddedItem);
        NO_MOVE_SEMANTIC(PaddedItem);
    };

    static constexpr size_t MAX_INDEX = 1ULL << NR_INDEX_BITS;
    static constexpr size_t PADDED_ITEM_SIZE = sizeof(PaddedItem);

    static PaddedItem *GetPaddedItem(Item *item)
    {
        ASSERT(ToUintPtr(item) % PADDED_ITEM_SIZE == 0);
        return reinterpret_cast<PaddedItem *>(item);
    }

public:
    static constexpr size_t MAX_POOL_SIZE = (static_cast<size_t>(1U) << NR_INDEX_BITS) * PADDED_ITEM_SIZE;

    ItemsPool(void *data, size_t size)
        : data_(reinterpret_cast<PaddedItem *>(data)),
          dataEnd_(reinterpret_cast<PaddedItem *>(ToUintPtr(data_) + size)),
          currentPos_(reinterpret_cast<PaddedItem *>(data))
    {
        ASSERT(data != nullptr);
        ASSERT(size > 0U);
        ASSERT(size % PADDED_ITEM_SIZE == 0);
        ASSERT(MaxSize() <= MAX_POOL_SIZE);
        // Always use 0 index as special internal space for count of allocated references
        ++currentPos_;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ASSERT(Capacity() == 1);
        // Atomic with relaxed order reason: data race with size loading with no synchronization or ordering constraints
        data_->size.store(0U, std::memory_order_relaxed);
    }
    ~ItemsPool() = default;

    Item *AllocItem()
    {
        if (freeList_ != nullptr) {
            PaddedItem *newItem = freeList_;
            freeList_ = freeList_->next;
            // Atomic with relaxed order reason: data race with size loading with no synchronization or ordering
            // constraints
            data_->size.fetch_add(1U, std::memory_order_relaxed);
            return &(new (newItem) PaddedItem())->item;
        }

        if (UNLIKELY(currentPos_ >= dataEnd_)) {
            // Out of memory
            return nullptr;
        }

        PaddedItem *newItem = currentPos_;
        ++currentPos_;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        // Atomic with relaxed order reason: data race with size loading with no synchronization or ordering constraints
        data_->size.fetch_add(1U, std::memory_order_relaxed);
        return &(new (newItem) PaddedItem())->item;
    }

    void FreeItem(Item *item)
    {
        ASSERT(item != nullptr);
        PaddedItem *paddedItem = GetPaddedItem(item);
        ASSERT_PRINT(paddedItem != data_, "0 index is reserved by the item pool, but try to free it");
        paddedItem->~PaddedItem();
        paddedItem->next = freeList_;
        freeList_ = paddedItem;
        // Atomic with relaxed order reason: data race with size loading with no synchronization or ordering constraints
        data_->size.fetch_sub(1U, std::memory_order_relaxed);
    }

    //  This method only checks the validity of the item in the allocated interval
    //  This method does not check whether the item has been allocated or not
    bool IsValidItem(const Item *item) const
    {
        if (UNLIKELY(!IsAligned<alignof(Item)>(ToUintPtr(item)))) {
            return false;
        }
        auto addr = ToUintPtr(item);
        return ToUintPtr(data_) < addr && addr < ToUintPtr(currentPos_);
    }

    inline uint32_t GetIndexByItem(Item *item)
    {
        ASSERT(IsValidItem(item));
        ASSERT(ToUintPtr(item) % PADDED_ITEM_SIZE == 0);

        PaddedItem *paddedItem = GetPaddedItem(item);
        return paddedItem - data_;
    }

    ALWAYS_INLINE size_t Size() const
    {
        // Atomic with relaxed order reason: data race with size with no synchronization or ordering constraints
        return data_->size.load(std::memory_order_relaxed);
    }

    ALWAYS_INLINE size_t Capacity() const
    {
        return currentPos_ - data_;
    }

    ALWAYS_INLINE size_t MaxSize() const
    {
        // 0 index is reserved by the item pool
        return (dataEnd_ - data_) - 1U;
    }

    ALWAYS_INLINE Item *GetItemByIndex(uint32_t idx) const
    {
        ASSERT(idx > 0U);
        ASSERT_PRINT(idx < Capacity(), "index: " << idx << ", capacity: " << Capacity());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic,cppcoreguidelines-pro-type-union-access)
        return &data_[idx].item;
    }

    NO_COPY_SEMANTIC(ItemsPool);
    NO_MOVE_SEMANTIC(ItemsPool);

private:
    PaddedItem *const data_ {};
    PaddedItem *const dataEnd_ {};
    PaddedItem *currentPos_ {};
    PaddedItem *freeList_ {};

    friend testing::ItemsPoolTest;
};

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ITEM_POOL_H_
