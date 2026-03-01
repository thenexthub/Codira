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


#include "ForwardBarrier.h"

#include "Base/SysCall.h"
#include "Common/ScopedObjectLock.h"
#include "Mutator/Mutator.h"
#include "ObjectModel/Field.inline.h"
#include "ObjectModel/MArray.h"
#include "ObjectModel/RefField.inline.h"
#if defined(CODIRA_TSAN_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif

namespace MapleRuntime {
BaseObject* ForwardBarrier::ReadReference(BaseObject* obj, RefField<false>& field) const
{
    do {
        RefField<> tmpField(field);
        if (LIKELY(!tmpField.IsTagged())) {
            return tmpField.GetTargetObject();
        }
        CHECK(!theCollector.IsOldPointer(tmpField));
        if (theCollector.IsCurrentPointer(tmpField)) {
            BaseObject* target = tmpField.GetTargetObject();
            BaseObject* toObj = nullptr;
            if (theCollector.IsUnmovableFromObject(target)) {
                if (theCollector.TryUntagRefField(obj, field, target)) {
                    return target;
                }
            } else if (theCollector.TryForwardRefField(obj, field, toObj)) {
                return toObj;
            }
        }
    } while (true);
    return nullptr;
}

BaseObject* ForwardBarrier::ReadStaticRef(RefField<false>& field) const { return ReadReference(nullptr, field); }

BaseObject* ForwardBarrier::ReadWeakRef(BaseObject* obj, RefField<false>& field) const
{
    return ReadReference(obj, field);
}

void ForwardBarrier::ReadStruct(MAddress dst, BaseObject* obj, MAddress src, size_t size) const
{
    CHECK(!Heap::IsHeapAddress(dst));
    if (obj != nullptr) {
        obj->ForEachRefInStruct(
            [this, obj](RefField<false>& field) {
                BaseObject* target = ReadReference(obj, field);
                (void)target;
            },
            src, src + size);
    }
    CHECK(memcpy_s(reinterpret_cast<void*>(dst), size, reinterpret_cast<void*>(src), size) == EOK);
}

void ForwardBarrier::ReadStaticStruct(MAddress dst, MAddress src, size_t size, const GCTib gctib) const
{
    CHECK(!Heap::IsHeapAddress(src));
    CHECK(!Heap::IsHeapAddress(dst));
    gctib.ForEachBitmapWord(src, [=](RefField<>& srcField) {
        BaseObject* target = ReadReference(nullptr, srcField);
        (void)target;
    });
    CHECK(memcpy_s(reinterpret_cast<void*>(dst), size, reinterpret_cast<void*>(src), size) == EOK);
}

BaseObject* ForwardBarrier::AtomicReadReference(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    RefField<false> tmpField(field.GetFieldValue(order));
    DCHECK(!theCollector.IsOldPointer(tmpField));

    if (theCollector.IsCurrentPointer(tmpField)) {
        BaseObject* target = tmpField.GetTargetObject();
        if (theCollector.IsUnmovableFromObject(target)) {
            if (theCollector.TryUntagRefField(obj, reinterpret_cast<RefField<>&>(field), target)) {
                DLOG(FBARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, tmpField.GetFieldValue(), target);
                return target;
            }
        } else {
            BaseObject* to = nullptr;
            // note TryForwardRefField is atomic operation.
            if (theCollector.TryForwardRefField(obj, reinterpret_cast<RefField<false>&>(field), to)) {
                DLOG(FBARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, tmpField.GetFieldValue(), to);
                return to;
            }
        }
    }
    BaseObject* target = ReadReference(nullptr, tmpField);
    DLOG(FBARRIER, "atomic read obj %p ref-field@%p: %#zx -> %p", obj, &field, tmpField.GetFieldValue(), target);
    return target;
}

void ForwardBarrier::AtomicWriteReference(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                          MemoryOrder order) const
{
    RefField<> newField(newRef);
    field.SetFieldValue(newField.GetFieldValue(), order);
    if (obj != nullptr) {
        DLOG(FBARRIER, "atomic write obj %p<%p>(%zu) ref@%p: %#zx", obj, obj->GetTypeInfo(), obj->GetSize(), &field,
             newField.GetFieldValue());
    } else {
        DLOG(FBARRIER, "atomic write static ref@%p: %#zx", &field, newField.GetFieldValue());
    }
}

BaseObject* ForwardBarrier::AtomicSwapReference(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                                MemoryOrder order) const
{
    MAddress oldValue = field.Exchange(newRef, order);
    RefField<> oldField(oldValue);
    BaseObject* oldRef = ReadReference(nullptr, oldField);
    DLOG(BARRIER, "atomic swap obj %p<%p>(%zu) ref-field@%p: old %#zx(%p), new %#zx(%p)", obj, obj->GetTypeInfo(),
         obj->GetSize(), &field, oldValue, oldRef, field.GetFieldValue(), newRef);
    return oldRef;
}

bool ForwardBarrier::CompareAndSwapReference(BaseObject* obj, RefField<true>& field, BaseObject* oldRef,
                                             BaseObject* newRef, MemoryOrder succOrder, MemoryOrder failOrder) const
{
    MAddress oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
    RefField<false> oldField(oldFieldValue);
    BaseObject* oldVersion = ReadReference(nullptr, oldField);
    while (oldVersion == oldRef) {
        RefField<> newField(newRef);
        if (field.CompareExchange(oldFieldValue, newField.GetFieldValue(), succOrder, failOrder)) {
            return true;
        }
        oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
        RefField<false> tmp(oldFieldValue);
        oldVersion = ReadReference(nullptr, tmp);
    }

    return false;
}

void ForwardBarrier::CopyStructArray(BaseObject* dstObj, MAddress dstField, MIndex dstSize, BaseObject* srcObj,
                                     MAddress srcField, MIndex srcSize) const
{
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    if (!(static_cast<MArray*>(dstObj)->GetComponentTypeInfo()->IsStructType())) {
        LOG(RTLOG_FATAL, "array %p type is not struct type", dstObj);
        return;
    }
#endif

    if (!dstObj->HasRefField()) {
        CHECK(memmove_s(reinterpret_cast<void*>(dstField), dstSize, reinterpret_cast<void*>(srcField), srcSize) == EOK);
#if defined(CODIRA_TSAN_SUPPORT)
        Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dstField), dstSize);
        Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(srcField), srcSize);
#endif
        return;
    }

    MArray* srcArray = static_cast<MArray*>(srcObj);
    RefFieldVisitor srcVisitor = [this, srcArray](RefField<false>& field) { (void)ReadReference(srcArray, field); };
    srcArray->ForEachRefFieldInRange(srcVisitor, srcField, srcField + srcSize);

    CHECK(memmove_s(reinterpret_cast<void*>(dstField), dstSize, reinterpret_cast<void*>(srcField), srcSize) == EOK);

#if defined(CODIRA_TSAN_SUPPORT)
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dstField), dstSize);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(srcField), srcSize);
#endif
}
} // namespace MapleRuntime
