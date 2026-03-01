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

#include "mem/gc/card_table.h"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <random>

#include "gtest/gtest.h"
#include "libarkbase/mem/mem.h"
#include "runtime/mem/gc/card_table-inl.h"
#include "runtime/mem/mem_stats_additional_info.h"
#include "runtime/mem/mem_stats_default.h"
#include "runtime/mem/internal_allocator-inl.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"

namespace ark::mem::test {

class CardTableTest : public testing::Test {
private:
    std::mt19937 gen_;
    std::uniform_int_distribution<uintptr_t> addrDis_;
    std::uniform_int_distribution<size_t> cardIndexDis_;

protected:
    //    static constexpr size_t kHeapSize = 0xffffffff;
    static constexpr size_t K_ALLOC_COUNT = 1000;
    //    static constexpr size_t maxCardIndex = kHeapSize / ::ark::mem::CardTable::GetCardSize();

    // NOLINTNEXTLINE(cert-msc51-cpp)
    CardTableTest()
    {
#ifdef PANDA_NIGHTLY_TEST_ON
        seed_ = std::time(NULL);
#else
        seed_ = RANDOM_SEED;
#endif
        RuntimeOptions options;
        // NOLINTNEXTLINE(readability-magic-numbers)
        options.SetHeapSizeLimit(64_MB);
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetGcType("epsilon");
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();

        internalAllocator_ = thread_->GetVM()->GetHeapManager()->GetInternalAllocator();
        addrDis_ = std::uniform_int_distribution<uintptr_t>(0, GetPoolSize() - 1);
        ASSERT(GetPoolSize() % CardTable::GetCardSize() == 0);
        cardIndexDis_ = std::uniform_int_distribution<size_t>(0, GetPoolSize() / CardTable::GetCardSize() - 1);
        cardTable_ = std::make_unique<CardTable>(internalAllocator_, GetMinAddress(), GetPoolSize());
        cardTable_->Initialize();
    }

    ~CardTableTest() override
    {
        cardTable_.reset(nullptr);
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(CardTableTest);
    NO_MOVE_SEMANTIC(CardTableTest);

    void SetUp() override
    {
        gen_.seed(seed_);
    }

    void TearDown() override
    {
        const ::testing::TestInfo *const testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
        if (testInfo->result()->Failed()) {
            std::cout << "CartTableTest seed = " << seed_ << std::endl;
        }
    }

    uintptr_t GetMinAddress()
    {
        return PoolManager::GetMmapMemPool()->GetMinObjectAddress();
    }

    size_t GetPoolSize()
    {
        return (PoolManager::GetMmapMemPool()->GetMaxObjectAddress() -
                PoolManager::GetMmapMemPool()->GetMinObjectAddress());
    }

    uintptr_t GetRandomAddress()
    {
        return PoolManager::GetMmapMemPool()->GetMinObjectAddress() + addrDis_(gen_) % GetPoolSize();
    }

    size_t GetRandomCardIndex()
    {
        return cardIndexDis_(gen_) % GetPoolSize();
    }

    // generate address at the begining of the card
    uintptr_t GetRandomCardAddress()
    {
        return PoolManager::GetMmapMemPool()->GetMinObjectAddress() + GetRandomCardIndex() * CardTable::GetCardSize();
    }

    std::set<uintptr_t> GetRandomCardAddressSet()
    {
        std::set<uintptr_t> addrSet;
        while (addrSet.size() <= K_ALLOC_COUNT) {
            uintptr_t addr = GetRandomCardAddress();
            if (!addrSet.insert(addr).second) {
                continue;
            }
            ASSERT(cardTable_->GetCardPtr(addr)->IsClear());
        }
        return addrSet;
    }

    void SetMaxHotValue(std::set<uintptr_t> &addrSet)
    {
        for (auto addr : addrSet) {
            auto *card = cardTable_->GetCardPtr(addr);
            auto cardValue = card->GetCard();
            while (!CardTable::Card::IsMaxHotValue(cardValue)) {
                card->IncrementHotValue();
                ASSERT(!card->IsClear());
                cardValue = card->GetCard();
            }
        }
    }

    void SetHotFlag(std::set<uintptr_t> &addrSet)
    {
        for (auto addr : addrSet) {
            auto *card = cardTable_->GetCardPtr(addr);
            card->SetHot();
            ASSERT(card->IsHot());
        }
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::unique_ptr<CardTable> cardTable_;

private:
    InternalAllocatorPtr internalAllocator_;
    unsigned int seed_ {};
    ark::MTManagedThread *thread_ {};

    static const unsigned int RANDOM_SEED = 123456U;
};

TEST_F(CardTableTest, MarkTest)
{
    size_t markedCnt = 0;

    for (size_t i = 0; i < K_ALLOC_COUNT; i++) {
        uintptr_t addr = GetRandomAddress();
        if (!cardTable_->IsMarked(addr)) {
            ++markedCnt;
            cardTable_->MarkCard(addr);
        }
    }

    for (auto card : *cardTable_) {
        if (card->IsMarked()) {
            markedCnt--;
        }
    }
    ASSERT_EQ(markedCnt, 0);
}

TEST_F(CardTableTest, MarkAndClearAllTest)
{
    // std::set<uintptr_t> addrSet;

    size_t cnt = 0;
    for (auto card : *cardTable_) {
        card->Mark();
        cnt++;
    }
    ASSERT_EQ(cnt, cardTable_->GetCardsCount());

    size_t cntCleared = 0;
    for (auto card : *cardTable_) {
        card->Clear();
        cntCleared++;
    }
    ASSERT_EQ(cntCleared, cardTable_->GetCardsCount());
}

TEST_F(CardTableTest, ClearTest)
{
    std::set<uintptr_t> addrSet;

    // Mark some cards not more than once
    while (addrSet.size() <= K_ALLOC_COUNT) {
        uintptr_t addr = GetRandomCardAddress();
        if (!addrSet.insert(addr).second) {
            continue;
        }
        cardTable_->MarkCard(addr);
    }

    size_t clearedCnt = 0;
    // clear all marked and count them
    for (auto card : *cardTable_) {
        if (card->IsMarked()) {
            card->UnMark();
            clearedCnt++;
        }
    }

    ASSERT_EQ(addrSet.size(), clearedCnt);
    // check that there are no marked
    for (auto card : *cardTable_) {
        ASSERT_EQ(card->IsMarked(), false);
    }
}

TEST_F(CardTableTest, ClearAllTest)
{
    std::set<uintptr_t> addrSet;

    // Mark some cards not more than once
    while (addrSet.size() < K_ALLOC_COUNT) {
        uintptr_t addr = GetRandomCardAddress();
        if (!addrSet.insert(addr).second) {
            continue;
        }
        cardTable_->MarkCard(addr);
    }

    cardTable_->ClearAll();
    for (auto card : *cardTable_) {
        ASSERT_EQ(card->IsMarked(), false);
    }
}

TEST_F(CardTableTest, double_initialization)
{
    EXPECT_DEATH(cardTable_->Initialize(), ".*");
}

TEST_F(CardTableTest, corner_cases)
{
    // Mark 1st byte in the heap
    auto cardIt = cardTable_->begin();
    auto *firstCard = *cardIt;
    auto *secondCard = *(++cardIt);
    ASSERT_EQ(firstCard->IsMarked(), false);
    cardTable_->MarkCard(GetMinAddress());
    ASSERT_EQ(firstCard->IsMarked(), true);
    // Mark last byte in the heap
    uintptr_t last = GetMinAddress() + GetPoolSize() - 1;
    ASSERT_EQ(cardTable_->IsMarked(last), false);
    cardTable_->MarkCard(last);
    ASSERT_EQ(cardTable_->IsMarked(last), true);
    // Mark last byte of second card
    uintptr_t secondLast = GetMinAddress() + 2 * cardTable_->GetCardSize() - 1;
    ASSERT_EQ(cardTable_->IsMarked(secondLast), false);
    cardTable_->MarkCard(secondLast);
    ASSERT_EQ(secondCard->IsMarked(), true);
}

TEST_F(CardTableTest, VisitMarked)
{
    size_t markedCnt = 0;

    while (markedCnt < K_ALLOC_COUNT) {
        uintptr_t addr = GetRandomAddress();
        if (!cardTable_->IsMarked(addr)) {
            ++markedCnt;
            cardTable_->MarkCard(addr);
        }
    }

    PandaVector<MemRange> memRanges;
    cardTable_->VisitMarked([&memRanges](MemRange memRange) { memRanges.emplace_back(memRange); },
                            CardTableProcessedFlag::VISIT_MARKED);

    // Got ranges one by one
    PandaVector<MemRange> expectedRanges;
    for (auto card : *cardTable_) {
        if (card->IsMarked()) {
            expectedRanges.emplace_back(cardTable_->GetMemoryRange(card));
        }
    }

    ASSERT_EQ(expectedRanges.size(), memRanges.size());
    for (size_t i = 0; i < expectedRanges.size(); i++) {
        ASSERT(memRanges[i].GetStartAddress() == expectedRanges[i].GetStartAddress());
        ASSERT(memRanges[i].GetEndAddress() == expectedRanges[i].GetEndAddress());
    }
}

TEST_F(CardTableTest, IncrementHotValueTest)
{
    auto addrSet = GetRandomCardAddressSet();
    for (auto addr : addrSet) {
        auto *card = cardTable_->GetCardPtr(addr);
        [[maybe_unused]] auto cardValue = card->GetCard();
        ASSERT(CardTable::Card::IsMinHotValue(cardValue));
    }
    SetMaxHotValue(addrSet);
    for (auto addr : addrSet) {
        auto *card = cardTable_->GetCardPtr(addr);
        auto cardValue = card->GetCard();
        [[maybe_unused]] auto cardStatus = CardTable::Card::GetStatus(cardValue);
        ASSERT(CardTable::Card::IsMaxHotValue(cardValue));
        ASSERT(!CardTable::Card::IsHot(cardValue));
        ASSERT(!CardTable::Card::IsMarked(cardStatus));
        ASSERT(!CardTable::Card::IsYoung(cardStatus));
        ASSERT(!CardTable::Card::IsProcessed(cardStatus));
    }
}

TEST_F(CardTableTest, SetHotFlagTest)
{
    auto addrSet = GetRandomCardAddressSet();
    SetHotFlag(addrSet);
    for (auto addr : addrSet) {
        auto *card = cardTable_->GetCardPtr(addr);
        auto cardValue = card->GetCard();
        [[maybe_unused]] auto cardStatus = CardTable::Card::GetStatus(cardValue);
        ASSERT(CardTable::Card::IsHot(cardValue));
        ASSERT(CardTable::Card::IsMinHotValue(cardValue));
        ASSERT(!CardTable::Card::IsMarked(cardStatus));
        ASSERT(!CardTable::Card::IsYoung(cardStatus));
        ASSERT(!CardTable::Card::IsProcessed(cardStatus));
    }
}

TEST_F(CardTableTest, HotCardTest)
{
    auto addrSet = GetRandomCardAddressSet();
    // Increment hot value
    SetMaxHotValue(addrSet);
    // Set hot flag
    SetHotFlag(addrSet);

    // Mark card
    for (auto addr : addrSet) {
        auto *card = cardTable_->GetCardPtr(addr);
        ASSERT(!card->IsMarked());
        card->Mark();
        auto cardValue = card->GetCard();
        [[maybe_unused]] auto cardStatus = CardTable::Card::GetStatus(cardValue);
        ASSERT(CardTable::Card::IsMaxHotValue(cardValue));
        ASSERT(CardTable::Card::IsHot(cardValue));
        ASSERT(CardTable::Card::IsMarked(cardStatus));
        ASSERT(!CardTable::Card::IsYoung(cardStatus));
        ASSERT(!CardTable::Card::IsProcessed(cardStatus));
    }

    // Make card clear
    for (auto addr : addrSet) {
        auto *card = cardTable_->GetCardPtr(addr);
        ASSERT(card->IsMarked());
        card->UnMark();
        auto cardValue = card->GetCard();
        [[maybe_unused]] auto cardStatus = CardTable::Card::GetStatus(cardValue);
        ASSERT(CardTable::Card::IsMaxHotValue(cardValue));
        ASSERT(CardTable::Card::IsHot(cardValue));
        ASSERT(!CardTable::Card::IsMarked(cardStatus));
        ASSERT(!CardTable::Card::IsYoung(cardStatus));
        ASSERT(!CardTable::Card::IsProcessed(cardStatus));

        card->ResetHot();
        cardValue = card->GetCard();
        cardStatus = CardTable::Card::GetStatus(cardValue);
        ASSERT(CardTable::Card::IsMaxHotValue(cardValue));
        ASSERT(!CardTable::Card::IsHot(cardValue));
        ASSERT(!CardTable::Card::IsMarked(cardStatus));
        ASSERT(!CardTable::Card::IsYoung(cardStatus));
        ASSERT(!CardTable::Card::IsProcessed(cardStatus));

        while (!CardTable::Card::IsMinHotValue(cardValue)) {
            ASSERT(!card->IsClear());
            card->DecrementHotValue();
            cardValue = card->GetCard();
        }
        ASSERT(card->IsClear());
    }
}

}  // namespace ark::mem::test
