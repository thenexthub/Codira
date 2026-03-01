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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_LOCATION_MASK_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_LOCATION_MASK_H

#include "mem/arena_allocator.h"
#include "utils/arena_containers.h"

namespace panda::compiler {
/**
 * Helper-class to hold information about registers and stack slots usage.
 */
class LocationMask {
public:
    explicit LocationMask(ArenaAllocator *allocator) : mask_(allocator->Adapter()), usage_(allocator->Adapter()) {}
    ~LocationMask() = default;
    DEFAULT_COPY_SEMANTIC(LocationMask);
    DEFAULT_MOVE_SEMANTIC(LocationMask);

    template <typename T>
    void Init(const T &mask)
    {
        usage_.resize(mask.size());
        mask_.resize(mask.size());
        for (size_t idx = 0; idx < mask.size(); idx++) {
            if (mask.test(idx)) {
                Set(idx);
            } else {
                Reset(idx);
            }
        }
    }

    void Resize(size_t size)
    {
        mask_.resize(size, false);
        usage_.resize(size, false);
    }

    void Set(size_t position)
    {
        ASSERT(position < mask_.size());
        mask_[position] = true;
        usage_[position] = true;
    }

    void Reset(size_t position)
    {
        ASSERT(position < mask_.size());
        mask_[position] = false;
    }

    bool IsSet(size_t position) const
    {
        ASSERT(position < mask_.size());
        return mask_[position];
    }

    void Reserve(size_t reserved_bit)
    {
        reserved_bit_ = reserved_bit;
        Set(reserved_bit);
    }

    std::optional<size_t> GetReserved() const
    {
        return reserved_bit_;
    }

    const auto &GetVector() const
    {
        return mask_;
    }

    std::optional<size_t> GetNextNotSet(size_t first_bit = 0)
    {
        for (size_t r = first_bit; r < mask_.size(); r++) {
            if (!mask_[r]) {
                Set(r);
                return r;
            }
        }
        for (size_t r = 0; r < first_bit; r++) {
            if (!mask_[r]) {
                Set(r);
                return r;
            }
        }
        return std::nullopt;
    }

    bool AllSet() const
    {
        return std::all_of(mask_.begin(), mask_.end(), [](bool v) { return v; });
    }

    size_t GetUsedCount() const
    {
        return std::count(usage_.begin(), usage_.end(), true);
    }

    size_t GetSize() const
    {
        return mask_.size();
    }

    bool Empty() const
    {
        return mask_.empty();
    }

    void Dump(std::ostream *out) const
    {
        for (bool val : mask_) {
            (*out) << val;
        }
        (*out) << "\n";
    }

private:
    ArenaVector<bool> mask_;
    ArenaVector<bool> usage_;
    std::optional<size_t> reserved_bit_ {std::nullopt};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_LOCATION_MASK_H