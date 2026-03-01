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


#ifndef MRT_MARRAY_INLINE_H
#define MRT_MARRAY_INLINE_H

#include "Inspector/CodeAllocData.h"
// model interface
#include "ExceptionManager.h"
#include "Heap/Barrier/Barrier.inline.h"
#include "Heap/Heap.h"
#include "HeapManager.inline.h"
// module internal interfaces
#include "MArray.h"
#include "MClass.inline.h"

namespace MapleRuntime {
constexpr MOffset MArray::GetContentOffset() { return sizeof(MArray); }

inline MIndex MArray::GetLength() const { return length; }

inline void MArray::SetLength(MIndex number) { length = number; }

inline U8* MArray::ConvertToCArray() const
{
    return reinterpret_cast<uint8_t*>(reinterpret_cast<Uptr>(this) + MArray::GetContentOffset());
}

inline MIndex MArray::GetMArraySize() const { return (MArray::GetContentOffset() + GetContentSize()); }

inline MSize MArray::GetElementSize() const
{
    TypeInfo* componentTypeInfo = GetComponentTypeInfo();
    if (componentTypeInfo->IsRef()) {
        return sizeof(BaseObject*);
    }
    return componentTypeInfo->GetComponentSize();
}

inline MIndex MArray::GetContentSize() const
{
    auto len = GetLength();
    auto elementSize = GetElementSize();
    return elementSize * len;
}

inline bool MArray::IsPrimitiveArray() const
{
    TypeInfo* componentTypeInfo = GetTypeInfo()->GetComponentTypeInfo();
    return componentTypeInfo == nullptr ? false : componentTypeInfo->IsPrimitiveType();
}

inline ObjectPtr MArray::GetRefElement(MIndex index)
{
    RefField<>& ref = GetRefField(MArray::GetContentOffset() + RefField<>::GetSize() * index);
    return Heap::GetBarrier().ReadReference(this, ref);
}

inline void MArray::SetRefElement(MIndex index, const ObjectPtr mObj)
{
    RefField<>& ref = GetRefField(MArray::GetContentOffset() + RefField<>::GetSize() * index);
    Heap::GetBarrier().WriteReference(this, ref, mObj);
}

template<typename T>
inline T MArray::GetPrimitiveElement(MIndex index) const
{
    Field<T>& field = GetField<T>(MArray::GetContentOffset() + GetElementSize() * index);
    // normally we do not barrier for reading primitive fields.
    return field.GetFieldValue();
}

template<typename T>
inline void MArray::SetPrimitiveElement(MIndex index, T value)
{
    Field<T>& field = GetField<T>(MArray::GetContentOffset() + GetElementSize() * index);
    Heap::GetBarrier().WriteField(this, field, value);
}

static inline MIndex CalculateArraySize(MIndex nElems, const U32 elemBytes)
{
    if (elemBytes == 0) {
        return static_cast<U64>(MArray::GetContentOffset());
    }
    U64 maxLength = (MAX_ARRAY_SIZE - MArray::GetContentOffset()) / elemBytes;
    U64 totalSize = static_cast<U64>(nElems) * elemBytes + static_cast<U64>(MArray::GetContentOffset());
    if (UNLIKELY(nElems >= maxLength)) {
        // check overflow
        return MAX_ARRAY_SIZE;
    }
    return static_cast<MIndex>(totalSize);
}

inline MArray* MArray::NewArray(MIndex nElems, TypeInfo& arrayClass, AllocType allocType)
{
    DCHECK_D(arrayClass.IsArrayType(), "Expect an array type");
    MArray* newArray = nullptr;
    if (arrayClass.GetComponentTypeInfo()->IsObjectType()) {
        newArray = NewRefArray(nElems, arrayClass, allocType);
    } else {
        // component is value type
        auto elem = arrayClass.GetSuperTypeInfo();
        auto elemBits = sizeof(void*);
        if (!elem->IsRef()) {
            elemBits = arrayClass.GetSuperTypeInfo()->GetComponentSize();
        }
        newArray = NewKnownWidthArray(nElems, arrayClass, elemBits, allocType);
    }
    return newArray;
}

inline MArray* MArray::NewRefArray(MIndex nElems, TypeInfo& arrayClass, AllocType allocType)
{
    return NewKnownWidthArray(nElems, arrayClass, RefField<>::GetSize(), allocType);
}

inline MArray* MArray::NewKnownWidthArray(MIndex nElems, TypeInfo& arrayClass, const U32 elemBytes, AllocType allocType)
{
    DCHECK_D(arrayClass.IsArrayType(), "Expect an array type");
    MIndex arraySize = CalculateArraySize(nElems, elemBytes);
    if (UNLIKELY(arraySize == MAX_ARRAY_SIZE || arraySize > Heap::GetHeap().GetMaxCapacity())) {
        ExceptionManager::OutOfMemory();
        return nullptr;
    }
    auto address = HeapManager::Allocate(arraySize, allocType);
    if (LIKELY(address != NULL_ADDRESS)) {
        MArray* newArray = reinterpret_cast<MArray*>(SetClassInfo(address, &arrayClass));
        newArray->SetLength(nElems);
#if defined(__OHOS__) && (__OHOS__ == 1)
        if (CodeAllocData::GetCodeAllocData()->IsRecording()) {
            CodeAllocData::GetCodeAllocData()->RecordAllocNodes(&arrayClass, arraySize);
        }
#endif
        return newArray;
    }
    return nullptr;
}
} // namespace MapleRuntime
#endif // MRT_MARRAY_INLINE_H
