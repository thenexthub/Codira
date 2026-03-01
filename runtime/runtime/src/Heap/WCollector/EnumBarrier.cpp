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


#include "EnumBarrier.h"
#include "Heap/Allocator/RegionSpace.h"
#include "Mutator/Mutator.h"
#include "ObjectModel/MArray.h"
#include "ObjectModel/RefField.inline.h"
#include "Collector/CopyCollector.h"
#if defined(CODIRA_TSAN_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif

namespace MapleRuntime {
// Because gc thread will also have impact on tagged pointer in enum and trace phase,
// so we don't expect reading barrier have the ability to modify the referent field.
BaseObject* EnumBarrier::ReadReference(BaseObject* obj, RefField<false>& field) const
{
    RefField<> tmpField(field);
    if (LIKELY(!tmpField.IsTagged())) {
        return tmpField.GetTargetObject();
    }
    if (theCollector.IsCurrentPointer(tmpField)) {
        return tmpField.GetTargetObject();
    }
    if (theCollector.IsOldPointer(tmpField)) {
        BaseObject* fromVersion = tmpField.GetTargetObject();
        BaseObject* toVersion = theCollector.FindToVersion(fromVersion);
        BaseObject* target = nullptr;
        if (toVersion != nullptr) {
            target = toVersion;
        } else {
            target = fromVersion;
        }
        return target;
    }
    return tmpField.GetTargetObject();
}

BaseObject* EnumBarrier::ReadStaticRef(RefField<false>& field) const { return ReadReference(nullptr, field); }

BaseObject* EnumBarrier::ReadWeakRef(BaseObject* obj, RefField<false>& field) const
{
    BaseObject* target = ReadReference(obj, field);
    DLOG(BARRIER, "read weakref obj %p ref@%p: 0x%zx", obj, &field, target);
    if (target != nullptr) {
        Mutator* mutator = Mutator::GetMutator();
        mutator->RememberObjectInSatbBuffer(target);
        // remark the referent because it may be used later.
    }
    return target;
}

void EnumBarrier::ReadStruct(MAddress dst, BaseObject* obj, MAddress src, size_t size) const
{
    LocalRefFieldContainer refFields;
    if (obj != nullptr) {
        obj->ForEachRefInStruct(
            [&refFields, dst, src, size](RefField<false>& field) {
                if (reinterpret_cast<MAddress>(&field) < src || reinterpret_cast<MAddress>(&field) >= (src + size)) {
                    return;
                }
                RefField<> oldField(field);
                MAddress offset = reinterpret_cast<MAddress>(&field) - src;
                refFields.Push(reinterpret_cast<RefField<>*>(dst + offset));
            },
            src, src + size);
    }

    CHECK(memcpy_s(reinterpret_cast<void*>(dst), size, reinterpret_cast<void*>(src), size) == EOK);
    refFields.VisitRefField([this](RefField<>& dstRef) {
        BaseObject* target = nullptr;
        if (theCollector.IsCurrentPointer(dstRef)) {
            theCollector.TryUntagRefField(nullptr, dstRef, target);
        } else if (theCollector.IsOldPointer(dstRef)) {
            dstRef.SetTargetObject(ReadReference(nullptr, dstRef));
        }
    });
}

void EnumBarrier::ReadStaticStruct(MAddress dst, MAddress src, size_t size, const GCTib gctib) const
{
    LocalRefFieldContainer refFields;
    gctib.ForEachBitmapWordInRange(
        src,
        [&refFields, dst, src](RefField<>& srcField) {
            MAddress offset = reinterpret_cast<MAddress>(&srcField) - src;
            refFields.Push(reinterpret_cast<RefField<>*>(dst + offset));
        },
        src, src + size);

    CHECK(memcpy_s(reinterpret_cast<void*>(dst), size, reinterpret_cast<void*>(src), size) == EOK);
    refFields.VisitRefField([this](RefField<>& dstRef) {
        BaseObject* target = nullptr;
        if (theCollector.IsCurrentPointer(dstRef)) {
            theCollector.TryUntagRefField(nullptr, dstRef, target);
        } else if (theCollector.IsOldPointer(dstRef)) {
            dstRef.SetTargetObject(ReadReference(nullptr, dstRef));
        }
    });
}

void EnumBarrier::WriteReference(BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    RefField<> tmpField(field);
    BaseObject* remeberedObject = nullptr;
    if (theCollector.IsOldPointer(tmpField)) {
        BaseObject* toVersion = nullptr;
        if (theCollector.TryUpdateRefField(obj, tmpField, toVersion)) {
            remeberedObject = toVersion;
        } else {
            remeberedObject = field.GetTargetObject();
        }
    } else {
        remeberedObject = tmpField.GetTargetObject();
    }
    Mutator* mutator = Mutator::GetMutator();
    if (remeberedObject != nullptr) {
        mutator->RememberObjectInSatbBuffer(remeberedObject);
    }
    if (ref != nullptr) {
        mutator->RememberObjectInSatbBuffer(ref);
    }
    DLOG(BARRIER, "write obj %p ref@%p: 0x%zx -> %p", obj, &field, remeberedObject, ref);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    RefField<> newField = theCollector.GetAndTryTagRefField(ref);
    field.SetFieldValue(newField.GetFieldValue());
}

void EnumBarrier::WriteStaticRef(RefField<false>& field, BaseObject* ref) const
{
    DLOG(BARRIER, "write static ref@%p: %p -|> %p", &field, field.GetTargetObject(), ref);
    WriteReference(nullptr, field, ref);
}

void EnumBarrier::WriteStruct(BaseObject* obj, MAddress dst, size_t dstLen, MAddress src, size_t srcLen) const
{
    if (obj != nullptr) {
        MRT_ASSERT(dst > reinterpret_cast<MAddress>(obj), "WriteStruct struct addr is less than obj!");
        Mutator* mutator = Mutator::GetMutator();
        obj->ForEachRefInStruct(
            [=](RefField<>& dstField) {
                mutator->RememberObjectInSatbBuffer(ReadReference(obj, dstField));
                MAddress offset = reinterpret_cast<MAddress>(&dstField) - dst;
                RefField<>* srcField = reinterpret_cast<RefField<>*>(src + offset);
                mutator->RememberObjectInSatbBuffer(ReadReference(nullptr, *srcField));
            },
            dst, dst + srcLen);
    }
    std::atomic_thread_fence(std::memory_order_seq_cst);
    CHECK_DETAIL(memcpy_s(reinterpret_cast<void*>(dst), dstLen, reinterpret_cast<void*>(src), srcLen) == EOK,
                 "memcpy_s failed");

    if (obj != nullptr) {
        obj->ForEachRefInStruct(
            [=](RefField<>& refField) {
                RefField<> oldField(refField);
                RefField<> toBeUpdated(oldField);
                BaseObject* untagged = ReadReference(nullptr, toBeUpdated);
                RefField<> newField = theCollector.GetAndTryTagRefField(untagged);
                if (oldField.GetFieldValue() != newField.GetFieldValue()) {
                    refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue());
                }
            },
            dst, dst + dstLen);
    }

#if defined(CODIRA_TSAN_SUPPORT)
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dst), dstLen);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(src), srcLen);
#endif
}

void EnumBarrier::WriteStaticStruct(MAddress dst, size_t dstLen, MAddress src, size_t srcLen, const GCTib gctib) const
{
    Mutator* mutator = Mutator::GetMutator();
    gctib.ForEachBitmapWord(dst, [=](RefField<>& dstField) {
        mutator->RememberObjectInSatbBuffer(ReadReference(nullptr, dstField));
        uint32_t offset = reinterpret_cast<MAddress>(&dstField) - dst;
        RefField<>* srcField = reinterpret_cast<RefField<>*>(src + offset);
        mutator->RememberObjectInSatbBuffer(ReadReference(nullptr, *srcField));
    });
    std::atomic_thread_fence(std::memory_order_seq_cst);
    CHECK_DETAIL(memcpy_s(reinterpret_cast<void*>(dst), dstLen, reinterpret_cast<void*>(src), srcLen) == EOK,
                 "memcpy_s failed");

    gctib.ForEachBitmapWord(dst, [=](RefField<>& refField) {
        RefField<> oldField(refField);
        RefField<> toBeUpdated(oldField);
        BaseObject* untagged = ReadReference(nullptr, toBeUpdated);
        RefField<> newField = theCollector.GetAndTryTagRefField(untagged);
        if (oldField.GetFieldValue() != newField.GetFieldValue()) {
            refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue());
        }
    });

#if defined(CODIRA_TSAN_SUPPORT)
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dst), dstLen);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(src), srcLen);
#endif
}

BaseObject* EnumBarrier::AtomicReadReference(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    BaseObject* target = nullptr;
    RefField<false> oldField(field.GetFieldValue(order));
    if (theCollector.IsCurrentPointer(oldField)) {
        target = oldField.GetTargetObject();
        DLOG(EBARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
        return target;
    }

    target = ReadReference(nullptr, oldField);
    DLOG(EBARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
    return target;
}

BaseObject* EnumBarrier::AtomicSwapReference(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                             MemoryOrder order) const
{
    RefField<> newField = theCollector.GetAndTryTagRefField(newRef);
    MAddress oldValue = field.Exchange(newField.GetFieldValue(), order);
    RefField<> oldField(oldValue);
    BaseObject* oldRef = ReadReference(nullptr, oldField);
    Mutator* mutator = Mutator::GetMutator();
    mutator->RememberObjectInSatbBuffer(oldRef);
    mutator->RememberObjectInSatbBuffer(newRef);
    DLOG(BARRIER, "atomic swap obj %p<%p>(%zu) ref@%p: old %#zx(%p), new %#zx(%p)", obj, obj->GetTypeInfo(),
         obj->GetSize(), &field, oldValue, oldRef, field.GetFieldValue(), newRef);
    return oldRef;
}

void EnumBarrier::AtomicWriteReference(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                       MemoryOrder order) const
{
    RefField<> oldField(field.GetFieldValue(order));
    MAddress oldValue = oldField.GetFieldValue();
    (void)oldValue;
    BaseObject* oldRef = ReadReference(nullptr, oldField);
    RefField<> newField = theCollector.GetAndTryTagRefField(newRef);
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

bool EnumBarrier::CompareAndSwapReference(BaseObject* obj, RefField<true>& field, BaseObject* oldRef,
                                          BaseObject* newRef, MemoryOrder sOrder, MemoryOrder fOrder) const
{
    RefField<> newField = theCollector.GetAndTryTagRefField(newRef);

    MAddress oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
    RefField<false> oldField(oldFieldValue);
    BaseObject* oldVersion = ReadReference(nullptr, oldField);

    while (oldVersion == oldRef) {
        if (field.CompareExchange(oldFieldValue, newField.GetFieldValue(), sOrder, fOrder)) {
            Mutator* mutator = Mutator::GetMutator();
            mutator->RememberObjectInSatbBuffer(oldRef);
            mutator->RememberObjectInSatbBuffer(newRef);
            return true;
        }
        oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
        RefField<false> tmp(oldFieldValue);
        oldVersion = ReadReference(nullptr, tmp);
    }

    return false;
}

void EnumBarrier::CopyStructArray(BaseObject* dstObj, MAddress dstField, MIndex dstSize, BaseObject* srcObj,
                                  MAddress srcField, MIndex srcSize) const
{
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    if (!(static_cast<MArray*>(dstObj)->GetComponentTypeInfo()->IsStructType())) {
        LOG(RTLOG_FATAL, "array %p type is not struct type", dstObj);
        return;
    }
#endif
    if (!dstObj->HasRefField()) {
        CHECK_DETAIL(
            memmove_s(reinterpret_cast<void*>(dstField), dstSize, reinterpret_cast<void*>(srcField), srcSize) == EOK,
            "memmove_s failed");
#if defined(CODIRA_TSAN_SUPPORT)
        Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dstField), dstSize);
        Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(srcField), srcSize);
#endif
        return;
    }

    Mutator* mutator = Mutator::GetMutator();
    RefFieldVisitor srcVisitor = [this, mutator](RefField<false>& field) {
        RefField<> oldField(field);
        RefField<> toBeUpdated(oldField);
        BaseObject* target = ReadReference(nullptr, toBeUpdated);
        mutator->RememberObjectInSatbBuffer(target);
        RefField<> newField = theCollector.GetAndTryTagRefField(target);
        if (newField.GetFieldValue() != oldField.GetFieldValue()) {
            field.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue());
        }
    };
    MArray* srcArray = static_cast<MArray*>(srcObj);
    srcArray->ForEachRefFieldInRange(srcVisitor, srcField, srcField + srcSize);

    RefFieldVisitor dstVisitor = [this, mutator](RefField<false>& field) {
        RefField<> oldField(field);
        BaseObject* target = ReadReference(nullptr, oldField);
        mutator->RememberObjectInSatbBuffer(target);
    };
    MArray* dstArray = static_cast<MArray*>(dstObj);
    dstArray->ForEachRefFieldInRange(dstVisitor, dstField, dstField + srcSize);

    CHECK_DETAIL(memmove_s(reinterpret_cast<void*>(dstField), dstSize, reinterpret_cast<void*>(srcField), srcSize) ==
                     EOK,
                 "memmove_s failed");

#if defined(CODIRA_TSAN_SUPPORT)
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dstField), dstSize);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(srcField), srcSize);
#endif
}

void EnumBarrier::WriteGeneric(const ObjectPtr obj, void* fieldPtr, const ObjectPtr src, size_t size) const
{
    if ((obj != nullptr && !obj->HasRefField()) || (!Heap::IsHeapAddress(obj) && !Heap::IsHeapAddress(src))) {
        CHECK_DETAIL(memcpy_s(fieldPtr, size,
                              reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(src) + TYPEINFO_PTR_SIZE),
                              size) == EOK,
                     "WriteGeneric memcpy_s failed");
#if defined(CODIRA_TSAN_SUPPORT)
        if (Heap::IsHeapAddress(src)) {
            Sanitizer::TsanReadMemoryRange(
                reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(src) + TYPEINFO_PTR_SIZE), size);
        }
        if (Heap::IsHeapAddress(obj)) {
            Sanitizer::TsanWriteMemoryRange(fieldPtr, size);
        }
#endif
    } else if (!Heap::IsHeapAddress(obj) && Heap::IsHeapAddress(src)) {
        MAddress dstAddr = reinterpret_cast<MAddress>(fieldPtr);
        MAddress srcAddr = reinterpret_cast<MAddress>(src) + TYPEINFO_PTR_SIZE;
        ReadStruct(dstAddr, src, srcAddr, size);
    } else if ((Heap::IsHeapAddress(obj) && !Heap::IsHeapAddress(src))) {
        MAddress dstAddr = reinterpret_cast<MAddress>(fieldPtr);
        MAddress srcAddr = reinterpret_cast<MAddress>(src) + TYPEINFO_PTR_SIZE;
        WriteStruct(obj, dstAddr, size, srcAddr, size);
    } else {
        MAddress dstAddr = reinterpret_cast<MAddress>(fieldPtr);
        MAddress srcAddr = reinterpret_cast<MAddress>(src) + TYPEINFO_PTR_SIZE;
        void* tmp = malloc(size);
        ReadStruct((MAddress)tmp, src, srcAddr, size);
        WriteStruct(obj, dstAddr, size, (MAddress)tmp, size);
        free(tmp);
    }
}

void EnumBarrier::ReadGeneric(const ObjectPtr dstObj, ObjectPtr obj, void* fieldPtr, size_t size) const
{
    if (!Heap::IsHeapAddress(dstObj) && !Heap::IsHeapAddress(obj)) {
        CHECK_DETAIL(memcpy_s(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(dstObj) + TYPEINFO_PTR_SIZE), size,
                              fieldPtr, size) == EOK,
                     "ReadGeneric memcpy_s failed");
    } else if (!Heap::IsHeapAddress(dstObj) && Heap::IsHeapAddress(obj)) {
        MAddress dstAddr = reinterpret_cast<MAddress>(dstObj) + TYPEINFO_PTR_SIZE;
        MAddress srcAddr = reinterpret_cast<MAddress>(fieldPtr);
        ReadStruct(dstAddr, obj, srcAddr, size);
    } else if ((Heap::IsHeapAddress(dstObj) && !Heap::IsHeapAddress(obj))) {
        MAddress dstAddr = reinterpret_cast<MAddress>(dstObj) + TYPEINFO_PTR_SIZE;
        MAddress srcAddr = reinterpret_cast<MAddress>(fieldPtr);
        WriteStruct(dstObj, dstAddr, size, srcAddr, size);
    } else {
        MAddress dstAddr = reinterpret_cast<MAddress>(dstObj) + TYPEINFO_PTR_SIZE;
        MAddress srcAddr = reinterpret_cast<MAddress>(fieldPtr);
        void* tmp = malloc(size);
        ReadStruct((MAddress)tmp, obj, srcAddr, size);
        WriteStruct(dstObj, dstAddr, size, (MAddress)tmp, size);
        free(tmp);
    }
}
} // namespace MapleRuntime
