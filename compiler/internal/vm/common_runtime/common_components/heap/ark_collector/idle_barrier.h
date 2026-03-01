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

#ifndef COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_IDLE_BARRIER_H
#define COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_IDLE_BARRIER_H

#include "common_components/heap/barrier/barrier.h"

namespace common {
// IdleBarrier is the barrier for concurrent enum phase
class IdleBarrier : public Barrier {
public:
    explicit IdleBarrier(Collector& collector) : Barrier(collector) {}

    BaseObject* ReadRefField(BaseObject* obj, RefField<false>& field) const override;
    BaseObject* ReadStaticRef(RefField<false>& field) const override;
    void ReadStruct(HeapAddress dst, BaseObject* obj, HeapAddress src, size_t size) const override;

    void WriteRoot(BaseObject *obj) const override;
    void WriteRefField(BaseObject* obj, RefField<false>& field, BaseObject* ref) const override;
    void WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const override;

    void WriteStaticRef(RefField<false>& field, BaseObject* ref) const override;
    void WriteStruct(BaseObject* obj, HeapAddress dst, size_t dstLen, HeapAddress src, size_t srcLen) const override;

    BaseObject* AtomicReadRefField(BaseObject* obj, RefField<true>& field, MemoryOrder order) const override;
    void AtomicWriteRefField(BaseObject* obj, RefField<true>& field, BaseObject* ref,
                              MemoryOrder order) const override;

    BaseObject* AtomicSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* ref,
                                    MemoryOrder order) const override;

    bool CompareAndSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* oldRef, BaseObject* newRef,
                                 MemoryOrder succOrder, MemoryOrder failOrder) const override;

    void CopyStructArray(BaseObject* dstObj, HeapAddress dstField, MIndex dstSize, BaseObject* srcObj,
                         HeapAddress srcField, MIndex srcSize) const override;
    
    void UpdateRememberSet(BaseObject* object, BaseObject* ref) const;
};
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_IDLE_BARRIER_H
