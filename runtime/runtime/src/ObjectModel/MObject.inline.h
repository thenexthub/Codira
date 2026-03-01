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

#ifndef MRT_MOBJECT_INLINE_H
#define MRT_MOBJECT_INLINE_H

// language dependence
#include <type_traits>

// cross module dependence
#include "Heap/Barrier/Barrier.inline.h"
#include "Heap/Heap.h"
#include "HeapManager.inline.h"

// module internal dependence
#include "Field.h"
#include "MClass.inline.h"
#include "MObject.h"

namespace MapleRuntime {
template<typename T0, typename T1>
inline T0* MObject::Cast(T1 o)
{
    static_assert(std::is_same<T1, void*>::value || std::is_same<T1, MAddress>::value, "wrong type");
    return reinterpret_cast<T0*>(o);
}

template<typename T0, typename T1>
inline T0* MObject::CastNonNull(T1 o)
{
    DCHECK(o != 0);
    return Cast<T0>(o);
}

inline bool MObject::IsArray() const { return GetTypeInfo()->IsArrayType(); }

inline bool MObject::IsPrimitiveArray() const
{
    TypeInfo* componentTypeInfo = GetTypeInfo()->GetComponentTypeInfo();
    return componentTypeInfo == nullptr ? false : componentTypeInfo->IsPrimitiveType();
}

inline bool MObject::IsStructArray() const
{
    TypeInfo* componentTypeInfo = GetTypeInfo()->GetComponentTypeInfo();
    return componentTypeInfo == nullptr ? false : (componentTypeInfo->IsStructType() || componentTypeInfo->IsTuple());
}

inline bool MObject::IsSubType(TypeInfo& mClass) { return GetTypeInfo()->IsSubType(&mClass); }

template<typename T>
inline T MObject::Load(size_t offset) const
{
    Field<T>& field = GetField<T>(offset);
    return field.GetFieldValue();
}

template<typename T>
inline void MObject::Store(size_t offset, T value)
{
    Field<T>& field = GetField<T>(offset);
    Heap::GetBarrier().WriteField(this, field, value);
}

inline MObject* MObject::LoadRef(size_t offset)
{
    RefField<>& ref = GetRefField<false>(offset);
    return static_cast<MObject*>(Heap::GetBarrier().ReadReference(this, ref));
}

inline void MObject::StoreRef(size_t offset, MObject* value)
{
    RefField<>& ref = GetRefField<false>(offset);
    Heap::GetBarrier().WriteReference(this, ref, value);
}
} // namespace MapleRuntime
#endif // MRT_MOBJECT_INLINE_H
