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

#include "common_components/heap/barrier/barrier.h"

#include "common_components/heap/collector/collector.h"
#include "common_components/heap/heap.h"
#if defined(COMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif
#include "common_components/log/log.h"
#include "securec.h"

namespace common {

void Barrier::WriteRefField(BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    DLOG(BARRIER, "write obj %p ref-field@%p: %p => %p", obj, &field, field.GetTargetObject(), ref);
    field.SetTargetObject(ref);
}

void Barrier::WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const
{
    DLOG(BARRIER, "write obj %p ref-field@%p: %p => %p", obj, &field, field.GetTargetObject(), ref);
}

void Barrier::WriteRoot(BaseObject *obj) const
{
    DLOG(BARRIER, "write root obj %p", obj);
}

void Barrier::WriteStruct(BaseObject* obj, HeapAddress dst, size_t dstLen, HeapAddress src, size_t srcLen) const
{
    LOGF_CHECK(memcpy_s(reinterpret_cast<void*>(dst), dstLen, reinterpret_cast<void*>(src), srcLen) == EOK) <<
        "memcpy_s failed";
#if defined(COMMON_TSAN_SUPPORT)
    CHECK_EQ(srcLen, dstLen);
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dst), dstLen);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(src), srcLen);
#endif
}

void Barrier::WriteStaticRef(RefField<false>& field, BaseObject* ref) const
{
    DLOG(BARRIER, "write (barrier) static ref@%p: %p", &field, ref);
    field.SetTargetObject(ref);
}

BaseObject* Barrier::ReadRefField(BaseObject* obj, RefField<false>& field) const
{
    HeapAddress target = field.GetFieldValue();
    return reinterpret_cast<BaseObject*>(target);
}

BaseObject* Barrier::ReadStaticRef(RefField<false>& field) const
{
    HeapAddress target = field.GetFieldValue();
    return reinterpret_cast<BaseObject*>(target);
}

BaseObject* Barrier::ReadStringTableStaticRef(RefField<false>& field) const
{
    HeapAddress target = field.GetFieldValue();
    return reinterpret_cast<BaseObject*>(target);
}

// barrier for atomic operation.
void Barrier::AtomicWriteRefField(BaseObject* obj, RefField<true>& field, BaseObject* ref, MemoryOrder order) const
{
    if (obj != nullptr) {
        DLOG(BARRIER, "atomic write obj %p<%p>(%zu) ref@%p: %#zx -> %p", obj, obj->GetTypeInfo(), obj->GetSize(),
             &field, field.GetFieldValue(), ref);
    } else {
        DLOG(BARRIER, "atomic write static ref@%p: %#zx -> %p", &field, field.GetFieldValue(), ref);
    }

    field.SetTargetObject(ref, order);
}

BaseObject* Barrier::AtomicSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                        MemoryOrder order) const
{
    HeapAddress oldValue = field.Exchange(newRef, order);
    RefField<> oldField(oldValue);
    BaseObject* oldRef = ReadRefField(nullptr, oldField);
    DLOG(BARRIER, "atomic swap obj %p<%p>(%zu) ref-field@%p: old %#zx(%p), new %#zx(%p)", obj, obj->GetTypeInfo(),
         obj->GetSize(), &field, oldValue, oldRef, field.GetFieldValue(), newRef);
    return oldRef;
}

BaseObject* Barrier::AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    RefField<false> tmpField(field.GetFieldValue(order));
    HeapAddress target = tmpField.GetFieldValue();
    DLOG(BARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, tmpField.GetFieldValue(), target);
    return (BaseObject*)target;
}

bool Barrier::CompareAndSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* oldRef, BaseObject* newRef,
                                     MemoryOrder succOrder, MemoryOrder failOrder) const
{
    HeapAddress oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
    RefField<false> oldField(oldFieldValue);
    BaseObject* oldVersion = ReadRefField(nullptr, oldField);
    (void)oldVersion;
    bool res = field.CompareExchange(oldRef, newRef, succOrder, failOrder);
    DLOG(BARRIER, "cas %u for obj %p reffield@%p: old %#zx->%p, expect %p, new %p", res, obj, &field, oldFieldValue,
         oldVersion, oldRef, newRef);
    return res;
}

void Barrier::CopyStructArray(BaseObject* dstObj, HeapAddress dstField, MIndex dstSize, BaseObject* srcObj,
                              HeapAddress srcField, MIndex srcSize) const
{
    LOGF_CHECK(memmove_s(reinterpret_cast<void*>(dstField), dstSize, reinterpret_cast<void*>(srcField), srcSize) ==
        EOK) << "memmove_s failed";
#if defined(COMMON_TSAN_SUPPORT)
    size_t copyLen = (dstSize < srcSize ? dstSize : srcSize);
    Sanitizer::TsanWriteMemoryRange(reinterpret_cast<void*>(dstField), copyLen);
    Sanitizer::TsanReadMemoryRange(reinterpret_cast<void*>(srcField), copyLen);
#endif
}

void Barrier::ReadStruct(HeapAddress dst, BaseObject* obj, HeapAddress src, size_t size) const
{
    size_t dstSize = size;
    size_t srcSize = size;
    if (obj != nullptr) {
        obj->ForEachRefInStruct(
            [this, obj](RefField<false>& field) {
                // HeapAddress bias = reinterpret_cast<HeapAddress>(&field) - reinterpret_cast<HeapAddress>(src);
                // RefField<false>* dstField = reinterpret_cast<RefField<false>*>(dst + bias);
                BaseObject* fromVersion = field.GetTargetObject();
                (void)fromVersion;
                BaseObject* toVersion = nullptr;
                theCollector.TryUpdateRefField(obj, field, toVersion);
            },
            src, src + size);
    }

    LOGF_CHECK(memcpy_s(reinterpret_cast<void*>(dst), dstSize, reinterpret_cast<void*>(src), srcSize) == EOK) <<
        "read struct memcpy_s failed";
}

} // namespace common
