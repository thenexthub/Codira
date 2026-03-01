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

#ifndef PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_THREAD_H
#define PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_THREAD_H

#include "runtime/mem/gc/g1/update_remset_worker.h"

#include "libarkbase/macros.h"
#include "libarkbase/os/mutex.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::mem {

constexpr bool REMSET_THREAD_USE_STATS = false;

class RemsetThreadStats {
public:
    void IncAddedCardToQueue(size_t value = 1)
    {
        if (REMSET_THREAD_USE_STATS) {
            addedCardsToQueue_ += value;
        }
    }

    void IncProcessedConcurrentCards(const PandaUnorderedSet<CardTable::CardPtr> &cards)
    {
        if (REMSET_THREAD_USE_STATS) {
            processedConcurrentCards_ += cards.size();
            for (const auto &card : cards) {
                uniqueCards_.insert(card);
            }
        }
    }

    void IncProcessedAtSTWCards(const PandaUnorderedSet<CardTable::CardPtr> &cards)
    {
        if (REMSET_THREAD_USE_STATS) {
            processedAtStwCards_ += cards.size();
            for (const auto &card : cards) {
                uniqueCards_.insert(card);
            }
        }
    }

    void Reset()
    {
        addedCardsToQueue_ = processedConcurrentCards_ = processedAtStwCards_ = 0;
        uniqueCards_.clear();
    }

    void PrintStats() const
    {
        if (REMSET_THREAD_USE_STATS) {
            LOG(DEBUG, GC) << "remset thread stats: "
                           << "added_cards_to_queue: " << addedCardsToQueue_
                           << " processed_concurrent_cards: " << processedConcurrentCards_
                           << " processed_at_stw_cards: " << processedAtStwCards_
                           << " uniq_cards_processed: " << uniqueCards_.size();
        }
    }

private:
    std::atomic<size_t> addedCardsToQueue_ {0};
    std::atomic<size_t> processedConcurrentCards_ {0};
    std::atomic<size_t> processedAtStwCards_ {0};
    PandaUnorderedSet<CardTable::CardPtr> uniqueCards_;
};

template <class LanguageConfig>
class UpdateRemsetThread final : public UpdateRemsetWorker<LanguageConfig> {
public:
    explicit UpdateRemsetThread(G1GC<LanguageConfig> *gc, GCG1BarrierSet::ThreadLocalCardQueues *queue,
                                os::memory::Mutex *queueLock, size_t regionSize, bool updateConcurrent,
                                size_t minConcurrentCardsToProcess, size_t hotCardsProcessingFrequency);
    ~UpdateRemsetThread() final;
    NO_COPY_SEMANTIC(UpdateRemsetThread);
    NO_MOVE_SEMANTIC(UpdateRemsetThread);

private:
    void CreateWorkerImpl() final;
    void DestroyWorkerImpl() final;
    void ContinueProcessCards() REQUIRES(this->updateRemsetLock_) final;

    void ThreadLoop();

    void Sleep() REQUIRES(this->updateRemsetLock_)
    {
        static constexpr uint64_t SLEEP_MS = 1;
        threadCondVar_.TimedWait(&this->updateRemsetLock_, SLEEP_MS);
    }

    /* Thread specific variables */
    std::thread *updateThread_ {nullptr};
    os::memory::ConditionVariable threadCondVar_ GUARDED_BY(this->updateRemsetLock_);
    RemsetThreadStats stats_;
};

}  // namespace ark::mem
#endif  // PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_THREAD_H
