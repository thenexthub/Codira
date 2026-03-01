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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ATOMIC_INT_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ATOMIC_INT_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets {

namespace test {
class EtsAtomicIntMembers;
}  // namespace test

class EtsAtomicInt : public EtsObject {
public:
    static EtsAtomicInt *FromCoreType(ObjectHeader *v)
    {
        return reinterpret_cast<EtsAtomicInt *>(v);
    }

    static const EtsAtomicInt *FromCoreType(const ObjectHeader *v)
    {
        return reinterpret_cast<const EtsAtomicInt *>(v);
    }

    ObjectHeader *GetCoreType()
    {
        return reinterpret_cast<ObjectHeader *>(this);
    }

    const ObjectHeader *GetCoreType() const
    {
        return reinterpret_cast<const ObjectHeader *>(this);
    }

    EtsObject *AsObject()
    {
        return reinterpret_cast<EtsObject *>(this);
    }

    const EtsObject *AsObject() const
    {
        return reinterpret_cast<const EtsObject *>(this);
    }

    static EtsAtomicInt *FromEtsObject(EtsObject *v)
    {
        return reinterpret_cast<EtsAtomicInt *>(v);
    }

    static const EtsAtomicInt *FromEtsObject(const EtsObject *v)
    {
        return reinterpret_cast<const EtsAtomicInt *>(v);
    }

    void SetValue(EtsInt v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        ObjectAccessor::SetFieldPrimitive<EtsInt>(this, GetValueOffset(), v, std::memory_order_seq_cst);
    }

    EtsInt GetValue() const
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        return ObjectAccessor::GetFieldPrimitive<EtsInt>(this, GetValueOffset(), std::memory_order_seq_cst);
    }

    EtsInt GetAndSet(EtsInt v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        return ObjectAccessor::GetAndSetFieldPrimitive<EtsInt>(this, GetValueOffset(), v, std::memory_order_seq_cst);
    }

    EtsInt CompareAndSet(EtsInt expected, EtsInt newValue)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        auto result = ObjectAccessor::CompareAndSetFieldPrimitive<EtsInt>(this, GetValueOffset(), expected, newValue,
                                                                          std::memory_order_seq_cst, true);
        return result.second;
    }

    EtsInt FetchAndAdd(EtsInt v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        return ObjectAccessor::GetAndAddFieldPrimitive<EtsInt>(this, GetValueOffset(), v, std::memory_order_seq_cst);
    }

    EtsInt FetchAndSub(EtsInt v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        return ObjectAccessor::GetAndSubFieldPrimitive<EtsInt>(this, GetValueOffset(), v, std::memory_order_seq_cst);
    }

    static constexpr size_t GetValueOffset()
    {
        return MEMBER_OFFSET(EtsAtomicInt, value_);
    }

    EtsAtomicInt() = delete;
    ~EtsAtomicInt() = delete;

private:
    NO_COPY_SEMANTIC(EtsAtomicInt);
    NO_MOVE_SEMANTIC(EtsAtomicInt);

    EtsInt value_;
    friend class test::EtsAtomicIntMembers;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ATOMIC_INT_H
