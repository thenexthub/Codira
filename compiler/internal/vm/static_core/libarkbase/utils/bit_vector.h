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

#ifndef PANDA_BIT_VECTOR_H
#define PANDA_BIT_VECTOR_H

#include "libarkbase/globals.h"
#include "libarkbase/mem/arena_allocator.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/small_vector.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/type_helpers.h"

#include <vector>

namespace ark {
template <bool IS_CONST>
class BitVectorIterator;
}  // namespace ark

namespace std {  // NOLINT(cert-dcl58-cpp)
template <bool IS_CONST>
// NOLINTNEXTLINE(readability-identifier-naming)
inline void fill(ark::BitVectorIterator<IS_CONST> first, ark::BitVectorIterator<IS_CONST> last, bool value);
}  // namespace std

namespace ark {

class BitReference final {
    using WordType = uint32_t;

public:
    BitReference(WordType *data, WordType mask) : data_(data), mask_(mask)
    {
        ASSERT(Popcount(mask) == 1);
    }
    BitReference(const BitReference &) = default;
    BitReference(BitReference &&) = default;

    ~BitReference() = default;

    BitReference &operator=(bool v)
    {
        if (v) {
            *data_ |= mask_;
        } else {
            *data_ &= ~mask_;
        }
        return *this;
    }
    BitReference &operator=(const BitReference &other)
    {
        if (&other != this) {
            *this = bool(other);
        }

        return *this;
    }
    BitReference &operator=(BitReference &&other) noexcept
    {
        *this = bool(other);
        return *this;
    }
    // NOLINTNEXTLINE(google-explicit-constructor)
    operator bool() const
    {
        return (*data_ & mask_) != 0;
    }
    bool operator==(const BitReference &other) const
    {
        return bool(*this) == bool(other);
    }
    bool operator<(const BitReference &other) const
    {
        return !bool(*this) && bool(other);
    }

private:
    WordType *data_;
    WordType mask_;
};

template <bool IS_CONST>
class BitVectorIterator {
    using WordType = std::conditional_t<IS_CONST, const uint32_t, uint32_t>;
    using Reference = std::conditional_t<IS_CONST, bool, BitReference>;
    using Pointer = std::conditional_t<IS_CONST, const bool *, BitReference *>;

    static constexpr size_t WORD_BITS = BITS_PER_BYTE * sizeof(WordType);

public:
    // NOLINTBEGIN(readability-identifier-naming)
    using iterator_category = std::random_access_iterator_tag;
    using value_type = bool;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;
    // NOLINTEND(readability-identifier-naming)

    BitVectorIterator(WordType *data, int offset) : data_(data), offset_(offset) {}
    ~BitVectorIterator() = default;

    DEFAULT_COPY_SEMANTIC(BitVectorIterator);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(BitVectorIterator);

    template <bool IS_CONST_OTHER>
    bool operator==(const BitVectorIterator<IS_CONST_OTHER> &other) const
    {
        return data_ == other.data_ && offset_ == other.offset_;
    }
    template <bool IS_CONST_OTHER>
    bool operator<(const BitVectorIterator<IS_CONST_OTHER> &other) const
    {
        return data_ < other.data_ || (data_ == other.data_ && offset_ < other.offset_);
    }
    template <bool IS_CONST_OTHER>
    bool operator!=(const BitVectorIterator<IS_CONST_OTHER> &other) const
    {
        return !(*this == other);
    }
    template <bool IS_CONST_OTHER>
    bool operator>(const BitVectorIterator<IS_CONST_OTHER> &other) const
    {
        return other < *this;
    }
    template <bool IS_CONST_OTHER>
    bool operator<=(const BitVectorIterator<IS_CONST_OTHER> &other) const
    {
        return !(other < *this);
    }
    template <bool IS_CONST_OTHER>
    bool operator>=(const BitVectorIterator<IS_CONST_OTHER> &other) const
    {
        return !(*this < other);
    }

    Reference operator*() const
    {
        if constexpr (IS_CONST) {  // NOLINT(readability-braces-around-statements)
            return (*data_ & (1U << helpers::ToUnsigned(offset_))) != 0;
        } else {  // NOLINT(readability-misleading-indentation)
            ASSERT(offset_ >= 0);
            return BitReference(data_, 1U << helpers::ToUnsigned(offset_));
        }
    }
    BitVectorIterator &operator++()
    {
        BumpUp();
        return *this;
    }
    BitVectorIterator operator++(int)  // NOLINT(cert-dcl21-cpp)
    {
        BitVectorIterator tmp = *this;
        BumpUp();
        return tmp;
    }
    BitVectorIterator &operator--()
    {
        BumpDown();
        return *this;
    }
    BitVectorIterator operator--(int)  // NOLINT(cert-dcl21-cpp)
    {
        BitVectorIterator tmp = *this;
        BumpDown();
        return tmp;
    }
    BitVectorIterator &operator+=(difference_type v)
    {
        Increase(v);
        return *this;
    }
    BitVectorIterator &operator-=(difference_type v)
    {
        Increase(-v);
        return *this;
    }
    BitVectorIterator operator+(difference_type v) const
    {
        auto tmp = *this;
        return tmp += v;
    }
    BitVectorIterator operator-(difference_type v) const
    {
        auto tmp = *this;
        return tmp -= v;
    }
    difference_type operator-(BitVectorIterator other) const
    {
        return (data_ - other.data_) * WORD_BITS + (offset_ - other.offset_);
    }

    Reference operator[](difference_type v) const
    {
        return *(*this + v);
    }

private:
    void BumpUp()
    {
        if (offset_++ == (WORD_BITS - 1)) {
            offset_ = 0;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ++data_;
        }
    }
    void BumpDown()
    {
        if (offset_-- == 0) {
            offset_ = WORD_BITS - 1;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            --data_;
        }
    }
    void Increase(ptrdiff_t n)
    {
        difference_type diff = offset_ + n;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        data_ += diff / helpers::ToSigned(WORD_BITS);
        diff = diff % helpers::ToSigned(WORD_BITS);
        if (diff < 0) {
            diff += helpers::ToSigned(WORD_BITS);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            --data_;
        }
        offset_ = diff;
    }

private:
    WordType *data_;
    int offset_;

    friend class BitVectorIterator<!IS_CONST>;
    template <bool IS_CONST_ITER>
    // NOLINTNEXTLINE(readability-redundant-declaration,readability-identifier-naming)
    friend void std::fill(ark::BitVectorIterator<IS_CONST_ITER> first, ark::BitVectorIterator<IS_CONST_ITER> last,
                          bool value);
};

/**
 * BitVector implements std::vector<bool> interface with some extensions:
 *   - ability to get storage data
 *   - iterate over set/zero indices
 *   - set/clear bits with automatic resizing (SetBit/ClearBit)
 *   - ability to operate over existing storage, without expanding (`FixedSize` template parameter)
 *   - several auxiliary methods: Popcount, GetHighestBitSet, etc
 * @tparam FixedSize whether storage can be expanded, if true - storage has fixed size and cannot be expanded
 * @tparam Allocator allocator to be used for underlying std::vector
 */
template <bool FIXED_SIZE, typename Allocator = StdAllocatorStub>
class BitVectorBase final {
    using WordType = uint32_t;
    using VectorType = std::conditional_t<FIXED_SIZE, Span<WordType>,
                                          std::vector<WordType, typename Allocator::template AdapterType<WordType>>>;

    static constexpr size_t WORD_BITS = BITS_PER_BYTE * sizeof(WordType);

public:
    BitVectorBase() : storage_(Allocator::GetInstance()->Adapter()) {}

    explicit BitVectorBase(Allocator *allocator) : storage_(allocator->Adapter())
    {
        static_assert(!FIXED_SIZE);
    }

    BitVectorBase(size_t size, Allocator *allocator)
        : size_(size), storage_(GetWordIndex(size) + 1, allocator->Adapter())
    {
        static_assert(!FIXED_SIZE);
    }

    BitVectorBase(size_t size, typename Allocator::template AdapterType<WordType> adapter)
        : size_(size), storage_(GetWordIndex(size) + 1, adapter)
    {
        static_assert(!FIXED_SIZE);
    }

    BitVectorBase(void *data, size_t bits)
        : size_(bits),
          storage_(reinterpret_cast<WordType *>(data), GetWordIndex(bits) + (((bits % WORD_BITS) != 0) ? 1 : 0))
    {
        static_assert(FIXED_SIZE);
    }

    explicit BitVectorBase(Span<WordType> span) : size_(span.size() * WORD_BITS), storage_(span)
    {
        static_assert(FIXED_SIZE);
    }

    ~BitVectorBase() = default;

    DEFAULT_COPY_SEMANTIC(BitVectorBase);
    DEFAULT_MOVE_SEMANTIC(BitVectorBase);

    // NOLINTBEGIN(readability-identifier-naming)
    using value_type = bool;
    using container_value_type = WordType;
    using reference = BitReference;
    using const_reference = bool;
    using pointer = BitReference *;
    using const_pointer = const BitReference *;
    using iterator = BitVectorIterator<false>;
    using const_iterator = BitVectorIterator<true>;
    using reverse_iterator = std::reverse_iterator<BitVectorIterator<false>>;
    using reverse_const_iterator = std::reverse_iterator<BitVectorIterator<true>>;
    using difference_type = ptrdiff_t;
    // NOLINTEND(readability-identifier-naming)

    template <bool BIT_VALUE>
    class BitIndexIterator;

    // NOLINTBEGIN(readability-identifier-naming)
    const_iterator begin() const
    {
        return const_iterator(storage_.data(), 0);
    }
    iterator begin()
    {
        return iterator(storage_.data(), 0);
    }
    const_iterator cbegin() const
    {
        return const_iterator(storage_.data(), 0);
    }
    const_iterator end() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return const_iterator(storage_.data() + GetWordIndex(size_), size_ % WORD_BITS);
    }
    iterator end()
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return iterator(storage_.data() + GetWordIndex(size_), size_ % WORD_BITS);
    }
    const_iterator cend() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return const_iterator(storage_.data() + GetWordIndex(size_), size_ % WORD_BITS);
    }

    reference operator[](size_t n)
    {
        return *iterator(&storage_[n / WORD_BITS], n % WORD_BITS);
    }

    const_reference operator[](size_t n) const
    {
        return *const_iterator(&storage_[n / WORD_BITS], n % WORD_BITS);
    }

    size_t size() const
    {
        return size_;
    }

    bool empty() const
    {
        return size() == 0;
    }

    size_t capacity() const
    {
        return storage_.capacity() * WORD_BITS;
    }

    void resize(size_t bits)
    {
        static_assert(!FIXED_SIZE);
        auto initial_size = size();
        if (bits > initial_size) {
            EnsureSpace(bits);
            std::fill(begin() + initial_size, begin() + bits, false);
        }
        size_ = bits;
    }

    void clear()
    {
        static_assert(!FIXED_SIZE);
        std::fill(begin(), end(), false);
        size_ = 0;
    }

    void push_back(bool v)
    {
        static_assert(!FIXED_SIZE);
        EnsureSpace(size() + 1);
        (*this)[size()] = v;
        size_++;
    }

    void *data()
    {
        return storage_.data();
    }
    const void *data() const
    {
        return storage_.data();
    }
    // NOLINTEND(readability-identifier-naming)

    Span<uint8_t> GetDataSpan()
    {
        auto sp = Span<WordType>(storage_.data(), storage_.size());
        return sp.SubSpan<uint8_t>(0, storage_.size() * sizeof(WordType));
    }

    Span<uint32_t> GetContainerDataSpan()
    {
        return Span<WordType>(storage_.data(), storage_.size());
    }

    Span<const uint32_t> GetContainerDataSpan() const
    {
        return Span<const WordType>(storage_.data(), storage_.size());
    }

    /// Set bit. If size of vector is less than bit, vector will be resized.
    void SetBit(size_t index)
    {
        EnsureSpace(index + 1);
        GetWord(index) |= GetBitMask(index);
        if (index >= size()) {
            size_ = index + 1;
        }
    }

    /// Clear bit. If size of vector is less than bit, vector will be resized.
    void ClearBit(size_t index)
    {
        EnsureSpace(index + 1);
        GetWord(index) &= ~GetBitMask(index);
        if (index >= size()) {
            size_ = index + 1;
        }
    }

    bool GetBit(size_t index) const
    {
        if (index >= size()) {
            return false;
        }
        return (GetWord(index) & GetBitMask(index)) != 0;
    }

    size_t PopCount() const
    {
        return PopCount(size());
    }

    size_t PopCount(size_t lastIndex) const
    {
        if (empty()) {
            return 0;
        }
        size_t res = 0;
        size_t words = GetWordIndex(lastIndex);
        size_t offset = lastIndex % WORD_BITS;
        for (size_t i = 0; i < words; i++) {
            res += Popcount(storage_[i]);
        }
        if (offset != 0) {
            res += Popcount(storage_[words] & ((1U << offset) - 1));
        }
        return res;
    }

    void Reset()
    {
        std::fill(storage_.begin(), storage_.end(), 0);
    }

    int GetHighestBitSet() const
    {
        for (int i = storage_.size() - 1; i >= 0; i--) {
            if (storage_[i] != 0) {
                return (BITS_PER_UINT32 - 1) - Clz(storage_[i]) + (i * WORD_BITS);
            }
        }
        return -1;
    }

    size_t GetSizeInBytes() const
    {
        return BitsToBytesRoundUp(size());
    }

    size_t GetContainerSize() const
    {
        return storage_.size();
    }

    size_t GetContainerSizeInBytes() const
    {
        return storage_.size() * sizeof(WordType);
    }

    BitVectorBase<true> GetFixed()
    {
        return BitVectorBase<true>(reinterpret_cast<WordType *>(data()), size());
    }

    template <bool IS_FIXED_SIZE, typename AllocatorType>
    bool operator==(const BitVectorBase<IS_FIXED_SIZE, AllocatorType> &other) const
    {
        return size_ == other.size() && (std::memcmp(data(), other.data(), BitsToBytesRoundUp(size_)) == 0);
    }

    template <bool IS_FIXED_SIZE, typename AllocatorType>
    bool operator!=(const BitVectorBase<IS_FIXED_SIZE, AllocatorType> &other) const
    {
        return !(*this == other);
    }

    template <bool IS_FIXED_SIZE, typename AllocatorType>
    BitVectorBase<IS_FIXED_SIZE, AllocatorType> &operator|=(const BitVectorBase<IS_FIXED_SIZE, AllocatorType> &other)
    {
        auto minSize = std::min<size_t>(storage_.size(), other.storage_.size());
        for (size_t i = 0; i < minSize; ++i) {
            storage_[i] |= other.storage_[i];
        }
        for (size_t i = minSize; i < other.storage_.size(); ++i) {
            storage_.push_back(other.storage_[i]);
        }
        size_ = std::max<size_t>(size_, other.size_);
        return *this;
    }

    template <bool BIT_VALUE>
    class BitsIndicesRange final {
    public:
        explicit BitsIndicesRange(const BitVectorBase &vector) : vector_(vector) {}
        ~BitsIndicesRange() = default;
        NO_COPY_SEMANTIC(BitsIndicesRange);
        NO_MOVE_SEMANTIC(BitsIndicesRange);

        // NOLINTNEXTLINE(readability-identifier-naming)
        auto begin()
        {
            return BitIndexIterator<BIT_VALUE>(vector_, 0);
        }

        // NOLINTNEXTLINE(readability-identifier-naming)
        auto end()
        {
            return BitIndexIterator<BIT_VALUE>(vector_, BitIndexIterator<BIT_VALUE>::INVAILD_OFFSET);
        }

    private:
        const BitVectorBase &vector_;
    };
    auto GetSetBitsIndices() const
    {
        return BitsIndicesRange<true>(*this);
    }
    auto GetZeroBitsIndices() const
    {
        return BitsIndicesRange<false>(*this);
    }

private:
    uint32_t &GetWord(size_t index)
    {
        return storage_[GetWordIndex(index)];
    }

    uint32_t GetWord(size_t index) const
    {
        return storage_[GetWordIndex(index)];
    }

    static constexpr size_t GetWordIndex(size_t index)
    {
        return index / (WORD_BITS);
    }

    static constexpr uint32_t GetBitMask(size_t index)
    {
        return 1U << (index & (WORD_BITS - 1));
    }

    void EnsureSpace(size_t bits)
    {
        static constexpr size_t GROW_MULTIPLIER = 2;
        size_t words = RoundUp(bits, WORD_BITS) / WORD_BITS;
        if (words > storage_.size()) {
            if constexpr (FIXED_SIZE) {  // NOLINT(readability-braces-around-statements)
                UNREACHABLE();
            } else {  // NOLINT(readability-misleading-indentation)
                storage_.resize(std::max(storage_.size() * GROW_MULTIPLIER, words));
            }
        }
    }

private:
    size_t size_ {0};
    VectorType storage_;
};

template <bool IS_FIXED, typename Allocator>
template <bool BIT_VALUE>
class BitVectorBase<IS_FIXED, Allocator>::BitIndexIterator {
    using WordType = uint32_t;

public:
    static constexpr int32_t INVAILD_OFFSET = std::numeric_limits<uint32_t>::max();

    BitIndexIterator(const BitVectorBase &data, int32_t offset) : data_(data), offset_(offset)
    {
        if (data_.empty()) {
            offset_ = INVAILD_OFFSET;
        } else if (offset_ != INVAILD_OFFSET && data_.GetBit(offset_) != BIT_VALUE) {
            Next(1);
        }
    }

    ~BitIndexIterator() = default;

    NO_COPY_SEMANTIC(BitIndexIterator);
    NO_MOVE_SEMANTIC(BitIndexIterator);

    bool operator==(const BitIndexIterator &rhs) const
    {
        return data_.data() == rhs.data_.data() && offset_ == rhs.offset_;
    }

    bool operator!=(const BitIndexIterator &rhs) const
    {
        return !(*this == rhs);
    }

    BitIndexIterator &operator++()
    {
        Next(1);
        return *this;
    }

    uint32_t operator*()
    {
        return offset_;
    }

private:
    void Next(uint32_t val)
    {
        ASSERT(val != 0);
        int step = (val > 0) ? 1 : -1;
        for (; val != 0; val--) {
            if ((offset_ + 1) == helpers::ToSigned(data_.size())) {
                offset_ = INVAILD_OFFSET;
                return;
            }
            for (offset_ += step; data_.GetBit(offset_) != BIT_VALUE; offset_ += step) {
                if ((offset_ + 1) == helpers::ToSigned(data_.size())) {
                    offset_ = INVAILD_OFFSET;
                    return;
                }
            }
        }
    }

private:
    const BitVectorBase &data_;
    int32_t offset_;
};

namespace internal {
template <bool IS_CONST>
inline void FillBitVector(ark::BitVectorIterator<IS_CONST> first, ark::BitVectorIterator<IS_CONST> last, bool value)
{
    for (; first != last; ++first) {
        *first = value;
    }
}
}  // namespace internal

template <typename Allocator = StdAllocatorStub>
using BitVector = BitVectorBase<false, Allocator>;
using BitVectorSpan = BitVectorBase<true>;
using ArenaBitVector = BitVectorBase<false, ArenaAllocator>;
using ArenaBitVectorSpan = BitVectorBase<true, ArenaAllocator>;

}  // namespace ark

namespace std {  // NOLINT(cert-dcl58-cpp)

template <bool IS_CONST>
// NOLINTNEXTLINE(readability-identifier-naming)
inline void fill(ark::BitVectorIterator<IS_CONST> first, ark::BitVectorIterator<IS_CONST> last, bool value)
{
    if (first.data_ != last.data_) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::fill(first.data_ + 1, last.data_, value ? ~0LLU : 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ark::internal::FillBitVector(first, ark::BitVectorIterator<IS_CONST>(first.data_ + 1, 0), value);
        ark::internal::FillBitVector(ark::BitVectorIterator<IS_CONST>(last.data_, 0), last, value);
    } else {
        ark::internal::FillBitVector(first, last, value);
    }
}

}  // namespace std

#endif  // PANDA_BIT_VECTOR_H
