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

#ifndef COMMON_COMPONENTS_HEAP_ALLOCATOR_SLOT_LIST_H
#define COMMON_COMPONENTS_HEAP_ALLOCATOR_SLOT_LIST_H

#include "common_components/common/base_object.h"

namespace common {
struct ObjectSlot {
    ObjectSlot* next;
};

class SlotList {
public:
    void PushFront(BaseObject* slot)
    {
        ObjectSlot* headSlot = reinterpret_cast<ObjectSlot*>(slot);
        headSlot->next = head_;
        head_ = headSlot;
    }

    uintptr_t PopFront(size_t size)
    {
        if (head_ == nullptr) {
            return 0;
        }
        ObjectSlot* allocSlot = head_;
        head_ = head_->next;
        allocSlot->next = nullptr;
        return reinterpret_cast<uintptr_t>(allocSlot);
    }

    void Clear() { head_ = nullptr; }
private:
    ObjectSlot* head_ = nullptr;
};
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_ALLOCATOR_SLOT_LIST_H
