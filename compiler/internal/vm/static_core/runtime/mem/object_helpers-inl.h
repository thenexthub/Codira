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

#ifndef PANDA_RUNTIME_MEM_OBJECT_HELPERS_INL_H
#define PANDA_RUNTIME_MEM_OBJECT_HELPERS_INL_H

#include "runtime/mem/object_helpers.h"
#include "runtime/include/class.h"
#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/coretypes/dyn_objects.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/include/hclass.h"

namespace ark::mem {

bool GCStaticObjectHelpers::IsClassObject(ObjectHeader *obj)
{
    return obj->ClassAddr<Class>()->IsClassClass();
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCStaticObjectHelpers::TraverseClass(Class *cls, Handler &handler)
{
    // Iterate over static fields
    uint32_t refNum = cls->GetRefFieldsNum<true>();
    if (refNum == 0) {
        return true;
    }
    uint32_t offset = cls->GetRefFieldsOffset<true>();
    ObjectHeader *object = cls->GetManagedObject();
    ASSERT(ToUintPtr(cls) + offset >= ToUintPtr(object));
    // The offset is relative to the class. Adjust it to make relative to the managed object
    uint32_t objOffset = ToUintPtr(cls) + offset - ToUintPtr(object);
    uint32_t refVolatileNum = cls->GetVolatileRefFieldsNum<true>();
    for (uint32_t i = 0; i < refNum;
         i++, offset += ClassHelper::OBJECT_POINTER_SIZE, objOffset += ClassHelper::OBJECT_POINTER_SIZE) {
        bool isVolatile = (i < refVolatileNum);
        auto *fieldObject = isVolatile ? cls->GetFieldObject<true>(offset) : cls->GetFieldObject<false>(offset);
        if (fieldObject == nullptr) {
            continue;
        }
        [[maybe_unused]] bool res = handler(object, fieldObject, objOffset, isVolatile);
        if constexpr (INTERRUPTIBLE) {
            if (!res) {
                return false;
            }
        }
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCStaticObjectHelpers::TraverseObject(ObjectHeader *object, Class *cls, Handler &handler)
{
    ASSERT(cls != nullptr);
    ASSERT(!cls->IsDynamicClass());
    // CC-OFFNXT(G.CTL.03): false positive
    while (true) {
        // Iterate over instance fields
        uint32_t refNum = cls->GetRefFieldsNum<false>();
        if (refNum == 0) {
            if (cls->HaveNoRefsInParents()) {
                break;
            }
            cls = cls->GetBase();
            continue;
        }

        uint32_t offset = cls->GetRefFieldsOffset<false>();
        uint32_t refVolatileNum = cls->GetVolatileRefFieldsNum<false>();
        for (uint32_t i = 0; i < refNum; i++, offset += ClassHelper::OBJECT_POINTER_SIZE) {
            bool isVolatile = (i < refVolatileNum);
            auto *fieldObject =
                isVolatile ? object->GetFieldObject<true>(offset) : object->GetFieldObject<false>(offset);
            if (fieldObject == nullptr) {
                continue;
            }
            ValidateObject(object, fieldObject);
            [[maybe_unused]] bool res = handler(object, fieldObject, offset, isVolatile);
            if constexpr (!INTERRUPTIBLE) {
                continue;
            }
            if (!res) {
                return false;
            }
        }

        if (cls->HaveNoRefsInParents()) {
            break;
        }
        cls = cls->GetBase();
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCStaticObjectHelpers::TraverseArray(coretypes::Array *array, [[maybe_unused]] Class *cls, void *begin, void *end,
                                          Handler &handler)
{
    ASSERT(cls != nullptr);
    ASSERT(!cls->IsDynamicClass());
    ASSERT(cls->IsObjectArrayClass());
    ASSERT(IsAligned(ToUintPtr(begin), DEFAULT_ALIGNMENT_IN_BYTES));

    auto *arrayStart = array->GetBase<ObjectPointerType *>();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *arrayEnd = arrayStart + array->GetLength();
    auto *p = begin < arrayStart ? arrayStart : reinterpret_cast<ObjectPointerType *>(begin);

    if (end > arrayEnd) {
        end = arrayEnd;
    }

    auto elementSize = coretypes::Array::GetElementSize<ObjectHeader *, false>();
    auto offset = ToUintPtr(p) - ToUintPtr(array);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (ArraySizeT i = p - arrayStart; p < end; ++p, ++i, offset += elementSize) {
        auto *arrayElement = array->Get<ObjectHeader *>(i);
        if (arrayElement != nullptr) {
            [[maybe_unused]] bool res = handler(array, arrayElement, offset, false);
            if constexpr (!INTERRUPTIBLE) {
                continue;
            }
            if (!res) {
                return false;
            }
        }
    }

    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCStaticObjectHelpers::TraverseAllObjectsWithInfo(ObjectHeader *objectHeader, Handler &handler, void *begin,
                                                       void *end)
{
    auto *cls = objectHeader->ClassAddr<Class>();
    ASSERT(cls != nullptr);

    if (cls->IsObjectArrayClass()) {
        return TraverseArray<INTERRUPTIBLE>(static_cast<coretypes::Array *>(objectHeader), cls, begin, end, handler);
    }
    if (cls->IsClassClass()) {
        auto objectCls = ark::Class::FromClassObject(objectHeader);
        if (objectCls->IsInitializing() || objectCls->IsInitialized()) {
            if (!TraverseClass<INTERRUPTIBLE>(objectCls, handler)) {
                return false;
            }
        }
    }
    return TraverseObject<INTERRUPTIBLE>(objectHeader, cls, handler);
}

bool GCDynamicObjectHelpers::IsClassObject(ObjectHeader *obj)
{
    return obj->ClassAddr<HClass>()->IsHClass();
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCDynamicObjectHelpers::TraverseClass(coretypes::DynClass *dynClass, Handler &handler)
{
    size_t hklassSize = dynClass->ClassAddr<HClass>()->GetObjectSize() - sizeof(coretypes::DynClass);
    size_t bodySize = hklassSize - sizeof(HClass);
    size_t numOfFields = bodySize / TaggedValue::TaggedTypeSize();
    for (size_t i = 0; i < numOfFields; i++) {
        size_t fieldOffset = sizeof(ObjectHeader) + sizeof(HClass) + i * TaggedValue::TaggedTypeSize();
        auto taggedValue = ObjectAccessor::GetDynValue<TaggedValue>(dynClass, fieldOffset);
        if (taggedValue.IsHeapObject()) {
            [[maybe_unused]] bool res = handler(dynClass, taggedValue.GetHeapObject(), fieldOffset, false);
            if constexpr (!INTERRUPTIBLE) {
                continue;
            }
            if (!res) {
                return false;
            }
        }
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCDynamicObjectHelpers::TraverseObject(ObjectHeader *object, HClass *cls, Handler &handler)
{
    ASSERT(cls->IsDynamicClass());
    LOG(DEBUG, GC) << "TraverseObject Current object: " << GetDebugInfoAboutObject(object);
    // handle object data
    uint32_t objBodySize = cls->GetObjectSize() - ObjectHeader::ObjectHeaderSize();
    ASSERT(objBodySize % TaggedValue::TaggedTypeSize() == 0);
    uint32_t numOfFields = objBodySize / TaggedValue::TaggedTypeSize();
    size_t dataOffset = ObjectHeader::ObjectHeaderSize();
    for (uint32_t i = 0; i < numOfFields; i++) {
        size_t fieldOffset = dataOffset + i * TaggedValue::TaggedTypeSize();
        if (cls->IsNativeField(fieldOffset)) {
            continue;
        }
        auto taggedValue = ObjectAccessor::GetDynValue<TaggedValue>(object, fieldOffset);
        if (taggedValue.IsHeapObject()) {
            [[maybe_unused]] bool res = handler(object, taggedValue.GetHeapObject(), fieldOffset, false);
            if constexpr (!INTERRUPTIBLE) {
                continue;
            }
            if (!res) {
                return false;
            }
        }
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCDynamicObjectHelpers::TraverseArray(coretypes::Array *array, [[maybe_unused]] HClass *cls, void *begin,
                                           void *end, Handler &handler)
{
    ASSERT(cls != nullptr);
    ASSERT(cls->IsDynamicClass());
    ASSERT(cls->IsArray());
    ASSERT(IsAligned(ToUintPtr(begin), DEFAULT_ALIGNMENT_IN_BYTES));

    auto *arrayStart = array->GetBase<TaggedType *>();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *arrayEnd = arrayStart + array->GetLength();
    auto *p = begin < arrayStart ? arrayStart : reinterpret_cast<TaggedType *>(begin);

    if (end > arrayEnd) {
        end = arrayEnd;
    }

    auto elementSize = coretypes::Array::GetElementSize<TaggedType, true>();
    auto offset = ToUintPtr(p) - ToUintPtr(array);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (ArraySizeT i = p - arrayStart; p < end; ++p, ++i, offset += elementSize) {
        TaggedValue arrayElement(array->Get<TaggedType, false, true>(i));
        if (arrayElement.IsHeapObject()) {
            [[maybe_unused]] bool res = handler(array, arrayElement.GetHeapObject(), offset, false);
            if constexpr (!INTERRUPTIBLE) {
                continue;
            }
            if (!res) {
                return false;
            }
        }
    }

    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCDynamicObjectHelpers::TraverseAllObjectsWithInfo(ObjectHeader *objectHeader, Handler &handler, void *begin,
                                                        void *end)
{
    auto *cls = objectHeader->ClassAddr<HClass>();
    ASSERT(cls != nullptr && cls->IsDynamicClass());
    if (cls->IsString() || cls->IsNativePointer()) {
        return true;
    }
    if (cls->IsArray()) {
        return TraverseArray<INTERRUPTIBLE>(static_cast<coretypes::Array *>(objectHeader), cls, begin, end, handler);
    }
    if (cls->IsHClass()) {
        return TraverseClass<INTERRUPTIBLE>(coretypes::DynClass::Cast(objectHeader), handler);
    }

    return TraverseObject<INTERRUPTIBLE>(objectHeader, cls, handler);
}

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_OBJECT_HELPERS_INL_H
