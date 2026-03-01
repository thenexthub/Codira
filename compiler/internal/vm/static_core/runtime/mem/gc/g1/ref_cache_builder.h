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
#ifndef PANDA_RUNTIME_MEM_GC_G1_REF_CACHE_BUILDER_H
#define PANDA_RUNTIME_MEM_GC_G1_REF_CACHE_BUILDER_H

#include "runtime/mem/gc/g1/g1-gc.h"

namespace ark::mem {
/**
 * Gets reference fields from an object and puts it to the ref collection.
 * The ref collection has limited size. If there is no room in the ref collection
 * the whole object is put to the object collection.
 */
template <class LanguageConfig, bool FAST_GC = false>
class RefCacheBuilder {
    using RefVector = typename G1GC<LanguageConfig>::RefVector;

public:
    RefCacheBuilder(G1GC<LanguageConfig> *gc, PandaList<RefVector *> *refs, size_t regionSizeBits,
                    GCMarkingStackType *objectsStack)
        : gc_(gc), refs_(refs), regionSizeBits_(regionSizeBits), objectsStack_(objectsStack)
    {
    }

    bool operator()(ObjectHeader *object, ObjectHeader *field, uint32_t offset, [[maybe_unused]] bool isVolatile)
    {
        if (!gc_->InGCSweepRange(field)) {
            allCrossRegionRefsProcessed_ &= ark::mem::IsSameRegion(object, field, regionSizeBits_);
            return true;
        }
        RefVector *refVector = refs_->back();
        if (refVector->size() == refVector->capacity()) {
            // There is no room to store references.
            // Create a new vector and store everithing inside it
            auto *newRefVector = gc_->GetInternalAllocator()->template New<RefVector>();
            ASSERT(newRefVector != nullptr);
            newRefVector->reserve(refVector->capacity() * 2U);
            refs_->push_back(newRefVector);
            refVector = newRefVector;
        }
        ASSERT(refVector->size() < refVector->capacity());
        if constexpr (!FAST_GC) {
            // There is room to store references
            ASSERT(objectsStack_ != nullptr);
            if (gc_->mixedMarker_.MarkIfNotMarkedInCollectionSet(field)) {
                objectsStack_->PushToStack(object, field);
            }
        }
        refVector->emplace_back(object, offset);
        return true;
    }

    bool AllCrossRegionRefsProcessed() const
    {
        return allCrossRegionRefsProcessed_;
    }

private:
    G1GC<LanguageConfig> *gc_;
    PandaList<RefVector *> *refs_;
    bool allCrossRegionRefsProcessed_ = true;
    size_t regionSizeBits_;
    // object stack pointer which will be used to store unmarked objects if it is not nullptr
    GCMarkingStackType *objectsStack_ = nullptr;
};
}  // namespace ark::mem
#endif  // PANDA_RUNTIME_MEM_GC_G1_REF_CACHE_BUILDER_H
