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

#ifndef COMMON_COMPONENTS_HEAP_BARRIER_BARRIER_H
#define COMMON_COMPONENTS_HEAP_BARRIER_BARRIER_H

#include "common_components/common/base_object.h"
#include "common_components/common/type_def.h"
#include <vector>

namespace common {
class Collector;
// Barrier is the base class to define read/write barriers.
class Barrier {
public:
    static constexpr uint64_t TAG_WEAK = 0x01ULL;
    explicit Barrier(Collector& collector) : theCollector(collector) {}
    virtual ~Barrier() {}

    virtual BaseObject* ReadRefField(BaseObject* obj, RefField<false>& field) const;
    virtual BaseObject* ReadStaticRef(RefField<false>& field) const;
    virtual BaseObject* ReadStringTableStaticRef(RefField<false> &field) const;

    virtual void ReadStruct(HeapAddress dst, BaseObject* obj, HeapAddress src, size_t size) const;

    virtual void WriteRoot(BaseObject* obj) const;
    virtual void WriteRefField(BaseObject* obj, RefField<false>& field, BaseObject* ref) const;
    virtual void WriteBarrier(Mutator *mutator, BaseObject* obj, RefField<false>& field, BaseObject* ref) const;
    
    virtual void WriteStaticRef(RefField<false>& field, BaseObject* ref) const;
    virtual void WriteStruct(BaseObject* obj, HeapAddress dst, size_t dstLen, HeapAddress src, size_t srcLen) const;

    virtual void CopyStructArray(BaseObject* dstObj, HeapAddress dstField, MIndex dstSize,
                              BaseObject* srcObj, HeapAddress srcField, MIndex srcSize) const;

    virtual BaseObject* AtomicReadRefField(BaseObject* obj, RefField<true>& field,
                                            MemoryOrder order) const;

    virtual void AtomicWriteRefField(BaseObject* obj, RefField<true>& field, BaseObject* ref, MemoryOrder order) const;
    virtual BaseObject* AtomicSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* ref,
                                            MemoryOrder order) const;
    virtual bool CompareAndSwapRefField(BaseObject* obj, RefField<true>& field, BaseObject* oldRef, BaseObject* newRef,
                                         MemoryOrder succOrder, MemoryOrder failOrder) const;

protected:
    class LocalRefFieldContainer {
    public:
        // multi-thread unsafe.
        void Push(RefField<>* ref)
        {
            if (size_ >= CACHE_CAPACITY) {
                excessive_.push_back(ref);
            } else {
                cache_[size_] = ref;
            }
            size_++;
        }
        void VisitRefField(const RefFieldVisitor &visitor)
        {
            size_t cacheSize = size_ < CACHE_CAPACITY ? size_ : CACHE_CAPACITY;
            for (size_t i = 0; i != cacheSize; ++i) {
                visitor(*cache_[i]);
            }
            for (auto* ref : excessive_) {
                visitor(*ref);
            }
        }
    private:
        static constexpr size_t CACHE_CAPACITY = 10;
        RefField<>* cache_[CACHE_CAPACITY]{ nullptr };
        size_t size_{ 0 };
        std::vector<RefField<>*> excessive_;
    };
    Collector& theCollector;
};
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_BARRIER_BARRIER_H
