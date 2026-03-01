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
#include "common_components/heap/ark_collector/remark_barrier.h"
#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/mutator/mutator.h"
#if defined(ARKCOMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

namespace common {
// Because gc thread will also have impact on tagged pointer in enum and marking phase,
// so we don't expect reading barrier have the ability to modify the referent field.
BaseObject* RemarkBarrier::ReadRefField(BaseObject* obj, RefField<false>& field) const
{
    RefField<> tmpField(field);
    return reinterpret_cast<BaseObject*>(tmpField.GetFieldValue());
}

BaseObject* RemarkBarrier::ReadStaticRef(RefField<false>& field) const { return ReadRefField(nullptr, field); }


// If the object is still alive, return it; if not, return nullptr
BaseObject* RemarkBarrier::ReadStringTableStaticRef(RefField<false> &field) const
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

void RemarkBarrier::ReadStruct(HeapAddress dst, BaseObject *obj, HeapAddress src, size_t size) const
{
    CHECK_CC(memcpy_s(reinterpret_cast<void *>(dst), size, reinterpret_cast<void *>(src), size) == EOK);
}

void RemarkBarrier::WriteRoot(BaseObject *obj) const
{
    DCHECK_CC(Heap::IsHeapAddress(obj));
    Mutator *mutator = Mutator::GetMutator();
    mutator->RememberObjectInSatbBuffer(obj);
    DLOG(BARRIER, "write root obj %p", obj);
}

void RemarkBarrier::WriteRefField(BaseObject *obj, RefField<false> &field, BaseObject *ref) const
{
    UpdateRememberSet(obj, ref);
    RefField<> tmpField(field);
    BaseObject *rememberedObject = nullptr;
    rememberedObject = tmpField.GetTargetObject();
    Mutator *mutator = Mutator::GetMutator();
    if (rememberedObject != nullptr) {
        mutator->RememberObjectInSatbBuffer(rememberedObject);
    }
    DLOG(BARRIER, "write obj %p ref-field@%p: %#zx -> %p", obj, &field,
        rememberedObject, ref);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    RefField<> newField(ref);
    field.SetFieldValue(newField.GetFieldValue());
}
#ifdef ARK_USE_SATB_BARRIER
void RemarkBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    RefField<> tmpField(field);
    BaseObject* rememberedObject = nullptr;
    rememberedObject = tmpField.GetTargetObject();
    if (!Heap::IsTaggedObject(field.GetFieldValue())) {
        return;
    }
    UpdateRememberSet(obj, ref);
    if (UNLIKELY_CC(mutator == nullptr)) {
        mutator = Mutator::GetMutator();
    }
    if (rememberedObject != nullptr) {
        mutator->RememberObjectInSatbBuffer(rememberedObject);
    }
    if (ref != nullptr) {
        if (!Heap::IsTaggedObject((HeapAddress)ref)) {
            return;
        }
        ref = reinterpret_cast<BaseObject*>(reinterpret_cast<uintptr_t>(ref) & ~TAG_WEAK);
        mutator->RememberObjectInSatbBuffer(ref);
    }

    DLOG(BARRIER, "write obj %p ref-field@%p: %#zx -> %p", obj, &field, rememberedObject, ref);
}
#else
void RemarkBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    if (!Heap::IsTaggedObject((HeapAddress)ref)) {
        return;
    }
    UpdateRememberSet(obj, ref);
    ref = reinterpret_cast<BaseObject*>(reinterpret_cast<uintptr_t>(ref) & ~TAG_WEAK);
    if (UNLIKELY_CC(mutator == nullptr)) {
        mutator = Mutator::GetMutator();
    }
    mutator->RememberObjectInSatbBuffer(ref);
    DLOG(BARRIER, "write obj %p ref-field@%p: -> %p", obj, &field, ref);
}
#endif

void RemarkBarrier::WriteStaticRef(RefField<false>& field, BaseObject* ref) const
{
    std::atomic_thread_fence(std::memory_order_seq_cst);
    RefField<> newField(ref);
    field.SetFieldValue(newField.GetFieldValue());
}

void RemarkBarrier::WriteStruct(BaseObject* obj, HeapAddress dst, size_t dstLen, HeapAddress src, size_t srcLen) const
{
    CHECK_CC(obj != nullptr);
    if (obj != nullptr) { //LCOV_EXCL_BR_LINE
        ASSERT_LOGF(dst > reinterpret_cast<HeapAddress>(obj), "WriteStruct struct addr is less than obj!");
        Mutator* mutator = Mutator::GetMutator();
        if (mutator != nullptr) {
            obj->ForEachRefInStruct(
                [=](RefField<>& refField) {
                    mutator->RememberObjectInSatbBuffer(ReadRefField(obj, refField));
                },
                dst, dst + dstLen);
        }
    }
    std::atomic_thread_fence(std::memory_order_seq_cst);
    CHECK_CC(memcpy_s(reinterpret_cast<void*>(dst), dstLen, reinterpret_cast<void*>(src), srcLen) == EOK);

#if defined(ARKCOMMON_TSAN_SUPPORT)
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dst), dstLen);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(src), srcLen);
#endif
}

BaseObject* RemarkBarrier::AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    BaseObject* target = nullptr;
    RefField<false> oldField(field.GetFieldValue(order));
    target =  reinterpret_cast<BaseObject*>((oldField.GetFieldValue()));
    DLOG(TBARRIER, "katomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
    return target;
}

void RemarkBarrier::AtomicWriteRefField(BaseObject *obj, RefField<true> &field,
                                        BaseObject *newRef,
                                        MemoryOrder order) const
{
    RefField<> oldField(field.GetFieldValue(order));
    HeapAddress oldValue = oldField.GetFieldValue();
    BaseObject* oldRef = oldField.GetTargetObject();
    RefField<> newField(newRef);
    field.SetFieldValue(newField.GetFieldValue(), order);
    Mutator* mutator = Mutator::GetMutator();
    mutator->RememberObjectInSatbBuffer(oldRef);
    if (obj != nullptr) {
        DLOG(TBARRIER, "atomic write obj %p<%p>(%zu) ref@%p: %#zx -> %#zx", obj, obj->GetTypeInfo(), obj->GetSize(),
             &field, oldValue, newField.GetFieldValue());
    } else {
        DLOG(TBARRIER, "atomic write static ref@%p: %#zx -> %#zx", &field, oldValue, newField.GetFieldValue());
    }
}

BaseObject *RemarkBarrier::AtomicSwapRefField(BaseObject *obj,
                                              RefField<true> &field,
                                              BaseObject *newRef,
                                              MemoryOrder order) const
{
    RefField<> newField(newRef);
    HeapAddress oldValue = field.Exchange(newField.GetFieldValue(), order);
    RefField<> oldField(oldValue);
    BaseObject* oldRef = oldField.GetTargetObject();
    Mutator* mutator = Mutator::GetMutator();
    mutator->RememberObjectInSatbBuffer(oldRef);
    DLOG(TRACE, "atomic swap obj %p<%p>(%zu) ref-field@%p: old %#zx(%p), new %#zx(%p)", obj, obj->GetTypeInfo(),
         obj->GetSize(), &field, oldValue, oldRef, field.GetFieldValue(), newRef);
    return oldRef;
}

bool RemarkBarrier::CompareAndSwapRefField(
    BaseObject *obj, RefField<true> &field, BaseObject *oldRef,
    BaseObject *newRef, MemoryOrder succOrder, MemoryOrder failOrder) const
{
    HeapAddress oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
    RefField<false> oldField(oldFieldValue);
    BaseObject* oldVersion = oldField.GetTargetObject();
    RefField<> newField(newRef);

    while (oldVersion == oldRef) {
        if (field.CompareExchange(oldFieldValue, newField.GetFieldValue(), succOrder, failOrder)) {
            Mutator* mutator = Mutator::GetMutator();
            mutator->RememberObjectInSatbBuffer(oldRef);
            return true;
        }
        oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
        RefField<false> tmp(oldFieldValue);
        oldVersion = tmp.GetTargetObject();
    }

    return false;
}

void RemarkBarrier::CopyStructArray(BaseObject *dstObj, HeapAddress dstField,
                                    MIndex dstSize, BaseObject *srcObj,
                                    HeapAddress srcField,
                                    MIndex srcSize) const
{
    LOG_COMMON(FATAL) << "Unresolved fatal";
}

} // namespace panda

