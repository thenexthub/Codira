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
#include "plugins/ets/runtime/types/ets_finreg_node.h"
#include "plugins/ets/runtime/types/ets_finalization_registry.h"

namespace ark::ets {
void EtsFinalizationRegistry::Enqueue(EtsFinRegNode *node, EtsFinalizationRegistry *finReg)
{
    Remove(node, finReg);
    EtsFinRegNode *tail = finReg->GetFinalizationQueueTail();
    if (tail == nullptr) {
        finReg->SetFinalizationQueueHead(node);
    } else {
        tail->SetNext(node);
    }
    finReg->SetFinalizationQueueTail(node);
}

void EtsFinalizationRegistry::Remove(EtsFinRegNode *node, EtsFinalizationRegistry *finReg)
{
    auto *prev = node->GetPrev();
    auto *next = node->GetNext();
    if (node->HasToken()) {
        EtsInt bucketIdx = node->GetBucketIdx();
        ASSERT(0 <= bucketIdx && static_cast<uint32_t>(bucketIdx) < finReg->GetBuckets()->GetLength());
        if (prev == nullptr && next == nullptr) {
            finReg->GetBuckets()->Set(static_cast<uint32_t>(bucketIdx), nullptr);
        } else if (prev == nullptr) {
            finReg->GetBuckets()->Set(static_cast<uint32_t>(bucketIdx), next);
            ASSERT(next != nullptr);
            next->SetPrev(nullptr);
        } else if (next == nullptr) {
            ASSERT(prev != nullptr);
            prev->SetNext(nullptr);
        } else {
            prev->SetNext(next);
            next->SetPrev(prev);
        }

        finReg->DecrementMapSize();
    } else {
        if (prev == nullptr && next == nullptr) {
            finReg->SetNonUnregistrableList(nullptr);
        } else if (next == nullptr) {
            ASSERT(prev != nullptr);
            prev->SetNext(next);
        } else if (prev == nullptr) {
            finReg->SetNonUnregistrableList(next);
            ASSERT(next != nullptr);
            next->SetPrev(prev);
        } else {
            prev->SetNext(next);
            next->SetPrev(prev);
        }

        finReg->DecrementListSize();
    }
    node->SetPrev(nullptr);
    node->SetNext(nullptr);
}
}  // namespace ark::ets
