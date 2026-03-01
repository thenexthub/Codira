/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "runtime/arch/memory_helpers.h"
#include "runtime/include/managed_thread.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "runtime/mem/gc/g1/g1-helpers.h"
#include "runtime/mem/gc/card_table-inl.h"

namespace ark::mem {

static inline GCG1BarrierSet *GetG1BarrierSet()
{
    Thread *thread = Thread::GetCurrent();
    ASSERT(thread != nullptr);
    GCBarrierSet *barrierSet = thread->GetBarrierSet();
    ASSERT(barrierSet != nullptr);
    return static_cast<GCG1BarrierSet *>(barrierSet);
}

extern "C" void PreWrbFuncEntrypoint(void *oldval)
{
    // The cast below is needed to truncate high 32bits from 64bit pointer
    // in case object pointers have 32bit.
    oldval = ToVoidPtr(ToObjPtr(oldval));
    ASSERT(IsAddressInObjectsHeap(oldval));
    ASSERT(IsAddressInObjectsHeap(static_cast<const ObjectHeader *>(oldval)->ClassAddr<BaseClass>()));
    LOG(DEBUG, GC) << "G1GC pre barrier val = " << std::hex << oldval;
    auto *thread = ManagedThread::GetCurrent();
    // thread can't be null here because pre-barrier is called only in concurrent-mark, but we don't process
    // weak-references in concurrent mark
    ASSERT(thread != nullptr);
    auto bufferVec = thread->GetPreBuff();
    bufferVec->push_back(static_cast<ObjectHeader *>(oldval));
}

// The declaration for PostWrbUpdateCardFuncEntrypoint is generated from entrypoints.yaml,
// so there is no need for it to be in the corresponding header file for this .cpp
extern "C" void PostWrbUpdateCardFuncEntrypoint(const void *from, const void *to)
{
    ASSERT(from != nullptr);
    ASSERT(to != nullptr);
    // The cast below is needed to truncate high 32bits from 64bit pointer
    // in case object pointers have 32bit.
    from = ToVoidPtr(ToObjPtr(from));
    GCG1BarrierSet *barriers = GetG1BarrierSet();
    ASSERT(barriers != nullptr);
    auto cardTable = barriers->GetCardTable();
    ASSERT(cardTable != nullptr);
    // No need to keep remsets for young->young
    // NOTE(dtrubenkov): add assert that we do not have young -> young reference here
    auto *card = cardTable->GetCardPtr(ToUintPtr(from));
    LOG(DEBUG, GC) << "G1GC post queue add ref: " << std::hex << from << " -> " << ToVoidPtr(ToObjPtr(to))
                   << " from_card: " << cardTable->GetMemoryRange(card);
    if (card->IsYoung()) {
        return;
    }
    // StoreLoad barrier is required to guarantee order of previous reference store and card load
    arch::StoreLoadBarrier();

    auto cardValue = card->GetCard();
    auto status = CardTable::Card::GetStatus(cardValue);
    if (!CardTable::Card::IsMarked(status)) {
        card->Mark();
        if (!CardTable::Card::IsHot(cardValue)) {
            barriers->Enqueue(card);
        }
    }
}

}  // namespace ark::mem
