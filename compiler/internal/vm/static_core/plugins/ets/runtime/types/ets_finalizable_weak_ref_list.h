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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINALIZABLE_WEAK_REF_LIST_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINALIZABLE_WEAK_REF_LIST_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_finalizable_weak_ref.h"

namespace ark::ets {

class EtsFinalizableWeakRefList : protected EtsFinalizableWeakRef {
public:
    using Node = EtsFinalizableWeakRef;
    using List = EtsFinalizableWeakRefList;

    void Push(EtsCoroutine *coro, Node *weakRef)
    {
        ASSERT(weakRef != nullptr);
        auto *head = GetHead();
        if (head == nullptr) {
            SetHead(coro, weakRef);
            return;
        }
        head->SetPrev(coro, weakRef);
        weakRef->SetNext(coro, head);
        SetHead(coro, weakRef);
    }

    bool Unlink(EtsCoroutine *coro, Node *weakRef)
    {
        ASSERT(weakRef != nullptr);
        auto *prev = weakRef->GetPrev();
        auto *next = weakRef->GetNext();
        if (prev == nullptr && next == nullptr && weakRef != GetHead()) {
            return false;
        }
        if (prev != nullptr) {
            prev->SetNext(coro, next);
        }
        if (next != nullptr) {
            next->SetPrev(coro, prev);
        }
        if (weakRef == GetHead()) {
            SetHead(coro, next);
        }
        weakRef->SetPrev(coro, nullptr);
        weakRef->SetNext(coro, nullptr);
        return true;
    }

    void UnlinkClearedReferences(EtsCoroutine *coro)
    {
        auto *weakRef = GetHead();
        while (weakRef != nullptr) {
            if (weakRef->GetReferent() == nullptr) {
                // Finalizer of the cleared reference must be enqueued
                ASSERT(weakRef->ReleaseFinalizer().IsEmpty());
                Unlink(coro, weakRef);
            }
            weakRef = weakRef->GetNext();
        }
    }

    static List *FromCoreType(ObjectHeader *intrusiveList)
    {
        return reinterpret_cast<List *>(intrusiveList);
    }

    ObjectHeader *GetCoreType()
    {
        return reinterpret_cast<ObjectHeader *>(const_cast<EtsObject *>(AsObject()));
    }

    const ObjectHeader *GetCoreType() const
    {
        return reinterpret_cast<const ObjectHeader *>(AsObject());
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    static List *FromEtsObject(EtsObject *intrusiveList)
    {
        return reinterpret_cast<List *>(intrusiveList);
    }

    void TraverseAndFinalize()
    {
        auto *weakRef = GetHead();
        while (weakRef != nullptr) {
            auto finalizer = weakRef->ReleaseFinalizer();
            if (!finalizer.IsEmpty()) {
                finalizer.Run();
            }
            weakRef = weakRef->GetNext();
        }
    }

private:
    Node *GetHead() const
    {
        ASSERT(GetPrev() == nullptr);
        ASSERT(GetReferent() == nullptr);
        return GetNext();
    }

    void SetHead(EtsCoroutine *coro, Node *weakRef)
    {
        ASSERT(GetPrev() == nullptr);
        ASSERT(GetReferent() == nullptr);
        return SetNext(coro, weakRef);
    }
};

static_assert(sizeof(EtsFinalizableWeakRefList) == sizeof(EtsFinalizableWeakRef));

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINALIZABLE_WEAK_REF_LIST_H
