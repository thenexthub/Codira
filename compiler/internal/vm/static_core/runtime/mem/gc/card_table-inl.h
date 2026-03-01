/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef RUNTIME_MEM_GC_CARD_TABLE_INL_H
#define RUNTIME_MEM_GC_CARD_TABLE_INL_H

#include "runtime/mem/gc/card_table.h"
#include "runtime/include/mem/panda_containers.h"

#include <atomic>

namespace ark::mem {

inline uint8_t CardTable::Card::GetCard() const
{
    // Atomic with relaxed order reason: data race with value_ with no synchronization or ordering constraints imposed
    // on other reads or writes
    return value_.load(std::memory_order_relaxed);
}

inline void CardTable::Card::SetCard(uint8_t newVal)
{
    // Atomic with relaxed order reason: data race with value_ with no synchronization or ordering constraints imposed
    // on other reads or writes
    value_.store(newVal, std::memory_order_relaxed);
}

inline CardTable::Card::Status CardTable::Card::GetStatus() const
{
    // Atomic with relaxed order reason: data race with value_ with no synchronization or ordering constraints imposed
    // on other reads or writes
    return value_.load(std::memory_order_relaxed) & STATUS_MASK;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline void CardTable::FillRanges(PandaVector<MemRange> *ranges, const Card *startCard, const Card *endCard)
{
    constexpr size_t MIN_RANGE = 32;
    constexpr size_t MAX_CARDS_COUNT = 1000;  // How many cards we can process at once
    static std::array<char, MAX_CARDS_COUNT> zeroArray {};

    if (static_cast<size_t>(endCard - startCard) < MIN_RANGE) {
        for (auto cardPtr = startCard; cardPtr <= endCard; cardPtr++) {
            if (cardPtr->IsMarked()) {
                ranges->emplace_back(minAddress_ + (cardPtr - cards_) * CARD_SIZE,
                                     minAddress_ + (cardPtr - cards_ + 1) * CARD_SIZE - 1);
            }
        }
    } else {
        size_t diff = endCard - startCard + 1;
        size_t splitSize = std::min(diff / 2, MAX_CARDS_COUNT);  // divide 2 to get smaller split_size
        if (memcmp(startCard, &zeroArray, splitSize) != 0) {
            FillRanges(ranges, startCard, ToNativePtr<Card>(ToUintPtr(startCard) + splitSize - 1));
        }
        // NOLINTNEXTLINE(bugprone-branch-clone)
        if (diff - splitSize > MAX_CARDS_COUNT) {
            FillRanges(ranges, ToNativePtr<Card>(ToUintPtr(startCard) + splitSize), endCard);
        } else if (memcmp(ToNativePtr<Card>(ToUintPtr(startCard) + splitSize), &zeroArray, diff - splitSize) != 0) {
            FillRanges(ranges, ToNativePtr<Card>(ToUintPtr(startCard) + splitSize), endCard);
        }
    }
}

// Make sure we can treat size_t as lockfree atomic
static_assert(std::atomic_size_t::is_always_lock_free);
static_assert(sizeof(std::atomic_size_t) == sizeof(size_t));

template <typename CardVisitor>
void CardTable::VisitMarked(CardVisitor cardVisitor, uint32_t processedFlag)
{
    bool visitMarked = processedFlag & CardTableProcessedFlag::VISIT_MARKED;
    bool visitProcessed = processedFlag & CardTableProcessedFlag::VISIT_PROCESSED;
    bool setProcessed = processedFlag & CardTableProcessedFlag::SET_PROCESSED;
    auto *card = cards_;
    auto *cardEnd = cards_ + (cardsCount_ / CHUNK_CARD_NUM) * CHUNK_CARD_NUM;
    while (card < cardEnd) {
        // NB! In general wide load/short store on overlapping memory of different address are allowed to be reordered
        // This optimization currently is allowed since additional VisitMarked is called after concurrent mark with
        // global Mutator lock held, so all previous managed thread's writes should be visible by GC thread
        // Atomic with relaxed order reason: data race with card with no synchronization or ordering constraints imposed
        // on other reads or writes
        if (LIKELY((reinterpret_cast<std::atomic_size_t *>(card))->load(std::memory_order_relaxed) == 0)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            card += CHUNK_CARD_NUM;
            continue;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *chunkEnd = card + CHUNK_CARD_NUM;
        while (card < chunkEnd) {
            auto cardStatus = card->GetStatus();
            if (!(visitMarked && Card::IsMarked(cardStatus)) && !(visitProcessed && Card::IsProcessed(cardStatus))) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                ++card;
                continue;
            }

            if (setProcessed) {
                card->SetProcessed();
            }
            cardVisitor(GetMemoryRange(card));
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ++card;
        }
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (; card < cards_ + cardsCount_; ++card) {
        auto cardStatus = card->GetStatus();
        if ((visitMarked && Card::IsMarked(cardStatus)) || (visitProcessed && Card::IsProcessed(cardStatus))) {
            if (setProcessed) {
                card->SetProcessed();
            }
            cardVisitor(GetMemoryRange(card));
        }
    }
}

template <typename CardVisitor>
void CardTable::VisitMarkedCompact(CardVisitor cardVisitor)
{
    constexpr size_t MAX_CARDS_COUNT = 1000;
    size_t curPos = 0;
    size_t endPos = 0;
    PandaVector<MemRange> memRanges;

    ASSERT(cardsCount_ > 0);
    auto maxPoolAddress = PoolManager::GetMmapMemPool()->GetMaxObjectAddress();
    while (curPos < cardsCount_) {
        endPos = std::min(curPos + MAX_CARDS_COUNT - 1, cardsCount_ - 1);
        FillRanges(&memRanges, &cards_[curPos], &cards_[endPos]);
        curPos = endPos + 1;
        if (GetCardStartAddress(&cards_[curPos]) > maxPoolAddress) {
            break;
        }
    }
    for (const auto &memRange : memRanges) {
        cardVisitor(memRange);
    }
}

#ifndef NDEBUG
inline bool CardTable::IsClear()
{
    bool clear = true;
    VisitMarked(
        [&clear](const MemRange &range) {
            LOG(ERROR, GC) << "Card [" << ToVoidPtr(range.GetStartAddress()) << " - "
                           << ToVoidPtr(range.GetEndAddress()) << "] is not clear";
            clear = false;
        },
        CardTableProcessedFlag::VISIT_MARKED | CardTableProcessedFlag::VISIT_PROCESSED);
    return clear;
}
#endif
}  // namespace ark::mem

#endif  // RUNTIME_MEM_GC_CARD_TABLE_INL_H
