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

#include "Codira/CHIR/Analysis/ConstantRange.h"
#include <algorithm>
#include <iostream>

namespace Codira::CHIR {
PreferredRangeType PreferFromBool(bool useUnsigned)
{
    return useUnsigned ? Unsigned : Signed;
}

ConstantRange::ConstantRange(IntWidth width, bool full)
    : lower(full ? SInt::UMaxValue(width) : SInt::UMinValue(width)), upper(lower)
{
}

ConstantRange::ConstantRange(const SInt& v) : lower(v), upper(lower + 1)
{
}

ConstantRange::ConstantRange(const SInt& l, const SInt& u) : lower(l), upper(u)
{
    CODEC_ASSERT(lower.Width() == upper.Width() && "ConstantRange with unequal int widths");
    CODEC_ASSERT((lower != upper || (lower.IsUMaxValue() || lower.IsUMinValue())) &&
        "lower == upper, but they aren't min or max value!");
}

ConstantRange ConstantRange::From(RelationalOperation rel, const SInt& v, bool isSigned)
{
    auto width = v.Width();
    auto isMin = (isSigned && v.IsSMinValue()) || (!isSigned && v.IsUMinValue());
    auto isMax = (isSigned && v.IsSMaxValue()) || (!isSigned && v.IsUMaxValue());
    if ((rel == RelationalOperation::LT && isMin) || (rel == RelationalOperation::GT && isMax)) {
        return Empty(width);
    }
    if ((rel == RelationalOperation::LE && isMax) || (rel == RelationalOperation::GE && isMin)) {
        return Full(width);
    }

    auto maxVal = isSigned ? SInt::SMaxValue(width) : SInt::UMaxValue(width);
    auto minVal = isSigned ? SInt::SMinValue(width) : SInt::UMinValue(width);
    switch (rel) {
        case RelationalOperation::EQ:
            return ConstantRange{v};
        case RelationalOperation::NE:
            // From(NE, 1) = {2, 3, 0} = [2, 1) if the full set is {0..3}
            return ConstantRange{v + 1, v};
        case RelationalOperation::GE:
            return ConstantRange{v, maxVal + 1};
        case RelationalOperation::GT:
            return ConstantRange{v + 1, maxVal + 1};
        case RelationalOperation::LE:
            return ConstantRange{minVal, v + 1};
        case RelationalOperation::LT:
            return ConstantRange{minVal, v};
        default:
            CODEC_ABORT();
            return ConstantRange{v};
    }
}

ConstantRange ConstantRange::Empty(IntWidth intWidth)
{
    return ConstantRange(intWidth, false);
}

ConstantRange ConstantRange::Full(IntWidth intWidth)
{
    return ConstantRange(intWidth, true);
}

// Create non-empty constant range with the given bounds. If lower and
// upper are the same, a full range is returned.
ConstantRange ConstantRange::NonEmpty(const SInt& l, const SInt& r)
{
    if (l == r) {
        return Full(l.Width());
    }
    return ConstantRange{l, r};
}

// Return the lower value of this range
const SInt& ConstantRange::Lower() const&
{
    return lower;
}

SInt ConstantRange::Lower() &&
{
    return lower;
}

// Return the upper value of this range
const SInt& ConstantRange::Upper() const&
{
    return upper;
}

SInt ConstantRange::Upper() &&
{
    return upper;
}

IntWidth ConstantRange::Width() const
{
    return lower.Width();
}


bool ConstantRange::IsFullSet() const
{
    return lower == upper && lower.IsUMaxValue();
}

bool ConstantRange::IsEmptySet() const
{
    return lower == upper && lower.IsUMinValue();
}

bool ConstantRange::IsNotEmptySet() const
{
    return !IsEmptySet();
}

bool ConstantRange::IsNonTrivial() const
{
    return !IsFullSet();
}

bool ConstantRange::IsWrappedSet() const
{
    return lower.Ugt(upper) && !upper.IsZero();
}

bool ConstantRange::IsUpperWrapped() const
{
    return lower.Ugt(upper);
}

bool ConstantRange::IsSignWrappedSet() const
{
    return lower.Sgt(upper) && !upper.IsSMinValue();
}

bool ConstantRange::IsUpperSignWrapped() const
{
    return lower.Sgt(upper);
}

bool ConstantRange::Contains(const SInt& v) const
{
    if (lower == upper) {
        return IsFullSet();
    }
    if (!IsUpperWrapped()) {
        // normal case:     L---------U
        // possible v:           ^
        return lower.Ule(v) && v.Ult(upper);
    }
    // wrapped set: -----U       L----
    // possible v:   ^
    return lower.Ule(v) || v.Ult(upper);
}

const SInt& ConstantRange::GetSingleElement() const
{
    CODEC_ASSERT(upper == lower + 1);
    return lower;
}

bool ConstantRange::IsSingleElement() const
{
    return upper == lower + 1;
}

bool ConstantRange::IsSizeStrictlySmallerThan(const ConstantRange& rhs) const
{
    CODEC_ASSERT(Width() == rhs.Width());
    if (IsFullSet()) {
        return false;
    }
    if (rhs.IsFullSet()) {
        return true;
    }
    // e.g. for a set {0..8},
    // [1, 8) contains seven elements, upper - lower = 7 = 1111 1111
    // [6, 4) six elements, upper - lower = -2 = 1111 1110 = 6
    // [6, 4) is size strictly smaller than [1, 8)
    return (upper - lower).Ult(rhs.upper - rhs.lower);
}

SInt ConstantRange::MaxValue(bool isUnsigned) const
{
    return isUnsigned ? UMaxValue() : SMaxValue();
}

SInt ConstantRange::MinValue(bool isUnsigned) const
{
    return isUnsigned ? UMinValue() : SMinValue();
}

std::ostream& operator<<(std::ostream& out, const ConstantRange::Formatter& fmt)
{
    auto& rg = fmt.range;
    auto& dividor = ConstantRange::Formatter::DIVIDOR;
    if (rg.IsSingleElement()) {
        // no dividor output for single element range
        return out << SInt::Formatter{{fmt.asUnsigned, fmt.radix}, rg.GetSingleElement()};
    }
    // output a dividor before any range output
    out << dividor;

    struct ConstantRangeFormatterLock {
        ~ConstantRangeFormatterLock()
        {
            out << ConstantRange::Formatter::DIVIDOR;
        }
        std::ostream& out;
    } lock{out}; // output a dividor after any range output
    if (rg.IsEmptySet()) {
        return out;
    }
    if (rg.IsFullSet()) {
        return out << "any";
    }

    bool asUnsigned = fmt.asUnsigned;
    auto width = rg.Upper().Width();
    SInt upperMax{width, 0};
    SInt lowerMin{width, 0};
    if (fmt.asUnsigned) {
        upperMax = SInt::UMaxValue(width), lowerMin = SInt::UMinValue(width);
    } else {
        upperMax = SInt::SMaxValue(width), lowerMin = SInt::SMinValue(width);
    }

    auto isWrapped = asUnsigned ? rg.IsWrappedSet() : rg.IsSignWrappedSet();
    auto upper = rg.Upper() - 1;
    auto lower = rg.Lower();
    if (!isWrapped) {
        if (upper == upperMax) {
            return out << ">=" << SInt::Formatter{{fmt.asUnsigned, fmt.radix}, lower};
        }
        if (lower == lowerMin) {
            return out << "<=" << SInt::Formatter{{fmt.asUnsigned, fmt.radix}, upper};
        }
        return out << ">=" << SInt::Formatter{{fmt.asUnsigned, fmt.radix}, lower}
        << ",<=" << SInt::Formatter{{fmt.asUnsigned, fmt.radix}, upper};
    }
    return out << "<=" << SInt::Formatter{{fmt.asUnsigned, fmt.radix}, upper}
    << "&>=" << SInt::Formatter{{fmt.asUnsigned, fmt.radix}, lower};
}

ConstantRange ConstantRange::Subtract(const SInt& v) const
{
    CODEC_ASSERT(v.Width() == Width());
    if (lower == upper) {
        return *this;
    }
    return ConstantRange{lower - v, upper - v};
}

ConstantRange ConstantRange::Difference(const ConstantRange& rhs) const
{
    return IntersectWith(rhs.Inverse());
}

SInt ConstantRange::UMaxValue() const
{
    if (IsFullSet() || IsUpperWrapped()) {
        return SInt::UMaxValue(Width());
    }
    return Upper() - 1;
}

SInt ConstantRange::UMinValue() const
{
    if (IsFullSet() || IsWrappedSet()) {
        return SInt::UMinValue(Width());
    }
    return Lower();
}

SInt ConstantRange::SMaxValue() const
{
    if (IsFullSet() || IsUpperSignWrapped()) {
        return SInt::SMaxValue(Width());
    }
    return Upper() - 1;
}

SInt ConstantRange::SMinValue() const
{
    if (IsFullSet() || IsSignWrappedSet()) {
        return SInt::SMinValue(Width());
    }
    return Lower();
}

/// Return true if this range is equal to another range.
bool ConstantRange::operator==(const ConstantRange& rhs) const
{
    return lower == rhs.lower && upper == rhs.upper;
}

/// Return true if this range is not equal to another range.
bool ConstantRange::operator!=(const ConstantRange& rhs) const
{
    return !operator==(rhs);
}


static ConstantRange GetPreferredRange(const ConstantRange& a, const ConstantRange& b, PreferredRangeType type)
{
    if (type == Unsigned) {
        if (!a.IsWrappedSet() && b.IsWrappedSet()) {
            return a;
        }
        if (a.IsWrappedSet() && !b.IsWrappedSet()) {
            return b;
        }
    } else if (type == Signed) {
        if (!a.IsSignWrappedSet() && b.IsSignWrappedSet()) {
            return a;
        }
        if (a.IsSignWrappedSet() && !b.IsSignWrappedSet()) {
            return b;
        }
    }

    // type == Smallest or both ranges are non-wrapping or all wrapping.
    return a.IsSizeStrictlySmallerThan(b) ? a : b;
}

#ifndef NDEBUG
void ConstantRange::Dump(bool asUnsigned)
{
    std::cout << ToString(asUnsigned, Radix::R10);
    std::cout.flush();
}
#endif

ConstantRange ConstantRange::Empty() const
{
    return ConstantRange(Width(), false);
}

ConstantRange ConstantRange::Full() const
{
    return ConstantRange(Width(), true);
}

ConstantRange ConstantRange::IntersectBothWrapped(const ConstantRange& rhs, PreferredRangeType type) const
{
    // Both ranges are wrapped, the upper bound of rhs has three possibilities:
    // 1. rhs.upper < this.upper
    // 2. this.upper <= rhs.upper <= this.lower
    // 3. rhs.upper > this.lower

    if (rhs.upper.Ult(upper)) {
        // the lower bound of rhs has three possibilities:
        // 1. rhs.lower < this.upper
        // 2. this.upper <= rhs.lower < this.lower
        // 3. rhs.lower >= this.lower

        // ------U   L--- : this
        // --U L--------- : rhs
        if (rhs.lower.Ult(upper)) {
            return GetPreferredRange(*this, rhs, type);
        }

        // -----U    L-- : this
        // ---U   L----- : rhs
        if (rhs.lower.Ult(lower)) {
            return ConstantRange{lower, rhs.upper};
        }

        // ----U  L----- : this
        // --U      L--- : rhs
        return rhs;
    } else if (rhs.upper.Ule(lower)) {
        // the lower bound of rhs has two possibilities:
        // 1. rhs.lower < this.lower
        // 2. rhs.lower >= this.lower

        // --U       L---- : this
        // ----U   L------ : rhs
        if (rhs.lower.Ult(lower)) {
            return *this;
        }

        // --U    L------- : this
        // ----U    L----- : rhs
        return ConstantRange{rhs.lower, upper};
    } else {
        // rhs.lower >= this.lower
        // --U   L-------- : this
        // --------U  L--- : rhs
        return GetPreferredRange(*this, rhs, type);
    }
}

ConstantRange ConstantRange::IntersectWrappedWithUnwrapped(const ConstantRange& rhs, PreferredRangeType type) const
{
    // this range is wrapped, and the other isn't. The lower bound of rhs has three possibilities:
    // 1. rhs.lower < this.upper
    // 2. this.upper <= rhs.lower < this.lower
    // 3. rhs.lower >= this.lower

    if (rhs.lower.Ult(upper)) {
        // the upper bound of rhs has three possibilities:
        // 1. rhs.upper < this.upper
        // 2. this.upper <= rhs.upper <= this.lower
        // 3. rhs.upper > this.lower

        // ------U   L--- : this
        //  L--U          : rhs
        if (rhs.upper.Ult(upper)) {
            return rhs;
        }

        // ------U   L--- : this
        //  L------U      : rhs
        if (rhs.upper.Ule(lower)) {
            return ConstantRange{rhs.lower, upper};
        }

        // ------U   L--- : this
        //  L----------U  : rhs
        return GetPreferredRange(*this, rhs, type);
    } else if (rhs.lower.Ult(lower)) {
        // the upper bound of rhs has two possibilities
        // 1. rhs.upper <= this.lower
        // 2. rhs.upper > this.lower

        // ------U    L-- : this
        //        L-U     : rhs
        if (rhs.upper.Ule(lower)) {
            return Empty();
        }

        // ------U   L--- : this
        //        L-----U : rhs
        return ConstantRange{lower, rhs.upper};
    } else {
        // rhs.lower >= this.lower
        // ------U  L---- : this
        //           L-U  : rhs
        return rhs;
    }
}

ConstantRange ConstantRange::IntersectBothUnwrapped(const ConstantRange& rhs) const
{
    // Both ranges are unwrapped, the lower bound of this range has two possibilities:
    // 1. this.lower < rhs.lower
    // 2. this.lower >= rhs.lower

    if (lower.Ult(rhs.lower)) {
        // the upper bound of this range has three possibilities:
        // 1. this.upper <= rhs.lower
        // 2. rhs.lower < this.upper < rhs.upper
        // 3. this.upper >= rhs.upper

        //  L--U          : this
        //          L---U : rhs
        if (upper.Ule(rhs.lower)) {
            return Empty();
        }

        //  L-----U       : this
        //     L--------U : rhs
        if (upper.Ult(rhs.upper)) {
            return ConstantRange{rhs.lower, upper};
        }

        //    L--------U  : this
        //      L---U     : rhs
        return rhs;
    } else {
        // this.lower >= rhs.lower
        // the upper bound of rhs has three possibilities:
        // 1. rhs.upper <= this.lower
        // 2. this.lower < rhs.upper < this.upper
        // 3. rhs.upper >= this.upper

        //          L---U : this
        //   L--U         : rhs
        if (rhs.upper.Ule(lower)) {
            return Empty();
        }

        //        L-----U : this
        //   L-------U    : rhs
        if (rhs.upper.Ult(upper)) {
            return ConstantRange{lower, rhs.upper};
        }

        //        L--U    : this
        //   L----------U : rhs
        return *this;
    }
}

ConstantRange ConstantRange::IntersectWith(const ConstantRange& rhs, PreferredRangeType type) const
{
    CODEC_ASSERT(Width() == rhs.Width() && "ConstantRange types don't agree!");

    // Handle common cases.
    if (IsEmptySet() || rhs.IsFullSet()) {
        return *this;
    }
    if (rhs.IsEmptySet() || IsFullSet()) {
        return rhs;
    }

    auto isThisUpperWrapped = IsUpperWrapped();
    auto isRhsUpperWrapped = rhs.IsUpperWrapped();
    if (!isThisUpperWrapped && isRhsUpperWrapped) {
        return rhs.IntersectWith(*this, type);
    } else if (!isThisUpperWrapped && !isRhsUpperWrapped) {
        return IntersectBothUnwrapped(rhs);
    } else if (isThisUpperWrapped && !isRhsUpperWrapped) {
        return IntersectWrappedWithUnwrapped(rhs, type);
    } else {
        return IntersectBothWrapped(rhs, type);
    }
}

ConstantRange ConstantRange::UnionBothWrapped(const ConstantRange& rhs) const
{
    // There are two possibilities, the union of the two ranges covers the full set or not.

    // 1. the union of the two ranges covers the full set.
    // ------U   L---  and  --U   L-------- : this
    // --U L---------       --------U  L--- : rhs
    if (rhs.lower.Ule(upper) || lower.Ule(rhs.upper)) {
        return Full();
    }

    // 2. the union of the two ranges does not cover the full set.
    // -----U    L-- : this
    // ---U   L----- : rhs
    auto newLower = rhs.lower.Ult(lower) ? rhs.lower : lower;
    auto newUpper = rhs.upper.Ugt(upper) ? rhs.upper : upper;

    return ConstantRange{newLower, newUpper};
}

ConstantRange ConstantRange::UnionWrappedWithUnwrapped(const ConstantRange& rhs, PreferredRangeType type) const
{
    // this range is wrapped, and the other isn't. The lower bound of rhs has three possibilities:
    // 1. rhs.lower <= this.upper
    // 2. this.upper < rhs.lower < this.lower
    // 3. rhs.lower >= this.lower

    if (rhs.lower.Ule(upper)) {
        // the upper bound of rhs has three possibilities:
        // 1. rhs.upper < this.upper
        // 2. this.upper <= rhs.upper < this.lower
        // 3. rhs.upper >= this.lower

        // ------U   L--- : this
        //  L--U          : rhs
        if (rhs.upper.Ult(upper)) {
            return *this;
        }

        // ------U   L--- : this
        //  L------U      : rhs
        if (rhs.upper.Ult(lower)) {
            return ConstantRange{lower, rhs.upper};
        }

        // ------U   L--- : this
        //  L----------U  : rhs
        return Full();
    } else if (rhs.lower.Ult(lower)) {
        // the upper bound of rhs has two possibilities
        // 1. rhs.upper < this.lower
        // 2. rhs.upper >= this.lower

        // ----U       L- : this
        //        L--U    : rhs
        // results in one of
        // ----U  L------ : this
        // ----------U L- : rhs
        // depending on the preferred range type.
        if (rhs.upper.Ult(lower)) {
            return GetPreferredRange(ConstantRange{lower, rhs.upper}, ConstantRange{rhs.lower, upper}, type);
        }

        // ------U    L-- : this
        //         L---U  : rhs
        return ConstantRange{rhs.lower, upper};
    } else {
        // rhs.lower >= this.lower
        // ------U  L---- : this
        //           L--U : rhs
        return *this;
    }
}

ConstantRange ConstantRange::UnionBothUnwrapped(const ConstantRange& rhs, PreferredRangeType type) const
{
    // There are two possibilities, the two ranges have an intersection or not.

    // 1. the two ranges are disjoint
    //         L---U  and  L--U         : this
    //  L---U                    L----U : rhs
    // result in one of
    //   L----------U
    // -----U   L----
    // depending on the preferred range type.
    if (rhs.upper.Ult(lower) || upper.Ult(rhs.lower)) {
        return GetPreferredRange(ConstantRange{lower, rhs.upper}, ConstantRange{rhs.lower, upper}, type);
    }

    // 2. the two ranges overlap
    //        L-----U : this
    //   L-------U    : rhs
    auto newLower = rhs.lower.Ult(lower) ? rhs.lower : lower;
    auto newUpper = (rhs.upper - 1).Ugt(upper - 1) ? rhs.upper : upper;
    if (newLower.IsZero() && newUpper.IsZero()) {
        return Full();
    }
    return ConstantRange{newLower, newUpper};
}

ConstantRange ConstantRange::UnionWith(const ConstantRange& rhs, PreferredRangeType type) const
{
    CODEC_ASSERT(Width() == rhs.Width() && "ConstantRange types don't agree!");

    if (IsFullSet() || rhs.IsEmptySet()) {
        return *this;
    }
    if (rhs.IsFullSet() || IsEmptySet()) {
        return rhs;
    }

    auto isThisUpperWrapped = IsUpperWrapped();
    auto isRhsUpperWrapped = rhs.IsUpperWrapped();

    if (isThisUpperWrapped) {
        if (isRhsUpperWrapped) {
            return UnionBothWrapped(rhs);
        } else {
            return UnionWrappedWithUnwrapped(rhs, type);
        }
    } else {
        if (isRhsUpperWrapped) {
            return rhs.UnionWith(*this, type);
        } else {
            return UnionBothUnwrapped(rhs, type);
        }
    }
}

ConstantRange ConstantRange::ZeroExtend(IntWidth width) const
{
    if (IsEmptySet()) {
        return Empty(width);
    }

    CODEC_ASSERT(Width() < width && "Not a value extension");

    // e.g. width 8->16
    // Full set: [255, 255) => [0, 256)
    // Wrapped set: [254, 3) => [0, 256)
    if (IsFullSet() || IsWrappedSet()) {
        return ConstantRange{SInt{width, 0}, SInt::GetOneBitSet(width, static_cast<unsigned>(Width()))};
    }

    // Special case: [254, 0) = {254, 255} => [254, 256)
    if (upper.IsUMinValue()) {
        return ConstantRange{lower.ZExt(width), SInt::GetOneBitSet(width, static_cast<unsigned>(Width()))};
    }

    // Normal case:
    return ConstantRange{lower.ZExt(width), upper.ZExt(width)};
}

ConstantRange ConstantRange::SignExtend(IntWidth width) const
{
    if (IsEmptySet()) {
        return Empty(width);
    }

    CODEC_ASSERT(Width() < width && "Not a value extension");

    // e.g. width 8->16
    // Full set: Full set => [-128, 128)
    // Wrapped set: [126, -2) => [-128, 128)
    if (IsFullSet() || IsSignWrappedSet()) {
        unsigned lowerWidth = static_cast<unsigned>(Width());
        CODEC_ASSERT(lowerWidth > 0);
        // -128 = 0xFF80
        SInt lo = SInt::GetHighBitsSet(width, (static_cast<unsigned>(width) - lowerWidth) + 1);
        // 128 = 0x007F + 1
        SInt up = SInt::GetLowBitsSet(width, lowerWidth - 1) + 1;
        return ConstantRange{lo, up};
    }

    // Special case: [-3, -128) => [-3, 128)
    if (upper.IsSMinValue()) {
        return ConstantRange{lower.SExt(width), upper.ZExt(width)};
    }

    // Normal case:
    return ConstantRange{lower.SExt(width), upper.SExt(width)};
}

std::pair<ConstantRange, ConstantRange> ConstantRange::SplitWrapping(bool asUnsigned) const
{
    CODEC_ASSERT(asUnsigned ? IsWrappedSet() : IsSignWrappedSet());
    return {ConstantRange::From(RelationalOperation::LT, upper, !asUnsigned),
        ConstantRange::From(RelationalOperation::GE, lower, !asUnsigned)};
}

ConstantRange ConstantRange::Truncate(IntWidth dstWidth) const
{
    CODEC_ASSERT(Width() > dstWidth && "Not a value truncation");
    if (IsEmptySet()) {
        return Empty(dstWidth);
    }
    if (IsFullSet()) {
        return Full(dstWidth);
    }

    SInt lowerDiv{lower};
    SInt upperDiv{upper};

    // Analyse unsigned wrapped sets in their two parts: [0, upper) ∪ [lower, maxVal],
    // which is equivalent to [maxVal, upper) ∪ [lower, maxVal).
    // -------------U             L-------
    //   [0, upper)           [lower, maxVal]
    // We first calculate the result of [maxVal, upper), and then use the non-wrapped
    // set code to analysis the [lower, maxVal) part.
    ConstantRange wrappedPart{dstWidth, false};
    if (IsUpperWrapped()) {
        // If upper is greater than or equal to maxVal(dstWidth),
        // it covers the whole truncated range.
        // e.g. width 16 -> 8
        // original range (16-bit): --------------------------U         L--------
        // full range (8-bit):      --------------
        //                          |            |
        //                          0           255
        // special case: when the upper is equal to maxVal(dstWidth)
        // e.g. width 16 -> 8
        // [60000, 255) = [60000, 65535) ∪ [65535, 255)
        // [65535, 255) covers the full set with width 8 ({0..255})
        // 65535 is 0xFFFF, and casting it to width 8 results in 255 (which is 0xFF)
        // Therefore, 255 is included.
        auto dstMaxVal = SInt::UMaxValue(dstWidth);
        if (upper.Uge(dstMaxVal.ZExt(Width()))) {
            return Full(dstWidth);
        }

        wrappedPart = ConstantRange{dstMaxVal, upper.Trunc(dstWidth)};

        // We have calculate the [maxVal, upper) part, and cover the maxVal case.
        // Therefore, return if the remaining is just [maxVal, maxVal).
        // original range (16-bit): ----------U                         L
        //                                    |                         |
        //                                   243                      65535
        if (lowerDiv.IsUMaxValue()) {
            return wrappedPart;
        }

        upperDiv.SetAllBits();
    }

    // Logic for non-wrapped sets

    // Cut off the most significant bits that exceeds the destination width.
    if (lowerDiv.ActiveBits() > static_cast<unsigned>(dstWidth)) {
        // e.g. 16->8, [0x0FF0, 0xFFF4)
        // 0x0FF0 & 0xFF00 = 0x0F00
        SInt adjust = lowerDiv & SInt::GetBitsSetFrom(Width(), static_cast<unsigned>(dstWidth));
        lowerDiv -= adjust; // 0x00F0
        upperDiv -= adjust; // 0xF0F4
    }

    ConstantRange normalPart{dstWidth, false};
    auto upperDivWidth = upperDiv.ActiveBits();
    if (upperDivWidth <= dstWidth) {
        // e.g. lowerDiv: 0000 0000 0011 0100
        //      upperDiv: 0000 0000 1100 0110
        // results in [0b00110100, 0b11000100)
        return ConstantRange{lowerDiv.Trunc(dstWidth), upperDiv.Trunc(dstWidth)}.UnionWith(wrappedPart);
    } else if (upperDivWidth == static_cast<unsigned>(dstWidth) + 1) {
        // Clear the the most significant bit so that upperDiv wraps around.
        upperDiv.ClearBit(static_cast<unsigned>(dstWidth));
        if (upperDiv.Ult(lowerDiv)) {
            // e.g. lowerDiv:   0000 0000 0111 0100
            //      upperDiv:   0000 0001 0000 0110
            // after clear bit: 0000 0000 0000 0110
            // results in [0b01110100, 0b00000110)
            return ConstantRange{lowerDiv.Trunc(dstWidth), upperDiv.Trunc(dstWidth)}.UnionWith(wrappedPart);
        } else {
            // e.g. lowerDiv:   0000 0000 0011 0100
            //      upperDiv:   0000 0001 1100 0110
            // after clear bit: 0000 0000 1100 0110
            // results in a full set with 8-bit width
            return Full(dstWidth);
        }
    } else {
        // e.g. lowerDiv: 0000 0000 0111 0100
        //      upperDiv: 0000 0111 0000 0110
        return Full(dstWidth);
    }
}

ConstantRange::Formatter ConstantRange::ToString(bool asUnsigned, Radix radix) const
{
    return {{asUnsigned, radix}, *this};
}

ConstantRange ConstantRange::Add(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet()) {
        return Empty();
    }
    if (IsFullSet() || rhs.IsFullSet()) {
        return Full();
    }

    auto newLower = Lower() + rhs.Lower();
    // [0, 1) + [0, 3) = {0} + {0, 1, 2} = {0, 1, 2} = [0, 3) if the full set is {0..3}
    auto newUpper = Upper() + rhs.Upper() - 1;
    if (newLower == newUpper) {
        return Full();
    }

    ConstantRange res{newLower, newUpper};
    if (res.IsSizeStrictlySmallerThan(*this) || res.IsSizeStrictlySmallerThan(rhs)) {
        // We've wrapped, therefore, full set.
        // e.g. [2, 240) + [2, 230) results in [4, 212)
        // [2, 240):  (2)----------------------------(240)
        // [2, 230):  (2)------------------------(230)
        // [4, 212):    (4)---------------(212)
        // actually:    (4)-------------------------------(255)
        //          (0)-------------------(212)
        return Full();
    }
    return res;
}

ConstantRange ConstantRange::Sub(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet()) {
        return Empty();
    }
    if (IsFullSet() || rhs.IsFullSet()) {
        return Full();
    }

    auto newLower = Lower() - rhs.Upper() + 1;
    auto newUpper = Upper() - rhs.Lower();
    if (newLower == newUpper) {
        return Full();
    }

    ConstantRange res{newLower, newUpper};
    if (res.IsSizeStrictlySmallerThan(*this) || res.IsSizeStrictlySmallerThan(rhs)) {
        // e.g. [10, 240) - [2, 231) results in [36, 238)
        // [10, 240):   (10)---------------------
        // [2, 231):  (2)---------------------
        // [36, 238):         (36)---------------------(238)
        // actually:  ----(8) (36)----------------------------
        //             (9)-----------------------------(238)
        return Full();
    }
    return res;
}

ConstantRange ConstantRange::UMul(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet()) {
        return Empty();
    }

    auto lhsMin = UMinValue();
    auto lhsMax = UMaxValue();
    auto rhsMin = rhs.UMinValue();
    auto rhsMax = rhs.UMaxValue();

    bool ovf1;
    bool ovf2;
    auto lo = lhsMin.UMulOvf(rhsMin, ovf1);
    auto up = lhsMax.UMulOvf(rhsMax, ovf2) + 1;

    if (ovf1 || ovf2) {
        return Full();
    } else {
        return NonEmpty(lo, up);
    }
}

ConstantRange ConstantRange::SMul(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet()) {
        return Empty();
    }

    auto lhsMin = SMinValue();
    auto lhsMax = SMaxValue();
    auto rhsMin = rhs.SMinValue();
    auto rhsMax = rhs.SMaxValue();

    bool ovf1;
    bool ovf2;
    bool ovf3;
    bool ovf4;
    auto products = {lhsMin.SMulOvf(rhsMin, ovf1), lhsMin.SMulOvf(rhsMax, ovf2), lhsMax.SMulOvf(rhsMin, ovf3),
        lhsMax.SMulOvf(rhsMax, ovf4)};

    if (ovf1 || ovf2 || ovf3 || ovf4) {
        return Full();
    } else {
        auto cmp = [](const SInt& a, const SInt& b) { return a.Slt(b); };
        auto lo = std::min(products, cmp);
        auto up = std::max(products, cmp) + 1;
        return NonEmpty(lo, up);
    }
}

ConstantRange ConstantRange::UDiv(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet() || rhs.UMaxValue().IsZero()) {
        return Empty();
    }

    // calculate the new lower bound
    auto newLower = UMinValue().UDiv(rhs.UMaxValue());

    // calculate the new upper bound, usually it's lhsMax/rhsMin
    // when rhs is wrapped, the rhsMin will be zero, and we need to find the non-zero minimum.
    auto rhsUMin = rhs.UMinValue();
    if (rhsUMin.IsZero()) {
        // Usually the mim value excluding zero will be 1, except for a range
        // in the form of [X, 1) in which case it would be X.
        // e.g. [6, 1) = {6,7,...,maxVal,0}, the minVal is 6
        if (rhs.Upper() == 1) {
            rhsUMin = rhs.Lower();
        } else {
            rhsUMin = 1;
        }
    }

    auto newUpper = UMaxValue().UDiv(rhsUMin) + 1;
    return NonEmpty(newLower, newUpper);
}

ConstantRange ConstantRange::SDivImpl(
    const ConstantRange& rhs, const std::function<SInt(const SInt&, const SInt& b)>& div) const
{
    // We split up the LHS and RHS into positive and negative components
    // and then also compute the positive and negative components of the result
    // separately by combining division results with the appropriate signs.
    auto zero = SInt::Zero(Width());
    auto signedMin = SInt::SMinValue(Width());

    // e.g. for a set {-5,...,5}, calculate [-4, 4) / [-2, 3)
    ConstantRange posFilter{SInt(Width(), 1), signedMin};
    ConstantRange negFilter{signedMin, zero};
    ConstantRange posL = IntersectWith(posFilter);     // [-4, 4) ∪ [1, -5) = [1, 4)
    ConstantRange negL = IntersectWith(negFilter);     // [-4, 4) ∪ [-5, 0) = [-4, 0)
    ConstantRange posR = rhs.IntersectWith(posFilter); // [-2, 3) ∪ [1, -5) = [1, 3)
    ConstantRange negR = rhs.IntersectWith(negFilter); // [-2, 3) ∪ [-5, 0) = [-2, 0)

    ConstantRange posDivPosRes = Empty();
    if (!posL.IsEmptySet() && !posR.IsEmptySet()) {
        // + / + = +
        // the lower bound is lhsPosMin / rhsPosMax, and the upper bound is lhsPosMax / rhsPosMin
        posDivPosRes = ConstantRange{div(posL.lower, posR.upper - 1), div(posL.upper - 1, posR.lower) + 1};
    }

    ConstantRange negDivNegRes = Empty();
    if (!negL.IsEmptySet() && !negR.IsEmptySet()) {
        // - / - = +
        // the lower bound is lhsNegMax / rhsNegMin, and the upper bound is lhsNegMin / rhsNegMax + 1
        // e.g. [-4, 0) / [-2, 0) = {-4, -3, -2, -1} / {-2, -1}
        // lower is -1/-2 = 0, and upper is -4/-1 + 1 = 5
        auto lo = div(negL.upper - 1, negR.lower);
        auto up = div(negL.lower, (negR.upper - 1)) + 1;

        // When calculate the upper bound,
        // we need to deal with one tricky case here: signedMin / -1 is UB,
        // so we'll want to exclude this case when calculating bounds.
        // e.g. {-128,...} / {...,-1}
        // We handle this by dropping either signedMin from the lhs or -1 from the rhs.
        if (negL.lower.IsSMinValue() && negR.upper.IsZero()) {
            // we first try to drop -1 from the negRhs, and find the adjacent upper
            if (!negR.lower.IsAllOnes()) {
                // if negR includes -1
                // normal case:
                // negR:     ---------------
                //           |             |
                //          [-X,           0)
                SInt adjNegRUpper = negR.upper - 1; // actually -1, and now the result is {-128,...} / {...,-2}
                // In a special case, we can obtain more precise upper
                // when rhs.lower = -1, the adjacent negative upper is -X
                // rhs: ------U     L-------------------
                //            |     |
                //           -X    -1
                // note: rhs ∪ negFilter results in [-128, 0)
                if (rhs.lower.IsAllOnes()) {
                    adjNegRUpper = rhs.upper;
                }

                up = div(negL.lower, adjNegRUpper - 1) + 1;
            }
            // if -1 is the only element in the rhs, we try to drop signedMin from the negLhs,
            // and find the adjecant lower
            if (negL.upper != signedMin + 1) {
                // normal case:
                // negL: ------------
                //       |          |
                //      [-128,     -X)
                SInt adjNegLLower = negL.lower + 1; // actually -127
                if (upper == signedMin + 1) {
                    // lhs: -U           L---
                    //       |           |
                    //     -127         -X
                    // note: negL.upper != -127, which means lower must be negative
                    // lhs ∪ negFilter results in [-128, 0)
                    // and -X is more precise than -127
                    adjNegLLower = lower;
                }

                up = div(adjNegLLower, (negR.upper - 1)) + 1;
            }
        }
        negDivNegRes = ConstantRange{lo, up};
    }
    auto posRes = posDivPosRes.UnionWith(negDivNegRes);

    ConstantRange posDivNegRes = Empty();
    if (!posL.IsEmptySet() && !negR.IsEmptySet()) {
        // + / - = -
        // the lower bound is lhsPosMax / rhsNegMax, and the upper bound is lhsPosMin / rhsNegMin + 1
        // e.g. [2, 5) / [-4, -1) = {2, 3, 4} / {-4, -3, -2}
        // lower is 4/-2 = -2, and upper is 2/-4 + 1 = 1
        posDivNegRes = ConstantRange{div(posL.upper - 1, negR.upper - 1), div(posL.lower, negR.lower) + 1};
    }
    ConstantRange negDivPosRes = Empty();
    if (!negL.IsEmptySet() && !posR.IsEmptySet()) {
        // - / + = -
        // the lower bound is lhsNegMin / rhsPosMin, and the upper bound is lhsNegMax / rhsPosMax + 1
        // e.g. [-4, -1) / [2, 5) = {-4, -3, -2} / {2, 3, 4}
        // lower is -4/2 = -2, and upper is -2/4 + 1 = 1
        negDivPosRes = ConstantRange{div(negL.lower, posR.lower), div(negL.upper - 1, posR.upper - 1) + 1};
    }
    auto negRes = posDivNegRes.UnionWith(negDivPosRes);

    // Prefer a non-wrapping signed range here.
    auto res = negRes.UnionWith(posRes, PreferredRangeType::Signed);

    // Preserve the zero that we dropped when splitting the lhs by sign.
    if (Contains(zero) && (!posR.IsEmptySet() || !negR.IsEmptySet())) {
        res = res.UnionWith(ConstantRange(zero));
    }
    return res;
}

ConstantRange ConstantRange::SDivSat(const ConstantRange& rhs) const
{
    auto div = [](const SInt& a, const SInt& b) {
        return a.SDivSat(b);
    };
    return SDivImpl(rhs, div);
}

ConstantRange ConstantRange::SDiv(const ConstantRange& rhs) const
{
    auto div = [](const SInt& a, const SInt& b) {
        return a.SDiv(b);
    };
    return SDivImpl(rhs, div);
}

ConstantRange ConstantRange::URem(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet() || rhs.UMaxValue().IsZero()) {
        return Empty();
    }

    if (rhs.IsSingleElement()) {
        auto rhsInt = rhs.GetSingleElement();
        if (rhsInt.IsZero()) {
            // X % 0 results in an empty set
            return Empty();
        }
        if (this->IsSingleElement()) {
            auto lhsInt = this->GetSingleElement();
            return ConstantRange{lhsInt.URem(rhsInt)};
        }
    }

    // L % R for L < R is L.
    // e.g. [2, 4) % [6, 8) = {2, 3} % {6, 7}
    if (UMaxValue().Ult(rhs.UMinValue())) {
        return *this;
    }

    // L % R is <= L and < R.
    // lhs:  -------
    // rhs:      ------
    //           ^^ makes the lower to be 0
    auto lo = SInt::Zero(Width());
    auto up = SInt::UMin(UMaxValue(), (rhs.UMaxValue() - 1)) + 1;
    return NonEmpty(lo, up);
}

ConstantRange ConstantRange::SRem(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet()) {
        return Empty();
    }

    if (rhs.IsSingleElement()) {
        auto rhsInt = rhs.GetSingleElement();
        if (rhsInt.IsZero()) {
            // X % 0 results in an empty set
            return Empty();
        }
        if (this->IsSingleElement()) {
            auto lhsInt = this->GetSingleElement();
            return ConstantRange{lhsInt.SRem(rhsInt)};
        }
    }

    auto absRhs = rhs.Abs();
    auto minAbsRhs = absRhs.UMinValue();
    auto maxAbsRhs = absRhs.UMaxValue();
    if (maxAbsRhs.IsZero()) {
        return Empty();
    }
    if (minAbsRhs.IsZero()) {
        ++minAbsRhs;
    }

    auto minLhs = SMinValue();
    auto maxLhs = SMaxValue();

    if (minLhs.IsNonNeg()) {
        // lhs is a set of non-negative integers.
        // Same logic as URem

        // L % R for L < R is L.
        if (maxLhs.Ult(minAbsRhs)) {
            return *this;
        }

        // L % R is <= L and < R.
        auto lo = SInt::Zero(Width());
        auto up = SInt::UMin(maxLhs, maxAbsRhs - 1) + 1;
        return ConstantRange{lo, up};
    } else if (maxLhs.IsNeg()) {
        // lhs is a set of negative integers.
        // Same basic logic as above, but the result is negative.

        // e.g. [-6, -2) % [7, 9)
        if (minLhs.Ugt(-minAbsRhs)) {
            return *this;
        }

        // -L % R is >= -L and > R.
        // e.g. [-6, -2) % [4, 8)
        // lhs:         ------
        // -rhs:   --------
        //              ^^^ makes the max result to be 0 (upper to be 1)
        auto lo = SInt::UMax(minLhs, -maxAbsRhs + 1);
        auto up = SInt(Width(), 1);
        return ConstantRange{lo, up};
    } else {
        // lhs range crosses zero.
        // minLhs = -128, maxLhs = 127, maxAbsRhs = 127
        // lo = UMax(-128, -(127) + 1)
        // up = UMin(127, 127 - 1) + 1
        auto lo = SInt::UMax(minLhs, -maxAbsRhs + 1);
        auto up = SInt::UMin(maxLhs, maxAbsRhs - 1) + 1;
        return ConstantRange{lo, up};
    }
}

ConstantRange ConstantRange::UAddSat(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet()) {
        return Empty();
    }

    auto lo = UMinValue().UAddSat(rhs.UMinValue());
    auto up = UMaxValue().UAddSat(rhs.UMaxValue()) + 1;
    return NonEmpty(lo, up);
}

ConstantRange ConstantRange::SAddSat(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet()) {
        return Empty();
    }

    auto lo = SMinValue().SAddSat(rhs.SMinValue());
    auto up = SMaxValue().SAddSat(rhs.SMaxValue()) + 1;
    return NonEmpty(lo, up);
}

ConstantRange ConstantRange::USubSat(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet()) {
        return Empty();
    }

    auto lo = UMinValue().USubSat(rhs.UMaxValue());
    auto up = UMaxValue().USubSat(rhs.UMinValue()) + 1;
    return NonEmpty(lo, up);
}

ConstantRange ConstantRange::SSubSat(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet()) {
        return Empty();
    }

    auto lo = SMinValue().SSubSat(rhs.SMaxValue());
    auto up = SMaxValue().SSubSat(rhs.SMinValue()) + 1;
    return NonEmpty(lo, up);
}

ConstantRange ConstantRange::UMulSat(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet()) {
        return Empty();
    }

    auto lo = UMinValue().UMulSat(rhs.UMinValue());
    auto up = UMaxValue().UMulSat(rhs.UMaxValue()) + 1;
    return NonEmpty(lo, up);
}

ConstantRange ConstantRange::SMulSat(const ConstantRange& rhs) const
{
    if (IsEmptySet() || rhs.IsEmptySet()) {
        return Empty();
    }

    auto min = SMinValue();
    auto max = SMaxValue();
    auto rhsMin = rhs.SMinValue();
    auto rhsMax = rhs.SMaxValue();

    auto products = {min.SMulSat(rhsMin), min.SMulSat(rhsMax), max.SMulSat(rhsMin), max.SMulSat(rhsMax)};
    auto cmp = [](const SInt& a, const SInt& b) { return a.Slt(b); };
    auto lo = std::min(products, cmp);
    auto up = std::max(products, cmp) + 1;
    return NonEmpty(lo, up);
}

ConstantRange ConstantRange::Abs(bool intMinIsPoison) const
{
    if (IsEmptySet()) {
        return Empty();
    }

    if (IsSignWrappedSet()) {
        SInt lo = SInt::UMin(lower, -upper + 1);
        // Check whether the range crosses zero.
        if (upper.IsPositive() || !lower.IsPositive()) {
            lo = SInt::Zero(Width());
        }

        if (intMinIsPoison) {
            return {lo, SInt::SMinValue(Width())};
        } else {
            return {lo, SInt::SMinValue(Width()) + 1};
        }
    }

    SInt sMin = SMinValue();
    SInt sMax = SMaxValue();

    if (intMinIsPoison && sMin.IsSMinValue()) {
        if (sMax.IsSMinValue()) {
            return Empty();
        }
        ++sMin;
    }

    // All non-negative.
    if (sMin.IsNonNeg()) {
        return *this;
    }

    // All negative.
    if (sMax.IsNeg()) {
        return ConstantRange(-sMax, -sMin + 1);
    }

    // Range crosses zero.
    return {SInt::Zero(Width()), SInt::UMax(-sMin, sMax) + 1};
}

ConstantRange ConstantRange::Inverse() const
{
    if (IsFullSet()) {
        return Empty();
    }
    if (IsEmptySet()) {
        return Full();
    }
    return ConstantRange{upper, lower};
}

ConstantRange ConstantRange::Negate() const
{
    if (IsEmptySet() || IsFullSet()) {
        return *this;
    }

    // e.g. for a set {-5,..,4}
    // normal case:
    // 1. [2, -5) = {2, 3, 4} => {-4, -3, -2} = [-4, -1)
    // 2. [4, -2) = {4, -5, -4, -3} => {3, 4, 5(-5), -4} = [3, -3)
    // 3. [-4, -1) = {-4, -3, -2} => {2, 3, 4} = [2, -5)
    auto lo = -(upper - 1);
    // special case: [-5, 1) = {-5, -4, -3, -2, -1, 0} = {0, 1, 2, 3, 4, 5(-5)} = [0, -4)
    auto up = lower.IsSMinValue() ? lower : ((-lower) + 1);
    return ConstantRange{lo, up};
}
}  // namespace codira::CHIR
