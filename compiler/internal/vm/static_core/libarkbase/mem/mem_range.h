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

#ifndef PANDA_LIBPANDABASE_MEM_MEM_RANGE_H
#define PANDA_LIBPANDABASE_MEM_MEM_RANGE_H

#include <ostream>

namespace ark::mem {

/// Represents range of bytes [start_address, end_address]
class MemRange final {
public:
    MemRange(uintptr_t startAddress, uintptr_t endAddress) : startAddress_(startAddress), endAddress_(endAddress)
    {
        ASSERT(endAddress_ >= startAddress_);
    }

    bool IsAddressInRange(uintptr_t addr) const
    {
        return (addr >= startAddress_) && (addr <= endAddress_);
    }

    uintptr_t GetStartAddress() const
    {
        return startAddress_;
    }

    uintptr_t GetEndAddress() const
    {
        return endAddress_;
    }

    bool IsIntersect(const MemRange &other) const
    {
        return ((endAddress_ >= other.startAddress_) && (endAddress_ <= other.endAddress_)) ||
               ((startAddress_ >= other.startAddress_) && (startAddress_ <= other.endAddress_)) ||
               ((startAddress_ < other.startAddress_) && (endAddress_ > other.endAddress_));
    }

    bool Contains(const MemRange &other) const
    {
        return startAddress_ <= other.startAddress_ && endAddress_ >= other.endAddress_;
    }

    bool Contains(uintptr_t addr) const
    {
        return startAddress_ <= addr && addr < endAddress_;
    }

    friend std::ostream &operator<<(std::ostream &os, MemRange const &memRange)
    {
        return os << std::hex << "[ 0x" << memRange.GetStartAddress() << " : 0x" << memRange.GetEndAddress() << "]";
    }

    ~MemRange() = default;

    DEFAULT_COPY_SEMANTIC(MemRange);
    DEFAULT_MOVE_SEMANTIC(MemRange);

private:
    uintptr_t startAddress_;  ///< Address of the first byte in memory range
    uintptr_t endAddress_;    ///< Address of the last byte in memory range
};

}  // namespace ark::mem

#endif  // PANDA_LIBPANDABASE_MEM_MEM_RANGE_H
