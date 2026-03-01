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

#ifndef PANDA_RUNTIME_MEM_GC_G1_HOT_CARDS_H
#define PANDA_RUNTIME_MEM_GC_G1_HOT_CARDS_H

#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/gc/card_table-inl.h"
#include "runtime/arch/memory_helpers.h"

namespace ark::mem {

class HotCards {
public:
    using Card = CardTable::Card;
    using CardPtr = CardTable::CardPtr;

    void PushCard(CardPtr cardPtr);

    void DecrementHotValue();

    // Should be called when mutator thread is on pause
    void DrainMarkedCards(PandaUnorderedSet<CardPtr> *markedCards);
    void ClearHotCards();

    template <typename CardHandler>
    size_t HandleCards(CardHandler handler)
    {
        UpdateCardsStatus();

        // clear card before we process it, because parallel mutator thread can make a write and we would miss it
        arch::StoreLoadBarrier();

        size_t processedCardsCnt = 0;
        auto cardIt = unprocessedCards_.begin();
        while (cardIt != unprocessedCards_.end()) {
            if (!handler.Handle(*cardIt)) {
                break;
            }
            ++processedCardsCnt;
            cardIt = unprocessedCards_.erase(cardIt);
        }
        return processedCardsCnt;
    }

    static ALWAYS_INLINE void SetHot(CardPtr cardPtr, uint8_t value)
    {
        cardPtr->SetCard(value | Card::GetHotFlag());
    }

    static ALWAYS_INLINE void ResetHot(CardPtr cardPtr, uint8_t value)
    {
        cardPtr->SetCard(value & RESET_HOT_MASK);
    }

    static ALWAYS_INLINE void SetMaxHotValue(CardPtr cardPtr, uint8_t value)
    {
        cardPtr->SetCard(value | Card::GetMaxHotValue());
    }

    static ALWAYS_INLINE void SetHotAndMaxHotValue(CardPtr cardPtr, uint8_t value)
    {
        static constexpr uint8_t HOT_FLAG_MAX_VALUE = Card::GetHotFlag() | Card::GetMaxHotValue();
        cardPtr->SetCard(value | HOT_FLAG_MAX_VALUE);
    }

    static ALWAYS_INLINE void IncrementHotValue(CardPtr cardPtr, uint8_t value)
    {
        ASSERT(!Card::IsMaxHotValue(value));
        cardPtr->SetCard(value + Card::GetHotValue());
    }

    static constexpr uint8_t RESET_HOT_MASK = uint8_t(~Card::GetHotFlag());
    static constexpr uint8_t RESET_MARK_MASK = ~CardTable::Card::GetMarkedValue();

private:
    void UpdateCardsStatus();

    template <typename F>
    void VisitHotCards(F fun)
    {
        std::for_each(hotCards_.begin(), hotCards_.end(), std::move(fun));
    }

    PandaUnorderedSet<CardPtr> hotCards_;
    PandaUnorderedSet<CardPtr> unprocessedCards_;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_G1_HOT_CARDS_H