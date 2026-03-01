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
#include "common_components/heap/ark_collector/idle_barrier.h"

#include "common_components/mutator/mutator.h"
#if defined(COMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif
#include "securec.h"

namespace common {
BaseObject* IdleBarrier::ReadRefField(BaseObject* obj, RefField<false>& field) const
{
    RefField<> oldField(field);
    return (BaseObject*)(oldField.GetFieldValue());
}

BaseObject* IdleBarrier::ReadStaticRef(RefField<false>& field) const { return ReadRefField(nullptr, field); }

BaseObject* IdleBarrier::AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    RefField<false> oldField(field.GetFieldValue(order));
    BaseObject* target = (BaseObject*)(oldField.GetFieldValue());
    DLOG(BARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
    return target;
}

void IdleBarrier::ReadStruct(HeapAddress dst, BaseObject* obj, HeapAddress src, size_t size) const
{
    LOGF_CHECK(memcpy_s(reinterpret_cast<void*>(dst), size, reinterpret_cast<void*>(src), size) == EOK) <<
        "read struct memcpy_s failed";
}

void IdleBarrier::AtomicWriteRefField(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                      MemoryOrder order) const
{
    if (obj != nullptr) {
        DLOG(BARRIER, "atomic write obj %p<%p>(%zu) ref@%p: %p", obj, obj->GetTypeInfo(), obj->GetSize(), &field,
             newRef);
    } else {
        DLOG(BARRIER, "atomic write static ref@%p: %p", &field, newRef);
    }
    field.SetTargetObject(newRef, order);
}

BaseObject* IdleBarrier::AtomicSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                            MemoryOrder order) const
{
    // newRef must be the latest versions.
    HeapAddress oldValue = field.Exchange(newRef, order);
    RefField<> oldField(oldValue);
    BaseObject* oldRef = ReadRefField(nullptr, oldField);
    DLOG(BARRIER, "atomic swap obj %p<%p>(%zu) ref@%p: old %#zx(%p), new %#zx(%p)", obj, obj->GetTypeInfo(),
         obj->GetSize(), &field, oldValue, oldRef, field.GetFieldValue(order), newRef);
    return oldRef;
}

bool IdleBarrier::CompareAndSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* oldRef,
                                         BaseObject* newRef, MemoryOrder sOrder, MemoryOrder fOrder) const
{
    HeapAddress oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
    RefField<false> oldField(oldFieldValue);
    BaseObject* oldVersion = ReadRefField(nullptr, oldField);

    // oldRef and newRef must be the latest versions.
    while (oldVersion == oldRef) {
        RefField<> newField(newRef);
        if (field.CompareExchange(oldFieldValue, newField.GetFieldValue(), sOrder, fOrder)) {
            return true;
        }
        oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
        RefField<false> tmp(oldFieldValue);
        oldVersion = ReadRefField(nullptr, tmp);
    }
    return false;
}

void IdleBarrier::UpdateRememberSet(BaseObject* object, BaseObject* ref) const
{
    DCHECK_CC(object != nullptr);
    RegionDesc::InlinedRegionMetaData *objMetaRegion = RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(
        reinterpret_cast<uintptr_t>(object));
    RegionDesc::InlinedRegionMetaData *refMetaRegion = RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(
        reinterpret_cast<uintptr_t>(ref));
    if (!objMetaRegion->IsInYoungSpaceForWB() && refMetaRegion->IsInYoungSpaceForWB()) {
        if (objMetaRegion->MarkRSetCardTable(object)) {
            DLOG(BARRIER, "update point-out remember set of region %p, obj %p, ref: %p<%p>",
                 objMetaRegion->GetRegionDesc(), object, ref, ref->GetTypeInfo());
        }
    }
}

void IdleBarrier::WriteRoot(BaseObject *obj) const
{
    DLOG(BARRIER, "write root obj %p", obj);
}

void IdleBarrier::WriteRefField(BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    DLOG(BARRIER, "write obj %p ref@%p: %p => %p", obj, &field, field.GetTargetObject(), ref);
    if (Heap::IsTaggedObject((HeapAddress)ref)) {
        UpdateRememberSet(obj, ref);
    }
    field.SetTargetObject(ref);
}

void IdleBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    if (!Heap::IsTaggedObject((HeapAddress)ref)) {
        return;
    }
    UpdateRememberSet(obj, ref);
    DLOG(BARRIER, "write obj %p ref@%p: %p => %p", obj, &field, field.GetTargetObject(), ref);
}

void IdleBarrier::WriteStruct(BaseObject* obj, HeapAddress dst, size_t dstLen, HeapAddress src, size_t srcLen) const
{
    CHECK_CC(memcpy_s(reinterpret_cast<void*>(dst), dstLen, reinterpret_cast<void*>(src), srcLen) == EOK);
#if defined(COMMON_TSAN_SUPPORT)
    CHECK_CC(srcLen == dstLen);
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dst), dstLen);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(src), srcLen);
#endif
}

void IdleBarrier::WriteStaticRef(RefField<false>& field, BaseObject* ref) const { WriteRefField(nullptr, field, ref); }

void IdleBarrier::CopyStructArray(BaseObject* dstObj, HeapAddress dstField, MIndex dstSize, BaseObject* srcObj,
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
