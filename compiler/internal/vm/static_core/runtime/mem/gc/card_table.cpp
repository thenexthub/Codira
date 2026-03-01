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

#include "runtime/mem/gc/card_table-inl.h"

#include "libarkbase/trace/trace.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/utils/logger.h"

namespace ark::mem {

using CardStatus = CardTable::Card::Status;

CardTable::CardTable(InternalAllocatorPtr internalAllocator, uintptr_t minAddress, size_t size)
    : minAddress_(minAddress),
      cardsCount_((size / CARD_SIZE) + (size % CARD_SIZE != 0 ? 1 : 0)),
      internalAllocator_(internalAllocator)
{
    /**
     * We use this assumption in compiler's post barriers in case of store pair.
     *
     * The idea is to check whether two sequential slots of an object/array being
     * written to belong to the same card or not.
     *
     * The only situation when they belong to different cards is when the second
     * slot of the store is placed at the beggining of a card.
     *
     * This could be checked by `(2nd_store_ptr - min_address) % card_size == 0`
     * condition, but if `min_address` is aligned at `card_size`, it may be simplified
     * to `2nd_store_ptr % card_size == 0`.
     */
    ASSERT(IsAligned<GetCardSize()>(minAddress));
}

CardTable::~CardTable()
{
    ASSERT(cards_ != nullptr);
    internalAllocator_->Free(cards_);
}

void CardTable::Initialize()
{
    trace::ScopedTrace scopedTrace(__PRETTY_FUNCTION__);
    if (cards_ != nullptr) {
        LOG(FATAL, GC) << "try to initialize already initialized CardTable";
    }
    cards_ = static_cast<CardPtr>(internalAllocator_->Alloc(cardsCount_));
    ClearCards(cards_, cardsCount_);
    ASSERT(cards_ != nullptr);
}

void CardTable::ClearCards(CardPtr start, size_t cardCount)
{
    static_assert(sizeof(Card) == sizeof(uint8_t));
    [[maybe_unused]] auto err =
        memset_s(reinterpret_cast<uint8_t *>(start), cardsCount_, Card::GetClearValue(), cardCount);
    ASSERT(err == EOK);
}

void CardTable::MarkCards(CardPtr start, size_t cardCount)
{
    static_assert(sizeof(Card) == sizeof(uint8_t));
    [[maybe_unused]] auto err =
        memset_s(reinterpret_cast<uint8_t *>(start), cardsCount_, Card::GetMarkedValue(), cardCount);
    ASSERT(err == EOK);
}

bool CardTable::IsMarked(uintptr_t addr) const
{
    CardPtr card = GetCardPtr(addr);
    return card->IsMarked();
}

void CardTable::MarkCard(uintptr_t addr)
{
    CardPtr card = GetCardPtr(addr);
    card->Mark();
}

bool CardTable::IsClear(uintptr_t addr) const
{
    CardPtr card = GetCardPtr(addr);
    return card->IsClear();
}

void CardTable::ClearCard(uintptr_t addr)
{
    CardPtr card = GetCardPtr(addr);
    card->Clear();
}

void CardTable::ClearAll()
{
    ClearCards(cards_, cardsCount_);
}

void CardTable::ClearCardRange(uintptr_t beginAddr, uintptr_t endAddr)
{
    ASSERT((beginAddr - minAddress_) % CARD_SIZE == 0);
    size_t cardsCount = (endAddr - beginAddr) / CARD_SIZE;
    CardPtr start = GetCardPtr(beginAddr);
    ClearCards(start, cardsCount);
}

void CardTable::MarkCardRange(uintptr_t beginAddr, uintptr_t endAddr)
{
    ASSERT((beginAddr - minAddress_) % CARD_SIZE == 0);
    size_t cardsCount = (endAddr - beginAddr) / CARD_SIZE;
    CardPtr start = GetCardPtr(beginAddr);
    MarkCards(start, cardsCount);
}

uintptr_t CardTable::GetCardStartAddress(CardPtr card) const
{
    return minAddress_ + (ToUintPtr(card) - ToUintPtr(cards_)) * CARD_SIZE;
}

uintptr_t CardTable::GetCardEndAddress(CardPtr card) const
{
    return minAddress_ + (ToUintPtr(card + 1) - ToUintPtr(cards_)) * CARD_SIZE - 1;
}

MemRange CardTable::GetMemoryRange(CardPtr card) const
{
    return MemRange(GetCardStartAddress(card), GetCardEndAddress(card));
}

CardTable::Card::Card(uint8_t val)
{
    SetCard(val);
}

void CardTable::Card::Mark()
{
    // Atomic with relaxed order reason: data race with value_ with no synchronization or ordering constraints imposed
    // on other reads or writes
    value_.fetch_or(MARKED_VALUE, std::memory_order_relaxed);
}

void CardTable::Card::UnMark()
{
    // Atomic with relaxed order reason: data race with value_ with no synchronization or ordering constraints imposed
    // on other reads or writes
    value_.fetch_and(~MARKED_VALUE, std::memory_order_relaxed);
}

void CardTable::Card::Clear()
{
    SetCard(CLEAR_VALUE);
}

void CardTable::Card::SetProcessed()
{
    SetCard(PROCESSED_VALUE);
}

void CardTable::Card::SetYoung()
{
    SetCard(YOUNG_VALUE);
}

bool CardTable::Card::IsMarked() const
{
    return IsMarked(GetStatus());
}

bool CardTable::Card::IsClear() const
{
    return GetCard() == CLEAR_VALUE;
}

bool CardTable::Card::IsProcessed() const
{
    return IsProcessed(GetStatus());
}

bool CardTable::Card::IsYoung() const
{
    return IsYoung(GetStatus());
}

/* static */
bool CardTable::Card::IsMarked(CardStatus status)
{
    return status == MARKED_VALUE;
}

/* static */
bool CardTable::Card::IsProcessed(CardStatus status)
{
    return status == PROCESSED_VALUE;
}

/* static */
bool CardTable::Card::IsYoung(CardStatus status)
{
    return status == YOUNG_VALUE;
}

/* static */
CardStatus CardTable::Card::GetStatus(uint8_t value)
{
    return value & STATUS_MASK;
}

bool CardTable::Card::IsHot() const
{
    return IsHot(GetCard());
}

void CardTable::Card::SetHot()
{
    // Atomic with relaxed order reason: data race with value_ with no synchronization or ordering constraints imposed
    // on other reads or writes
    value_.fetch_or(HOT_FLAG, std::memory_order_relaxed);
}

void CardTable::Card::ResetHot()
{
    static constexpr auto RESET_HOT_MASK = uint8_t(~HOT_FLAG);
    // Atomic with relaxed order reason: data race with value_ with no synchronization or ordering constraints imposed
    // on other reads or writes
    value_.fetch_and(RESET_HOT_MASK, std::memory_order_relaxed);
}

void CardTable::Card::SetMaxHotValue()
{
    // Atomic with relaxed order reason: data race with value_ with no synchronization or ordering constraints imposed
    // on other reads or writes
    value_.fetch_or(MAX_HOT_VALUE, std::memory_order_relaxed);
}

void CardTable::Card::IncrementHotValue()
{
    ASSERT(!IsMaxHotValue(GetCard()));
    // Atomic with relaxed order reason: data race with value_ with no synchronization or ordering constraints imposed
    // on other reads or writes
    value_.fetch_add(HOT_VALUE, std::memory_order_relaxed);
}

void CardTable::Card::DecrementHotValue()
{
    ASSERT(!IsMinHotValue(GetCard()));
    // Atomic with relaxed order reason: data race with value_ with no synchronization or ordering constraints imposed
    // on other reads or writes
    value_.fetch_sub(HOT_VALUE, std::memory_order_relaxed);
}

/* static */
bool CardTable::Card::IsMaxHotValue(uint8_t value)
{
    return (value & MAX_HOT_VALUE) == MAX_HOT_VALUE;
}

/* static */
bool CardTable::Card::IsMinHotValue(uint8_t value)
{
    return (value & MAX_HOT_VALUE) == 0;
}

/* static */
bool CardTable::Card::IsHot(uint8_t value)
{
    return (value & HOT_FLAG) == HOT_FLAG;
}

CardTable::CardPtr CardTable::GetCardPtr(uintptr_t addr) const
{
    ASSERT(addr >= minAddress_);
    ASSERT(addr < minAddress_ + cardsCount_ * CARD_SIZE);
    auto card = static_cast<CardPtr>(ToVoidPtr(ToUintPtr(cards_) + ((addr - minAddress_) >> LOG2_CARD_SIZE)));
    return card;
}

void CardTable::MarkCardsAsYoung(const MemRange &memRange)
{
    CardPtrIterator curCard = CardPtrIterator(GetCardPtr(memRange.GetStartAddress()));
    auto endCard = CardPtrIterator(GetCardPtr(memRange.GetEndAddress() - 1));
    while (curCard != endCard) {
        (*curCard)->SetYoung();
        ++curCard;
    }
    (*curCard)->SetYoung();
}
}  // namespace ark::mem
