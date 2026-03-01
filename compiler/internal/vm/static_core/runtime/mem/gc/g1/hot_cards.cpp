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

#include "runtime/mem/gc/g1/hot_cards.h"
#include "libarkbase/macros.h"

namespace ark::mem {

void HotCards::UpdateCardsStatus()
{
    VisitHotCards([this](CardPtr card) {
        auto cardValue = card->GetCard();
        auto cardStatus = CardTable::Card::GetStatus(cardValue);

        ASSERT_PRINT(!CardTable::Card::IsProcessed(cardStatus) && !CardTable::Card::IsYoung(cardStatus),
                     "UpdateHotCardStatus card: " << card << " cardValue: " << (int)cardValue);

        if (CardTable::Card::IsMarked(cardStatus)) {
            SetHotAndMaxHotValue(card, cardValue & RESET_MARK_MASK);
            unprocessedCards_.insert(card);
        }
    });
}

void HotCards::PushCard(CardPtr card)
{
    ASSERT(card->IsHot());
    hotCards_.insert(card);
}

void HotCards::DecrementHotValue()
{
    VisitHotCards([](CardPtr card) {
        auto cardValue = card->GetCard();
        auto cardStatus = Card::GetStatus(cardValue);

        ASSERT_PRINT(!CardTable::Card::IsProcessed(cardStatus) && !CardTable::Card::IsYoung(cardStatus),
                     "DecrementHotValue card: " << card << " cardValue: " << (int)cardValue);

        if (Card::IsMarked(cardStatus) || !Card::IsHot(cardValue)) {
            return;
        }

        if (Card::IsMinHotValue(cardValue)) {
            card->ResetHot();
            return;
        }
        card->DecrementHotValue();
    });
}

void HotCards::DrainMarkedCards(PandaUnorderedSet<CardPtr> *markedCards)
{
    auto cardIt = hotCards_.begin();
    while (cardIt != hotCards_.end()) {
        auto *cardPtr = *cardIt;
        auto cardValue = cardPtr->GetCard();
        auto cardStatus = CardTable::Card::GetStatus(cardValue);
        if (CardTable::Card::IsMarked(cardStatus)) {
            markedCards->insert(cardPtr);
        }
        ResetHot(cardPtr, cardValue);
        cardIt = hotCards_.erase(cardIt);
    }
    markedCards->insert(unprocessedCards_.begin(), unprocessedCards_.end());
    unprocessedCards_.clear();
}

void HotCards::ClearHotCards()
{
    auto cardIt = hotCards_.begin();
    while (cardIt != hotCards_.end()) {
        (*cardIt)->ResetHot();
        cardIt = hotCards_.erase(cardIt);
    }
    ASSERT(unprocessedCards_.empty());
}

}  // namespace ark::mem
