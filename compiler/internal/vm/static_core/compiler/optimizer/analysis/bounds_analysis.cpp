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
#include "bounds_analysis.h"
#include "dominators_tree.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "compiler/optimizer/ir/analysis.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace ark::compiler {
BoundsRange::BoundsRange(int64_t val, DataType::Type type) : BoundsRange(val, val, nullptr, type) {}

static bool IsStringLength(const Inst *inst)
{
    ASSERT(inst != nullptr);
    if (inst->GetOpcode() != Opcode::Shr || inst->GetInput(0).GetInst()->GetOpcode() != Opcode::LenArray) {
        return false;
    }
    auto c = inst->GetInput(1).GetInst();
    return c->IsConst() && c->CastToConstant()->GetInt64Value() == 1;
}

static bool IsLenArray(const Inst *inst)
{
    ASSERT(inst != nullptr);
    return inst->GetOpcode() == Opcode::LenArray || IsStringLength(inst);
}

BoundsRange::BoundsRange(int64_t left, int64_t right, const Inst *inst, [[maybe_unused]] DataType::Type type)
    : left_(left), right_(right), lenArray_(inst)
{
    ASSERT(inst == nullptr || IsLenArray(inst));
    ASSERT(left <= right);
    ASSERT(GetMin(type) <= left);
    ASSERT(right <= GetMax(type));
}

void BoundsRange::SetLenArray(const Inst *inst)
{
    ASSERT(inst != nullptr && IsLenArray(inst));
    lenArray_ = inst;
}

int64_t BoundsRange::GetLeft() const
{
    return left_;
}

int64_t BoundsRange::GetRight() const
{
    return right_;
}

/**
 * Neg current range.  Type of current range is saved.
 * NEG([x1, x2]) = [-x2, -x1]
 */
BoundsRange BoundsRange::Neg() const
{
    auto right = left_ == MIN_RANGE_VALUE ? MAX_RANGE_VALUE : -left_;
    auto left = right_ == MIN_RANGE_VALUE ? MAX_RANGE_VALUE : -right_;
    return BoundsRange(left, right);
}

/**
 * Abs current range.  Type of current range is saved.
 * 1. if (x1 >= 0 && x2 >= 0) -> ABS([x1, x2]) = [x1, x2]
 * 2. if (x1 < 0 && x2 < 0) -> ABS([x1, x2]) = [-x2, -x1]
 * 3. if (x1 * x2 < 0) -> ABS([x1, x2]) = [0, MAX(ABS(x1), ABS(x2))]
 */
BoundsRange BoundsRange::Abs() const
{
    auto val1 = left_ == MIN_RANGE_VALUE ? MAX_RANGE_VALUE : std::abs(left_);
    auto val2 = right_ == MIN_RANGE_VALUE ? MAX_RANGE_VALUE : std::abs(right_);
    auto right = std::max(val1, val2);
    int64_t left = 0;
    // NOLINTNEXTLINE (hicpp-signed-bitwise)
    if ((left_ ^ right_) >= 0) {
        left = std::min(val1, val2);
    }
    return BoundsRange(left, right);
}

/**
 * Add to current range.  Type of current range is saved.
 * [x1, x2] + [y1, y2] = [x1 + y1, x2 + y2]
 */
BoundsRange BoundsRange::Add(const BoundsRange &range) const
{
    auto left = AddWithOverflowCheck(left_, range.GetLeft());
    auto right = AddWithOverflowCheck(right_, range.GetRight());
    if (!left || !right) {
        return BoundsRange();
    }
    return BoundsRange(left.value(), right.value());
}

/**
 * Subtract from current range.
 * [x1, x2] - [y1, y2] = [x1 - y2, x2 - y1]
 */
BoundsRange BoundsRange::Sub(const BoundsRange &range) const
{
    auto negRight = (range.GetRight() == MIN_RANGE_VALUE ? MAX_RANGE_VALUE : -range.GetRight());
    auto left = AddWithOverflowCheck(left_, negRight);
    auto negLeft = (range.GetLeft() == MIN_RANGE_VALUE ? MAX_RANGE_VALUE : -range.GetLeft());
    auto right = AddWithOverflowCheck(right_, negLeft);
    if (!left || !right) {
        return BoundsRange();
    }
    return BoundsRange(left.value(), right.value());
}

/**
 * Multiply current range.
 * [x1, x2] * [y1, y2] = [min(x1y1, x1y2, x2y1, x2y2), max(x1y1, x1y2, x2y1, x2y2)]
 */
BoundsRange BoundsRange::Mul(const BoundsRange &range) const
{
    auto m1 = MulWithOverflowCheck(left_, range.GetLeft());
    auto m2 = MulWithOverflowCheck(left_, range.GetRight());
    auto m3 = MulWithOverflowCheck(right_, range.GetLeft());
    auto m4 = MulWithOverflowCheck(right_, range.GetRight());
    if (!m1 || !m2 || !m3 || !m4) {
        return BoundsRange();
    }
    auto left = std::min(m1.value(), std::min(m2.value(), std::min(m3.value(), m4.value())));
    auto right = std::max(m1.value(), std::max(m2.value(), std::max(m3.value(), m4.value())));
    return BoundsRange(left, right);
}

/**
 * Division current range on constant range.
 * [x1, x2] / [y, y] = [min(x1/y, x2/y), max(x1/y, x2/y)]
 */
BoundsRange BoundsRange::Div(const BoundsRange &range) const
{
    if (range.GetLeft() != range.GetRight() || range.GetLeft() == 0) {
        return BoundsRange();
    }
    auto m1 = DivWithOverflowCheck(left_, range.GetLeft());
    auto m2 = DivWithOverflowCheck(right_, range.GetLeft());
    if (!m1 || !m2) {
        return BoundsRange();
    }
    auto left = std::min(m1.value(), m2.value());
    auto right = std::max(m1.value(), m2.value());
    return BoundsRange(left, right);
}

/**
 * Modulus of current range.
 * mod := max(abs(y1), abs(y2))
 * [x1, x2] % [y1, y2] = [min(max(x1, -(mod - 1)), 0), max(min(x2, mod - 1), 0)]
 */
/* static */
BoundsRange BoundsRange::Mod(const BoundsRange &range)
{
    int64_t maxMod = 0;
    if (range.GetRight() > 0) {
        maxMod = range.GetRight() - 1;
    }
    if (range.GetLeft() == MIN_RANGE_VALUE) {
        maxMod = MAX_RANGE_VALUE;
    } else if (range.GetLeft() < 0) {
        maxMod = std::max(maxMod, -range.GetLeft() - 1);
    }
    if (maxMod == 0) {
        return BoundsRange();
    }
    auto left = left_ < 0 ? std::max(left_, -maxMod) : 0;
    auto right = right_ > 0 ? std::min(right_, maxMod) : 0;
    return BoundsRange(left, right);
}

// right shift current range, work only if 'range' is positive constant range
BoundsRange BoundsRange::Shr(const BoundsRange &range, DataType::Type type)
{
    if (!range.IsConst() || range.IsNegative()) {
        return BoundsRange();
    }
    uint64_t sizeMask = DataType::GetTypeSize(type, Arch::NONE) - 1;
    auto n = static_cast<uint64_t>(range.GetLeft()) & sizeMask;
    auto narrowed = BoundsRange(*this).FitInType(type);
    // for fixed n > 0 (x Shr n) is increasing on [MIN_RANGE_VALUE, -1] and
    // on [0, MAX_RANGE_VALUE], but (-1 Shr n) > (0 Shr n)
    if (narrowed.GetLeft() < 0 && narrowed.GetRight() >= 0 && n > 0) {
        auto r = static_cast<int64_t>(~static_cast<uint64_t>(0) >> n);
        return BoundsRange(0, r);
    }
    auto l = static_cast<int64_t>(static_cast<uint64_t>(narrowed.GetLeft()) >> n);
    auto r = static_cast<int64_t>(static_cast<uint64_t>(narrowed.GetRight()) >> n);
    return BoundsRange(l, r);
}

// arithmetic right shift current range, work only if 'range' is positive constant range
BoundsRange BoundsRange::AShr(const BoundsRange &range, DataType::Type type)
{
    if (!range.IsConst() || range.IsNegative()) {
        return BoundsRange();
    }
    uint64_t sizeMask = DataType::GetTypeSize(type, Arch::NONE) - 1;
    auto n = static_cast<uint64_t>(range.GetLeft()) & sizeMask;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return BoundsRange(left_ >> n, right_ >> n);
}

// left shift current range, work only if 'range' is positive constant range
BoundsRange BoundsRange::Shl(const BoundsRange &range, DataType::Type type)
{
    if (!range.IsConst() || range.IsNegative()) {
        return BoundsRange();
    }
    uint64_t sizeMask = DataType::GetTypeSize(type, Arch::NONE) - 1;
    auto n = static_cast<uint64_t>(range.GetLeft()) & sizeMask;
    if (n > 0) {
        auto maxBits = BITS_PER_UINT64 - n - 1;
        auto maxValue = static_cast<int64_t>(static_cast<uint64_t>(1) << maxBits);
        if (left_ < -maxValue || right_ >= maxValue) {
            // shift can overflow
            return BoundsRange();
        }
    }
    auto l = static_cast<int64_t>(static_cast<uint64_t>(left_) << n);
    auto r = static_cast<int64_t>(static_cast<uint64_t>(right_) << n);
    return BoundsRange(l, r);
}

/**
 * Count difference between range and current range. Type of current range is saved.
 * DIFF([x1, x2], [y1, y2]) = [x1 - y1, x2 - y2]
 * If result range is incorrect (left > right), it returns maximal range
 */
BoundsRange BoundsRange::Diff(const BoundsRange &range) const
{
    auto negLeft = (range.GetLeft() == MIN_RANGE_VALUE ? MAX_RANGE_VALUE : -range.GetLeft());
    auto left = AddWithOverflowCheck(left_, negLeft);
    auto negRight = (range.GetRight() == MIN_RANGE_VALUE ? MAX_RANGE_VALUE : -range.GetRight());
    auto right = AddWithOverflowCheck(right_, negRight);
    if (!left || !right || left.value() > right.value()) {
        return BoundsRange();
    }
    return BoundsRange(left.value(), right.value());
}

BoundsRange BoundsRange::And(const BoundsRange &range)
{
    if (!range.IsConst()) {
        return BoundsRange();
    }
    auto n = static_cast<uint64_t>(range.GetLeft());
    static constexpr uint32_t BITS_63 = 63;
    if ((n >> BITS_63) == 1) {
        return BoundsRange();
    }
    return BoundsRange(0, n);
}

bool BoundsRange::IsConst() const
{
    return left_ == right_;
}

bool BoundsRange::IsMaxRange(DataType::Type type) const
{
    return left_ <= GetMin(type) && right_ >= GetMax(type);
}

bool BoundsRange::IsEqual(const BoundsRange &range) const
{
    return left_ == range.GetLeft() && right_ == range.GetRight();
}

bool BoundsRange::IsLess(const BoundsRange &range) const
{
    return right_ < range.GetLeft();
}

bool BoundsRange::IsLess(const Inst *inst) const
{
    ASSERT(inst != nullptr);
    if (!IsLenArray(inst)) {
        return false;
    }
    return inst == lenArray_;
}

bool BoundsRange::IsMore(const BoundsRange &range) const
{
    return left_ > range.GetRight();
}

bool BoundsRange::IsMoreOrEqual(const BoundsRange &range) const
{
    return left_ >= range.GetRight();
}

bool BoundsRange::IsWithin(const BoundsRange &range) const
{
    return left_ > range.GetLeft() && right_ < range.GetRight();
}

bool BoundsRange::IsNotNegative() const
{
    return left_ >= 0;
}

bool BoundsRange::IsNegative() const
{
    return right_ < 0;
}

bool BoundsRange::IsPositive() const
{
    return left_ > 0;
}

bool BoundsRange::IsNotPositive() const
{
    return right_ <= 0;
}

bool BoundsRange::CanOverflow(DataType::Type type) const
{
    return left_ <= GetMin(type) || right_ >= GetMax(type);
}

bool BoundsRange::CanOverflowNeg(DataType::Type type) const
{
    return right_ >= GetMax(type) || (left_ <= 0 && 0 <= right_);
}

/**
 * Return the minimal value for a type.
 *
 * We consider pointer types to have only non-negative address values
 */
int64_t BoundsRange::GetMin(DataType::Type type)
{
    ASSERT(!IsFloatType(type));
    switch (type) {
        case DataType::BOOL:
        case DataType::UINT8:
        case DataType::UINT16:
        case DataType::UINT32:
        case DataType::UINT64:
        case DataType::REFERENCE:
        case DataType::POINTER:
            return 0;
        case DataType::INT8:
            return INT8_MIN;
        case DataType::INT16:
            return INT16_MIN;
        case DataType::INT32:
            return INT32_MIN;
        case DataType::INT64:
            return INT64_MIN;
        default:
            UNREACHABLE();
    }
}

/**
 * Return the maximal value for a type.
 *
 * For REFERENCE we are interested in whether it is NULL or not.  Set the
 * maximum to INT64_MAX regardless the real architecture bitness.
 */
int64_t BoundsRange::GetMax(DataType::Type type)
{
    ASSERT(!IsFloatType(type));
    switch (type) {
        case DataType::BOOL:
            return 1;
        case DataType::UINT8:
            return UINT8_MAX;
        case DataType::UINT16:
            return UINT16_MAX;
        case DataType::UINT32:
            return UINT32_MAX;
        case DataType::INT8:
            return INT8_MAX;
        case DataType::INT16:
            return INT16_MAX;
        case DataType::INT32:
            return INT32_MAX;
        case DataType::INT64:
        case DataType::UINT64:
        case DataType::REFERENCE:
        case DataType::POINTER:
            return INT64_MAX;
        default:
            UNREACHABLE();
    }
}

BoundsRange BoundsRange::FitInType(DataType::Type type) const
{
    auto typeMin = BoundsRange::GetMin(type);
    auto typeMax = BoundsRange::GetMax(type);
    if (left_ < typeMin || left_ > typeMax || right_ < typeMin || right_ > typeMax) {
        return BoundsRange(typeMin, typeMax);
    }
    return *this;
}

BoundsRange BoundsRange::Union(const ArenaVector<BoundsRange> &ranges)
{
    int64_t min = MAX_RANGE_VALUE;
    int64_t max = MIN_RANGE_VALUE;
    for (const auto &range : ranges) {
        if (range.GetLeft() < min) {
            min = range.GetLeft();
        }
        if (range.GetRight() > max) {
            max = range.GetRight();
        }
    }
    return BoundsRange(min, max);
}

BoundsRange::RangePair BoundsRange::NarrowBoundsByNE(BoundsRange::RangePair const &ranges)
{
    auto &[left_range, right_range] = ranges;
    int64_t ll = left_range.GetLeft();
    int64_t lr = left_range.GetRight();
    int64_t rl = right_range.GetLeft();
    int64_t rr = right_range.GetRight();
    // We can narrow bounds of a range if another is a constant and matches one of the bounds
    // Mostly needed for a reference comparison with null
    if (left_range.IsConst() && !right_range.IsConst()) {
        if (ll == rl) {
            return {left_range, BoundsRange(rl + 1, rr)};
        }
        if (ll == rr) {
            return {left_range, BoundsRange(rl, rr - 1)};
        }
    }
    if (!left_range.IsConst() && right_range.IsConst()) {
        if (rl == ll) {
            return {BoundsRange(ll + 1, lr), right_range};
        }
        if (rl == lr) {
            return {BoundsRange(ll, lr - 1), right_range};
        }
    }
    return ranges;
}

BoundsRange::RangePair BoundsRange::NarrowBoundsCase1(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    auto &[left_range, right_range] = ranges;
    int64_t lr = left_range.GetRight();
    int64_t rl = right_range.GetLeft();
    if (cc == ConditionCode::CC_GT || cc == ConditionCode::CC_A) {
        // With equal rl and lr left_range cannot be greater than right_range
        if (rl == lr) {
            return {BoundsRange(), BoundsRange()};
        }
        return {BoundsRange(rl + 1, lr), BoundsRange(rl, lr - 1)};
    }
    if (cc == ConditionCode::CC_GE || cc == ConditionCode::CC_AE || cc == ConditionCode::CC_EQ) {
        return {BoundsRange(rl, lr), BoundsRange(rl, lr)};
    }
    return ranges;
}

BoundsRange::RangePair BoundsRange::NarrowBoundsCase2(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    if (cc == ConditionCode::CC_GT || cc == ConditionCode::CC_GE || cc == ConditionCode::CC_EQ ||
        cc == ConditionCode::CC_A || cc == ConditionCode::CC_AE) {
        return {BoundsRange(), BoundsRange()};
    }
    return ranges;
}

BoundsRange::RangePair BoundsRange::NarrowBoundsCase3(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    auto &[left_range, right_range] = ranges;
    int64_t ll = left_range.GetLeft();
    int64_t lr = left_range.GetRight();
    int64_t rl = right_range.GetLeft();
    int64_t rr = right_range.GetRight();
    if (cc == ConditionCode::CC_GT || cc == ConditionCode::CC_A) {
        // rl == lr handled in case 1
        return {BoundsRange(rl + 1, lr), right_range};
    }
    if (cc == ConditionCode::CC_GE || cc == ConditionCode::CC_AE) {
        return {BoundsRange(rl, lr), right_range};
    }
    if (cc == ConditionCode::CC_LT || cc == ConditionCode::CC_B) {
        // With equal ll and rr left_range cannot be less than right_range
        if (ll == rr) {
            return {BoundsRange(), BoundsRange()};
        }
        return {BoundsRange(ll, rr - 1), right_range};
    }
    if (cc == ConditionCode::CC_LE || cc == ConditionCode::CC_BE) {
        return {BoundsRange(ll, rr), right_range};
    }
    if (cc == ConditionCode::CC_EQ) {
        return {BoundsRange(rl, rr), right_range};
    }
    return ranges;
}

BoundsRange::RangePair BoundsRange::NarrowBoundsCase4(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    auto &[left_range, right_range] = ranges;
    int64_t ll = left_range.GetLeft();
    int64_t rr = right_range.GetRight();
    if (cc == ConditionCode::CC_LT || cc == ConditionCode::CC_B) {
        // With equal ll and rr left_range cannot be less than right_range
        if (ll == rr) {
            return {BoundsRange(), BoundsRange()};
        }
        return {BoundsRange(ll, rr - 1), BoundsRange(ll + 1, rr)};
    }
    if (cc == ConditionCode::CC_LE || cc == ConditionCode::CC_BE || cc == ConditionCode::CC_EQ) {
        return {BoundsRange(ll, rr), BoundsRange(ll, rr)};
    }
    return ranges;
}

BoundsRange::RangePair BoundsRange::NarrowBoundsCase5(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    if (cc == ConditionCode::CC_LT || cc == ConditionCode::CC_LE || cc == ConditionCode::CC_EQ ||
        cc == ConditionCode::CC_B || cc == ConditionCode::CC_BE) {
        return {BoundsRange(), BoundsRange()};
    }
    return ranges;
}

BoundsRange::RangePair BoundsRange::NarrowBoundsCase6(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    auto &[left_range, right_range] = ranges;
    int64_t ll = left_range.GetLeft();
    int64_t lr = left_range.GetRight();
    int64_t rl = right_range.GetLeft();
    int64_t rr = right_range.GetRight();
    if (cc == ConditionCode::CC_GT || cc == ConditionCode::CC_A) {
        // rl == lr handled in case 1
        return {left_range, BoundsRange(rl, lr - 1)};
    }
    if (cc == ConditionCode::CC_GE || cc == ConditionCode::CC_AE) {
        return {left_range, BoundsRange(rl, lr)};
    }
    if (cc == ConditionCode::CC_LT || cc == ConditionCode::CC_B) {
        // ll == rr handled in case 4
        return {left_range, BoundsRange(ll + 1, rr)};
    }
    if (cc == ConditionCode::CC_LE || cc == ConditionCode::CC_BE) {
        return {left_range, BoundsRange(ll, rr)};
    }
    if (cc == ConditionCode::CC_EQ) {
        return {left_range, BoundsRange(ll, lr)};
    }
    return ranges;
}

/**
 * Try narrow bounds range for <if (left CC right)> situation
 * Return a pair of narrowed left and right intervals
 */
BoundsRange::RangePair BoundsRange::TryNarrowBoundsByCC(ConditionCode cc, BoundsRange::RangePair const &ranges)
{
    if (cc == ConditionCode::CC_NE) {
        return NarrowBoundsByNE(ranges);
    }
    auto &[left_range, right_range] = ranges;
    int64_t ll = left_range.GetLeft();
    int64_t lr = left_range.GetRight();
    int64_t rl = right_range.GetLeft();
    int64_t rr = right_range.GetRight();
    // For further description () is for left_range bounds and [] is for right_range bounds
    // case 1: ( [ ) ]
    if (ll <= rl && rl <= lr && lr <= rr) {
        return NarrowBoundsCase1(cc, ranges);
    }
    // case 2: ( ) [ ]
    if (ll <= lr && lr < rl && rl <= rr) {
        return NarrowBoundsCase2(cc, ranges);
    }
    // case 3: ( [ ] )
    if (ll <= rl && rl <= rr && rr <= lr) {
        return NarrowBoundsCase3(cc, ranges);
    }
    // case 4: [ ( ] )
    if (rl <= ll && ll <= rr && rr <= lr) {
        return NarrowBoundsCase4(cc, ranges);
    }
    // case 5: [ ] ( )
    if (rl <= rr && rr < ll && ll <= lr) {
        return NarrowBoundsCase5(cc, ranges);
    }
    // case 6: [ ( ) ]
    if (rl <= ll && ll <= lr && lr <= rr) {
        return NarrowBoundsCase6(cc, ranges);
    }
    return ranges;
}

/// Return (left + right) or if overflows or underflows return nullopt of range type.
std::optional<int64_t> BoundsRange::AddWithOverflowCheck(int64_t left, int64_t right)
{
    if (right == 0) {
        return left;
    }
    if (right > 0) {
        if (left <= (MAX_RANGE_VALUE - right)) {
            // No overflow.
            return left + right;
        }
        return std::nullopt;
    }
    if (left >= (MIN_RANGE_VALUE - right)) {
        // No overflow.
        return left + right;
    }
    return std::nullopt;
}

std::optional<uint64_t> BoundsRange::AddWithOverflowCheck(uint64_t left, uint64_t right)
{
    if (right == 0) {
        return left;
    }
    if (left <= (UINT64_MAX - right)) {
        // No overflow.
        return left + right;
    }
    return std::nullopt;
}

/// Return (left * right) or if overflows or underflows return nullopt of range type.
std::optional<int64_t> BoundsRange::MulWithOverflowCheck(int64_t left, int64_t right)
{
    if (left == 0 || right == 0) {
        return 0;
    }
    int64_t leftAbs = (left == MIN_RANGE_VALUE ? MAX_RANGE_VALUE : std::abs(left));
    int64_t rightAbs = (right == MIN_RANGE_VALUE ? MAX_RANGE_VALUE : std::abs(right));
    if (leftAbs <= (MAX_RANGE_VALUE / rightAbs)) {
        // No overflow.
        return left * right;
    }
    if ((right > 0 && left > 0) || (right < 0 && left < 0)) {
        return std::nullopt;
    }
    return std::nullopt;
}

std::optional<uint64_t> BoundsRange::MulWithOverflowCheck(uint64_t left, uint64_t right)
{
    if (left == 0 || right == 0) {
        return 0;
    }
    if (left <= (UINT64_MAX / right)) {
        // No overflow.
        return left * right;
    }
    return std::nullopt;
}

/// Return (left / right) or nullopt VALUE for MIN_RANGE_VALUE / -1.
std::optional<int64_t> BoundsRange::DivWithOverflowCheck(int64_t left, int64_t right)
{
    ASSERT(right != 0);
    if (left == MIN_RANGE_VALUE && right == -1) {
        return std::nullopt;
    }
    return left / right;
}

BoundsRange BoundsRangeInfo::FindBoundsRange(const BasicBlock *block, const Inst *inst) const
{
    ASSERT(block != nullptr && inst != nullptr);
    ASSERT(DataType::IsGeneralizedIntType(inst->GetType()));
    if (inst->GetOpcode() == Opcode::NullPtr) {
        ASSERT(inst->GetType() == DataType::REFERENCE);
        return BoundsRange(0);
    }
    if (IsInstNotNull(inst)) {
        ASSERT(inst->GetType() == DataType::REFERENCE);
        return BoundsRange(1, BoundsRange::GetMax(DataType::REFERENCE));
    }
    while (block != nullptr) {
        if (boundsRangeInfo_.find(block) != boundsRangeInfo_.end() &&
            boundsRangeInfo_.at(block).find(inst) != boundsRangeInfo_.at(block).end()) {
            return boundsRangeInfo_.at(block).at(inst);
        }
        block = block->GetDominator();
    }
    // BoundsRange for constant can have len_array, so we should process it after the loop above
    if (inst->IsConst()) {
        ASSERT(inst->GetType() == DataType::INT64);
        auto val = static_cast<int64_t>(inst->CastToConstant()->GetIntValue());
        return BoundsRange(val);
    }
    if (IsLenArray(inst)) {
        ASSERT(inst->GetType() == DataType::INT32);
        auto maxLength = INT32_MAX;
        if (inst->GetOpcode() == Opcode::LenArray) {
            auto arrayInst = inst->CastToLenArray()->GetDataFlowInput(0);
            auto typeInfo = arrayInst->GetObjectTypeInfo();
            if (typeInfo) {
                auto klass = typeInfo.GetClass();
                auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
                maxLength = static_cast<int32_t>(runtime->GetMaxArrayLength(klass));
            }
        }
        return BoundsRange(0, maxLength, nullptr, inst->GetType());
    }
    // if we know nothing about inst return the complete range of type
    return BoundsRange(inst->GetType());
}

void BoundsRangeInfo::SetBoundsRange(const BasicBlock *block, const Inst *inst, BoundsRange range)
{
    if (inst->IsConst() && range.GetLenArray() == nullptr) {
        return;
    }
    if (inst->IsConst()) {
        auto val = static_cast<int64_t>(static_cast<const ConstantInst *>(inst)->GetIntValue());
        range = BoundsRange(val, val, range.GetLenArray());
    }
    ASSERT(DataType::IsGeneralizedIntType(inst->GetType()));
    ASSERT(range.GetLeft() >= BoundsRange::GetMin(inst->GetType()));
    ASSERT(range.GetRight() <= BoundsRange::GetMax(inst->GetType()));
    if (!range.IsMaxRange() || range.GetLenArray() != nullptr) {
        if (boundsRangeInfo_.find(block) == boundsRangeInfo_.end()) {
            auto it1 = boundsRangeInfo_.emplace(block, aa_.Adapter());
            ASSERT(it1.second);
            it1.first->second.emplace(inst, range);
        } else if (boundsRangeInfo_.at(block).find(inst) == boundsRangeInfo_.at(block).end()) {
            boundsRangeInfo_.at(block).emplace(inst, range);
        } else {
            boundsRangeInfo_.at(block).at(inst) = range;
        }
    }
}

BoundsAnalysis::BoundsAnalysis(Graph *graph)
    : Analysis(graph), boundsRangeInfo_(graph->GetAllocator()), loopsInfoTable_(graph->GetAllocator()->Adapter())
{
}

bool BoundsAnalysis::RunImpl()
{
    MarkerHolder holder {GetGraph()};
    visited_ = holder.GetMarker();
    ASSERT(!GetGraph()->IsBytecodeOptimizer());
    boundsRangeInfo_.Clear();
    loopsInfoTable_.clear();

    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<LoopAnalyzer>();

    VisitGraph();

    return true;
}

bool BoundsAnalysis::IsInstNotNull(const Inst *inst, BasicBlock *block)
{
    if (compiler::IsInstNotNull(inst)) {
        return true;
    }
    auto bri = block->GetGraph()->GetBoundsRangeInfo();
    auto range = bri->FindBoundsRange(block, inst);
    return range.IsMore(BoundsRange(0));
}

const ArenaVector<BasicBlock *> &BoundsAnalysis::GetBlocksToVisit() const
{
    return GetGraph()->GetBlocksRPO();
}

void BoundsAnalysis::VisitNeg(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeUnary<Opcode::Neg>(v, inst);
}

void BoundsAnalysis::VisitNegOverflowAndZeroCheck(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeUnary<Opcode::Neg>(v, inst);
}

void BoundsAnalysis::VisitAbs(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeUnary<Opcode::Abs>(v, inst);
}

void BoundsAnalysis::VisitAdd(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeBinary<Opcode::Add>(v, inst);
}

void BoundsAnalysis::VisitAddOverflowCheck(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeBinary<Opcode::Add>(v, inst);
}

void BoundsAnalysis::VisitSub(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeBinary<Opcode::Sub>(v, inst);
}

void BoundsAnalysis::VisitSubOverflowCheck(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeBinary<Opcode::Sub>(v, inst);
}

void BoundsAnalysis::VisitMod(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeBinary<Opcode::Mod>(v, inst);
}

void BoundsAnalysis::VisitDiv(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeBinary<Opcode::Div>(v, inst);
}

void BoundsAnalysis::VisitMul(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeBinary<Opcode::Mul>(v, inst);
}

void BoundsAnalysis::VisitAnd(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeBinary<Opcode::And>(v, inst);
}

void BoundsAnalysis::VisitShr(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeBinary<Opcode::Shr>(v, inst);
}

void BoundsAnalysis::VisitAShr(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeBinary<Opcode::AShr>(v, inst);
}

void BoundsAnalysis::VisitShl(GraphVisitor *v, Inst *inst)
{
    CalcNewBoundsRangeBinary<Opcode::Shl>(v, inst);
}

void BoundsAnalysis::VisitIfImm(GraphVisitor *v, Inst *inst)
{
    auto ifInst = inst->CastToIfImm();
    if (ifInst->GetOperandsType() != DataType::BOOL) {
        return;
    }
    ASSERT(ifInst->GetCc() == ConditionCode::CC_NE || ifInst->GetCc() == ConditionCode::CC_EQ);
    ASSERT(ifInst->GetImm() == 0);

    auto input = inst->GetInput(0).GetInst();
    if (input->GetOpcode() == Opcode::IsInstance) {
        CalcNewBoundsRangeForIsInstanceInput(v, input->CastToIsInstance(), ifInst);
        return;
    }
    if (input->GetOpcode() != Opcode::Compare) {
        return;
    }
    auto compare = input->CastToCompare();
    auto op0 = compare->GetInput(0).GetInst();
    auto op1 = compare->GetInput(1).GetInst();
    if ((DataType::GetCommonType(op0->GetType()) != DataType::INT64 && op0->GetType() != DataType::REFERENCE) ||
        (DataType::GetCommonType(op1->GetType()) != DataType::INT64 && op1->GetType() != DataType::REFERENCE)) {
        return;
    }

    auto cc = compare->GetCc();
    auto block = inst->GetBasicBlock();
    BasicBlock *trueBlock;
    BasicBlock *falseBlock;
    if (ifInst->GetCc() == ConditionCode::CC_NE) {
        // Corresponds to Compare result
        trueBlock = block->GetTrueSuccessor();
        falseBlock = block->GetFalseSuccessor();
    } else if (ifInst->GetCc() == ConditionCode::CC_EQ) {
        // Corresponds to inversion of Compare result
        trueBlock = block->GetFalseSuccessor();
        falseBlock = block->GetTrueSuccessor();
    } else {
        UNREACHABLE();
    }
    CalcNewBoundsRangeForCompare(v, block, cc, {op0, op1}, trueBlock);
    CalcNewBoundsRangeForCompare(v, block, GetInverseConditionCode(cc), {op0, op1}, falseBlock);
}

void BoundsAnalysis::VisitPhi(GraphVisitor *v, Inst *inst)
{
    if (IsFloatType(inst->GetType()) || inst->GetType() == DataType::ANY || inst->GetType() == DataType::POINTER) {
        return;
    }
    auto boundsAnalysis = static_cast<BoundsAnalysis *>(v);
    auto bri = boundsAnalysis->GetBoundsRangeInfo();
    auto phi = inst->CastToPhi();
    // for phi in a countable loop
    if (inst->GetType() != DataType::REFERENCE && boundsAnalysis->ProcessCountableLoop(phi, bri)) {
        return;
    }
    // for other phis
    MergePhiPredecessors(phi, bri);
}

template <bool CHECK_TYPE>
bool BoundsAnalysis::MergePhiPredecessors(PhiInst *phi, BoundsRangeInfo *bri)
{
    auto phiBlock = phi->GetBasicBlock();
    ArenaVector<BoundsRange> ranges(phiBlock->GetGraph()->GetLocalAllocator()->Adapter());
    for (auto *block : phiBlock->GetPredsBlocks()) {
        auto *inst = phi->GetPhiInput(block);
        if constexpr (CHECK_TYPE) {
            if (inst->GetInputs().size() <= 1) {
                return false;
            }
            auto loop = phi->GetBasicBlock()->GetLoop();
            auto input0 = inst->GetInput(0).GetInst();
            auto loop0 = input0->GetBasicBlock()->GetLoop();
            auto input1 = inst->GetInput(1).GetInst();
            auto loop1 = input1->GetBasicBlock()->GetLoop();
            if (!(inst->IsAddSub() && (input0->IsPhi() || input1->IsPhi()) &&
                  (!loop0->IsInside(loop) || !loop1->IsInside(loop))) &&
                !inst->IsPhi()) {
                // NOTE: support Mul-Div and maybe more cases
                return false;
            }
        }
        ranges.emplace_back(bri->FindBoundsRange(block, inst));
    }
    bri->SetBoundsRange(phiBlock, phi, BoundsRange::Union(ranges).FitInType(phi->GetType()));
    return true;
}

void BoundsAnalysis::VisitNullCheck(GraphVisitor *v, Inst *inst)
{
    ProcessNullCheck(v, inst, inst->GetDataFlowInput(0));
}

std::optional<uint64_t> BoundsAnalysis::GetNestedLoopIterations(Loop *loop, CountableLoopInfo &loopInfo)
{
    auto iterations = CountableLoopParser::GetLoopIterations(loopInfo);
    if (!iterations.has_value()) {
        return std::nullopt;
    }
    auto iterationsValue = iterations.value();
    auto *outer = loop->GetOuterLoop();
    if (outer->IsRoot()) {
        return iterationsValue;
    }
    auto it = loopsInfoTable_.find(loop->GetOuterLoop());
    if (it == loopsInfoTable_.end()) {
        return std::nullopt;
    }
    auto outerLoopInfo = (*it).second;
    if (!outerLoopInfo.has_value()) {
        return std::nullopt;
    }
    auto outerIters = outerLoopInfo.value().second;
    if (outerIters == std::numeric_limits<uint64_t>::max()) {
        return outerIters;
    }
    return BoundsRange::MulWithOverflowCheck(iterationsValue, outerIters);
}

std::optional<LoopIterationsInfo> BoundsAnalysis::GetSimpleLoopIterationsInfo(Loop *loop)
{
    auto loopParser = CountableLoopParser(*loop);
    auto loopInfo = loopParser.Parse();
    if (!loopInfo.has_value()) {
        // Loop is not countable
        loopsInfoTable_.emplace(std::make_pair(loop, std::nullopt));
        return std::nullopt;
    }
    auto loopInfoValue = loopInfo.value();
    // Generally speaking, countable loop can have compile-time unknown number of iterations,
    // i.e. when its init instruction is not a constant
    // In this case it is possible to get the index phi range, but not for other phis
    auto iterations = GetNestedLoopIterations(loop, loopInfoValue);
    auto iterationsValue = std::numeric_limits<uint64_t>::max();
    if (iterations.has_value()) {
        iterationsValue = iterations.value();
    }
    auto loopIterInfo = std::make_pair(loopInfoValue, iterationsValue);
    loopsInfoTable_.emplace(std::make_pair(loop, loopIterInfo));
    return loopIterInfo;
}

std::optional<LoopIterationsInfo> BoundsAnalysis::GetNestedLoopIterationsInfo(Loop *loop)
{
    auto it = loopsInfoTable_.find(loop);
    if (it == loopsInfoTable_.end()) {
        auto loopIterInfo = GetSimpleLoopIterationsInfo(loop);
        // If loop is visited first time - parse loop and fill the table
        for (auto *innerLoop : loop->GetInnerLoops()) {
            // If some of inner loops is not countable - we cannot process phis, except index
            if (!GetSimpleLoopIterationsInfo(innerLoop).has_value()) {
                return std::nullopt;
            }
        }
        return loopIterInfo;
    }
    auto loopIterInfo = (*it).second;
    if (!loopIterInfo.has_value()) {
        // Loop has been visited and it is not countable
        return std::nullopt;
    }
    return loopIterInfo.value();
}

bool BoundsAnalysis::ProcessCountableLoop(PhiInst *phi, BoundsRangeInfo *bri)
{
    auto *phiBlock = phi->GetBasicBlock();
    auto *loop = phiBlock->GetLoop();
    auto loopIterInfo = GetNestedLoopIterationsInfo(loop);
    if (!loopIterInfo.has_value()) {
        // loop is not countable
        return false;
    }
    [[maybe_unused]] auto [loopInfo, iterations] = loopIterInfo.value();
    if (phi == loopInfo.index) {
        // Index phi was processed while parsing the loop
        return loopsRevisiting_ ? true : ProcessIndexPhi(loop, bri, loopInfo);
    }
    if (iterations != std::numeric_limits<uint64_t>::max()) {
        if (phiBlock == loop->GetHeader()) {
            // At the beginning assign init phi a value range from its input
            return ProcessInitPhi(phi, bri);
        } else {  // NOLINT(readability-else-after-return)
            // For update phi merge predecessors and propagate bounds info to a loop
            return ProcessUpdatePhi(phi, bri, iterations);
        }
    }
    return false;
}

// "static" keyword for internal linkage
static void MoveRangeAccordingCC(ConditionCode cc, BoundsRange &lowerRange, BoundsRange &upperRange, const Inst *upper)
{
    if (cc == CC_GT) {
        lowerRange = lowerRange.Add(BoundsRange(1));
    } else if (cc == CC_LT) {
        upperRange = upperRange.Sub(BoundsRange(1));
        if (IsLenArray(upper)) {
            upperRange.SetLenArray(upper);
        }
    }
}

bool BoundsAnalysis::ProcessIndexPhi(Loop *loop, BoundsRangeInfo *bri, CountableLoopInfo &loopInfoValue)
{
    if (loopInfoValue.update->IsAddSub()) {
        return ProcessIndexPhiForAddSub(loop, bri, loopInfoValue);
    }

    if (loopInfoValue.update->IsShift()) {
        return false;
    }

    UNREACHABLE();
}

bool BoundsAnalysis::ProcessIndexPhiForAddSub(Loop *loop, BoundsRangeInfo *bri, CountableLoopInfo &loopInfoValue)
{
    auto *indexPhi = loopInfoValue.index;
    auto *phiBlock = indexPhi->GetBasicBlock();
    ASSERT(loopInfoValue.update->IsAddSub());
    auto *lower = loopInfoValue.init;
    auto *upper = loopInfoValue.test;
    auto cc = loopInfoValue.normalizedCc;
    ASSERT(cc == CC_LE || cc == CC_LT || cc == CC_GE || cc == CC_GT);
    if (!loopInfoValue.isInc) {
        lower = loopInfoValue.test;
        upper = loopInfoValue.init;
    }
    auto lowerRange = bri->FindBoundsRange(phiBlock, lower);
    auto upperRange = bri->FindBoundsRange(phiBlock, upper);
    MoveRangeAccordingCC(cc, lowerRange, upperRange, upper);
    if (lowerRange.GetLeft() > upperRange.GetRight()) {
        return false;
    }
    auto phiType = indexPhi->GetType();
    auto left = lowerRange.GetLeft();
    auto right = upperRange.GetRight();
    auto lenArray = upperRange.GetLenArray();
    auto isHeadLoopExit = loopInfoValue.ifImm->GetBasicBlock() == loop->GetHeader();
    if (!upperRange.IsMoreOrEqual(lowerRange) && !isHeadLoopExit) {
        return false;
    }
    if (!upperRange.IsMoreOrEqual(lowerRange) && !CountableLoopParser::HasPreHeaderCompare(loop, loopInfoValue)) {
        ASSERT(phiBlock == loop->GetHeader());
        if (loopInfoValue.ifImm->GetBasicBlock() == phiBlock) {
            auto nextLoopBlock = phiBlock->GetTrueSuccessor();
            if (nextLoopBlock->GetLoop() != loop && !nextLoopBlock->GetLoop()->IsInside(loop)) {
                nextLoopBlock = phiBlock->GetFalseSuccessor();
                ASSERT(nextLoopBlock->GetLoop() == loop || nextLoopBlock->GetLoop()->IsInside(loop));
            }
            if (nextLoopBlock != phiBlock) {
                auto range = BoundsRange(left, right, lenArray);
                bri->SetBoundsRange(nextLoopBlock, indexPhi, range.FitInType(phiType));
            }
        }
        // index can be more (less) than loop bound on first iteration
        if (cc == CC_LE || cc == CC_LT) {
            right = std::max(right, lowerRange.GetRight());
            lenArray = nullptr;
        } else {
            left = std::min(left, upperRange.GetLeft());
        }
    }
    auto range = BoundsRange(left, right, lenArray);
    bri->SetBoundsRange(phiBlock, indexPhi, range.FitInType(phiType));
    return true;
}

bool BoundsAnalysis::ProcessInitPhi(PhiInst *initPhi, BoundsRangeInfo *bri)
{
    auto *phiBlock = initPhi->GetBasicBlock();
    auto *curLoop = phiBlock->GetLoop();
    auto inputs = initPhi->GetInputs();

    // Find an init instruction, coming inside of the current loop
    auto *initInst = inputs[0].GetInst();
    auto *updateInst = inputs[1].GetInst();
    if (curLoop->IsInside(updateInst->GetBasicBlock()->GetLoop())) {
        std::swap(initInst, updateInst);
    }

    if (!loopsRevisiting_) {
        if (!updateInst->IsPhi()) {
            // For cases, when an init phi has no corresponding update phi
            return false;
        }
        auto initRange = bri->FindBoundsRange(initInst->GetBasicBlock(), initInst);
        bri->SetBoundsRange(phiBlock, initPhi, initRange.FitInType(initPhi->GetType()));
        return true;
    }

    // Else init phi range is the union range
    // (which is [min(init.left, update.left), max(init.right, update.right)])
    if (!updateInst->IsMarked(visited_)) {
        // If corresponding update phi was not processed yet
        return true;
    }
    auto *lenArray = bri->FindBoundsRange(initPhi->GetBasicBlock(), initPhi).GetLenArray();
    MergePhiPredecessors<false>(initPhi, bri);
    if (lenArray != nullptr) {
        bri->FindBoundsRange(initPhi->GetBasicBlock(), initPhi).SetLenArray(lenArray);
    }
    return true;
}

bool BoundsAnalysis::ProcessUpdatePhi(PhiInst *updatePhi, BoundsRangeInfo *bri, uint64_t iterations)
{
    auto *updatePhiBlock = updatePhi->GetBasicBlock();
    auto *loop = updatePhiBlock->GetLoop();
    auto *header = loop->GetHeader();

    // Find corresponding init phi
    auto *initPhi = TryFindCorrespondingInitPhi(updatePhi, header);
    if (initPhi == nullptr) {
        // For the case, when an update phi has no init phi prototype in the header
        // (i.e. some local loop variable control flow)
        // it is not required to reprocess the loop
        return true;
    }

    // Unite inputs' bounds range info
    if (!MergePhiPredecessors<true>(updatePhi, bri)) {
        // Cannot count update phi range => need to invalidate init phi range
        // Set maximal possible range and revisit loop to invalidate dependent ranges
        bri->SetBoundsRange(header, initPhi, BoundsRange(initPhi->GetType()));
        VisitLoop(header, updatePhiBlock);
        return false;
    }

    // While revisiting, we just merge predecessors, because they can be changed
    if (loopsRevisiting_) {
        return true;
    }

    auto initRange = bri->FindBoundsRange(header, initPhi);
    auto updateRange = bri->FindBoundsRange(updatePhiBlock, updatePhi);
    auto iterRange = BoundsRange(iterations);

    // Update phi range is changing by adding a range difference multiplied by iterations number
    auto diffRange = updateRange.Diff(initRange);
    auto updateNewRange = initRange.Add(diffRange.Mul(iterRange));
    bri->SetBoundsRange(updatePhiBlock, updatePhi, updateNewRange.FitInType(updatePhi->GetType()));
    updatePhi->SetMarker(visited_);

    // Propagate init phi bounds info to a loop
    VisitLoop(header, updatePhiBlock);
    return true;
}

Inst *BoundsAnalysis::TryFindCorrespondingInitPhi(PhiInst *updatePhi, BasicBlock *header)
{
    for (auto &user : updatePhi->GetUsers()) {
        if (user.GetInst()->GetBasicBlock() != header) {
            continue;
        }
        auto initPhi = user.GetInst();
        ASSERT(initPhi->IsPhi());
        return initPhi;
    }
    return nullptr;
}

void BoundsAnalysis::VisitLoop(BasicBlock *header, BasicBlock *updatePhiBlock)
{
    auto &rpo = GetGraph()->GetBlocksRPO();
    auto *curLoop = header->GetLoop();
    auto startIt = std::find(rpo.begin(), rpo.end(), header);
    auto endIt = std::find(rpo.begin(), rpo.end(), updatePhiBlock);
    loopsRevisiting_ = true;  // To avoid complex recalculations and not to loop the algorithm

    // First part: revisit current loop and its inner loops, which have been visited before
    for (auto it = startIt; it != endIt; ++it) {
        auto *block = *it;
        // Process loop and its nested loops only
        if (block->GetLoop() != curLoop && !block->GetLoop()->IsInside(curLoop)) {
            continue;
        }
        for (auto *inst : block->AllInsts()) {
            TABLE[static_cast<unsigned>(inst->GetOpcode())](this, inst);
        }
    }

    // Second part: revisit outer loops, going out of nest
    curLoop = curLoop->GetOuterLoop();
    auto *innerHeader = header;
    while (!curLoop->IsRoot()) {
        auto *curHeader = curLoop->GetHeader();
        startIt = std::find(rpo.begin(), rpo.end(), curHeader);
        endIt = std::find(rpo.begin(), rpo.end(), innerHeader);
        for (auto it = startIt; it != endIt; ++it) {
            auto *block = *it;
            // Process this loop only
            if (block->GetLoop() != curLoop) {
                continue;
            }
            for (auto *inst : block->AllInsts()) {
                TABLE[static_cast<unsigned>(inst->GetOpcode())](this, inst);
            }
        }
        curLoop = curLoop->GetOuterLoop();
        innerHeader = curHeader;
    }

    loopsRevisiting_ = false;
}

bool BoundsAnalysis::CheckTriangleCase(const BasicBlock *block, const BasicBlock *tgtBlock)
{
    auto &predsBlocks = tgtBlock->GetPredsBlocks();
    auto loop = tgtBlock->GetLoop();
    auto &backEdges = loop->GetBackEdges();
    if (predsBlocks.size() == 1) {
        return true;
    }
    if (!loop->IsRoot() && backEdges.size() == 1 && predsBlocks.size() == 2U) {
        if (predsBlocks[0] == block && predsBlocks[1] == backEdges[0]) {
            return true;
        }
        if (predsBlocks[1] == block && predsBlocks[0] == backEdges[0]) {
            return true;
        }
        return false;
    }
    return false;
}

void BoundsAnalysis::ProcessNullCheck(GraphVisitor *v, const Inst *checkInst, const Inst *refInput)
{
    ASSERT(checkInst->IsNullCheck() || checkInst->GetOpcode() == Opcode::DeoptimizeIf);
    ASSERT(refInput->GetType() == DataType::REFERENCE);
    auto bri = static_cast<BoundsAnalysis *>(v)->GetBoundsRangeInfo();
    auto block = checkInst->GetBasicBlock();
    auto range = BoundsRange(1, BoundsRange::GetMax(DataType::REFERENCE));
    for (auto domBlock : block->GetDominatedBlocks()) {
        bri->SetBoundsRange(domBlock, refInput, range);
    }
}

void BoundsAnalysis::CalcNewBoundsRangeForIsInstanceInput(GraphVisitor *v, IsInstanceInst *isInstance, IfImmInst *ifImm)
{
    ASSERT(isInstance == ifImm->GetInput(0).GetInst());
    auto block = ifImm->GetBasicBlock();
    auto trueBlock = ifImm->GetEdgeIfInputTrue();
    if (CheckTriangleCase(block, trueBlock)) {
        auto bri = static_cast<BoundsAnalysis *>(v)->GetBoundsRangeInfo();
        auto ref = isInstance->GetInput(0).GetInst();
        // if IsInstance evaluates to True, its input is not null
        auto range = BoundsRange(1, BoundsRange::GetMax(DataType::REFERENCE));
        bri->SetBoundsRange(trueBlock, ref, range);
    }
}

void BoundsAnalysis::CalcNewBoundsRangeForCompare(GraphVisitor *v, BasicBlock *block, ConditionCode cc, InstPair args,
                                                  BasicBlock *tgtBlock)
{
    auto bri = static_cast<BoundsAnalysis *>(v)->GetBoundsRangeInfo();
    auto [left, right] = args;
    auto leftRange = bri->FindBoundsRange(block, left);
    auto rightRange = bri->FindBoundsRange(block, right);
    // try to skip triangle:
    /* [block]
     *    |  \
     *    |   \
     *    |   [BB]
     *    |   /
     *    |  /
     * [tgt_block]
     */
    if (CheckTriangleCase(block, tgtBlock)) {
        auto ranges = BoundsRange::TryNarrowBoundsByCC(cc, {leftRange, rightRange});
        if (cc == ConditionCode::CC_LT && IsLenArray(right)) {
            ranges.first.SetLenArray(right);
        } else if (cc == ConditionCode::CC_GT && IsLenArray(left)) {
            ranges.second.SetLenArray(left);
        } else if (leftRange.GetLenArray() != nullptr) {
            ranges.first.SetLenArray(leftRange.GetLenArray());
        } else if (rightRange.GetLenArray() != nullptr) {
            ranges.second.SetLenArray(rightRange.GetLenArray());
        }
        bri->SetBoundsRange(tgtBlock, left, ranges.first.FitInType(left->GetType()));
        bri->SetBoundsRange(tgtBlock, right, ranges.second.FitInType(right->GetType()));
    }
}

template <Opcode OPC>
void BoundsAnalysis::CalcNewBoundsRangeUnary(GraphVisitor *v, const Inst *inst)
{
    if (IsFloatType(inst->GetType()) || inst->GetType() == DataType::REFERENCE) {
        return;
    }
    auto bri = static_cast<BoundsAnalysis *>(v)->GetBoundsRangeInfo();
    auto input0 = inst->GetInput(0).GetInst();
    auto range0 = bri->FindBoundsRange(inst->GetBasicBlock(), input0);
    BoundsRange range;
    // clang-format off
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (OPC == Opcode::Neg) {
            range = range0.Neg();
        // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (OPC == Opcode::Abs) {
            range = range0.Abs();
        } else {
            UNREACHABLE();
        }
    // clang-format on
    bri->SetBoundsRange(inst->GetBasicBlock(), inst, range.FitInType(inst->GetType()));
}

BoundsRange BoundsAnalysis::CalcNewBoundsRangeAdd(const BoundsRangeInfo *bri, const Inst *inst)
{
    auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    auto range0 = bri->FindBoundsRange(inst->GetBasicBlock(), input0);
    auto range1 = bri->FindBoundsRange(inst->GetBasicBlock(), input1);
    auto lenArray0 = range0.GetLenArray();
    auto lenArray1 = range1.GetLenArray();

    BoundsRange range = range0.Add(range1);
    if (!range.IsMaxRange(inst->GetType())) {
        if (BoundsRange(0).IsMoreOrEqual(range1) && lenArray0 != nullptr) {
            range.SetLenArray(lenArray0);
        } else if (BoundsRange(0).IsMoreOrEqual(range0) && lenArray1 != nullptr) {
            range.SetLenArray(lenArray1);
        } else if (IsLenArray(input0) && range1.IsNegative()) {
            range.SetLenArray(input0);
        } else if (IsLenArray(input1) && range0.IsNegative()) {
            range.SetLenArray(input1);
        }
    }
    return range;
}

BoundsRange BoundsAnalysis::CalcNewBoundsRangeSub(const BoundsRangeInfo *bri, const Inst *inst)
{
    auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    auto range0 = bri->FindBoundsRange(inst->GetBasicBlock(), input0);
    auto range1 = bri->FindBoundsRange(inst->GetBasicBlock(), input1);
    auto lenArray0 = range0.GetLenArray();

    BoundsRange range = range0.Sub(range1);
    if (!range.IsMaxRange(inst->GetType())) {
        if (range1.IsNotNegative() && lenArray0 != nullptr) {
            range.SetLenArray(lenArray0);
        } else if (IsLenArray(input0) && range1.IsMore(BoundsRange(0))) {
            range.SetLenArray(input0);
        }
    }
    return range;
}

BoundsRange BoundsAnalysis::CalcNewBoundsRangeMod(const BoundsRangeInfo *bri, const Inst *inst)
{
    auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    auto range0 = bri->FindBoundsRange(inst->GetBasicBlock(), input0);
    auto range1 = bri->FindBoundsRange(inst->GetBasicBlock(), input1);
    auto lenArray0 = range0.GetLenArray();
    auto lenArray1 = range1.GetLenArray();

    BoundsRange range = range0.Mod(range1);
    if (lenArray1 != nullptr && range1.IsNotNegative()) {
        range.SetLenArray(lenArray1);
    } else if (lenArray0 != nullptr) {
        // a % b always has the same sign as a, so if a < LenArray, then (a % b) < LenArray
        range.SetLenArray(lenArray0);
    } else if (IsLenArray(input1)) {
        range.SetLenArray(input1);
    }
    return range;
}

BoundsRange BoundsAnalysis::CalcNewBoundsRangeMul(const BoundsRangeInfo *bri, const Inst *inst)
{
    auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    auto range0 = bri->FindBoundsRange(inst->GetBasicBlock(), input0);
    auto range1 = bri->FindBoundsRange(inst->GetBasicBlock(), input1);
    if (!range0.IsMaxRange() || !range1.IsMaxRange()) {
        return range0.Mul(range1);
    }
    return BoundsRange();
}

BoundsRange BoundsAnalysis::CalcNewBoundsRangeDiv(const BoundsRangeInfo *bri, const Inst *inst)
{
    auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    auto range0 = bri->FindBoundsRange(inst->GetBasicBlock(), input0);
    auto range1 = bri->FindBoundsRange(inst->GetBasicBlock(), input1);
    auto lenArray0 = range0.GetLenArray();

    BoundsRange range = range0.Div(range1);
    if (range0.IsNotNegative() && range1.IsNotNegative() && lenArray0 != nullptr) {
        range.SetLenArray(lenArray0);
    }
    return range;
}

BoundsRange BoundsAnalysis::CalcNewBoundsRangeShr(const BoundsRangeInfo *bri, const Inst *inst)
{
    auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    auto range0 = bri->FindBoundsRange(inst->GetBasicBlock(), input0);
    auto range1 = bri->FindBoundsRange(inst->GetBasicBlock(), input1);
    auto lenArray0 = range0.GetLenArray();

    BoundsRange range = range0.Shr(range1, inst->GetType());
    if (range0.IsNotNegative() && lenArray0 != nullptr) {
        range.SetLenArray(lenArray0);
    }
    return range;
}

BoundsRange BoundsAnalysis::CalcNewBoundsRangeAShr(const BoundsRangeInfo *bri, const Inst *inst)
{
    auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    auto range0 = bri->FindBoundsRange(inst->GetBasicBlock(), input0);
    auto range1 = bri->FindBoundsRange(inst->GetBasicBlock(), input1);
    auto lenArray0 = range0.GetLenArray();

    BoundsRange range = range0.AShr(range1, inst->GetType());
    if (lenArray0 != nullptr) {
        range.SetLenArray(lenArray0);
    }
    return range;
}

BoundsRange BoundsAnalysis::CalcNewBoundsRangeShl(const BoundsRangeInfo *bri, const Inst *inst)
{
    auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    auto range0 = bri->FindBoundsRange(inst->GetBasicBlock(), input0);
    auto range1 = bri->FindBoundsRange(inst->GetBasicBlock(), input1);

    return range0.Shl(range1, inst->GetType());
}

BoundsRange BoundsAnalysis::CalcNewBoundsRangeAnd(const BoundsRangeInfo *bri, const Inst *inst)
{
    auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    auto range0 = bri->FindBoundsRange(inst->GetBasicBlock(), input0);
    auto range1 = bri->FindBoundsRange(inst->GetBasicBlock(), input1);

    return range0.And(range1);
}

bool BoundsAnalysis::CheckBoundsRange(const BoundsRangeInfo *bri, const Inst *inst)
{
    auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    auto range0 = bri->FindBoundsRange(inst->GetBasicBlock(), input0);
    auto range1 = bri->FindBoundsRange(inst->GetBasicBlock(), input1);
    return (input0->GetType() != DataType::UINT64 || !range0.IsMaxRange(DataType::UINT64) ||
            range0.GetLenArray() != nullptr) &&
           (input1->GetType() != DataType::UINT64 || !range1.IsMaxRange(DataType::UINT64) ||
            range1.GetLenArray() != nullptr);
}

template <Opcode OPC>
void BoundsAnalysis::CalcNewBoundsRangeBinary(GraphVisitor *v, const Inst *inst)
{
    if (IsFloatType(inst->GetType()) || inst->GetType() == DataType::REFERENCE) {
        return;
    }
    auto bri = static_cast<BoundsAnalysis *>(v)->GetBoundsRangeInfo();
    BoundsRange range;
    if (CheckBoundsRange(bri, inst)) {
        if constexpr (OPC == Opcode::Add) {  // NOLINT
            range = CalcNewBoundsRangeAdd(bri, inst);
        } else if constexpr (OPC == Opcode::Sub) {  // NOLINT
            range = CalcNewBoundsRangeSub(bri, inst);
        } else if constexpr (OPC == Opcode::Mod) {  // NOLINT
            range = CalcNewBoundsRangeMod(bri, inst);
        } else if constexpr (OPC == Opcode::Mul) {  // NOLINT
            range = CalcNewBoundsRangeMul(bri, inst);
        } else if constexpr (OPC == Opcode::Div) {  // NOLINT
            range = CalcNewBoundsRangeDiv(bri, inst);
        } else if constexpr (OPC == Opcode::Shr) {  // NOLINT
            range = CalcNewBoundsRangeShr(bri, inst);
        } else if constexpr (OPC == Opcode::AShr) {  // NOLINT
            range = CalcNewBoundsRangeAShr(bri, inst);
        } else if constexpr (OPC == Opcode::Shl) {  // NOLINT
            range = CalcNewBoundsRangeShl(bri, inst);
        } else if constexpr (OPC == Opcode::And) {
            range = CalcNewBoundsRangeAnd(bri, inst);
        } else {
            UNREACHABLE();
        }
    }
    bri->SetBoundsRange(inst->GetBasicBlock(), inst, range.FitInType(inst->GetType()));
}

}  // namespace ark::compiler
