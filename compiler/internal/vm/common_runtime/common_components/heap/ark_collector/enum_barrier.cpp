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
#include "common_components/heap/ark_collector/enum_barrier.h"

#include "common_components/mutator/mutator.h"
#if defined(COMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

namespace common {
// Because gc thread will also have impact on tagged pointer in enum and marking phase,
// so we don't expect reading barrier have the ability to modify the referent field.
BaseObject* EnumBarrier::ReadRefField(BaseObject* obj, RefField<false>& field) const
{
    RefField<> tmpField(field);
    return (BaseObject*)tmpField.GetFieldValue();
}

BaseObject* EnumBarrier::ReadStaticRef(RefField<false>& field) const { return ReadRefField(nullptr, field); }

void EnumBarrier::ReadStruct(HeapAddress dst, BaseObject* obj, HeapAddress src, size_t size) const
{
    CHECK_CC(memcpy_s(reinterpret_cast<void*>(dst), size, reinterpret_cast<void*>(src), size) == EOK);
}

void EnumBarrier::WriteRoot(BaseObject *obj) const
{
    DCHECK_CC(Heap::IsHeapAddress(obj));
    Mutator *mutator = Mutator::GetMutator();
    mutator->RememberObjectInSatbBuffer(obj);
    DLOG(BARRIER, "write root obj %p", obj);
}

void EnumBarrier::WriteRefField(BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    UpdateRememberSet(obj, ref);
    RefField<> tmpField(field);
    BaseObject* remeberedObject = nullptr;
    remeberedObject = tmpField.GetTargetObject();
    Mutator* mutator = Mutator::GetMutator();
    if (remeberedObject != nullptr) {
        mutator->RememberObjectInSatbBuffer(remeberedObject);
    }
    if (ref != nullptr) { //LCOV_EXCL_BR_LINE
        mutator->RememberObjectInSatbBuffer(ref);
    }
    DLOG(BARRIER, "write obj %p ref@%p: 0x%zx -> %p", obj, &field, remeberedObject, ref);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    RefField<> newField(ref);
    field.SetFieldValue(newField.GetFieldValue());
}
#ifdef ARK_USE_SATB_BARRIER
void EnumBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    RefField<> tmpField(field);
    BaseObject* remeberedObject = nullptr;
    //Because it is possible to read a Double,
    // and the lower 48 bits happen to be a HeapAddress, we need to avoid this situation
    if (!Heap::IsTaggedObject(field.GetFieldValue())) {
        return;
    }
    UpdateRememberSet(obj, ref);
    remeberedObject = tmpField.GetTargetObject();
    if (UNLIKELY_CC(mutator == nullptr)) {
        mutator = Mutator::GetMutator();
    }
    if (remeberedObject != nullptr) {
        mutator->RememberObjectInSatbBuffer(remeberedObject);
    }
    if (ref != nullptr) {
        // Wait Conccurent Enum Fix
        // ref maybe not valid
        // mutator->RememberObjectInSatbBuffer(ref)
    }
    DLOG(BARRIER, "write obj %p ref@%p: 0x%zx -> %p", obj, &field, remeberedObject, ref);
}
#else
void EnumBarrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    if (!Heap::IsTaggedObject((HeapAddress)ref)) {
        return;
    }
    if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
        UpdateRememberSet(obj, ref);
    }
    ref = (BaseObject*)((uintptr_t)ref & ~(TAG_WEAK));
    if (UNLIKELY_CC(mutator == nullptr)) {
        mutator = Mutator::GetMutator();
    }
    mutator->RememberObjectInSatbBuffer(ref);
    DLOG(BARRIER, "write obj %p ref-field@%p: -> %p", obj, &field, ref);
}
#endif
void EnumBarrier::WriteStaticRef(RefField<false>& field, BaseObject* ref) const
{
    DLOG(BARRIER, "write static ref@%p: %p -|> %p", &field, field.GetTargetObject(), ref);
    WriteRefField(nullptr, field, ref);
}

void EnumBarrier::WriteStruct(BaseObject* obj, HeapAddress dst, size_t dstLen, HeapAddress src, size_t srcLen) const
{
    if (obj != nullptr) {
        ASSERT_LOGF(dst > reinterpret_cast<HeapAddress>(obj), "WriteStruct struct addr is less than obj!");
        Mutator* mutator = Mutator::GetMutator();
        obj->ForEachRefInStruct(
            [=](RefField<>& dstField) {
                mutator->RememberObjectInSatbBuffer(ReadRefField(obj, dstField));
                HeapAddress offset = reinterpret_cast<HeapAddress>(&dstField) - dst;
                RefField<>* srcField = reinterpret_cast<RefField<>*>(src + offset);
                mutator->RememberObjectInSatbBuffer(ReadRefField(nullptr, *srcField));
            },
            dst, dst + srcLen);
    }
    std::atomic_thread_fence(std::memory_order_seq_cst);
    LOGF_CHECK(memcpy_s(reinterpret_cast<void*>(dst), dstLen, reinterpret_cast<void*>(src), srcLen) == EOK) <<
        "memcpy_s failed";
#if defined(COMMON_TSAN_SUPPORT)
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dst), dstLen);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(src), srcLen);
#endif
}

BaseObject* EnumBarrier::AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    BaseObject* target = nullptr;
    RefField<false> oldField(field.GetFieldValue(order));
    target = ReadRefField(obj, oldField);
    DLOG(EBARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
    return target;
}

BaseObject* EnumBarrier::AtomicSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                            MemoryOrder order) const
{
    RefField<> newField(newRef);
    HeapAddress oldValue = field.Exchange(newField.GetFieldValue(), order);
    RefField<> oldField(oldValue);
    BaseObject* oldRef = oldField.GetTargetObject();
    Mutator* mutator = Mutator::GetMutator();
    mutator->RememberObjectInSatbBuffer(oldRef);
    mutator->RememberObjectInSatbBuffer(newRef);
    DLOG(BARRIER, "atomic swap obj %p<%p>(%zu) ref@%p: old %#zx(%p), new %#zx(%p)", obj, obj->GetTypeInfo(),
         obj->GetSize(), &field, oldValue, oldRef, field.GetFieldValue(), newRef);
    return oldRef;
}

void EnumBarrier::AtomicWriteRefField(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                      MemoryOrder order) const
{
    RefField<> oldField(field.GetFieldValue(order));
    HeapAddress oldValue = oldField.GetFieldValue();
    (void)oldValue;
    BaseObject* oldRef = oldField.GetTargetObject();
    RefField<> newField(newRef);
    field.SetFieldValue(newField.GetFieldValue(), order);
    Mutator* mutator = Mutator::GetMutator();
    mutator->RememberObjectInSatbBuffer(oldRef);
    mutator->RememberObjectInSatbBuffer(newRef);
    if (obj != nullptr) {
        DLOG(EBARRIER, "atomic write obj %p<%p>(%zu) ref@%p: %#zx -> %#zx", obj, obj->GetTypeInfo(), obj->GetSize(),
             &field, oldValue, newField.GetFieldValue());
    } else {
        DLOG(EBARRIER, "atomic write static ref@%p: %#zx -> %#zx", &field, oldValue, newField.GetFieldValue());
    }
}

bool EnumBarrier::CompareAndSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* oldRef,
                                         BaseObject* newRef, MemoryOrder sOrder, MemoryOrder fOrder) const
{
    RefField<> newField(newRef);
    HeapAddress oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
    RefField<false> oldField(oldFieldValue);
    BaseObject* oldVersion = oldField.GetTargetObject();

    while (oldVersion == oldRef) {
        if (field.CompareExchange(oldFieldValue, newField.GetFieldValue(), sOrder, fOrder)) {
            Mutator* mutator = Mutator::GetMutator();
            mutator->RememberObjectInSatbBuffer(oldRef);
            mutator->RememberObjectInSatbBuffer(newRef);
            return true;
        }
        oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
        RefField<false> tmp(oldFieldValue);
        oldVersion = tmp.GetTargetObject();
    }

    return false;
}

void EnumBarrier::CopyStructArray(BaseObject* dstObj, HeapAddress dstField, MIndex dstSize, BaseObject* srcObj,
                                  HeapAddress srcField, MIndex srcSize) const
{
    LOG_COMMON(FATAL) << "Unresolved fatal";
    UNREACHABLE_CC();
}

} // namespace common
