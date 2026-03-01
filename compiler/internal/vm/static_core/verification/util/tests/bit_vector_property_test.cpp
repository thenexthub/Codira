/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "util/bit_vector.h"
#include "util/set_operations.h"
#include "util/range.h"
#include "util/lazy.h"

#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>
#include "util/tests/environment.h"

#include <cstdint>
#include <initializer_list>
#include <string>
#include <set>

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark::verifier;

using StdSet = std::set<size_t>;

struct BSet {
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    StdSet indices;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    BitVector bits;

    bool IsEqual() const
    {
        if (bits.SetBitsCount() != indices.size()) {
            return false;
        }
        for (const auto &elem : indices) {
            if (!bits[elem]) {
                return false;
            }
        }
        return true;
    }
};

namespace rc {

constexpr size_t MAX_VALUE = 1024U;
// NOLINTNEXTLINE(cert-err58-cpp)
auto g_valueGen = gen::inRange<size_t>(0, MAX_VALUE);

template <>
struct Arbitrary<BSet> {
    static Gen<BSet> arbitrary()  // NOLINT(readability-identifier-naming)
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto setNInc = gen::pair(gen::container<std::set<size_t>>(g_valueGen), gen::inRange(1, 100));  // N = 100
        return gen::map(setNInc, [](auto paramSetNInc) {
            auto &[set, inc] = paramSetNInc;
            size_t size = (set.empty() ? 0 : *set.rbegin()) + inc;
            BitVector bits {size};
            for (const auto &idx : set) {
                bits[idx] = 1;
            }
            return BSet {set, bits};
        });
    }
};

template <>
struct Arbitrary<Range<size_t>> {
    static Gen<Range<size_t>> arbitrary()  // NOLINT(readability-identifier-naming)
    {
        return gen::map(gen::pair(g_valueGen, g_valueGen), [](auto pair) {
            return Range<size_t> {std::min(pair.first, pair.second), std::max(pair.first, pair.second)};
        });
    }
};

}  // namespace rc

namespace ark::verifier::test {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace rc;

using Interval = ark::verifier::Range<size_t>;
using Intervals = std::initializer_list<Interval>;

void ClassifySize(const std::string &name, size_t size, const Intervals &intervals)
{
    for (const auto &i : intervals) {
        RC_CLASSIFY(i.Contains(size), name + " " + std::to_string(i));
    }
}

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects,cert-err58-cpp)
const EnvOptions OPTIONS {"VERIFIER_TEST"};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects,cert-err58-cpp,readability-magic-numbers)
Intervals g_statIntervals = {{0, 10U}, {11U, 50U}, {51U, 100U}, {101U, MAX_VALUE}};

void Stat(const BSet &bitset)
{
    if (OPTIONS.Get<bool>("verbose", false)) {
        ClassifySize("Bits.size() in", bitset.bits.Size(), g_statIntervals);
        ClassifySize("Indices.size() in", bitset.indices.size(), g_statIntervals);
    }
}

StdSet Universum(size_t size)
{
    return ToSet<StdSet>(Interval(0, size - 1));
}

RC_GTEST_PROP(TestBitvector, BasicTestSetBitsCount, (const BSet &bit_set))
{
    Stat(bit_set);
    RC_ASSERT(bit_set.bits.SetBitsCount() == bit_set.indices.size());
}

RC_GTEST_PROP(TestBitvector, BasicTestOperatorEq, (const BSet &bit_set))
{
    Stat(bit_set);
    auto bits = bit_set.bits;
    RC_ASSERT(bits.Size() == bit_set.bits.Size());
    for (size_t idx = 0; idx < bits.Size(); ++idx) {
        RC_ASSERT(bits[idx] == bit_set.bits[idx]);
    }
    RC_ASSERT(bits.SetBitsCount() == bit_set.bits.SetBitsCount());
}

RC_GTEST_PROP(TestBitvector, BasicTestClr, (BSet && bset))
{
    Stat(bset);
    bset.bits.Clr();
    RC_ASSERT(bset.bits.SetBitsCount() == 0U);
}

RC_GTEST_PROP(TestBitvector, BasicTestSet, (BSet && bset))
{
    Stat(bset);
    bset.bits.Set();
    RC_ASSERT(bset.bits.SetBitsCount() == bset.bits.Size());
}

RC_GTEST_PROP(TestBitvector, BasicTestInvert, (BSet && bset))
{
    Stat(bset);
    auto zeroBits = bset.bits.Size() - bset.bits.SetBitsCount();
    bset.bits.Invert();
    RC_ASSERT(bset.bits.SetBitsCount() == zeroBits);
}

RC_GTEST_PROP(TestBitvector, BasicTestClrIdx, (BSet && bset, std::set<size_t> &&indices))
{
    Stat(bset);
    auto size = bset.bits.Size();
    for (const auto &idx : indices) {
        bset.bits.Clr(idx % size);
        bset.indices.erase(idx % size);
    }
    RC_ASSERT(bset.bits.SetBitsCount() == bset.indices.size());
}

RC_GTEST_PROP(TestBitvector, BasicTestSetIdx, (BSet && bset, std::set<size_t> &&indices))
{
    Stat(bset);
    auto size = bset.bits.Size();
    for (const auto &idx : indices) {
        auto i = idx % size;
        bset.bits.Set(i);
        bset.indices.insert(i);
    }
    RC_ASSERT(bset.bits.SetBitsCount() == bset.indices.size());
}

RC_GTEST_PROP(TestBitvector, BasicTestInvertIdx, (BSet && bset, std::set<size_t> &&indices))
{
    Stat(bset);
    auto size = bset.bits.Size();
    for (const auto &idx : indices) {
        auto i = idx % size;
        bset.bits.Invert(i);
        if (bset.indices.count(i) > 0) {
            bset.indices.erase(i);
        } else {
            bset.indices.insert(i);
        }
    }
    RC_ASSERT(bset.bits.SetBitsCount() == bset.indices.size());
}

RC_GTEST_PROP(TestBitvector, BasicTestClrFromTo, (BSet && bset, Interval &&interval))
{
    RC_PRE(interval.Finish() < bset.bits.Size());
    Stat(bset);
    bset.bits.Clr(interval.Start(), interval.Finish());
    for (auto idx : interval) {
        bset.indices.erase(idx);
    }
    RC_ASSERT(bset.bits.SetBitsCount() == bset.indices.size());
}

RC_GTEST_PROP(TestBitvector, BasicTestSetFromTo, (BSet && bset, Interval &&interval))
{
    RC_PRE(interval.Finish() < bset.bits.Size());
    Stat(bset);
    bset.bits.Set(interval.Start(), interval.Finish());
    for (auto idx : interval) {
        bset.indices.insert(idx);
    }
    RC_ASSERT(bset.bits.SetBitsCount() == bset.indices.size());
}

RC_GTEST_PROP(TestBitvector, BasicTestInvertFromTo, (BSet && bset, Interval &&interval))
{
    RC_PRE(interval.Finish() < bset.bits.Size());
    Stat(bset);
    bset.bits.Invert(interval.Start(), interval.Finish());
    for (auto idx : interval) {
        if (bset.indices.count(idx) > 0) {
            bset.indices.erase(idx);
        } else {
            bset.indices.insert(idx);
        }
    }
    RC_ASSERT(bset.bits.SetBitsCount() == bset.indices.size());
}

RC_GTEST_PROP(TestBitvector, BasicTestOperatorBitwise, (BSet && lhs, BSet &&rhs))
{
    Stat(rhs);
    Stat(lhs);
    lhs.bits &= rhs.bits;
    lhs.indices = SetIntersection(lhs.indices, rhs.indices);
    RC_ASSERT(lhs.IsEqual());
}

RC_GTEST_PROP(TestBitvector, BasicTestOperatorOr, (BSet && lhs, BSet &&rhs))
{
    Stat(rhs);
    Stat(lhs);
    StdSet universum = Universum(std::max(lhs.bits.Size(), rhs.bits.Size()));
    StdSet clamped = SetIntersection(rhs.indices, universum);
    lhs.bits |= rhs.bits;
    lhs.indices = SetUnion(lhs.indices, clamped);
    RC_ASSERT(lhs.IsEqual());
}

RC_GTEST_PROP(TestBitvector, BasicTestOperatorXor, (BSet && lhs, BSet &&rhs))
{
    Stat(rhs);
    Stat(lhs);
    lhs.bits ^= rhs.bits;
    lhs.indices = SetDifference(SetUnion(lhs.indices, rhs.indices), SetIntersection(lhs.indices, rhs.indices));
    RC_ASSERT(lhs.IsEqual());
}

RC_GTEST_PROP(TestBitvector, BasicTestResise, (BSet && bset, size_t new_size))
{
    Stat(bset);
    new_size %= bset.bits.Size();
    bset.bits.Resize(new_size);
    auto universum = Universum(new_size);
    bset.indices = SetIntersection(universum, bset.indices);
    RC_ASSERT(bset.bits.SetBitsCount() == bset.indices.size());
    RC_ASSERT(bset.bits.Size() == new_size);
}

RC_GTEST_PROP(TestBitvector, IteratorForAllIdxVal, (BSet && bset))
{
    Stat(bset);
    StdSet selected;
    auto handler = [&selected](auto idx, auto val) {
        while (val) {
            if (val & 1U) {
                selected.insert(idx);
            }
            val >>= 1U;
            ++idx;
        }
        return true;
    };
    bset.bits.ForAllIdxVal(handler);
    RC_ASSERT(selected == bset.indices);
}

RC_GTEST_PROP(TestBitvector, IteratorForAllIdxOf1, (BSet && bset))
{
    Stat(bset);
    StdSet result;
    bset.bits.ForAllIdxOf<1>([&result](auto idx) {
        result.insert(idx);
        return true;
    });
    RC_ASSERT(result == bset.indices);
}

RC_GTEST_PROP(TestBitvector, IteratorForAllIdxOf0, (BSet && bset))
{
    Stat(bset);
    StdSet result;
    bset.bits.ForAllIdxOf<0>([&result](auto idx) {
        result.insert(idx);
        return true;
    });
    StdSet universum = Universum(bset.bits.Size());
    RC_ASSERT(result == SetDifference(universum, bset.indices));
}

RC_GTEST_PROP(TestBitvector, LazyIteratorsLazyIndicesOf1, (BSet && bset))
{
    Stat(bset);
    size_t from = *gen::inRange<size_t>(0, bset.bits.Size() - 1);
    size_t to =
        *gen::oneOf(gen::inRange<size_t>(from, bset.bits.Size()), gen::just(std::numeric_limits<size_t>::max()));
    auto result = ContainerOf<StdSet>(bset.bits.LazyIndicesOf<1>(from, to));
    StdSet expected = bset.indices;
    if (from > 0) {
        expected.erase(expected.begin(), expected.lower_bound(from));
    }
    if (!expected.empty() && to < *expected.rbegin()) {
        expected.erase(expected.upper_bound(to), expected.end());
    }
    RC_ASSERT(result == expected);
}

RC_GTEST_PROP(TestBitvector, LazyIteratorsLazyIndicesOf0, (BSet && bset))
{
    Stat(bset);
    size_t from = *gen::inRange<size_t>(0, bset.bits.Size() - 1);
    auto result = ContainerOf<StdSet>(bset.bits.LazyIndicesOf<0>(from));
    StdSet universum = Universum(bset.bits.Size());
    StdSet expected = SetDifference(universum, bset.indices);
    if (from > 0) {
        expected.erase(expected.begin(), expected.lower_bound(from));
    }
    RC_ASSERT(result == expected);
}

RC_GTEST_PROP(TestBitvector, PowerOfFoldedBitsetsAnd2Arg, (BSet && bset1, BSet &&bset2))
{
    Stat(bset1);
    Stat(bset2);
    auto result = BitVector::PowerOfAnd(bset1.bits, bset2.bits);
    RC_ASSERT(result == SetIntersection(bset1.indices, bset2.indices).size());
}

RC_GTEST_PROP(TestBitvector, PowerOfFoldedBitsetsAnd3Arg, (BSet && bset1, BSet &&bset2, BSet &&bset3))
{
    Stat(bset1);
    Stat(bset2);
    Stat(bset3);
    auto result = BitVector::PowerOfAnd(bset1.bits, bset2.bits, bset3.bits);
    RC_ASSERT(result == SetIntersection(bset1.indices, bset2.indices, bset3.indices).size());
}

RC_GTEST_PROP(TestBitvector, PowerOfFoldedBitsetsOr2Arg, (BSet && bset1, BSet &&bset2))
{
    Stat(bset1);
    Stat(bset2);
    auto result = BitVector::PowerOfOr(bset1.bits, bset2.bits);
    auto size = std::min(bset1.bits.Size(), bset2.bits.Size());
    auto universum = Universum(size);
    RC_ASSERT(result == SetIntersection(universum, SetUnion(bset1.indices, bset2.indices)).size());
}

RC_GTEST_PROP(TestBitvector, PowerOfFoldedBitsetsOr3Arg, (BSet && bset1, BSet &&bset2, BSet &&bset3))
{
    Stat(bset1);
    Stat(bset2);
    Stat(bset3);
    auto result = BitVector::PowerOfOr(bset1.bits, bset2.bits, bset3.bits);
    auto size = std::min(std::min(bset1.bits.Size(), bset2.bits.Size()), bset3.bits.Size());
    auto universum = Universum(size);
    RC_ASSERT(result == SetIntersection(universum, SetUnion(bset1.indices, bset2.indices, bset3.indices)).size());
}

RC_GTEST_PROP(TestBitvector, PowerOfFoldedBitsetsXor2Arg, (BSet && bset1, BSet &&bset2))
{
    Stat(bset1);
    Stat(bset2);
    auto result = BitVector::PowerOfXor(bset1.bits, bset2.bits);
    auto size = std::min(bset1.bits.Size(), bset2.bits.Size());
    auto universum = Universum(size);
    auto setResult = SetIntersection(universum, SetDifference(SetUnion(bset1.indices, bset2.indices),
                                                              SetIntersection(bset1.indices, bset2.indices)));
    RC_ASSERT(result == setResult.size());
}

RC_GTEST_PROP(TestBitvector, PowerOfFoldedBitsetsXor3Arg, (BSet && bset1, BSet &&bset2, BSet &&bset3))
{
    Stat(bset1);
    Stat(bset2);
    Stat(bset3);
    auto result = BitVector::PowerOfXor(bset1.bits, bset2.bits, bset3.bits);
    auto size = std::min(std::min(bset1.bits.Size(), bset2.bits.Size()), bset3.bits.Size());
    auto universum = Universum(size);
    auto xor1 = SetDifference(SetUnion(bset1.indices, bset2.indices), SetIntersection(bset1.indices, bset2.indices));
    auto xor2 = SetDifference(SetUnion(xor1, bset3.indices), SetIntersection(xor1, bset3.indices));
    auto setResult = SetIntersection(universum, xor2);
    RC_ASSERT(result == setResult.size());
}

RC_GTEST_PROP(TestBitvector, PowerOfFoldedBitsetsNot2Arg, (BSet && bset1, BSet &&bset2))
{
    Stat(bset1);
    Stat(bset2);
    auto result = BitVector::PowerOfAndNot(bset1.bits, bset2.bits);
    auto size = std::min(bset1.bits.Size(), bset2.bits.Size());
    auto universum = Universum(size);
    auto setResult = SetIntersection(universum, SetDifference(bset1.indices, bset2.indices));
    RC_ASSERT(result == setResult.size());
}

RC_GTEST_PROP(TestBitvector, LazyIteratorsOverFoldedBitsetsAnd1, (BSet && bset1, BSet &&bset2, BSet &&bset3))
{
    Stat(bset1);
    Stat(bset2);
    Stat(bset3);
    auto result = ContainerOf<StdSet>(BitVector::LazyAndThenIndicesOf<1>(bset1.bits, bset2.bits, bset3.bits));
    auto size = std::min(std::min(bset1.bits.Size(), bset2.bits.Size()), bset3.bits.Size());
    auto universum = Universum(size);
    auto setResult = SetIntersection(universum, bset1.indices, bset2.indices, bset3.indices);
    RC_ASSERT(result == setResult);
}

RC_GTEST_PROP(TestBitvector, LazyIteratorsOverFoldedBitsetsAnd0, (BSet && bset1, BSet &&bset2, BSet &&bset3))
{
    Stat(bset1);
    Stat(bset2);
    Stat(bset3);
    auto result = ContainerOf<StdSet>(BitVector::LazyAndThenIndicesOf<0>(bset1.bits, bset2.bits, bset3.bits));
    auto size = std::min(std::min(bset1.bits.Size(), bset2.bits.Size()), bset3.bits.Size());
    auto universum = Universum(size);
    auto setResult = SetDifference(universum, SetIntersection(bset1.indices, bset2.indices, bset3.indices));
    RC_ASSERT(result == setResult);
}

RC_GTEST_PROP(TestBitvector, LazyIteratorsOverFoldedBitsetsOr1, (BSet && bset1, BSet &&bset2, BSet &&bset3))
{
    Stat(bset1);
    Stat(bset2);
    Stat(bset3);
    auto result = ContainerOf<StdSet>(BitVector::LazyOrThenIndicesOf<1>(bset1.bits, bset2.bits, bset3.bits));
    auto size = std::min(std::min(bset1.bits.Size(), bset2.bits.Size()), bset3.bits.Size());
    auto universum = Universum(size);
    auto setResult = SetIntersection(universum, SetUnion(bset1.indices, bset2.indices, bset3.indices));
    RC_ASSERT(result == setResult);
}

RC_GTEST_PROP(TestBitvector, LazyIteratorsOverFoldedBitsetsOr0, (BSet && bset1, BSet &&bset2, BSet &&bset3))
{
    Stat(bset1);
    Stat(bset2);
    Stat(bset3);
    auto result = ContainerOf<StdSet>(BitVector::LazyOrThenIndicesOf<0>(bset1.bits, bset2.bits, bset3.bits));
    auto size = std::min(std::min(bset1.bits.Size(), bset2.bits.Size()), bset3.bits.Size());
    auto universum = Universum(size);
    auto setResult = SetDifference(universum, SetUnion(bset1.indices, bset2.indices, bset3.indices));
    RC_ASSERT(result == setResult);
}

RC_GTEST_PROP(TestBitvector, LazyIteratorsOverFoldedBitsetsXor1, (BSet && bset1, BSet &&bset2, BSet &&bset3))
{
    Stat(bset1);
    Stat(bset2);
    Stat(bset3);
    auto result = ContainerOf<StdSet>(BitVector::LazyXorThenIndicesOf<1>(bset1.bits, bset2.bits, bset3.bits));
    auto size = std::min(std::min(bset1.bits.Size(), bset2.bits.Size()), bset3.bits.Size());
    auto universum = Universum(size);
    auto xor1 = SetDifference(SetUnion(bset1.indices, bset2.indices), SetIntersection(bset1.indices, bset2.indices));
    auto xor2 = SetDifference(SetUnion(xor1, bset3.indices), SetIntersection(xor1, bset3.indices));
    auto setResult = SetIntersection(universum, xor2);
    RC_ASSERT(result == setResult);
}

RC_GTEST_PROP(TestBitvector, LazyIteratorsOverFoldedBitsetsXor0, (BSet && bset1, BSet &&bset2, BSet &&bset3))
{
    Stat(bset1);
    Stat(bset2);
    Stat(bset3);
    auto result = ContainerOf<StdSet>(BitVector::LazyXorThenIndicesOf<0>(bset1.bits, bset2.bits, bset3.bits));
    auto size = std::min(std::min(bset1.bits.Size(), bset2.bits.Size()), bset3.bits.Size());
    auto universum = Universum(size);
    auto xor1 = SetDifference(SetUnion(bset1.indices, bset2.indices), SetIntersection(bset1.indices, bset2.indices));
    auto xor2 = SetDifference(SetUnion(xor1, bset3.indices), SetIntersection(xor1, bset3.indices));
    auto setResult = SetDifference(universum, xor2);
    RC_ASSERT(result == setResult);
}

RC_GTEST_PROP(TestBitvector, LazyIteratorsOverFoldedBitsetsNot1, (BSet && bset1, BSet &&bset2, BSet &&bset3))
{
    Stat(bset1);
    Stat(bset2);
    Stat(bset3);
    auto result = ContainerOf<StdSet>(BitVector::LazyAndNotThenIndicesOf<1>(bset1.bits, bset2.bits, bset3.bits));
    auto size = std::min(std::min(bset1.bits.Size(), bset2.bits.Size()), bset3.bits.Size());
    auto universum = Universum(size);
    auto setAnd = SetIntersection(bset1.indices, bset2.indices);
    auto setNot = SetDifference(universum, bset3.indices);
    auto setResult = SetIntersection(setAnd, setNot);
    RC_ASSERT(result == setResult);
}

RC_GTEST_PROP(TestBitvector, LazyIteratorsOverFoldedBitsetsNot0, (BSet && bset1, BSet &&bset2, BSet &&bset3))
{
    Stat(bset1);
    Stat(bset2);
    Stat(bset3);
    auto result = ContainerOf<StdSet>(BitVector::LazyAndNotThenIndicesOf<0>(bset1.bits, bset2.bits, bset3.bits));
    auto size = std::min(std::min(bset1.bits.Size(), bset2.bits.Size()), bset3.bits.Size());
    auto universum = Universum(size);
    auto setAnd = SetIntersection(bset1.indices, bset2.indices);
    auto setNot = SetDifference(universum, bset3.indices);
    auto setResult = SetDifference(universum, SetIntersection(setAnd, setNot));
    RC_ASSERT(result == setResult);
}

}  // namespace ark::verifier::test
