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

#ifndef CODIRA_CHIR_ANALYSIS_CONSTANTRANGE_H
#define CODIRA_CHIR_ANALYSIS_CONSTANTRANGE_H

#include <functional>
#include "Codira/CHIR/Analysis/SInt.h"

namespace Codira::CHIR {
// If represented precisely, the result of some range operations may consist
// of multiple disjoint ranges. As only a single range may be returned, any
// range covering these disjoint ranges constitutes a valid result, but some
// may be more useful than others depending on context. The preferred range
// type specifies whether a range that is non-wrapping in the unsigned or
// signed domain, or has the smallest size, is preferred. If a signedness is
// preferred but all ranges are non-wrapping or all wrapping, then the
// smallest set size is preferred. If there are multiple smallest sets, any
// one of them may be returned.
enum PreferredRangeType { Smallest, Unsigned, Signed };

/**
 * @brief return preferred type whether use signed or unsigned.
 * @param useUnsigned flag whether is unsigned.
 * @return preferred range type which indicate unsigned or unsigned.
 */
PreferredRangeType PreferFromBool(bool useUnsigned);

/// relation operations
enum class RelationalOperation : uint8_t { LT, LE, EQ, GT, GE, NE };

/**
 * @brief A constant range represents a set of integer values between an interval.
 * It consists of two values of type SInt, representing the lower and upper
 * bounds of the interval. It is to be noted that the bounds may **wrap around**
 * the end of the numeric range.
 *
 * e.g. for a set {0,1,2,3},
 *
 * [0, 0) = {}          = Empty set
 * [0, 1) = {0}
 * [0, 2) = {0, 1}
 * [0, 3) = {0, 1, 2}
 *
 * [1, 1) = illegal
 * [1, 2) = {1}
 * [1, 3) = {1, 2}
 * [1, 0) = {1, 2, 3}
 *
 * [2, 2) = illegal
 * [2, 3) = {2}
 * [2, 0) = {2, 3}
 * [2, 1) = {2, 3, 0}
 *
 * [3, 3) = {3, 0, 1, 2} = Full set
 * [3, 0) = {3}
 * [3, 1) = {3, 0}
 * [3, 2) = {3, 0, 1}
 *
 * Note that ConstantRange can be used to represent either signed or unsigned ranges.
 */
class ConstantRange {
public:
    /// Initialise a full or empty (unsigned)set for the sepcified int width.
    explicit ConstantRange(IntWidth width, bool full);

    /// Initialize a range to hold the single specified value.
    ConstantRange(const SInt& v);

    /**
     * @brief Initialise a range of values explicitly.
     * This will assert out if (lower==upper) && (lower!=min or max value for its type).
     * It will also assert out if the two SInt's are not the same width.
     */
    ConstantRange(const SInt& l, const SInt& u);

    /**
     * @brief Take a binary relationship between two SInt, and a specific value of type SInt,
     * create a constraint (e.g. SInt is (I16, 3), relationship is NE, the constraint will be >= 3).
     * Return a set of all values in the full set that satisfy the cosntraint.
     *
     * e.g. For a set {0,1,2,3},
     * From(EQ, 1) = {1}
     * From(NE, 1) = {2, 3, 0}
     * From(GE, 1) = {1, 2, 3}
     * From(GT, 1) = {2, 3}
     * From(LE, 1) = {0, 1}
     * From(LT, 1) = {0}
     */
    static ConstantRange From(RelationalOperation rel, const SInt& v, bool isSigned);

    /// return empty range
    static ConstantRange Empty(IntWidth intWidth);

    /// return full range from minimum to maximum
    static ConstantRange Full(IntWidth intWidth);

    /// Create non-empty constant range with the given bounds. If lower and
    /// upper are the same, a full range is returned.
    static ConstantRange NonEmpty(const SInt& l, const SInt& r);

    /// Return the const lower value of this range
    const SInt& Lower() const&;

    /// Return the lower value of this range
    SInt Lower() &&;

    // Return the upper value of this range
    const SInt& Upper() const&;

    /// get upper bound of sInt
    SInt Upper() &&;

    /// get width of sInt
    IntWidth Width() const;

    /// check whether range is full set
    bool IsFullSet() const;

    /// check whether range is empty set
    bool IsEmptySet() const;

    /// check whether range is not empty set
    bool IsNotEmptySet() const;

    /**
     * @brief Return true if this set is not a trivial set, which means it contains
     * no value range information i.e. it's not a full set.
     * The reason is doing value range analysis on a variable that can be any
     * value is meaningless.
     */
    bool IsNonTrivial() const;

    /**
     * @brief Return true if this set wraps around the unsigned domain. e.g. [3, 1)
     * Special cases:
     * Empty set: Not wrapped.
     * Full set: Not wrapped.
     * [X, 0) == [X, max]: Not wrapped. (e.g. [2, 0) = {2, 3} for the set {0,1,2,3})
     */
    bool IsWrappedSet() const;

    /**
     * @brief Return true if the exclusive upper bound wraps around the unsigned domain.
     * Special cases:
     * Empty set: Not wrapped.
     * Full set: Not wrapped.
     * [X, 0): Wrapped.
     */
    bool IsUpperWrapped() const;

    /**
     * @brief Return true if this set wraps around the signed domain.
     * Special cases:
     * Empty set: Not wrapped. {-1, 0, 1}
     * Full set: Not wrapped.
     * [X, signedMin) == [X, signedMax]: Not wrapped. (e.g. [-1, -2) = {-1, 0 ,1, 2} for a set {-2,..,2})
     */
    bool IsSignWrappedSet() const;

    /**
     * @brief Return true if the exclusive upper bound wraps around the signed domain.
     * Special cases:
     * Empty set: Not wrapped.
     * Full set: Not wrapped.
     * [X, signedMin): Wrapped.
     */
    bool IsUpperSignWrapped() const;

    /**
     * @brief Split the range into two if it is a wrapped range with given signess \p asUnsigned.
     * The behaviour is undefined if \p this is not wrapped.
     */
    std::pair<ConstantRange, ConstantRange> SplitWrapping(bool asUnsigned) const;

    /// Return true if the specified value is in the set.
    bool Contains(const SInt& v) const;

    /// Assume that there is only one value in this range, and return this value.
    const SInt& GetSingleElement() const;

    /// Return true if this set contains exactly one member.
    bool IsSingleElement() const;

    /// Compare set size of this range with the range rhs.
    bool IsSizeStrictlySmallerThan(const ConstantRange& rhs) const;

    /// Return the largest unsigned value contained in the ConstantRange.
    SInt UMaxValue() const;

    /// Return the smallest unsigned value contained in the ConstantRange.
    SInt UMinValue() const;

    /// Return the largest signed value contained in the ConstantRange.
    SInt SMaxValue() const;

    /// Return the smallest signed value contained in the ConstantRange.
    SInt SMinValue() const;

    /// Return real max value.
    SInt MaxValue(bool isUnsigned) const;

    /// Return real min value.
    SInt MinValue(bool isUnsigned) const;

    /// Return true if this range is equal to another range.
    bool operator==(const ConstantRange& rhs) const;

    /// Return true if this range is not equal to another range.
    bool operator!=(const ConstantRange& rhs) const;

    /// Subtract the specified constant from the endpoints of this constant range.
    /// e.g. [5, 8) subtract 3 = [2, 5)
    ConstantRange Subtract(const SInt& v) const;

    /// Subtract the specified range from this range.
    /// e.g. [5, 8) diff [6, 9) = [5, 6)
    ConstantRange Difference(const ConstantRange& rhs) const;

    /// Return the range that results from the intersection of this range with
    /// another range. If the intersection is disjoint, such that two results
    /// are possible, the preferred range is determined by the PreferredRangeType.
    ConstantRange IntersectWith(const ConstantRange& rhs, PreferredRangeType type = Smallest) const;

    /// Return the range that results from the union of this range with another
    /// range. The resultant range is guaranteed to include the elements of both
    /// sets, but may contain more. For example, [3, 9) union [12, 15) is [3, 15),
    /// which includes 9, 10, and 11, which were not included in either set before.
    ConstantRange UnionWith(const ConstantRange& rhs, PreferredRangeType type = Smallest) const;

    /// Return a new range in the specified bit width, which must be strictly
    /// larger than the current type. The returned range will correspond to
    /// the possible range of values if the source range had been zero extended
    /// to the specified bit width.
    ConstantRange ZeroExtend(IntWidth width) const;

    /// Return a new range in the specified bit width, which must be strictly
    /// larger than the current type. The returned range will correspond to
    /// the possible range of values if the source range had been sign extended
    /// to the specified bit width.
    ConstantRange SignExtend(IntWidth width) const;

    /// Return a new range in the specified bit width, which must be
    /// strictly smaller than the current type. The returned range will
    /// correspond to the possible range of values if the source range had
    /// been truncated to the specified bit width.
    /// e.g. width 16 -> 8
    /// original lower: 0000 0110 0100 1111   upper: 0011 1000 0011 1001
    ///                           |       |                    |       |
    ///                           ---------                    ---------
    /// In this example, the result range collects the values that can be
    /// represented by the last 8-bit positions. And the result will be a
    /// full set with 8-bit width.
    ConstantRange Truncate(IntWidth dstWidth) const;

    /**
     * @brief print sint fomat
     */
    struct Formatter : SIntFormatterBase {
        const ConstantRange& range;
        static constexpr char DIVIDOR = '|';

        friend std::ostream& operator<<(std::ostream& out, const Formatter& fmt);
    };

    /// const range beauty print format.
    Formatter ToString(bool asUnsigned = true, Radix radix = Radix::R10) const;

#ifndef NDEBUG
    void Dump(bool asUnsigned) __attribute__((used));
#endif

    /// Return a new range representing the possible values resulting
    /// from an addition of a value in this range and a value in rhs.
    ConstantRange Add(const ConstantRange& rhs) const;

    /// Return a new range representing the possible values resulting
    /// from a subtraction of a value in this range and a value in rhs.
    ConstantRange Sub(const ConstantRange& rhs) const;

    /// Return a new range representing the possible values resulting
    /// from a multiplication of a value in this range and a value in rhs,
    /// treating both this and rhs as unsigned ranges.
    ConstantRange UMul(const ConstantRange& rhs) const;

    /// Return a new range representing the possible values resulting
    /// from a multiplication of a value in this range and a value in rhs,
    /// treating both this and rhs as signed ranges.
    ConstantRange SMul(const ConstantRange& rhs) const;

    /// Return a new range representing the possible values resulting from an
    /// unsigned remainder operation of a value in this range and a value in rhs.
    ConstantRange UDiv(const ConstantRange& rhs) const;

    /// Return a new range representing the possible values resulting from an
    /// unsigned division of a value in this range and a value in rhs.
    ConstantRange SDiv(const ConstantRange& rhs) const;

    /// Return a new range representing the possible values resulting from
    /// an unsigned remainder operation of a value in this range and a
    /// value in rhs.
    ConstantRange URem(const ConstantRange& rhs) const;

    /// Return a new range representing the possible values resulting
    /// from a signed remainder operation of a value in this range and a
    /// value in rhs.
    ConstantRange SRem(const ConstantRange& rhs) const;

    /// Perform an unsigned saturating addition of two constant ranges.
    ConstantRange UAddSat(const ConstantRange& rhs) const;

    /// Perform a signed saturating addition of two constant ranges.
    ConstantRange SAddSat(const ConstantRange& rhs) const;

    /// Perform an unsigned saturating subtraction of two constant ranges.
    ConstantRange USubSat(const ConstantRange& rhs) const;

    /// Perform a signed saturating subtraction of two constant ranges.
    ConstantRange SSubSat(const ConstantRange& rhs) const;

    /// Perform an unsigned saturating multiplication of two constant ranges.
    ConstantRange UMulSat(const ConstantRange& rhs) const;

    /// Perform a signed saturating multiplication of two constant ranges.
    ConstantRange SMulSat(const ConstantRange& rhs) const;

    /// Perform a signed saturating division of two constant ranges.
    ConstantRange SDivSat(const ConstantRange& rhs) const;

    /// Return a new range that is the complement set of the current set.
    ConstantRange Inverse() const;

    /// Calculate absolute value range. If the original range contains signed
    /// min, then the resulting range will contain signed min if and only if
    /// intMinIsPoison is false.
    ConstantRange Abs(bool intMinIsPoison = false) const;

    /// Negate all the values in this range and return the new range.
    /// e.g. [2, 5) = {2, 3, 4} => {-4, -3, -2} = [-4, -1)
    /// note: If the original range contains signed min, then the result
    /// will also contain signed min.
    ConstantRange Negate() const;

private:
    SInt lower, upper;

    /// Create an empty set with the same width.
    ConstantRange Empty() const;

    /// Create a full set with the same width.
    ConstantRange Full() const;

    /// Return the range that results from the intersection of this range with another range
    /// assuming both ranges are unwrapped. If the intersection is disjoint, such that two
    /// results are possible, the preferred range is determined by the PreferredRangeType.
    ConstantRange IntersectBothWrapped(const ConstantRange& rhs, PreferredRangeType type) const;

    /// Return the range that results from the intersection of this range with another range
    /// assuming this range is wrapped and the another is unwrapped. If the intersection is disjoint,
    /// such that two results are possible, the preferred range is determined by the PreferredRangeType.
    ConstantRange IntersectWrappedWithUnwrapped(const ConstantRange& rhs, PreferredRangeType type) const;

    /// Return the range that results from the intersection of this range with another range
    /// assuming both ranges are unwrapped.
    ConstantRange IntersectBothUnwrapped(const ConstantRange& rhs) const;

    /// Return the range that results from the union of this range with another range
    /// assuming both ranges are wrapped.
    ConstantRange UnionBothWrapped(const ConstantRange& rhs) const;

    /// Return the range that results from the union of this range with another range
    /// assuming this range is wrapped and the another is unwrapped. If the union is disjoint,
    /// such that two results are possible, the preferred range is determined by the PreferredRangeType.
    ConstantRange UnionWrappedWithUnwrapped(const ConstantRange& rhs, PreferredRangeType type) const;

    /// Return the range that results from the union of this range with another range
    /// assuming both ranges are unwrapped. If the union is disjoint, such that two
    /// results are possible, the preferred range is determined by the PreferredRangeType.
    ConstantRange UnionBothUnwrapped(const ConstantRange& rhs, PreferredRangeType type) const;

    /// Div implement for both sat div and normal div
    ConstantRange SDivImpl(const ConstantRange& rhs, const std::function<SInt(const SInt&, const SInt& b)>& div) const;
};
} // namespace Codira::CHIR
#endif
