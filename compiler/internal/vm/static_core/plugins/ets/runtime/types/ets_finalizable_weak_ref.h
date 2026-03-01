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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINALIZABLE_WEAK_REF_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINALIZABLE_WEAK_REF_H

#include "plugins/ets/runtime/types/ets_weak_reference.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/ets_coroutine.h"

namespace ark::ets {

namespace test {
class EtsFinalizableWeakRefTest;
}  // namespace test

/// @class EtsWeakReference represent std.core.FinalizableWeakRef class
class EtsFinalizableWeakRef : public EtsWeakReference {
public:
    using FinalizerFunc = void (*)(void *);
    using FinalizerArg = void *;

    class Finalizer {
    public:
        explicit Finalizer(FinalizerFunc funcPtr, FinalizerArg argPtr) : funcPtr_(funcPtr), argPtr_(argPtr) {}

        void Run()
        {
            ASSERT(!IsEmpty());
            funcPtr_(argPtr_);
        }

        bool IsEmpty() const
        {
            return funcPtr_ == nullptr;
        }

        FinalizerArg GetFinalizerArg() const
        {
            return argPtr_;
        }

    private:
        FinalizerFunc funcPtr_;
        FinalizerArg argPtr_;
    };

    static EtsFinalizableWeakRef *Create(EtsCoroutine *etsCoroutine);

    static EtsFinalizableWeakRef *FromCoreType(ObjectHeader *finalizableWeakRef)
    {
        return reinterpret_cast<EtsFinalizableWeakRef *>(finalizableWeakRef);
    }

    static const EtsFinalizableWeakRef *FromCoreType(const ObjectHeader *finalizableWeakRef)
    {
        return reinterpret_cast<const EtsFinalizableWeakRef *>(finalizableWeakRef);
    }

    static EtsFinalizableWeakRef *FromEtsObject(EtsObject *finalizableWeakRef)
    {
        return reinterpret_cast<EtsFinalizableWeakRef *>(finalizableWeakRef);
    }

    void SetFinalizer(FinalizerFunc funcPtr, FinalizerArg argPtr)
    {
        finalizerPtr_ = reinterpret_cast<EtsLong>(funcPtr);
        finalizerArgPtr_ = reinterpret_cast<EtsLong>(argPtr);
    }

    Finalizer ReleaseFinalizer()
    {
        return Finalizer(reinterpret_cast<FinalizerFunc>(std::exchange(finalizerPtr_, 0)),
                         reinterpret_cast<FinalizerArg>(std::exchange(finalizerArgPtr_, 0)));
    }

    EtsFinalizableWeakRef *GetPrev() const
    {
        auto *prev = ObjectAccessor::GetObject(this, MEMBER_OFFSET(EtsFinalizableWeakRef, prev_));
        return FromCoreType(prev);
    }

    static constexpr size_t GetPrevOffset()
    {
        return MEMBER_OFFSET(EtsFinalizableWeakRef, prev_);
    }

    EtsFinalizableWeakRef *GetNext() const
    {
        auto *next = ObjectAccessor::GetObject(this, MEMBER_OFFSET(EtsFinalizableWeakRef, next_));
        return FromCoreType(next);
    }

    static constexpr size_t GetNextOffset()
    {
        return MEMBER_OFFSET(EtsFinalizableWeakRef, next_);
    }

    void SetPrev(EtsCoroutine *coro, EtsFinalizableWeakRef *weakRef)
    {
        auto *prev = weakRef != nullptr ? weakRef->GetCoreType() : nullptr;
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsFinalizableWeakRef, prev_), prev);
    }

    void SetNext(EtsCoroutine *coro, EtsFinalizableWeakRef *weakRef)
    {
        auto *next = weakRef != nullptr ? weakRef->GetCoreType() : nullptr;
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsFinalizableWeakRef, next_), next);
    }

private:
    // Such field has the same layout as referent in std.core.FinalizableWeakRef class
    ObjectPointer<EtsFinalizableWeakRef> prev_;
    ObjectPointer<EtsFinalizableWeakRef> next_;
    EtsLong finalizerPtr_;
    EtsLong finalizerArgPtr_;

    friend class test::EtsFinalizableWeakRefTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINALIZABLE_WEAK_REF_H
