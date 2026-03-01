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

#ifndef LIBPANDABASE_UTILS_REGMASK_H
#define LIBPANDABASE_UTILS_REGMASK_H

#include <array>
#include "utils/bit_utils.h"
#include "utils/type_helpers.h"

namespace panda {

template <typename T, size_t N>
static constexpr size_t MakeMask(const std::array<T, N> &indexes)
{
    size_t res = 0;
    for (size_t i : indexes) {
        res |= (1UL << i);
    }
    return res;
}

template <typename... Indexes>
static constexpr size_t MakeMask(Indexes... indexes)
{
    return ((1UL << helpers::ToUnsigned(indexes)) | ...);
}

template <typename... Indexes>
static constexpr size_t MakeMaskByExcluding(size_t width, Indexes... indexes)
{
    size_t res = (1ULL << width) - 1;
    size_t exclude = ((1ULL << helpers::ToUnsigned(indexes)) | ...);
    return res & ~exclude;
}

/**
 * Base struct for registers mask, template-parametrized by number of registers.
 * Currently we don't support registers number greater than 32.
 * Previously, Regmask class just inherited std::bitset, but std::bitset has poor constexpr support, that was the main
 * reason to implement own RegMask class.
 * Regmask has interface, similar to std::bitset.
 */
template <size_t N>
class RegMaskImpl {
public:
    // We don't support architectures with CPU registers number, greater than 32.
    static_assert(N <= sizeof(uint32_t) * BITS_PER_BYTE);

    using ValueType = uint32_t;
    using Self = RegMaskImpl<N>;

    constexpr RegMaskImpl() = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr RegMaskImpl(ValueType v) : value_(v) {}

    constexpr ValueType GetValue() const
    {
        return value_;
    }

    static constexpr size_t Size()
    {
        return N;
    }

    constexpr bool Any() const
    {
        return value_ != 0;
    }

    constexpr bool None() const
    {
        return value_ == 0;
    }

    constexpr bool Test(size_t bit) const
    {
        ASSERT(bit < Size());
        return ((value_ >> static_cast<ValueType>(bit)) & 1U) != 0;
    }

    constexpr void Set()
    {
        value_ = ~static_cast<ValueType>(0U);
    }
    constexpr void Reset()
    {
        value_ = 0;
    }

    constexpr void Set(size_t bit)
    {
        ASSERT(bit < Size());
        value_ |= (1U << bit);
    }

    constexpr void Set(size_t bit, bool value)
    {
        ASSERT(bit < Size());
        if (value) {
            Set(bit);
        } else {
            Reset(bit);
        }
    }

    constexpr void Reset(size_t bit)
    {
        ASSERT(bit < Size());
        value_ &= ~(1U << bit);
    }

    constexpr size_t Count() const
    {
        return Popcount(GetValue());
    }

    constexpr bool CountIsEven() const
    {
        return (Count() & 1U) == 0;
    }

    // Get number of registers from tail to the given register, counting only set bits.
    // Given `reg` is not counted even if it is set.
    constexpr size_t GetDistanceFromTail(size_t reg) const
    {
        ASSERT(reg < Size());
        uint32_t val = GetValue() & ((1U << reg) - 1);
        return Popcount(val);
    }

    // Get number of registers from head to the given register, counting only set bits.
    // Given `reg` is not counted even if it is set.
    constexpr size_t GetDistanceFromHead(size_t reg) const
    {
        if (reg < (Size() - 1)) {
            uint32_t val = GetValue() & ~((1U << (reg + 1)) - 1);
            return Popcount(val);
        }
        if (reg == Size() - 1) {
            return 0;
        }
        // reg > (Size() - 1), something goes wrong...
        UNREACHABLE();
        return 0;
    }

    constexpr uint32_t GetMinRegister() const
    {
        ASSERT(Any());
        return panda::Ctz(GetValue());
    }

    constexpr uint32_t GetMaxRegister() const
    {
        ASSERT(Any());
        return (sizeof(decltype(GetValue())) * BITS_PER_BYTE) - 1 - panda::Clz(GetValue());
    }

    constexpr Self operator~() const
    {
        return Self(~GetValue());
    }

    constexpr Self operator&(Self other) const
    {
        return Self(GetValue() & other.GetValue());
    }

    constexpr Self operator|(Self other) const
    {
        return Self(GetValue() | other.GetValue());
    }

    constexpr Self operator^(Self other) const
    {
        return Self(GetValue() ^ other.GetValue());
    }

    constexpr Self operator&=(Self other)
    {
        value_ &= other.GetValue();
        return *this;
    }

    constexpr Self operator|=(Self other)
    {
        value_ |= other.GetValue();
        return *this;
    }

    constexpr Self operator^=(Self other)
    {
        value_ ^= other.GetValue();
        return *this;
    }
    constexpr bool operator[](size_t bit) const
    {
        return Test(bit);
    }
    constexpr bool operator==(Self other) const
    {
        return value_ == other.value_;
    }

    constexpr bool operator!=(Self other) const
    {
        return !(*this == other);
    }

    constexpr ValueType to_ulong() const
    {
        return GetValue();
    }

    void Dump(std::ostream &out = std::cerr) const
    {
        out << "Regmask[" << N << "]: ";
        for (size_t i = 0; i < N; i++) {
            if (Test(i)) {
                out << i << " ";
            }
        }
    }

    // The following methods are for compatibility with `std::bitset`, since we used `std::bitset` before.
    // Don't use these method in a new code.
    constexpr bool any() const
    {
        return Any();
    }
    constexpr bool none() const
    {
        return None();
    }
    constexpr bool test(size_t bit) const
    {
        return Test(bit);
    }
    constexpr void set(size_t bit)
    {
        Set(bit);
    }
    constexpr void set(size_t bit, bool value)
    {
        Set(bit, value);
    }
    constexpr Self set()
    {
        Set();
        return *this;
    }
    constexpr Self reset()
    {
        Reset();
        return *this;
    }
    constexpr void reset(size_t bit)
    {
        Reset(bit);
    }
    constexpr size_t count() const
    {
        return Count();
    }
    constexpr size_t size() const
    {
        return Size();
    }

private:
    ValueType value_ {0};
};

static constexpr uint8_t REGISTERS_NUM = 32;
static constexpr uint8_t VREGISTERS_NUM = 32;

using RegMask = RegMaskImpl<REGISTERS_NUM>;
using VRegMask = RegMaskImpl<VREGISTERS_NUM>;

inline std::ostream &operator<<(std::ostream &stream, const RegMask &mask)
{
    mask.Dump(stream);
    return stream;
}

}  // namespace panda

#endif  // LIBPANDABASE_UTILS_REGMASK_H
