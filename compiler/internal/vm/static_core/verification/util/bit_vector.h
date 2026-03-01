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

#ifndef PANDA_VERIFIER_BIT_VECTOR_HPP_
#define PANDA_VERIFIER_BIT_VECTOR_HPP_

#include "libarkbase/utils/bit_utils.h"
#include "function_traits.h"
#include "panda_or_std.h"
#include "libarkbase/macros.h"
#include "index.h"

#include <utility>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <cstddef>
#include <algorithm>
#include <tuple>
#include <iostream>

namespace ark::verifier {

#ifdef PANDA_TARGET_64
using Word = uint64_t;
#else
using Word = uint32_t;
#endif

template <typename GetFunc>
class ConstBits {
public:
    explicit ConstBits(GetFunc &&f) : getF_ {std::move(f)} {}
    ~ConstBits() = default;
    ConstBits() = delete;
    ConstBits(const ConstBits & /* unused */) = delete;
    ConstBits(ConstBits && /* unused */) = default;
    ConstBits &operator=(const ConstBits & /* unused */) = delete;
    ConstBits &operator=(ConstBits && /* unused */) = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator Word() const
    {
        return getF_();
    }

    template <typename Rhs>
    bool operator==(const Rhs &rhs) const
    {
        return getF_() == rhs.getF_();
    }

private:
    GetFunc getF_;
    template <typename A>
    friend class ConstBits;
};

template <typename GetFunc, typename SetFunc>
class Bits : public ConstBits<GetFunc> {
public:
    Bits(GetFunc &&get, SetFunc &&set) : ConstBits<GetFunc>(std::move(get)), setF_ {std::move(set)} {}
    ~Bits() = default;
    Bits() = delete;
    Bits(const Bits &) = delete;
    Bits(Bits &&) = default;
    Bits &operator=(const Bits &rhs)
    {
        setF_(rhs);
        return *this;
    }
    Bits &operator=(Bits &&) = default;

    Bits &operator=(Word val)
    {
        setF_(val);
        return *this;
    }

private:
    SetFunc setF_;
};

class BitVector {
    using Allocator = MPandaAllocator<Word>;
    static constexpr size_t BITS_IN_WORD = sizeof(Word) * 8;
    static constexpr size_t BITS_IN_INT = sizeof(int) * 8;
    size_t MaxBitIdx() const
    {
        return size_ - 1;
    }
    static constexpr size_t POS_SHIFT = ark::Ctz(BITS_IN_WORD);
    static constexpr size_t POS_MASK = BITS_IN_WORD - 1;
    static constexpr Word MAX_WORD = std::numeric_limits<Word>::max();

    static Word MaskForIndex(size_t idx)
    {
        return static_cast<Word>(1) << idx;
    }

    static Word MaskUpToIndex(size_t idx)
    {
        return idx >= BITS_IN_WORD ? MAX_WORD : ((static_cast<Word>(1) << idx) - 1);
    }

    class Bit {
    public:
        Bit(BitVector &bitVector, size_t index) : bitVector_ {bitVector}, index_ {index} {};

        // NOLINTNEXTLINE(google-explicit-constructor)
        operator bool() const
        {
            return const_cast<const BitVector &>(bitVector_)[index_];
        }

        Bit &operator=(bool value)
        {
            if (value) {
                bitVector_.Set(index_);
            } else {
                bitVector_.Clr(index_);
            }
            return *this;
        }

    private:
        BitVector &bitVector_;
        size_t index_;
    };

    static constexpr size_t SizeInBitsFromSizeInWords(size_t size)
    {
        return size << POS_SHIFT;
    }

    static constexpr size_t SizeInWordsFromSizeInBits(size_t size)
    {
        return (size + POS_MASK) >> POS_SHIFT;
    }

    void Deallocate()
    {
        if (data_ != nullptr) {
            Allocator allocator;
            allocator.deallocate(data_, SizeInWords());
        }
        size_ = 0;
        data_ = nullptr;
    }

public:
    explicit BitVector(size_t sz) : size_ {sz}, data_ {Allocator().allocate(SizeInWords())}
    {
        Clr();
    }
    ~BitVector()
    {
        Deallocate();
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    BitVector(const BitVector &other)
    {
        CopyFrom(other);
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    BitVector(BitVector &&other) noexcept
    {
        MoveFrom(std::move(other));
    }

    BitVector &operator=(const BitVector &rhs)
    {
        if (&rhs == this) {
            return *this;
        }
        Deallocate();
        CopyFrom(rhs);
        return *this;
    }

    BitVector &operator=(BitVector &&rhs) noexcept
    {
        Deallocate();
        MoveFrom(std::move(rhs));
        return *this;
    }

    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic,hicpp-signed-bitwise)

    auto bits(size_t from, size_t to) const  // NOLINT(readability-identifier-naming)
    {
        ASSERT(data_ != nullptr);
        ASSERT(from <= to);
        ASSERT(to <= MaxBitIdx());
        ASSERT(to - from <= BITS_IN_WORD - 1);
        const Word mask = MaskUpToIndex(to - from + 1);
        const size_t posFrom = from >> POS_SHIFT;
        const size_t posTo = to >> POS_SHIFT;
        const size_t idxFrom = from & POS_MASK;
        return ConstBits([this, mask, posFrom, posTo, idxFrom]() -> Word {
            if (posFrom == posTo) {
                return (data_[posFrom] >> idxFrom) & mask;
            }
            Word data = (data_[posFrom] >> idxFrom) | (data_[posTo] << (BITS_IN_WORD - idxFrom));
            return data & mask;
        });
    }

    auto bits(size_t from, size_t to)  // NOLINT(readability-identifier-naming)
    {
        ASSERT(data_ != nullptr);
        ASSERT(from <= to);
        ASSERT(to <= MaxBitIdx());
        ASSERT(to - from <= BITS_IN_WORD - 1);
        const Word mask = MaskUpToIndex(to - from + 1);
        const size_t posFrom = from >> POS_SHIFT;
        const size_t posTo = to >> POS_SHIFT;
        const size_t idxFrom = from & POS_MASK;
        return Bits(
            [this, mask, posFrom, posTo, idxFrom]() -> Word {
                if (posFrom == posTo) {
                    return (data_[posFrom] >> idxFrom) & mask;
                }
                Word data = (data_[posFrom] >> idxFrom) | (data_[posTo] << (BITS_IN_WORD - idxFrom));
                return data & mask;
            },
            [this, mask, posFrom, posTo, idxFrom](Word valIncomming) {
                const Word val = valIncomming & mask;
                const auto lowMask = mask << idxFrom;
                const auto lowVal = val << idxFrom;
                if (posFrom == posTo) {
                    data_[posFrom] &= ~lowMask;
                    data_[posFrom] |= lowVal;
                } else {
                    const auto highShift = BITS_IN_WORD - idxFrom;
                    const auto highMask = mask >> highShift;
                    const auto highVal = val >> highShift;
                    data_[posFrom] &= ~lowMask;
                    data_[posFrom] |= lowVal;
                    data_[posTo] &= ~highMask;
                    data_[posTo] |= highVal;
                }
            });
    }

    bool operator[](size_t idx) const
    {
        ASSERT(idx < Size());
        return (data_[idx >> POS_SHIFT] & MaskForIndex(idx % BITS_IN_WORD)) != 0;
    }
    Bit operator[](size_t idx)
    {
        return {*this, idx};
    }

    void Clr()
    {
        for (size_t pos = 0; pos < SizeInWords(); ++pos) {
            data_[pos] = 0;
        }
    }
    void Set()
    {
        for (size_t pos = 0; pos < SizeInWords(); ++pos) {
            data_[pos] = MAX_WORD;
        }
    }
    void Invert()
    {
        for (size_t pos = 0; pos < SizeInWords(); ++pos) {
            data_[pos] = ~data_[pos];
        }
    }
    void Clr(size_t idx)
    {
        ASSERT(idx < Size());
        data_[idx >> POS_SHIFT] &= ~MaskForIndex(idx % BITS_IN_WORD);
    }
    void Set(size_t idx)
    {
        ASSERT(idx < Size());
        data_[idx >> POS_SHIFT] |= MaskForIndex(idx % BITS_IN_WORD);
    }
    void Invert(size_t idx)
    {
        operator[](idx) = !operator[](idx);
    }

    BitVector operator~() const
    {
        BitVector result {*this};
        result.Invert();
        return result;
    }

    bool operator==(const BitVector &rhs) const
    {
        if (Size() != rhs.Size()) {
            return false;
        }
        size_t lastWordPartialBits = Size() % BITS_IN_WORD;
        size_t numFullWords = SizeInWords() - ((lastWordPartialBits != 0) ? 1 : 0);
        for (size_t pos = 0; pos < numFullWords; pos++) {
            if (data_[pos] != rhs.data_[pos]) {
                return false;
            }
        }
        if (lastWordPartialBits != 0) {
            size_t lastWordStart = Size() - lastWordPartialBits;
            return bits(lastWordStart, Size() - 1) == rhs.bits(lastWordStart, Size() - 1);
        }
        return true;
    }

    bool operator!=(const BitVector &rhs) const
    {
        return !(*this == rhs);
    }

    template <typename Handler>
    void Process(size_t from, size_t to, Handler handler)
    {
        ASSERT(data_ != nullptr);
        ASSERT(from <= to);
        ASSERT(to <= MaxBitIdx());
        const size_t posFrom = from >> POS_SHIFT;
        const size_t posTo = to >> POS_SHIFT;
        const size_t idxFrom = from & POS_MASK;
        const size_t idxTo = to & POS_MASK;
        auto processWord = [this, &handler](size_t pos) {
            const Word val = handler(data_[pos], BITS_IN_WORD);
            data_[pos] = val;
        };
        auto processPart = [this, &handler, &processWord](size_t pos, size_t idxStart, size_t idxDest) {
            const auto len = idxDest - idxStart + 1;
            if (len == BITS_IN_WORD) {
                processWord(pos);
            } else {
                const Word mask = MaskUpToIndex(len);
                const Word val = handler((data_[pos] >> idxStart) & mask, len) & mask;
                data_[pos] &= ~(mask << idxStart);
                data_[pos] |= val << idxStart;
            }
        };
        if (posFrom == posTo) {
            processPart(posFrom, idxFrom, idxTo);
        } else {
            processPart(posFrom, idxFrom, BITS_IN_WORD - 1);
            for (size_t pos = posFrom + 1; pos < posTo; ++pos) {
                processWord(pos);
            }
            processPart(posTo, 0, idxTo);
        }
    }

    void Clr(size_t from, size_t to)
    {
        Process(from, to, [](auto, auto) { return static_cast<Word>(0); });
    }

    void Set(size_t from, size_t to)
    {
        Process(from, to, [](auto, auto) { return MAX_WORD; });
    }

    void Invert(size_t from, size_t to)
    {
        Process(from, to, [](auto val, auto) { return ~val; });
    }

    template <typename Handler>
    void Process(const BitVector &rhs,
                 Handler &&handler)  // every handler result bit must depend only from corresponding bit pair
    {
        size_t sz = std::min(Size(), rhs.Size());
        size_t words = SizeInWordsFromSizeInBits(sz);
        size_t lhsWords = SizeInWords();
        size_t pos = 0;
        for (; pos < words; ++pos) {
            data_[pos] = handler(data_[pos], rhs.data_[pos]);
        }
        if ((pos >= lhsWords) || ((handler(0U, 0U) == 0U) && (handler(1U, 0U) == 1U))) {
            return;
        }
        for (; pos < lhsWords; ++pos) {
            data_[pos] = handler(data_[pos], 0U);
        }
    }

    BitVector &operator&=(const BitVector &rhs)
    {
        Process(rhs, [](const auto l, const auto r) { return l & r; });
        return *this;
    }

    BitVector &operator|=(const BitVector &rhs)
    {
        if (Size() < rhs.Size()) {
            Resize(rhs.Size());
        }
        Process(rhs, [](const auto l, const auto r) { return l | r; });
        return *this;
    }

    BitVector &operator^=(const BitVector &rhs)
    {
        if (Size() < rhs.Size()) {
            Resize(rhs.Size());
        }
        Process(rhs, [](const auto l, const auto r) { return l ^ r; });
        return *this;
    }

    BitVector operator&(const BitVector &rhs) const
    {
        if (Size() > rhs.Size()) {
            return rhs & *this;
        }
        BitVector result {*this};
        result &= rhs;
        return result;
    }

    BitVector operator|(const BitVector &rhs) const
    {
        if (Size() < rhs.Size()) {
            return rhs | *this;
        }
        BitVector result {*this};
        result |= rhs;
        return result;
    }

    BitVector operator^(const BitVector &rhs) const
    {
        if (Size() < rhs.Size()) {
            return rhs ^ *this;
        }
        BitVector result {*this};
        result ^= rhs;
        return result;
    }

    template <typename Handler>
    void ForAllIdxVal(Handler handler) const
    {
        size_t lastWordPartialBits = Size() % BITS_IN_WORD;
        size_t numFullWords = SizeInWords() - (lastWordPartialBits ? 1 : 0);
        for (size_t pos = 0; pos < numFullWords; pos++) {
            Word val = data_[pos];
            if (!handler(pos * BITS_IN_WORD, val)) {
                return;
            }
        }
        if (lastWordPartialBits) {
            size_t lastWordStart = Size() - lastWordPartialBits;
            Word val = bits(lastWordStart, Size() - 1);
            handler(lastWordStart, val);
        }
    }

    template <const int VAL, typename Handler>
    bool ForAllIdxOf(Handler handler) const
    {
        for (size_t pos = 0; pos < SizeInWords(); ++pos) {
            auto val = data_[pos];
            val = VAL ? val : ~val;
            size_t idx = pos << POS_SHIFT;
            while (val) {
                auto i = static_cast<size_t>(ark::Ctz(val));
                idx += i;
                if (idx >= Size()) {
                    return true;
                }
                if (!handler(idx)) {
                    return false;
                }
                ++idx;
                val >>= i;
                val >>= 1;
            }
        }
        return true;
    }

    template <const int VAL>
    auto LazyIndicesOf(size_t from = 0, size_t to = std::numeric_limits<size_t>::max()) const
    {
        size_t idx = from;
        size_t pos = from >> POS_SHIFT;
        auto val = (VAL ? data_[pos] : ~data_[pos]) >> (idx % BITS_IN_WORD);
        ++pos;
        to = std::min(size_ - 1, to);
        auto sz = SizeInWordsFromSizeInBits(to + 1);
        return [this, sz, to, pos, val, idx]() mutable -> Index<size_t> {
            while (true) {
                if (idx > to) {
                    return Index<size_t>();
                }
                if (val) {
                    return ValLazyIndicesOf(idx, to, val);
                }
                while (val == 0 && pos < sz) {
                    val = VAL ? data_[pos] : ~data_[pos];
                    ++pos;
                }
                idx = (pos - 1) << POS_SHIFT;
                if (pos >= sz && val == 0) {
                    return Index<size_t>();
                }
            }
        };
    }

    size_t SetBitsCount() const
    {
        size_t result = 0;

        size_t pos = 0;
        bool lastWordPartiallyFilled = (Size() & POS_MASK) != 0;
        if (SizeInWords() > 0) {
            for (; pos < (SizeInWords() - (lastWordPartiallyFilled ? 1 : 0)); ++pos) {
                result += static_cast<size_t>(ark::Popcount(data_[pos]));
            }
        }
        if (lastWordPartiallyFilled) {
            const Word mask = MaskUpToIndex(Size() & POS_MASK);
            result += static_cast<size_t>(ark::Popcount(data_[pos] & mask));
        }
        return result;
    }

    template <typename Op, typename Binop, typename... Args>
    static size_t PowerOfOpThenFold(Op op, Binop binop, const Args &...args)
    {
        size_t result = 0;

        size_t sz = NAry {[](size_t a, size_t b) { return std::min(a, b); }}(args.SizeInWords()...);
        size_t size = NAry {[](size_t a, size_t b) { return std::min(a, b); }}(args.Size()...);
        size_t numArgs = sizeof...(Args);
        auto getProcessedWord = [&op, &binop, numArgs, &args...](size_t idx) {
            size_t n = 0;
            auto unop = [&n, numArgs, &op](Word val) { return op(val, n++, numArgs); };
            return NAry {binop}(std::tuple<std::decay_t<decltype(args.data_[idx])>...> {unop(args.data_[idx])...});
        };

        size_t pos = 0;
        bool lastWordPartiallyFilled = (size & POS_MASK) != 0;
        if (sz > 0) {
            for (; pos < (sz - (lastWordPartiallyFilled ? 1 : 0)); ++pos) {
                auto val = getProcessedWord(pos);
                result += static_cast<size_t>(ark::Popcount(val));
            }
        }
        if (lastWordPartiallyFilled) {
            const Word mask = MaskUpToIndex(size & POS_MASK);
            result += static_cast<size_t>(ark::Popcount(getProcessedWord(pos) & mask));
        }
        return result;
    }

    template <typename... Args>
    static size_t PowerOfAnd(const Args &...args)
    {
        return PowerOfOpThenFold([](Word val, size_t, size_t) { return val; },
                                 [](Word lhs, Word rhs) { return lhs & rhs; }, args...);
    }

    template <typename... Args>
    static size_t PowerOfOr(const Args &...args)
    {
        return PowerOfOpThenFold([](Word val, size_t, size_t) { return val; },
                                 [](Word lhs, Word rhs) { return lhs | rhs; }, args...);
    }

    template <typename... Args>
    static size_t PowerOfXor(const Args &...args)
    {
        return PowerOfOpThenFold([](Word val, size_t, size_t) { return val; },
                                 [](Word lhs, Word rhs) { return lhs ^ rhs; }, args...);
    }

    template <typename... Args>
    static size_t PowerOfAndNot(const Args &...args)
    {
        return PowerOfOpThenFold([](Word val, size_t idx, size_t numArgs) { return (idx < numArgs - 1) ? val : ~val; },
                                 [](Word lhs, Word rhs) { return lhs & rhs; }, args...);
    }

    size_t Size() const
    {
        return size_;
    }

    size_t SizeInWords() const
    {
        return SizeInWordsFromSizeInBits(size_);
    }

    void Resize(size_t sz)
    {
        if (sz == 0) {
            Deallocate();
        } else {
            size_t newSizeInWords = SizeInWordsFromSizeInBits(sz);
            size_t oldSizeInWords = SizeInWordsFromSizeInBits(size_);
            if (oldSizeInWords != newSizeInWords) {
                Allocator allocator;
                Word *newData = allocator.allocate(newSizeInWords);
                ASSERT(newData != nullptr);
                size_t pos = 0;
                for (; pos < std::min(oldSizeInWords, newSizeInWords); ++pos) {
                    newData[pos] = data_[pos];
                }
                for (; pos < newSizeInWords; ++pos) {
                    newData[pos] = 0;
                }
                Deallocate();
                data_ = newData;
            }
            size_ = sz;
        }
    }

    template <const int V, typename Op, typename BinOp, typename... Args>
    static auto LazyOpThenFoldThenIndicesOf(Op op, BinOp binop, const Args &...args)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace ark::verifier;
        size_t sz = NAry {[](size_t a, size_t b) { return std::min(a, b); }}(args.SizeInWords()...);
        size_t size = NAry {[](size_t a, size_t b) { return std::min(a, b); }}(args.Size()...);
        size_t numArgs = sizeof...(Args);
        auto getProcessedWord = [op, binop, numArgs, &args...](size_t idx) {
            size_t n = 0;
            auto unop = [&n, numArgs, &op](Word val) { return op(val, n++, numArgs); };
            Word val = NAry {binop}(std::tuple<std::decay_t<decltype(args.data_[idx])>...> {unop(args.data_[idx])...});
            return V ? val : ~val;
        };
        size_t pos = 0;
        auto val = getProcessedWord(pos++);
        size_t idx = 0;
        return [sz, size, pos, val, idx, getProcessedWord]() mutable -> Index<size_t> {
            do {
                if (idx >= size) {
                    return {};
                }
                if (val) {
                    return ValLazyOpIndicesOf(idx, size, val);
                }
                while (val == 0 && pos < sz) {
                    val = getProcessedWord(pos++);
                }
                idx = (pos - 1) << POS_SHIFT;
                if (pos >= sz && val == 0) {
                    return {};
                }
            } while (true);
        };
    }

    template <const int V, typename... Args>
    static auto LazyAndThenIndicesOf(const Args &...args)
    {
        return LazyOpThenFoldThenIndicesOf<V>([](Word val, size_t, size_t) { return val; },
                                              [](Word lhs, Word rhs) { return lhs & rhs; }, args...);
    }

    template <const int V, typename... Args>
    static auto LazyOrThenIndicesOf(const Args &...args)
    {
        return LazyOpThenFoldThenIndicesOf<V>([](Word val, size_t, size_t) { return val; },
                                              [](Word lhs, Word rhs) { return lhs | rhs; }, args...);
    }

    template <const int V, typename... Args>
    static auto LazyXorThenIndicesOf(const Args &...args)
    {
        return LazyOpThenFoldThenIndicesOf<V>([](Word val, size_t, size_t) { return val; },
                                              [](Word lhs, Word rhs) { return lhs ^ rhs; }, args...);
    }

    template <const int V, typename... Args>
    static auto LazyAndNotThenIndicesOf(const Args &...args)
    {
        return LazyOpThenFoldThenIndicesOf<V>(
            [](Word val, size_t idx, size_t numArgs) {
                val = (idx < numArgs - 1) ? val : ~val;
                return val;
            },
            [](Word lhs, Word rhs) { return lhs & rhs; }, args...);
    }

    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic,hicpp-signed-bitwise)

private:
    size_t size_;
    Word *data_ = nullptr;

    void CopyFrom(const BitVector &other)
    {
        size_ = other.size_;
        size_t sizeInWords = other.SizeInWords();
        data_ = Allocator().allocate(sizeInWords);
        std::copy_n(other.data_, sizeInWords, data_);
    }

    void MoveFrom(BitVector &&other) noexcept
    {
        size_ = other.size_;
        data_ = other.data_;
        // don't rhs.Deallocate() as we stole its data_!
        other.size_ = 0;
        other.data_ = nullptr;
    }

    static Index<size_t> ValLazyIndicesOf(size_t &idx, size_t &to, Word &val)
    {
        auto i = static_cast<size_t>(ark::Ctz(val));
        idx += i;
        if (idx > to) {
            return {};
        }
        val >>= i;
        val >>= 1U;
        return idx++;
    }

    static Index<size_t> ValLazyOpIndicesOf(size_t &idx, size_t &size, Word &val)
    {
        auto i = static_cast<size_t>(ark::Ctz(val));
        idx += i;
        if (idx >= size) {
            return {};
        }
        val >>= i;
        val >>= 1U;
        return idx++;
    }
};

}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_BIT_VECTOR_HPP_
