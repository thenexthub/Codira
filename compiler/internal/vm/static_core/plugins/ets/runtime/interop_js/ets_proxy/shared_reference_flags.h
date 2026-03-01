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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_FLAGS_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_FLAGS_H

#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::interop::js::ets_proxy {

// |------------------------------------------------------------|
// |          SharedReference flag bits represenatation:32      |
// |------------------------------------------------------------|
// |     NextIndex:28|12 | ... | MarkBit:1 | JSbit:1 | ETSbit:1 |
// |------------------------------------------------------------|
class SharedReferenceFlags {
public:
    using ValueType = ObjectPointerType;
    using IndexType = decltype(std::declval<EtsObject>().GetInteropIndex());
    static_assert(sizeof(IndexType) <= sizeof(ValueType));

    enum class Bit : ValueType {
        ETS = 1U << 0U,
        JS = 1U << 1U,
        MARK = 1U << 2U,
    };

    bool IsEmpty() const
    {
        return GetRawFlags() == EMPTY_FLAGS;
    }

    void ClearFlags()
    {
        SetRawFlags(EMPTY_FLAGS);
    }

    template <SharedReferenceFlags::Bit BIT>
    bool SetBit()
    {
        static constexpr auto BIT_VALUE = static_cast<ValueType>(BIT);
        return SetFlagCAS<BIT_VALUE>();
    }

    template <SharedReferenceFlags::Bit BIT>
    bool HasBit() const
    {
        static constexpr auto BIT_VALUE = static_cast<ValueType>(BIT);
        return (GetRawFlags() & BIT_VALUE) != 0U;
    }

    void Unmark()
    {
        UnsetFlagCAS<static_cast<ValueType>(Bit::MARK)>();
    }

    void SetNextIndex(IndexType index)
    {
        static constexpr ValueType NO_NEXT_INDEX_MASK = (1U << NEXT_INDEX_SHIFT) - 1U;
        ValueType indexMask = index << NEXT_INDEX_SHIFT;
        ValueType oldFlags = GetRawFlags();
        while (!flags_.compare_exchange_weak(oldFlags, (oldFlags & NO_NEXT_INDEX_MASK) | indexMask,
                                             std::memory_order_seq_cst)) {
        }
    }

    IndexType GetNextIndex() const
    {
        return static_cast<IndexType>(GetRawFlags() >> NEXT_INDEX_SHIFT);
    }

    friend std::ostream &operator<<(std::ostream &out, const SharedReferenceFlags &flags)
    {
        out << "idx:" << flags.GetNextIndex() << " mark:" << flags.HasBit<SharedReferenceFlags::Bit::MARK>()
            << " JS:" << flags.HasBit<SharedReferenceFlags::Bit::JS>()
            << " ETS:" << flags.HasBit<SharedReferenceFlags::Bit::ETS>();
        return out;
    }

private:
    static constexpr ValueType EMPTY_FLAGS = 0U;
    static constexpr ValueType NEXT_INDEX_SHIFT =
        sizeof(ValueType) * BITS_PER_BYTE - MarkWord::MarkWordRepresentation::HASH_SIZE;
    static_assert(static_cast<ValueType>(Bit::MARK) < (1U << NEXT_INDEX_SHIFT));

    ALWAYS_INLINE ValueType GetRawFlags() const
    {
        // Atomic with acquire order reason: data race with flags_ with dependecies on reads after the load which
        // should become visible
        return flags_.load(std::memory_order_acquire);
    }

    ALWAYS_INLINE void SetRawFlags(ValueType flags)
    {
        // Atomic with release order reason: other thread should see last value of flags
        flags_.store(flags, std::memory_order_release);
    }

    template <ValueType FLAG>
    ALWAYS_INLINE bool SetFlagCAS()
    {
        static_assert(Popcount(FLAG) == 1);
        ValueType oldFlags;
        do {
            oldFlags = GetRawFlags();
            if ((oldFlags & FLAG) != 0) {
                return false;
            }
        } while (!flags_.compare_exchange_weak(oldFlags, oldFlags | FLAG, std::memory_order_seq_cst));
        return true;
    }

    template <ValueType FLAG>
    ALWAYS_INLINE void UnsetFlagCAS()
    {
        static_assert(Popcount(FLAG) == 1);
        static constexpr ValueType MASK = ~FLAG;
        ValueType oldFlags;
        do {
            oldFlags = GetRawFlags();
            if ((oldFlags & FLAG) == 0) {
                return;
            }
        } while (!flags_.compare_exchange_weak(oldFlags, oldFlags & MASK, std::memory_order_seq_cst));
    }

    std::atomic<ValueType> flags_ {};
};

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_FLAGS_H
