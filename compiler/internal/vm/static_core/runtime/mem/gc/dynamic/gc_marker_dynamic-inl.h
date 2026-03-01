/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_MEM_GC_DYNAMIC_GC_MARKER_DYNAMIC_INL_H
#define PANDA_RUNTIME_MEM_GC_DYNAMIC_GC_MARKER_DYNAMIC_INL_H

#include "runtime/mem/gc/gc_marker.h"
#include "runtime/include/coretypes/dyn_objects.h"

namespace ark::mem {
template <typename Marker>
void GCMarker<Marker, LANG_TYPE_DYNAMIC>::HandleObject(GCMarkingStackType *objectsStack, const ObjectHeader *object,
                                                       const BaseClass *baseCls)
{
    ASSERT(baseCls->IsDynamicClass());
    auto *cls = static_cast<const HClass *>(baseCls);
    // Handle dyn_class
    ObjectHeader *dynClass = cls->GetManagedObject();
    if (AsMarker()->MarkIfNotMarked(dynClass)) {
        objectsStack->PushToStack(object, dynClass);
    }
    // mark object data
    uint32_t objBodySize = cls->GetObjectSize() - ObjectHeader::ObjectHeaderSize();
    ASSERT(objBodySize % TaggedValue::TaggedTypeSize() == 0);
    uint32_t numOfFields = objBodySize / TaggedValue::TaggedTypeSize();
    size_t startAddr = reinterpret_cast<uintptr_t>(object) + ObjectHeader::ObjectHeaderSize();
    for (uint32_t i = 0; i < numOfFields; i++) {
        uint32_t fieldOffset = i * TaggedValue::TaggedTypeSize();
        if (cls->IsNativeField(ObjectHeader::ObjectHeaderSize() + fieldOffset)) {
            continue;
        }
        auto *fieldAddr = reinterpret_cast<std::atomic<TaggedType> *>(startAddr + fieldOffset);
        // Atomic with relaxed order reason: to correct read the value
        TaggedValue taggedValue(fieldAddr->load(std::memory_order_relaxed));
        if (!taggedValue.IsHeapObject()) {
            continue;
        }

        ObjectHeader *objectHeader = taggedValue.GetHeapObject();
        if (AsMarker()->MarkIfNotMarked(objectHeader)) {
            objectsStack->PushToStack(object, objectHeader);
        }
    }
}

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_DYNAMIC>::HandleClass(GCMarkingStackType *objectsStack, const coretypes::DynClass *cls)
{
    // mark Hclass Data & Prototype
    HClass *klass = const_cast<coretypes::DynClass *>(cls)->GetHClass();
    // klass_size is sizeof DynClass include JSHClass, which is saved in root DynClass.
    size_t klassSize = cls->ClassAddr<HClass>()->GetObjectSize();

    uintptr_t startAddr = reinterpret_cast<uintptr_t>(klass) + sizeof(HClass);
    size_t bodySize = klassSize - sizeof(coretypes::DynClass) - sizeof(HClass);
    size_t numOfFields = bodySize / TaggedValue::TaggedTypeSize();
    for (size_t i = 0; i < numOfFields; i++) {
        auto *addr = reinterpret_cast<std::atomic<TaggedType> *>(startAddr + i * TaggedValue::TaggedTypeSize());
        // Atomic with relaxed order reason: to correct read the value
        coretypes::TaggedValue taggedValue(addr->load(std::memory_order_relaxed));
        if (!taggedValue.IsHeapObject()) {
            continue;
        }
        ObjectHeader *objectHeader = taggedValue.GetHeapObject();
        if (AsMarker()->MarkIfNotMarked(objectHeader)) {
            objectsStack->PushToStack(cls, objectHeader);
        }
    }
}

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_DYNAMIC>::HandleArrayClass(GCMarkingStackType *objectsStack,
                                                           const coretypes::Array *arrayObject,
                                                           [[maybe_unused]] const BaseClass *cls)
{
    LOG(DEBUG, GC) << "Dyn Array object: " << GetDebugInfoAboutObject(arrayObject);
    ArraySizeT arrayLength = arrayObject->GetLength();
    ASSERT(cls->IsDynamicClass());
    for (coretypes::ArraySizeT i = 0; i < arrayLength; i++) {
        TaggedValue arrayElement(arrayObject->Get<TaggedType, true, true>(i));
        if (!arrayElement.IsHeapObject()) {
            continue;
        }
        ObjectHeader *elementObject = arrayElement.GetHeapObject();
        if (AsMarker()->MarkIfNotMarked(elementObject)) {
            objectsStack->PushToStack(arrayObject, elementObject);
        }
    }
}

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_DYNAMIC>::MarkInstance(GCMarkingStackType *objectsStack, const ObjectHeader *object,
                                                       const BaseClass *baseCls,
                                                       const ReferenceCheckPredicateT &refPred)
{
    ASSERT(baseCls->IsDynamicClass());
    if (GetGC()->IsReference(nullptr, object, refPred)) {
        GetGC()->ProcessReference(objectsStack, nullptr, object, GC::EmptyReferenceProcessPredicate);
        return;
    }
    MarkInstance(objectsStack, object, baseCls);
}

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_DYNAMIC>::MarkInstance(GCMarkingStackType *objectsStack, const ObjectHeader *object,
                                                       const BaseClass *baseCls)
{
    ASSERT(baseCls->IsDynamicClass());
    auto *cls = static_cast<const HClass *>(baseCls);
    // push to stack after marked, so just return here.
    if (cls->IsNativePointer() || cls->IsString()) {
        return;
    }
    if (cls->IsHClass()) {
        auto dynClass = static_cast<const ark::coretypes::DynClass *>(object);
        HandleClass(objectsStack, dynClass);
    } else if (cls->IsArray()) {
        auto *arrayObject = static_cast<const ark::coretypes::Array *>(object);
        HandleArrayClass(objectsStack, arrayObject, cls);
    } else {
        HandleObject(objectsStack, object, cls);
    }
}
}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_DYNAMIC_GC_MARKER_DYNAMIC_INL_H
