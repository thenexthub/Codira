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
#ifndef PANDA_RUNTIME_MEM_GC_G1_COLLECTION_SET_H
#define PANDA_RUNTIME_MEM_GC_G1_COLLECTION_SET_H

#include "libarkbase/macros.h"
#include "libarkbase/utils/range.h"
#include "runtime/mem/region_space.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::mem {

/// Represent a set of regions grouped by type.
class CollectionSet {
public:
    CollectionSet()
    {
        tenuredBegin_ = 0;
        humongousBegin_ = 0;
    }

    template <typename Container>
    explicit CollectionSet(const Container &set) : collectionSet_(set.begin(), set.end())
    {
        tenuredBegin_ = collectionSet_.size();
        humongousBegin_ = tenuredBegin_;
    }

    explicit CollectionSet(PandaVector<Region *> &&youngRegions) : collectionSet_(youngRegions)
    {
        tenuredBegin_ = collectionSet_.size();
        humongousBegin_ = tenuredBegin_;
    }

    ~CollectionSet() = default;

    void AddRegion(Region *region)
    {
        ASSERT(region->HasFlag(RegionFlag::IS_OLD));
        collectionSet_.push_back(region);
        if (!region->HasFlag(RegionFlag::IS_LARGE_OBJECT) && humongousBegin_ != collectionSet_.size()) {
            std::swap(collectionSet_[humongousBegin_], collectionSet_.back());
            ++humongousBegin_;
        }
    }

    auto begin()  // NOLINT(readability-identifier-naming)
    {
        return collectionSet_.begin();
    }

    auto begin() const  // NOLINT(readability-identifier-naming)
    {
        return collectionSet_.begin();
    }

    auto end()  // NOLINT(readability-identifier-naming)
    {
        return collectionSet_.end();
    }

    auto end() const  // NOLINT(readability-identifier-naming)
    {
        return collectionSet_.end();
    }

    size_t size() const  // NOLINT(readability-identifier-naming)
    {
        return collectionSet_.size();
    }

    bool empty() const  // NOLINT(readability-identifier-naming)
    {
        return collectionSet_.empty();
    }

    void clear()  // NOLINT(readability-identifier-naming)
    {
        collectionSet_.clear();
        tenuredBegin_ = 0;
        humongousBegin_ = 0;
    }

    auto Young()
    {
        return Range<PandaVector<Region *>::iterator>(begin(), begin() + tenuredBegin_);
    }

    auto Young() const
    {
        return Range<PandaVector<Region *>::const_iterator>(begin(), begin() + tenuredBegin_);
    }

    auto Tenured()
    {
        return Range<PandaVector<Region *>::iterator>(begin() + tenuredBegin_, begin() + humongousBegin_);
    }

    auto Tenured() const
    {
        return Range<PandaVector<Region *>::const_iterator>(begin() + tenuredBegin_, begin() + humongousBegin_);
    }

    auto Humongous()
    {
        return Range<PandaVector<Region *>::iterator>(begin() + humongousBegin_, end());
    }

    auto Humongous() const
    {
        return Range<PandaVector<Region *>::const_iterator>(begin() + humongousBegin_, end());
    }

    auto Movable()
    {
        return Range<PandaVector<Region *>::iterator>(begin(), begin() + humongousBegin_);
    }

    auto Movable() const
    {
        return Range<PandaVector<Region *>::const_iterator>(begin(), begin() + humongousBegin_);
    }

    DEFAULT_COPY_SEMANTIC(CollectionSet);
    DEFAULT_MOVE_SEMANTIC(CollectionSet);

private:
    PandaVector<Region *> collectionSet_;
    size_t tenuredBegin_;
    size_t humongousBegin_;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_G1_COLLECTION_SET_H
