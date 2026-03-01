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


#ifndef MRT_SLOT_LIST_H
#define MRT_SLOT_LIST_H

#include "Common/BaseObject.h"

namespace MapleRuntime {
struct ObjectSlot {
    StateWord stateWord; // same with BaseObject::stateWord
    ObjectSlot* next;
};

class SlotList {
public:
    void PushFront(BaseObject* slot)
    {
        ObjectSlot* headSlot = reinterpret_cast<ObjectSlot*>(slot);
        ClearExtraContent(slot);
        headSlot->next = head;
        head = headSlot;
    }

    uintptr_t PopFront(size_t size)
    {
        if (head == nullptr || size != reinterpret_cast<BaseObject*>(head)->GetSize()) {
            return 0;
        }
        ObjectSlot* allocSlot = head;
        head = head->next;
        allocSlot->next = nullptr;
        return reinterpret_cast<uintptr_t>(allocSlot);
    }

    void Clear() { head = nullptr; }

    // Clear the rest memory of slot object if the slot object size is greater than ObjectSlot(16 Bytes).
    void ClearExtraContent(BaseObject* slot)
    {
        size_t size = slot->GetSize() - sizeof(ObjectSlot);
        if (size > 0) {
            MAddress start = reinterpret_cast<uintptr_t>(slot) + sizeof(ObjectSlot);
            CHECK_E((memset_s(reinterpret_cast<void*>(start), size, 0, size) != EOK), "memset_s fail");
        }
    }

private:
    ObjectSlot* head = nullptr;
};
} // namespace MapleRuntime
#endif // MRT_SLOT_LIST_H
