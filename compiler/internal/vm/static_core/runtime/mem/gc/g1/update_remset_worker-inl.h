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

#ifndef PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_WORKER_INL_H
#define PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_WORKER_INL_H

#include "libarkbase/os/mutex.h"
#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "runtime/mem/gc/g1/hot_cards.h"
#include "runtime/mem/gc/g1/update_remset_worker.h"
#include "runtime/mem/gc/g1/card_handler.h"

namespace ark::mem {
template <class LanguageConfig>
template <typename Handler>
void UpdateRemsetWorker<LanguageConfig>::GCProcessCards(const Handler &handler)
{
    ASSERT(IsFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD));
    os::memory::LockHolder holder(updateRemsetLock_);
    ProcessCommonCards(handler);
    ProcessHotCards(handler);
    hotCards_.ClearHotCards();
}

template <class LanguageConfig>
template <typename Handler>
size_t UpdateRemsetWorker<LanguageConfig>::ProcessCommonCards(const Handler &handler)
{
    FillFromQueue(&cards_);
    FillFromThreads(&cards_);
    FillFromPostBarrierBuffers(&cards_);

    std::for_each(cards_.begin(), cards_.end(), [this](auto *card) { UpdateCardStatus(card); });

    // clear cards before we process it, because parallel mutator thread can make a write and we would miss it
    arch::StoreLoadBarrier();

    LOG_IF(!cards_.empty(), DEBUG, GC) << "Started process: " << cards_.size() << " cards";

    size_t cardsSize = 0;
    CardHandler<Handler, LanguageConfig> cardHandler(gc_->GetCardTable(), regionSizeBits_, deferCards_, handler);
    for (auto it = cards_.begin(); it != cards_.end();) {
        auto cardPtr = *it;
        if (!cardPtr->IsHot()) {
            if (!cardHandler.Handle(cardPtr)) {
                break;
            }
            ++cardsSize;
        }
        it = cards_.erase(it);
    }
    LOG_IF(!cards_.empty(), DEBUG, GC) << "Processed " << cardsSize << " cards";

    hotCards_.DecrementHotValue();
    return cardsSize;
}

template <class LanguageConfig>
template <typename Handler>
size_t UpdateRemsetWorker<LanguageConfig>::ProcessHotCards(const Handler &handler)
{
    CardHandler<Handler, LanguageConfig> cardHandler(gc_->GetCardTable(), regionSizeBits_, deferCards_, handler);
    auto processedCardsCnt = hotCards_.HandleCards(cardHandler);
    return processedCardsCnt;
}
}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_WORKER_INL_H
