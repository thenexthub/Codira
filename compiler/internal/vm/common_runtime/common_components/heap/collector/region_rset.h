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

#ifndef COMMON_COMPONENTS_HEAP_COLLECTOR_REGION_RSET_H
#define COMMON_COMPONENTS_HEAP_COLLECTOR_REGION_RSET_H

#include <atomic>
#include <errno.h>

namespace common {
class RegionRSet {
public:
    static constexpr size_t CARD_SIZE = 512;
    using CardElement = uint64_t;
    static_assert(std::atomic<CardElement>::is_always_lock_free);
    static constexpr size_t CARD_TABLE_DATA_OFFSET = AlignUp<size_t>(sizeof(size_t), sizeof(CardElement));

    static RegionRSet *CreateRegionRSet(size_t regionSize)
    {
        CHECK_CC(regionSize % CARD_SIZE == 0);
        size_t cardCnt = regionSize / CARD_SIZE;
        size_t cardSize = cardCnt * sizeof(CardElement);
        void *ptr = malloc(CARD_TABLE_DATA_OFFSET + cardSize);
        if (ptr == nullptr) {
            LOG_COMMON(FATAL) << "malloc failed, regionSize=" << regionSize << ", cardSize=" << cardSize
                              << ", errnor=" << errno;
            UNREACHABLE_CC();
        }
        RegionRSet *rset = new (ptr) RegionRSet(cardCnt);
        rset->ClearCardTable();
        return rset;
    }

    static void DestroyRegionRSet(RegionRSet *rset)
    {
        free(rset);
    }

    bool MarkCardTable(size_t offset)
    {
        size_t cardIdx = (offset / kMarkedBytesPerBit) / kBitsPerWord;
        size_t headMaskBitStart = (offset / kMarkedBytesPerBit) % kBitsPerWord;
        CardElement headMaskBits = static_cast<CardElement>(1ULL << headMaskBitStart);
        std::atomic<CardElement> *card = reinterpret_cast<std::atomic<CardElement> *>(&GetCardTable()[cardIdx]);
        bool isMarked = ((card->load(std::memory_order_relaxed) & headMaskBits) != 0);
        if (!isMarked) {
            CardElement prev = card->fetch_or(headMaskBits, std::memory_order_relaxed);
            isMarked = ((prev & headMaskBits) != 0);
            return isMarked;
        }
        return true;
    }

    void ClearCardTable()
    {
        LOGF_CHECK(memset_s(GetCardTable(), cardCnt_ * sizeof(CardElement), 0, cardCnt_ * sizeof(CardElement)) == EOK)
            << "memset_s fail, cardCnt=" << cardCnt_;
    }

    void VisitAllMarkedCardBefore(const std::function<void(BaseObject*)>& func,
                                  HeapAddress regionStart, HeapAddress end)
    {
        for (size_t i = 0; i < cardCnt_; i++) {
            CardElement card = GetCardTable()[i];
            size_t index = kBitsPerWord;
            while (card != 0) {
                index = static_cast<size_t>(__builtin_ctzll(card));
                DCHECK_CC(index < kBitsPerWord);
                HeapAddress offset = static_cast<HeapAddress>((i * kBitsPerWord) * kBitsPerByte + index * kBitsPerByte);
                HeapAddress obj = regionStart + offset;
                if (obj >= end) {
                    return;
                }
                func(reinterpret_cast<BaseObject*>(obj));
                card &= ~(static_cast<CardElement>(1ULL << index));
            }
        }
    }
private:
    explicit RegionRSet(size_t cardCnt) : cardCnt_(cardCnt) {}
    ~RegionRSet() = default;

    CardElement *GetCardTable() const
    {
        return reinterpret_cast<CardElement *>(reinterpret_cast<uintptr_t>(this) + CARD_TABLE_DATA_OFFSET);
    }

    size_t cardCnt_ {0};
};

static_assert(RegionRSet::CARD_TABLE_DATA_OFFSET == AlignUp<size_t>(sizeof(RegionRSet),
                                                                    sizeof(RegionRSet::CardElement)));
}
#endif // COMMON_COMPONENTS_HEAP_COLLECTOR_REGION_RSET_H
