/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "runtime/mem/gc/g1/update_remset_worker.h"
#include "runtime/mem/gc/g1/update_remset_worker-inl.h"
#include "mem/gc/card_table.h"
#include "runtime/include/language_context.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/g1/card_handler.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/object_helpers-inl.h"
#include "runtime/mem/rem_set-inl.h"

namespace ark::mem {

template <class LanguageConfig>
UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorker(G1GC<LanguageConfig> *gc,
                                                       GCG1BarrierSet::ThreadLocalCardQueues *queue,
                                                       os::memory::Mutex *queueLock, size_t regionSize,
                                                       bool updateConcurrent, size_t minConcurrentCardsToProcess,
                                                       size_t hotCardsProcessingFrequency)
    : gc_(gc),
      regionSizeBits_(ark::helpers::math::GetIntLog2(regionSize)),
      minConcurrentCardsToProcess_(minConcurrentCardsToProcess),
      hotCardsProcessingFrequency_(hotCardsProcessingFrequency),
      queue_(queue),
      queueLock_(queueLock),
      updateConcurrent_(updateConcurrent)
{
    static constexpr size_t PREALLOCATED_CARDS_SET_SIZE = 256;
    cards_.reserve(PREALLOCATED_CARDS_SET_SIZE);
}

template <class LanguageConfig>
UpdateRemsetWorker<LanguageConfig>::~UpdateRemsetWorker()
{
    auto allocator = gc_->GetInternalAllocator();
    // Take the lock to satisfy TSAN.
    // Actually at this moment all mutators should be destroyed and the lock is not needed.
    os::memory::LockHolder holder(postBarrierBuffersLock_);
    for (auto *buffer : postBarrierBuffers_) {
        allocator->Delete(buffer);
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::CreateWorker()
{
    if (IsFlag(UpdateRemsetWorkerFlags::IS_STOP_WORKER)) {
        RemoveFlag(UpdateRemsetWorkerFlags::IS_STOP_WORKER);
    }
    if (updateConcurrent_) {
        this->CreateWorkerImpl();
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::DestroyWorker()
{
    SetFlag(UpdateRemsetWorkerFlags::IS_STOP_WORKER);
    if (updateConcurrent_) {
        this->DestroyWorkerImpl();
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::AddPostBarrierBuffer(PandaVector<mem::CardTable::CardPtr> *buffer)
{
    os::memory::LockHolder holder(postBarrierBuffersLock_);
    postBarrierBuffers_.push_back(buffer);
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromDefered(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    os::memory::LockHolder holder(*queueLock_);
    std::copy(cards_.begin(), cards_.end(), std::inserter(*cards, cards->end()));
    cards_.clear();
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromQueue(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    os::memory::LockHolder holder(*queueLock_);
    std::copy(queue_->begin(), queue_->end(), std::inserter(*cards, cards->end()));
    queue_->clear();
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromThreads(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    auto *vm = gc_->GetPandaVm();
    ASSERT(vm != nullptr);
    auto *threadManager = vm->GetThreadManager();
    ASSERT(threadManager != nullptr);
    threadManager->EnumerateThreads([this, cards](ManagedThread *thread) {
        auto *buffer = thread->GetG1PostBarrierBuffer();
        if (buffer != nullptr) {
            FillFromPostBarrierBuffer(buffer, cards);
        }
        return true;
    });
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromPostBarrierBuffers(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    auto allocator = gc_->GetInternalAllocator();
    os::memory::LockHolder holder(postBarrierBuffersLock_);
    while (!postBarrierBuffers_.empty()) {
        auto *buffer = postBarrierBuffers_.back();
        postBarrierBuffers_.pop_back();
        FillFromPostBarrierBuffer(buffer, cards);
        allocator->Delete(buffer);
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromHotCards(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    hotCards_.DrainMarkedCards(cards);
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromPostBarrierBuffer(GCG1BarrierSet::G1PostBarrierRingBufferType *postWrb,
                                                                   PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    if (postWrb == nullptr) {
        return;
    }
    bool hasElement;
    while (true) {
        mem::CardTable::CardPtr card;
        hasElement = postWrb->TryPop(&card);
        if (!hasElement) {
            break;
        }
        cards->insert(card);
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromPostBarrierBuffer(GCG1BarrierSet::ThreadLocalCardQueues *postWrb,
                                                                   PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    while (!postWrb->empty()) {
        cards->insert(postWrb->back());
        postWrb->pop_back();
    }
}

template <class LanguageConfig>
bool UpdateRemsetWorker<LanguageConfig>::NeedProcessHotCards()
{
    static size_t processingCnt = 0;
    if (processingCnt++ == hotCardsProcessingFrequency_) {
        processingCnt = 0;
        return true;
    }
    return false;
}

template <class LanguageConfig>
size_t UpdateRemsetWorker<LanguageConfig>::ProcessAllCards()
{
    auto emptyHandler = [](ObjectHeader *, size_t) {};
    size_t processedCardsCnt = ProcessCommonCards(emptyHandler);
    if (NeedProcessHotCards()) {
        processedCardsCnt += ProcessHotCards(emptyHandler);
    }
    return processedCardsCnt;
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::UpdateCardStatus(CardTable::CardPtr card)
{
    auto cardValue = card->GetCard();
    auto cardStatus = CardTable::Card::GetStatus(cardValue);

    ASSERT_PRINT(!CardTable::Card::IsProcessed(cardStatus) && !CardTable::Card::IsYoung(cardStatus),
                 "UpdateCardStatus card: " << card << " cardValue: " << (int)cardValue);

    if (!CardTable::Card::IsMarked(cardStatus)) {
        return;
    }

    if (CardTable::Card::IsHot(cardValue)) {
        HotCards::SetMaxHotValue(card, cardValue);
        return;
    }

    if (CardTable::Card::IsMaxHotValue(cardValue)) {
        HotCards::SetHot(card, cardValue);
        hotCards_.PushCard(card);
        return;
    }

    HotCards::IncrementHotValue(card, cardValue & HotCards::RESET_MARK_MASK);
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::DrainAllCards(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    ASSERT(IsFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD));
    os::memory::LockHolder holder(updateRemsetLock_);
    FillFromDefered(cards);
    FillFromQueue(cards);
    FillFromThreads(cards);
    FillFromPostBarrierBuffers(cards);
    FillFromHotCards(cards);
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::DoInvalidateRegions(RegionVector *allRegions)
{
    for (Region *region : *allRegions) {
        if (!region->HasFlag(RegionFlag::IS_INVALID)) {
            RemSet<> *remset = region->GetRemSet();
            remset->RemoveInvalidRegions();
        }
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::GCInvalidateRegions(RegionVector *allRegions)
{
    // Do invalidate region on pause in GCThread
    ASSERT(IsFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD));
    os::memory::LockHolder holder(updateRemsetLock_);
    DoInvalidateRegions(allRegions);
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::InvalidateRegions(RegionVector *allRegions)
{
    // Do invalidate region during concurrent sweep
    ASSERT(IsFlag(UpdateRemsetWorkerFlags::IS_PROCESS_CARD));
    SetFlag(UpdateRemsetWorkerFlags::IS_INVALIDATE_REGIONS);
    // Atomic with relaxed order reason: memory order is not required
    deferCards_.store(true, std::memory_order_relaxed);
    // Aquare lock to be sure that UpdateRemsetWorker has been stopped
    os::memory::LockHolder holder(updateRemsetLock_);
    LOG(DEBUG, GC) << "Remset worker has been paused to invalidate region on concurrent";
    DoInvalidateRegions(allRegions);
    // Atomic with relaxed order reason: memory order is not required
    deferCards_.store(false, std::memory_order_relaxed);
    RemoveFlag(UpdateRemsetWorkerFlags::IS_INVALIDATE_REGIONS);
    if (updateConcurrent_) {
        this->ContinueProcessCards();
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::SuspendWorkerForGCPause()
{
    ASSERT(!IsFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD));
    SetFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD);
    // Atomic with relaxed order reason: memory order is not required
    deferCards_.store(true, std::memory_order_relaxed);
    // Aquare lock to be sure that UpdateRemsetWorker has been stopped
    os::memory::LockHolder holder(updateRemsetLock_);
#ifndef NDEBUG
    pausedByGcThread_ = true;
#endif
    LOG(DEBUG, GC) << "Remset worker has been paused for GC";
    // Atomic with relaxed order reason: memory order is not required
    deferCards_.store(false, std::memory_order_relaxed);
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::ResumeWorkerAfterGCPause()
{
    ASSERT(IsFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD));
    os::memory::LockHolder holder(updateRemsetLock_);
#ifndef NDEBUG
    pausedByGcThread_ = false;
#endif
    RemoveFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD);
    if (updateConcurrent_) {
        this->ContinueProcessCards();
    }
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(UpdateRemsetWorker);
}  // namespace ark::mem
