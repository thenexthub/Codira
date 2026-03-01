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

#ifndef PANDA_RUNTIME_MEM_GC_STATIC_GC_MARKER_STATIC_INL_H
#define PANDA_RUNTIME_MEM_GC_STATIC_GC_MARKER_STATIC_INL_H

#include "runtime/mem/gc/gc_marker.h"
#include "runtime/mem/gc/workers/gc_workers_task_pool.h"

namespace ark::mem {

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_STATIC>::HandleObject(GCMarkingStackType *objectsStack, const ObjectHeader *object,
                                                      const Class *cls)
{
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
        for (uint32_t i = 0; i < refVolatileNum; i++, offset += ClassHelper::OBJECT_POINTER_SIZE) {
            auto *fieldObject = object->GetFieldObject<true>(offset);
            ValidateObject(object, fieldObject);
            if (fieldObject != nullptr && AsMarker()->MarkIfNotMarked(fieldObject)) {
                objectsStack->PushToStack(object, fieldObject);
            }
        }
        for (uint32_t i = refVolatileNum; i < refNum; i++, offset += ClassHelper::OBJECT_POINTER_SIZE) {
            auto *fieldObject = object->GetFieldObject<false>(offset);
            ValidateObject(object, fieldObject);
            if (fieldObject != nullptr && AsMarker()->MarkIfNotMarked(fieldObject)) {
                objectsStack->PushToStack(object, fieldObject);
            }
        }
        if (cls->HaveNoRefsInParents()) {
            break;
        }
        cls = cls->GetBase();
    }
}
template <typename Marker>
void GCMarker<Marker, LANG_TYPE_STATIC>::HandleClass(GCMarkingStackType *objectsStack, const Class *cls)
{
    // Iterate over static fields
    uint32_t refNum = cls->GetRefFieldsNum<true>();
    if (refNum > 0) {
        uint32_t offset = cls->GetRefFieldsOffset<true>();
        uint32_t refVolatileNum = cls->GetVolatileRefFieldsNum<true>();
        for (uint32_t i = 0; i < refVolatileNum; i++, offset += ClassHelper::OBJECT_POINTER_SIZE) {
            auto *fieldObject = cls->GetFieldObject<true>(offset);
            if (fieldObject != nullptr && AsMarker()->MarkIfNotMarked(fieldObject)) {
                objectsStack->PushToStack(cls->GetManagedObject(), fieldObject);
            }
        }
        for (uint32_t i = refVolatileNum; i < refNum; i++, offset += ClassHelper::OBJECT_POINTER_SIZE) {
            auto *fieldObject = cls->GetFieldObject<false>(offset);
            if (fieldObject != nullptr && AsMarker()->MarkIfNotMarked(fieldObject)) {
                objectsStack->PushToStack(cls->GetManagedObject(), fieldObject);
            }
        }
    }
}

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_STATIC>::HandleArrayClass(GCMarkingStackType *objectsStack,
                                                          const coretypes::Array *arrayObject,
                                                          [[maybe_unused]] const Class *cls)
{
    LOG(DEBUG, GC) << "Array object: " << GetDebugInfoAboutObject(arrayObject);
    ArraySizeT arrayLength = arrayObject->GetLength();

    ASSERT(cls->IsObjectArrayClass());

    LOG(DEBUG, GC) << "Iterate over: " << arrayLength << " elements in array";
    size_t currentPartition = 0;
    size_t partitionSize = 0;
    constexpr size_t THRESHOLD_ARRAY_SIZE = 50000U;
    auto *gc = GetGC();
    bool useGcWorkers = gc->GetSettings()->ParallelMarkingEnabled();
    auto taskType = objectsStack->GetTaskType();
    ASSERT(taskType != GCWorkersTaskTypes::TASK_HUGE_ARRAY_MARKING_REMARK);
    if (arrayLength >= THRESHOLD_ARRAY_SIZE && useGcWorkers && taskType == GCWorkersTaskTypes::TASK_REMARK) {
        size_t gcWorkersNum = Runtime::GetOptions().GetTaskmanagerWorkersCount();
        ASSERT(gcWorkersNum > 0);
        ArraySizeT arrayPartitionsNumber = gcWorkersNum;
        partitionSize = arrayLength / arrayPartitionsNumber;
        auto allocator = gc->GetInternalAllocator();
        // The rest of the array (last partition till the end of the array) is going to be traversed in current worker
        for (; currentPartition < arrayPartitionsNumber - 1; ++currentPartition) {
            auto *newStack = allocator->template New<GCAdaptiveMarkingStack>(
                gc, useGcWorkers ? gc->GetSettings()->GCRootMarkingStackMaxSize() : 0,
                useGcWorkers ? gc->GetSettings()->GCWorkersMarkingStackMaxSize() : 0,
                GCWorkersTaskTypes::TASK_HUGE_ARRAY_MARKING_REMARK);
            ASSERT(newStack != nullptr);
            static_cast<GCAdaptiveStack<ObjectHeader *> *>(newStack)->PushToStack(
                const_cast<coretypes::Array *>(arrayObject));
            size_t taskStartIndex = currentPartition * partitionSize;
            void *markingRangeInfo =
                allocator->template New<std::pair<size_t, size_t>>(taskStartIndex, taskStartIndex + partitionSize);
            newStack->SetAdditionalMarkingInfo(markingRangeInfo);
            if (!gc->GetWorkersTaskPool()->AddTask(GCMarkWorkersTask(newStack->GetTaskType(), newStack))) {
                // Couldn't create a new task. Going to process the rest of the array here
                allocator->Delete(newStack);
                break;
            }
        }
    }
    for (coretypes::ArraySizeT i = currentPartition * partitionSize; i < arrayLength; i++) {
        auto *arrayElement = arrayObject->Get<ObjectHeader *>(i);
        if (arrayElement == nullptr) {
            continue;
        }
#ifndef NDEBUG
        auto arrayElementCls = arrayElement->ClassAddr<Class>();
        LOG_IF(arrayElementCls == nullptr, ERROR, GC)
            << " object's class is nullptr: " << arrayElement << " from array: " << arrayObject;
        ASSERT(arrayElementCls != nullptr);
#endif
        if (AsMarker()->MarkIfNotMarked(arrayElement)) {
            objectsStack->PushToStack(arrayObject, arrayElement);
        }
    }
}

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_STATIC>::MarkInstance(GCMarkingStackType *objectsStack, const ObjectHeader *object,
                                                      const BaseClass *baseCls, const ReferenceCheckPredicateT &refPred)
{
    ASSERT(!baseCls->IsDynamicClass());
    auto cls = static_cast<const Class *>(baseCls);
    if (GetGC()->IsReference(cls, object, refPred)) {
        GetGC()->ProcessReference(objectsStack, cls, object, GC::EmptyReferenceProcessPredicate);
    } else {
        MarkInstance(objectsStack, object, baseCls);
    }
}

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_STATIC>::MarkInstance(GCMarkingStackType *objectsStack, const ObjectHeader *object,
                                                      const BaseClass *baseCls)
{
    ASSERT(!baseCls->IsDynamicClass());
    const auto *cls = static_cast<const Class *>(baseCls);
    if (cls->IsObjectArrayClass()) {
        auto *arrayObject = static_cast<const ark::coretypes::Array *>(object);
        HandleArrayClass(objectsStack, arrayObject, cls);
    } else if (cls->IsClassClass()) {
        // Handle Class handles static fields only, so we need to Handle regular fields explicitly too
        auto objectCls = ark::Class::FromClassObject(object);
        ASSERT(objectCls != nullptr);
        if (objectCls->IsInitializing() || objectCls->IsInitialized()) {
            HandleClass(objectsStack, objectCls);
        }
        HandleObject(objectsStack, object, cls);
    } else if (cls->IsInstantiable()) {
        HandleObject(objectsStack, object, cls);
    } else {
        if (!cls->IsPrimitive()) {
            LOG(FATAL, GC) << "Wrong handling, missed type: " << cls->GetDescriptor();
        }
    }
}

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_STATIC_GC_MARKER_STATIC_INL_H
