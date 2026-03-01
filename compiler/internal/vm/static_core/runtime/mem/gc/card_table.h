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

#ifndef ACCOUNTING_CARD_TABLE_H
#define ACCOUNTING_CARD_TABLE_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "runtime/include/mem/panda_containers.h"

namespace ark::mem {

template <typename PtrType>
class CardPtrIterator {
public:
    using Reference = PtrType &;
    using ConstReference = typename std::add_const<PtrType &>::type;
    using Pointer = PtrType *;

    // NOLINTNEXTLINE(readability-identifier-naming)
    explicit CardPtrIterator(PtrType c) : card_(c) {}

    CardPtrIterator &operator++()
    {
        card_ += 1;
        return *this;
    }

    // NOLINTNEXTLINE(cert-dcl21-cpp)
    CardPtrIterator operator++(int)
    {
        CardPtrIterator retval = *this;
        ++(*this);
        return retval;
    }

    bool operator==(CardPtrIterator other) const
    {
        return card_ == other.card_;
    }

    bool operator!=(CardPtrIterator other) const
    {
        return !(*this == other);
    }

    ConstReference operator*() const
    {
        return card_;
    }

    Reference operator*()
    {
        return card_;
    }

    virtual ~CardPtrIterator() = default;

    DEFAULT_COPY_SEMANTIC(CardPtrIterator);
    NO_MOVE_SEMANTIC(CardPtrIterator);

private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    PtrType card_;
};

enum CardTableProcessedFlag : uint32_t {
    VISIT_MARKED = 1U,           // visit marked cards
    VISIT_PROCESSED = 1U << 1U,  // visit parocessed cards
    SET_PROCESSED = 1U << 2U,    // set the visited cards processed
};

/**
 *   ---------------------------------------------------------------------------------------
 *   |                                        Card                                         |
 *   ---------------------------------------------------------------------------------------
 *   |         | |                             | |                   | |                   |
 *   |    0    | |    0    |    0    |    0    | |    0    |    0    | |    0    |    0    |
 *   |         | |                             | |                   | |                   |
 *   ----------- ----------------------------------------------------- ---------------------
 *        |                     |                          |                     |
 *        ---- hot flag         ---- hotness value         ---- unused bits      ---- status bits
 */
class CardTable {
public:
    class Card;
    using CardPtr = Card *;
    using CardAddress = uintptr_t;
    using Iterator = CardPtrIterator<CardPtr>;
    using ConstIterator = CardPtrIterator<const CardPtr>;

    explicit CardTable(InternalAllocatorPtr internalAllocator, uintptr_t minAddress, size_t size);
    ~CardTable();
    CardTable(const CardTable &other) = delete;
    CardTable &operator=(const CardTable &other) = delete;
    CardTable(CardTable &&other) = delete;
    CardTable &operator=(CardTable &&other) = delete;

    void Initialize();
    bool IsMarked(uintptr_t addr) const;  // returns true if the card(for the addr) state is marked
    void MarkCard(uintptr_t addr);        // set card state to the marked
    bool IsClear(uintptr_t addr) const;   // returns true if the card(for the addr) state is clear
    void ClearCard(uintptr_t addr);       // set card state to the cleared
    void ClearAll();                      // set card state to the cleared for the all cards
    void ClearCardRange(uintptr_t beginAddr, uintptr_t endAddr);
    void MarkCardRange(uintptr_t beginAddr, uintptr_t endAddr);
    static constexpr uint32_t GetCardSize()
    {
        return CARD_SIZE;
    }
    size_t GetCardsCount() const
    {
        return cardsCount_;
    }

    uintptr_t GetCardStartAddress(CardPtr card) const;  // returns address of the first byte in the card
    uintptr_t GetCardEndAddress(CardPtr card) const;    // returns address of the last byte in the card
    MemRange GetMemoryRange(CardPtr card) const;        // returns memory range for the card

    template <typename CardVisitor>
    void VisitMarked(CardVisitor cardVisitor, uint32_t processedFlag);

    template <typename CardVisitor>
    void VisitMarkedCompact(CardVisitor cardVisitor);

    // NOLINTNEXTLINE(readability-identifier-naming)
    Iterator begin()
    {
        return Iterator(cards_);
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    Iterator end()
    {
        return Iterator(cards_ + cardsCount_);
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    ConstIterator begin() const
    {
        return ConstIterator(cards_);
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    ConstIterator end() const
    {
        return ConstIterator(cards_ + cardsCount_);
    }

    static constexpr uint8_t GetCardBits()
    {
        return LOG2_CARD_SIZE;
    }

    static constexpr uint8_t GetCardDirtyValue()
    {
        return DIRTY_CARD;
    }

#ifndef NDEBUG
    bool IsClear();
#endif

    class Card {
    public:
        using Status = uint8_t;

        Card() = default;
        explicit Card(uint8_t val);

        void Mark();
        void UnMark();
        void Clear();
        void SetProcessed();
        void SetYoung();
        uint8_t GetCard() const;
        void SetCard(uint8_t newVal);
        Status GetStatus() const;

        bool IsMarked() const;
        bool IsClear() const;
        bool IsProcessed() const;
        bool IsYoung() const;

        static bool IsMarked(Status status);
        static bool IsProcessed(Status status);
        static bool IsYoung(Status status);

        bool IsHot() const;
        void SetHot();
        void ResetHot();
        void SetMaxHotValue();
        void IncrementHotValue();
        void DecrementHotValue();

        static bool IsMaxHotValue(uint8_t value);
        static bool IsMinHotValue(uint8_t value);
        static bool IsHot(uint8_t value);

        static Status GetStatus(uint8_t value);

        static constexpr uint32_t GetValueOffset()
        {
            return MEMBER_OFFSET(Card, value_);
        }

        static constexpr auto GetClearValue()
        {
            return CLEAR_VALUE;
        }

        static constexpr auto GetMarkedValue()
        {
            return MARKED_VALUE;
        }

        static constexpr auto GetYoungValue()
        {
            return YOUNG_VALUE;
        }

        static constexpr auto GetMaxHotValue()
        {
            return MAX_HOT_VALUE;
        }

        static constexpr auto GetHotValue()
        {
            return HOT_VALUE;
        }

        static constexpr auto GetHotFlag()
        {
            return HOT_FLAG;
        }

        static constexpr auto GetStatusMask()
        {
            return STATUS_MASK;
        }

        ~Card() = default;

        NO_COPY_SEMANTIC(Card);
        NO_MOVE_SEMANTIC(Card);

    private:
        static constexpr uint8_t YOUNG_VALUE = 3U;
        static constexpr uint8_t PROCESSED_VALUE = 2U;
        static constexpr uint8_t MARKED_VALUE = 1U;
        static constexpr uint8_t CLEAR_VALUE = 0U;
        static constexpr uint8_t STATUS_MASK = 3U;
        static constexpr uint8_t STATUS_BITS = 2U;
        static constexpr uint8_t UNUSED_BITS = 2U;
        static constexpr uint8_t HOT_VALUE = 1U << (uint8_t)(STATUS_BITS + UNUSED_BITS);
        static constexpr uint8_t MAX_HOT_VALUE = 7U << (uint8_t)(STATUS_BITS + UNUSED_BITS);
        static constexpr uint8_t HOT_FLAG = 1U << 7U;

        std::atomic_uint8_t value_ = CLEAR_VALUE;
    };

    CardPtr GetCardPtr(uintptr_t addr) const;  // returns card address for the addr

    ALWAYS_INLINE uintptr_t GetMinAddress() const
    {
        return minAddress_;
    }

    void MarkCardsAsYoung(const MemRange &memRange);

private:
    void ClearCards(CardPtr start, size_t cardCount);
    void MarkCards(CardPtr start, size_t cardCount);
    size_t GetSize() const;  // returns size of card table array
    inline void FillRanges(PandaVector<MemRange> *ranges, const Card *startCard, const Card *endCard);

    static constexpr uint8_t LOG2_CARD_SIZE = 12;
    static constexpr uint32_t CARD_SIZE = 1U << LOG2_CARD_SIZE;
    static constexpr uint8_t DIRTY_CARD = 1U;
    static constexpr size_t CHUNK_CARD_NUM = sizeof(std::atomic_size_t) / sizeof(Card);

    CardPtr cards_ {nullptr};
    uintptr_t minAddress_ {0};
    size_t cardsCount_ {0};
    InternalAllocatorPtr internalAllocator_ {nullptr};
};

static_assert(sizeof(std::atomic_size_t) % sizeof(CardTable::Card) == 0);

using CardVisitor = std::function<void(CardTable::CardPtr)>;

}  // namespace ark::mem

#endif  // ACCOUNTING_CARD_TABLE_H
