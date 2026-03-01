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
#include "ref_block.h"

namespace ark::mem {

RefBlock::RefBlock(RefBlock *prev)
{
    slots_ = START_VALUE;
    prevBlock_ = prev;
}

void RefBlock::Reset(RefBlock *prev)
{
    slots_ = START_VALUE;
    prevBlock_ = prev;
}

Reference *RefBlock::AddRef(const ObjectHeader *object, Reference::ObjectType type)
{
    ASSERT(!IsFull());
    uint8_t index = GetFreeIndex();
    Set(index, object);
    auto *ref = reinterpret_cast<Reference *>(&refs_[index]);
    ref = Reference::SetType(ref, type);
    return ref;
}

void RefBlock::Remove(const Reference *ref)
{
    ASSERT(!IsEmpty());
    ref = Reference::GetRefWithoutType(ref);

    auto refPtr = ToUintPtr(ref);
    auto blockPtr = ToUintPtr(this);
    auto index = (refPtr - blockPtr) / sizeof(ObjectPointer<ObjectHeader>);
    ASSERT(IsBusyIndex(index));
    slots_ |= static_cast<uint64_t>(1U) << index;
    ASAN_POISON_MEMORY_REGION(refs_[index], sizeof(refs_[index]));
}

void RefBlock::VisitObjects(const GCRootVisitor &gcRootVisitor, mem::RootType rootType)
{
    for (auto *block : *this) {
        if (block->IsEmpty()) {
            continue;
        }
        for (size_t index = 0; index < REFS_IN_BLOCK; index++) {
            if (block->IsBusyIndex(index)) {
                ObjectPointerType *objectPointer = &(block->refs_[index].GetPointer());
                [[maybe_unused]] auto *obj = block->refs_[index].ReinterpretCast<ObjectHeader *>();
                ASSERT(obj->ClassAddr<BaseClass>() != nullptr);
                LOG(DEBUG, GC) << " Found root from ref-storage: " << mem::GetDebugInfoAboutObject(obj);
                gcRootVisitor({rootType, objectPointer});
            }
        }
    }
}

void RefBlock::UpdateMovedRefs([[maybe_unused]] const GCRootUpdater &gcRootUpdater)
{
    for (auto *block : *this) {
        if (block->IsEmpty()) {
            continue;
        }
        for (size_t index = 0; index < REFS_IN_BLOCK; index++) {
            if (!block->IsBusyIndex(index)) {
                continue;
            }

            auto *obj = block->refs_[index].ReinterpretCast<ObjectHeader *>();

            if (!gcRootUpdater(&obj)) {
                continue;
            }

            auto *movedObject = block->refs_[index].ReinterpretCast<ObjectHeader *>();

            LOG_IF(obj != movedObject, DEBUG, GC) << " Update pointer for obj: " << mem::GetDebugInfoAboutObject(obj);
            block->refs_[index] = obj;
        }
    }
}

// used only for dumping and tests
PandaVector<Reference *> RefBlock::GetAllReferencesInFrame()
{
    PandaVector<Reference *> refs;
    for (auto *block : *this) {
        if (block->IsEmpty()) {
            continue;
        }
        for (size_t index = 0; index < REFS_IN_BLOCK; index++) {
            if (block->IsBusyIndex(index)) {
                auto *currentRef = reinterpret_cast<Reference *>(&block->refs_[index]);
                refs.push_back(currentRef);
            }
        }
    }
    return refs;
}

uint8_t RefBlock::GetFreeIndex()
{
    ASSERT(!IsFull());
    auto res = Ffs(slots_) - 1;
#ifndef NDEBUG
    uint8_t index = 0;
    while (IsBusyIndex(index)) {
        index++;
    }
    ASSERT(index == res);
#endif
    return res;
}

void RefBlock::Set(uint8_t index, const ObjectHeader *object)
{
    ASSERT(IsFreeIndex(index));
    ASAN_UNPOISON_MEMORY_REGION(refs_[index], sizeof(refs_[index]));
    refs_[index] = object;
    slots_ &= ~(static_cast<uint64_t>(1U) << index);
}

bool RefBlock::IsFreeIndex(uint8_t index)
{
    return !IsBusyIndex(index);
}

bool RefBlock::IsBusyIndex(uint8_t index)
{
    return ((slots_ >> index) & 1U) == 0;
}

void RefBlock::PrintBlock()
{
    auto refs = GetAllReferencesInFrame();
    for (const auto &ref : refs) {
        std::cout << ref << " ";
    }
}

}  // namespace ark::mem
