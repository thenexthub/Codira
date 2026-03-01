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
#include "common_components/heap/ark_collector/post_marking_barrier.h"

#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/mutator/mutator.h"
#include "heap/space/young_space.h"
#if defined(COMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

namespace common {
BaseObject* PostMarkingBarrier::ReadRefField(BaseObject* obj, RefField<false>& field) const
{
    RefField<> tmpField(field);
    return (BaseObject*)tmpField.GetFieldValue();
}

BaseObject* PostMarkingBarrier::ReadStaticRef(RefField<false>& field) const { return ReadRefField(nullptr, field); }

// If the object is still alive, return it; if not, return nullptr
BaseObject* PostMarkingBarrier::ReadStringTableStaticRef(RefField<false> &field) const
{
    // Note: CMC GC assumes all objects in string table are not in young space. Based on the assumption, CMC GC skip
    // read barrier in young GC
    if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
        return reinterpret_cast<BaseObject*>(field.GetFieldValue());
    }

    auto isSurvivor = [](BaseObject* obj) {
        RegionDesc* region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));

        return region->IsMarkedObject(obj) || region->IsNewObjectSinceMarking(obj);
    };

    auto obj = ReadRefField(nullptr, field);
    if (obj != nullptr && isSurvivor(obj)) {
        return obj;
    } else {
        return nullptr;
    }
}

void PostMarkingBarrier::ReadStruct(HeapAddress dst, BaseObject* obj, HeapAddress src, size_t size) const
{
    CHECK_CC(memcpy_s(reinterpret_cast<void*>(dst), size, reinterpret_cast<void*>(src), size) == EOK);
}

void PostMarkingBarrier::WriteRefField(BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    RefField<> newField(ref);
    UpdateRememberSet(obj, ref);
    field.SetFieldValue(newField.GetFieldValue());
}

void PostMarkingBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
        UpdateRememberSet(obj, ref);
    }
}

void PostMarkingBarrier::WriteStaticRef(RefField<false>& field, BaseObject* ref) const
{
    RefField<> newField(ref);
    field.SetFieldValue(newField.GetFieldValue());
}

void PostMarkingBarrier::WriteStruct(BaseObject* obj, HeapAddress dst, size_t dstLen,
                                     HeapAddress src, size_t srcLen) const
{
    CHECK_CC(obj != nullptr);
    CHECK_CC(memcpy_s(reinterpret_cast<void*>(dst), dstLen, reinterpret_cast<void*>(src), srcLen) == EOK);

#if defined(COMMON_TSAN_SUPPORT)
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dst), dstLen);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(src), srcLen);
#endif
}

BaseObject* PostMarkingBarrier::AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    BaseObject* target = nullptr;
    RefField<false> oldField(field.GetFieldValue(order));

    target = ReadRefField(nullptr, oldField);
    DLOG(TBARRIER, "katomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
    return target;
}

void PostMarkingBarrier::AtomicWriteRefField(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                             MemoryOrder order) const
{
    RefField<> oldField(field.GetFieldValue(order));
    HeapAddress oldValue = oldField.GetFieldValue();
    (void)oldValue;
    RefField<> newField(newRef);
    field.SetFieldValue(newField.GetFieldValue(), order);
    if (obj != nullptr) {
        DLOG(TBARRIER, "atomic write obj %p<%p>(%zu) ref@%p: %#zx -> %#zx", obj, obj->GetTypeInfo(), obj->GetSize(),
             &field, oldValue, newField.GetFieldValue());
    } else {
        DLOG(TBARRIER, "atomic write static ref@%p: %#zx -> %#zx", &field, oldValue, newField.GetFieldValue());
    }
}

BaseObject* PostMarkingBarrier::AtomicSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                                   MemoryOrder order) const
{
    RefField<> newField(newRef);
    HeapAddress oldValue = field.Exchange(newField.GetFieldValue(), order);
    RefField<> oldField(oldValue);
    BaseObject* oldRef = oldField.GetTargetObject();
    DLOG(TRACE, "atomic swap obj %p<%p>(%zu) ref-field@%p: old %#zx(%p), new %#zx(%p)", obj, obj->GetTypeInfo(),
         obj->GetSize(), &field, oldValue, oldRef, field.GetFieldValue(), newRef);
    return oldRef;
}

bool PostMarkingBarrier::CompareAndSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* oldRef,
                                                BaseObject* newRef, MemoryOrder succOrder, MemoryOrder failOrder) const
{
    HeapAddress oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
    RefField<false> oldField(oldFieldValue);
    BaseObject* oldVersion = oldField.GetTargetObject();
    RefField<> newField(newRef);
    while (oldVersion == oldRef) {
        if (field.CompareExchange(oldFieldValue, newField.GetFieldValue(), succOrder, failOrder)) {
            return true;
        }
        oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
        RefField<false> tmpField(oldFieldValue);
        oldVersion = tmpField.GetTargetObject();
    }
    return false;
}

void PostMarkingBarrier::CopyStructArray(BaseObject* dstObj, HeapAddress dstField, MIndex dstSize, BaseObject* srcObj,
                                         HeapAddress srcField, MIndex srcSize) const
{
#ifndef NDEBUG
    if (!dstObj->HasRefField()) {
        LOG_COMMON(FATAL) << "array " << dstObj << " doesn't have class-type element";
        return;
    }
#endif
    LOGF_CHECK(memmove_s(reinterpret_cast<void*>(dstField), dstSize, reinterpret_cast<void*>(srcField), srcSize) ==
                     EOK) << "memmove_s failed";

#if defined(COMMON_TSAN_SUPPORT)
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dstField), dstSize);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(srcField), srcSize);
#endif
}
} // namespace common
