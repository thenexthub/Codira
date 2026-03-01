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


#ifndef MRT_COPY_BARRIER_H
#define MRT_COPY_BARRIER_H

#include "IdleBarrier.h"

namespace MapleRuntime {
// ForwardBarrier is the barrier for concurrent forwarding.
class ForwardBarrier : public IdleBarrier {
public:
    explicit ForwardBarrier(Collector& collector) : IdleBarrier(collector) {}

    BaseObject* ReadReference(BaseObject* obj, RefField<false>& field) const override;
    BaseObject* ReadStaticRef(RefField<false>& field) const override;
    BaseObject* ReadWeakRef(BaseObject* obj, RefField<false>& field) const override;
    void ReadStruct(MAddress dst, BaseObject* obj, MAddress src, size_t size) const override;
    void ReadStaticStruct(MAddress dst, MAddress src, size_t size, const GCTib gctib) const override;

    BaseObject* AtomicReadReference(BaseObject* obj, RefField<true>& field, MemoryOrder order) const override;
    void AtomicWriteReference(BaseObject* obj, RefField<true>& field, BaseObject* ref,
                              MemoryOrder order) const override;

    BaseObject* AtomicSwapReference(BaseObject* obj, RefField<true>& field, BaseObject* ref,
                                    MemoryOrder order) const override;

    bool CompareAndSwapReference(BaseObject* obj, RefField<true>& field, BaseObject* oldRef, BaseObject* newRef,
                                 MemoryOrder succOrder, MemoryOrder failOrder) const override;

    void CopyStructArray(BaseObject* dstObj, MAddress dstField, MIndex dstSize, BaseObject* srcObj, MAddress srcField,
                         MIndex srcSize) const override;
};
} // namespace MapleRuntime
#endif // MRT_COPY_BARRIER_H
