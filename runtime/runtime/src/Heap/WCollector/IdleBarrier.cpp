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


#include "IdleBarrier.h"

#include "Mutator/Mutator.h"
#include "ObjectModel/MArray.h"
#include "ObjectModel/RefField.inline.h"
#if defined(CODIRA_TSAN_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif
#include "securec.h"

namespace MapleRuntime {
BaseObject* IdleBarrier::ReadReference(BaseObject* obj, RefField<false>& field) const
{
    do {
        RefField<> oldField(field);
        if (LIKELY(!oldField.IsTagged())) {
            return oldField.GetTargetObject();
        }
        BaseObject* toVersion = nullptr;
        if (theCollector.TryUpdateRefField(obj, field, toVersion)) {
            DLOG(BARRIER, "update obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), toVersion);
            return toVersion;
        }

        BaseObject* target = nullptr;
        if (theCollector.TryUntagRefField(obj, field, target)) {
            DLOG(BARRIER, "untag obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
            return target;
        }
    } while (true);
    // unreachable path.
    return nullptr;
}

BaseObject* IdleBarrier::ReadStaticRef(RefField<false>& field) const { return ReadReference(nullptr, field); }

BaseObject* IdleBarrier::ReadWeakRef(BaseObject* obj, RefField<false>& field) const
{
    return ReadReference(obj, field);
}

BaseObject* IdleBarrier::AtomicReadReference(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    RefField<false> oldField(field.GetFieldValue(order));
    BaseObject* toVersion = nullptr;
    // note TryUpdateRefField and TryUntagRefField are all atomic operations.
    if (theCollector.TryUpdateRefField(obj, reinterpret_cast<RefField<false>&>(field), toVersion)) {
        DLOG(BARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), toVersion);
        return toVersion;
    }

    BaseObject* target = nullptr;
    if (theCollector.TryUntagRefField(obj, reinterpret_cast<RefField<false>&>(field), target)) {
        DLOG(BARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
        return target;
    }

    target = ReadReference(nullptr, oldField);
    DLOG(BARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, oldField.GetFieldValue(), target);
    return target;
}

void IdleBarrier::ReadStruct(MAddress dst, BaseObject* obj, MAddress src, size_t size) const
{
    if (obj != nullptr) {
        // note fix/untag dst would be better.
        obj->ForEachRefInStruct(
            [this, obj](RefField<false>& field) {
                RefField<> oldField(field);
                BaseObject* target = ReadReference(obj, field);
                (void)target;
            },
            src, src + size);
    }

    CHECK_DETAIL(memcpy_s(reinterpret_cast<void*>(dst), size, reinterpret_cast<void*>(src), size) == EOK,
                 "read struct memcpy_s failed");
}

void IdleBarrier::ReadStaticStruct(MAddress dst, MAddress src, size_t size, const GCTib gctib) const
{
    CHECK_DETAIL(memcpy_s(reinterpret_cast<void*>(dst), size, reinterpret_cast<void*>(src), size) == EOK,
                 "read struct memcpy_s failed");
    gctib.ForEachBitmapWord(dst, [=](RefField<>& field) {
        BaseObject* target = ReadReference(nullptr, field);
        (void)target;
    });
}

void IdleBarrier::AtomicWriteReference(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
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

BaseObject* IdleBarrier::AtomicSwapReference(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                             MemoryOrder order) const
{
    // newRef must be the latest versions.
    MAddress oldValue = field.Exchange(newRef, order);
    RefField<> oldField(oldValue);
    BaseObject* oldRef = ReadReference(nullptr, oldField);
    DLOG(BARRIER, "atomic swap obj %p<%p>(%zu) ref@%p: old %#zx(%p), new %#zx(%p)", obj, obj->GetTypeInfo(),
         obj->GetSize(), &field, oldValue, oldRef, field.GetFieldValue(order), newRef);
    return oldRef;
}

bool IdleBarrier::CompareAndSwapReference(BaseObject* obj, RefField<true>& field, BaseObject* oldRef,
                                          BaseObject* newRef, MemoryOrder sOrder, MemoryOrder fOrder) const
{
    MAddress oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
    RefField<false> oldField(oldFieldValue);
    BaseObject* oldVersion = ReadReference(nullptr, oldField);

    // oldRef and newRef must be the latest versions.
    while (oldVersion == oldRef) {
        RefField<> newField(newRef);
        if (field.CompareExchange(oldFieldValue, newField.GetFieldValue(), sOrder, fOrder)) {
            return true;
        }
        oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
        RefField<false> tmp(oldFieldValue);
        oldVersion = ReadReference(nullptr, tmp);
    }
    return false;
}

void IdleBarrier::WriteReference(BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    DLOG(BARRIER, "write obj %p ref@%p: %p => %p", obj, &field, field.GetTargetObject(), ref);
    field.SetTargetObject(ref);
}

void IdleBarrier::WriteStruct(BaseObject* obj, MAddress dst, size_t dstLen, MAddress src, size_t srcLen) const
{
    CHECK(memcpy_s(reinterpret_cast<void*>(dst), dstLen, reinterpret_cast<void*>(src), srcLen) == EOK);
#if defined(CODIRA_TSAN_SUPPORT)
    CHECK(srcLen == dstLen);
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dst), dstLen);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(src), srcLen);
#endif
}

void IdleBarrier::WriteStaticRef(RefField<false>& field, BaseObject* ref) const { WriteReference(nullptr, field, ref); }

void IdleBarrier::WriteStaticStruct(MAddress dst, size_t dstLen, MAddress src, size_t srcLen, const GCTib gctib) const
{
    WriteStruct(nullptr, dst, dstLen, src, srcLen);
}

void IdleBarrier::CopyRefArray(BaseObject* dstObj, MAddress dst, MIndex dstSize, BaseObject* srcObj, MAddress src,
                               MIndex srcSize) const
{
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    if (!dstObj->HasRefField()) {
        LOG(RTLOG_FATAL, "array %p doesn't have class-type element\n", dstObj);
        return;
    }
    if (!static_cast<MArray*>(dstObj)->GetComponentTypeInfo()->IsObjectType() &&
        !static_cast<MArray*>(dstObj)->GetComponentTypeInfo()->IsInterface() &&
        !static_cast<MArray*>(dstObj)->GetComponentTypeInfo()->IsArrayType()) {
        LOG(RTLOG_FATAL, "array %p type is not class", dstObj);
        return;
    }
#endif
    if (dst == src) {
        return;
    }
    bool inHeap = Heap::IsHeapAddress(dstObj);
    if (dst < src) {
        MAddress currentDst = dst;
        MAddress currentSrc = src;
        MAddress fieldBound = dst + dstSize;
        while (currentDst < fieldBound) {
            RefField<false>* currentDstField = reinterpret_cast<RefField<false>*>(currentDst);
            RefField<false>* currentSrcField = reinterpret_cast<RefField<false>*>(currentSrc);
            BaseObject* newRef = ReadReference(srcObj, *currentSrcField);
            if (inHeap) {
                WriteReference(dstObj, *currentDstField, newRef);
            } else {
                currentDstField->SetTargetObject(newRef);
            }
            currentDst += sizeof(RefField<false>);
            currentSrc += sizeof(RefField<false>);
        }
    } else {
        MAddress currentDst = dst + dstSize - sizeof(RefField<>);
        MAddress currentSrc = src + srcSize - sizeof(RefField<>);
        MAddress fieldBound = dst;
        while (currentDst >= fieldBound) {
            RefField<false>* currentDstField = reinterpret_cast<RefField<false>*>(currentDst);
            RefField<false>* currentSrcField = reinterpret_cast<RefField<false>*>(currentSrc);
            BaseObject* newRef = ReadReference(srcObj, *currentSrcField);
            if (inHeap) {
                WriteReference(dstObj, *currentDstField, newRef);
            } else {
                currentDstField->SetTargetObject(newRef);
            }
            currentDst -= sizeof(RefField<false>);
            currentSrc -= sizeof(RefField<false>);
        }
    }
#if defined(CODIRA_TSAN_SUPPORT)
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dst), dstSize);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(src), srcSize);
#endif
}

void IdleBarrier::CopyStructArray(BaseObject* dstObj, MAddress dstField, MIndex dstSize, BaseObject* srcObj,
                                  MAddress srcField, MIndex srcSize) const
{
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    if (!dstObj->HasRefField()) {
        LOG(RTLOG_FATAL, "array %p doesn't have class-type element\n", dstObj);
        return;
    }
    if (!(static_cast<MArray*>(dstObj)->GetComponentTypeInfo()->IsStructType())) {
        LOG(RTLOG_FATAL, "array %p type is not struct type", dstObj);
        return;
    }
#endif
    MArray* srcArray = static_cast<MArray*>(srcObj);
    RefFieldVisitor srcVisitor = [this, srcArray](RefField<false>& field) { (void)ReadReference(srcArray, field); };
    srcArray->ForEachRefFieldInRange(srcVisitor, srcField, srcField + srcSize);

    CHECK_DETAIL(memmove_s(reinterpret_cast<void*>(dstField), dstSize, reinterpret_cast<void*>(srcField), srcSize) ==
                     EOK,
                 "memmove_s failed");

#if defined(CODIRA_TSAN_SUPPORT)
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dstField), dstSize);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(srcField), srcSize);
#endif
}
} // namespace MapleRuntime
