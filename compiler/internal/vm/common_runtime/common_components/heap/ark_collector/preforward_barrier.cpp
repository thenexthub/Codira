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
#include "common_components/heap/ark_collector/preforward_barrier.h"

#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/base/sys_call.h"
#include "common_components/common/scoped_object_lock.h"
#include "common_components/mutator/mutator.h"
#include "heap/collector/collector_proxy.h"
#if defined(COMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

namespace common {
BaseObject* PreforwardBarrier::ReadRefField(BaseObject* obj, RefField<false>& field) const
{
    do {
        RefField<> tmpField(field);
        BaseObject* oldRef = reinterpret_cast<BaseObject *>(tmpField.GetAddress());
        if (LIKELY_CC(!static_cast<CollectorProxy *>(&theCollector)->IsFromObject(oldRef))) {
            return oldRef;
        }

        auto weakMask = reinterpret_cast<MAddress>(oldRef) & TAG_WEAK;
        oldRef = reinterpret_cast<BaseObject *>(reinterpret_cast<MAddress>(oldRef) & (~TAG_WEAK));
        BaseObject* toObj = nullptr;
        if (static_cast<CollectorProxy *>(&theCollector)->TryForwardRefField(obj, field, toObj)) {
            return (BaseObject*)((uintptr_t)toObj | weakMask);
        }
    } while (true);
    // unreachable path.
    return nullptr;
}

BaseObject* PreforwardBarrier::ReadStaticRef(RefField<false>& field) const { return ReadRefField(nullptr, field); }

// If the object is still alive, return its toSpace object; if not, return nullptr
BaseObject* PreforwardBarrier::ReadStringTableStaticRef(RefField<false>& field) const
{
    // Note: CMC GC assumes all objects in string table are not in young space. Based on the assumption, CMC GC skip
    // read barrier in young GC
    if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
        return reinterpret_cast<BaseObject*>(field.GetFieldValue());
    }

    auto isSurvivor = [](BaseObject* obj) {
        auto gcReason = Heap::GetHeap().GetGCReason();
        RegionDesc *regionInfo = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
        return (regionInfo->IsNewObjectSinceMarking(obj) ||
            regionInfo->IsToRegion() || regionInfo->IsMarkedObject(obj));
    };

    RefField<> tmpField(field);
    BaseObject* obj = tmpField.GetTargetObject();
    if (obj != nullptr && isSurvivor(obj)) {
        return ReadRefField(nullptr, field);
    } else {
        return nullptr;
    }
}


void PreforwardBarrier::ReadStruct(HeapAddress dst, BaseObject* obj, HeapAddress src, size_t size) const
{
    if (obj != nullptr) {
        // note fix/untag dst would be better.
        obj->ForEachRefInStruct(
            [this, obj](RefField<false>& field) {
                RefField<> oldField(field);
                BaseObject* target = ReadRefField(obj, field);
                (void)target;
            },
            src, src + size);
    }

    LOGF_CHECK(memcpy_s(reinterpret_cast<void*>(dst), size, reinterpret_cast<void*>(src), size) == EOK) <<
        "read struct memcpy_s failed";
}

BaseObject* PreforwardBarrier::AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const
{
    RefField<false> tmpField(field.GetFieldValue(order));
    BaseObject* target = ReadRefField(nullptr, tmpField);
    DLOG(PBARRIER, "atomic read obj %p ref@%p: %#zx -> %p", obj, &field, tmpField.GetFieldValue(), target);
    return target;
}

void PreforwardBarrier::AtomicWriteRefField(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                            MemoryOrder order) const
{
    RefField<> newField(newRef);
    field.SetFieldValue(newField.GetFieldValue(), order);
    if (obj != nullptr) {
        DLOG(PBARRIER, "atomic write obj %p<%p>(%zu) ref@%p: %#zx", obj, obj->GetTypeInfo(), obj->GetSize(), &field,
             newField.GetFieldValue());
    } else {
        DLOG(PBARRIER, "atomic write static ref@%p: %#zx", &field, newField.GetFieldValue());
    }
}

BaseObject* PreforwardBarrier::AtomicSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* newRef,
                                                  MemoryOrder order) const
{
    HeapAddress oldValue = field.Exchange(newRef, order);
    RefField<> oldField(oldValue);
    BaseObject* oldRef = ReadRefField(nullptr, oldField);
    DLOG(BARRIER, "atomic swap obj %p<%p>(%zu) ref@%p: old %#zx(%p), new %#zx(%p)", obj, obj->GetTypeInfo(),
         obj->GetSize(), &field, oldValue, oldRef, field.GetFieldValue(), newRef);
    return oldRef;
}

bool PreforwardBarrier::CompareAndSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* oldRef,
                                               BaseObject* newRef, MemoryOrder succOrder, MemoryOrder failOrder) const
{
    HeapAddress oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
    RefField<false> oldField(oldFieldValue);
    BaseObject* oldVersion = ReadRefField(nullptr, oldField);
    while (oldVersion == oldRef) {
        RefField<> newField(newRef);
        if (field.CompareExchange(oldFieldValue, newField.GetFieldValue(), succOrder, failOrder)) {
            return true;
        }
        oldFieldValue = field.GetFieldValue(std::memory_order_seq_cst);
        RefField<false> tmp(oldFieldValue);
        oldVersion = ReadRefField(nullptr, tmp);
    }

    return false;
}

void PreforwardBarrier::CopyStructArray(BaseObject* dstObj, HeapAddress dstField, MIndex dstSize, BaseObject* srcObj,
                                        HeapAddress srcField, MIndex srcSize) const
{
    LOG_COMMON(FATAL) << "Unresolved fatal";
    UNREACHABLE_CC();
}
} // namespace common
