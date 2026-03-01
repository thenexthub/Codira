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

#ifndef LIBPANDABASE_MEM_MEM_RANGE_H
#define LIBPANDABASE_MEM_MEM_RANGE_H

#include <ostream>

namespace panda::mem {

/**
 * Represents range of bytes [start_address, end_address]
 */
class MemRange {
public:
    MemRange(uintptr_t start_address, uintptr_t end_address) : start_address_(start_address), end_address_(end_address)
    {
        ASSERT(end_address_ >= start_address_);
    }

    bool IsAddressInRange(uintptr_t addr) const
    {
        return (addr >= start_address_) && (addr <= end_address_);
    }

    uintptr_t GetStartAddress() const
    {
        return start_address_;
    }

    uintptr_t GetEndAddress() const
    {
        return end_address_;
    }

    bool IsIntersect(const MemRange &other) const
    {
        return ((end_address_ >= other.start_address_) && (end_address_ <= other.end_address_)) ||
               ((start_address_ >= other.start_address_) && (start_address_ <= other.end_address_)) ||
               ((start_address_ < other.start_address_) && (end_address_ > other.end_address_));
    }

    bool Contains(const MemRange &other) const
    {
        return start_address_ <= other.start_address_ && end_address_ >= other.end_address_;
    }

    bool Contains(uintptr_t addr) const
    {
        return start_address_ <= addr && addr < end_address_;
    }

    friend std::ostream &operator<<(std::ostream &os, MemRange const &mem_range)
    {
        return os << std::hex << "[ 0x" << mem_range.GetStartAddress() << " : 0x" << mem_range.GetEndAddress() << "]";
    }

    virtual ~MemRange() = default;

    DEFAULT_COPY_SEMANTIC(MemRange);
    DEFAULT_MOVE_SEMANTIC(MemRange);

private:
    uintptr_t start_address_;  /// < address of the first byte in memory range
    uintptr_t end_address_;    /// < address of the last byte in memory range
};

}  // namespace panda::mem

#endif  // LIBPANDABASE_MEM_MEM_RANGE_H
