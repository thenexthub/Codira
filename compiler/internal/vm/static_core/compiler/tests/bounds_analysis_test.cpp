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

#include "unit_test.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/peepholes.h"
#include "libarkbase/utils/arena_containers.h"

namespace ark::compiler {
class BoundsAnalysisTest : public CommonTest {
public:
    BoundsAnalysisTest() : graph_(CreateGraphStartEndBlocks()) {}

    Graph *GetGraph()
    {
        return graph_;
    }

    // ll, lr, rl, rr - test bounds
    // rll, rlr, rrl, rrr - result bounds
    void CCTest(ConditionCode cc, BoundsRange l, BoundsRange r, BoundsRange rl, BoundsRange rr)
    {
        auto [nrl, nrr] = BoundsRange::TryNarrowBoundsByCC(cc, {l, r});
        ASSERT_EQ(nrl.GetLeft(), rl.GetLeft());
        ASSERT_EQ(nrl.GetRight(), rl.GetRight());
        ASSERT_EQ(nrr.GetLeft(), rr.GetLeft());
        ASSERT_EQ(nrr.GetRight(), rr.GetRight());
    }

    using BR = BoundsRange;

    void TypeFittingBuildGraph();

private:
    Graph *graph_ {nullptr};
};
// NOLINTBEGIN(readability-magic-numbers)
TEST_F(BoundsAnalysisTest, NegTest)
{
    BoundsRange r1 = BoundsRange(-1L, 5U);
    BoundsRange res1 = r1.Neg();
    EXPECT_EQ(res1.GetLeft(), -5L);
    EXPECT_EQ(res1.GetRight(), 1U);

    BoundsRange r2 = BoundsRange(1U, 5U);
    BoundsRange res2 = r2.Neg();
    EXPECT_EQ(res2.GetLeft(), -5L);
    EXPECT_EQ(res2.GetRight(), -1L);

    BoundsRange r3 = BoundsRange(-5L, -1L);
    BoundsRange res3 = r3.Neg();
    EXPECT_EQ(res3.GetLeft(), 1U);
    EXPECT_EQ(res3.GetRight(), 5U);
}

TEST_F(BoundsAnalysisTest, AbsTest)
{
    BoundsRange r1 = BoundsRange(1U, 5U);
    BoundsRange res1 = r1.Abs();
    EXPECT_EQ(res1.GetLeft(), 1U);
    EXPECT_EQ(res1.GetRight(), 5U);

    BoundsRange r2 = BoundsRange(-5L, -1L);
    BoundsRange res2 = r2.Abs();
    EXPECT_EQ(res2.GetLeft(), 1U);
    EXPECT_EQ(res2.GetRight(), 5U);

    BoundsRange r3 = BoundsRange(-1L, 5U);
    BoundsRange res3 = r3.Abs();
    EXPECT_EQ(res3.GetLeft(), 0U);
    EXPECT_EQ(res3.GetRight(), 5U);

    BoundsRange r4 = BoundsRange(-10L, 5U);
    BoundsRange res4 = r4.Abs();
    EXPECT_EQ(res4.GetLeft(), 0U);
    EXPECT_EQ(res4.GetRight(), 10U);
}

TEST_F(BoundsAnalysisTest, MulTest)
{
    BoundsRange r1 = BoundsRange(-7L, 5U);
    BoundsRange r2 = BoundsRange(9U, 13U);
    BoundsRange r3 = BoundsRange(2U, 5U);
    BoundsRange r4 = BoundsRange(-4L, -1L);
    BoundsRange r5 = BoundsRange(-5L, -2L);
    BoundsRange r6 = BoundsRange(1U, 4U);

    // All posible variations for GetLeft and GetRight
    BoundsRange res = r1.Mul(r1);
    EXPECT_EQ(res.GetLeft(), -7L * 5L);    // min = ll * rr
    EXPECT_EQ(res.GetRight(), -7L * -7L);  // max = ll * rl

    res = r2.Mul(r2);
    EXPECT_EQ(res.GetLeft(), 9U * 9U);     // min = ll * rl
    EXPECT_EQ(res.GetRight(), 13U * 13U);  // max = lr * rr

    res = r3.Mul(r4);
    EXPECT_EQ(res.GetLeft(), 5L * -4L);   // min = lr * rl
    EXPECT_EQ(res.GetRight(), 2L * -1L);  // max = ll * rr

    res = r4.Mul(r5);
    EXPECT_EQ(res.GetLeft(), -1L * -2L);   // min = lr * rr
    EXPECT_EQ(res.GetRight(), -4L * -5L);  // max = ll * rl

    res = r5.Mul(r6);
    EXPECT_EQ(res.GetLeft(), -5L * 4L);   // min = ll * rr
    EXPECT_EQ(res.GetRight(), -2L * 1L);  // max = lr * rl
}

TEST_F(BoundsAnalysisTest, DivTest)
{
    auto l1 = BoundsRange(-7L, 5U);
    auto l2 = BoundsRange(5U, 7U);
    auto l3 = BoundsRange(-7L, -5L);
    auto r1 = BoundsRange(2U);
    auto r2 = BoundsRange(-2L);
    BoundsRange res;

    res = l1.Div(r1);
    EXPECT_EQ(res.GetLeft(), -3L);
    EXPECT_EQ(res.GetRight(), 2U);
    res = l1.Div(r2);
    EXPECT_EQ(res.GetLeft(), -2L);
    EXPECT_EQ(res.GetRight(), 3U);

    res = l2.Div(r1);
    EXPECT_EQ(res.GetLeft(), 2U);
    EXPECT_EQ(res.GetRight(), 3U);
    res = l2.Div(r2);
    EXPECT_EQ(res.GetLeft(), -3L);
    EXPECT_EQ(res.GetRight(), -2L);

    res = l3.Div(r1);
    EXPECT_EQ(res.GetLeft(), -3L);
    EXPECT_EQ(res.GetRight(), -2L);
    res = l3.Div(r2);
    EXPECT_EQ(res.GetLeft(), 2U);
    EXPECT_EQ(res.GetRight(), 3U);
}

TEST_F(BoundsAnalysisTest, OverflowTest)
{
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(INT64_MAX, INT64_MAX), std::nullopt);
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(INT64_MAX, 1U), std::nullopt);
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(1U, INT64_MAX), std::nullopt);
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(INT64_MIN, INT64_MIN), std::nullopt);
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(INT64_MIN, -1L), std::nullopt);
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(-1L, INT64_MIN), std::nullopt);

    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(0U, INT64_MAX), 0U);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MAX, 0U), 0U);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(0U, INT64_MIN), 0U);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MIN, 0U), 0U);

    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MAX, INT64_MAX), std::nullopt);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MAX, 2U), std::nullopt);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MIN, INT64_MIN), std::nullopt);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MIN, -2L), std::nullopt);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MAX, INT64_MIN), std::nullopt);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MAX, -2L), std::nullopt);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MIN, INT64_MAX), std::nullopt);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MIN, 2U), std::nullopt);

    ASSERT_EQ(BoundsRange::DivWithOverflowCheck(INT64_MIN, -1L), std::nullopt);

    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(UINT64_MAX, UINT64_MAX), std::nullopt);
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(UINT64_MAX, 1U), std::nullopt);
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(1U, UINT64_MAX), std::nullopt);
    ASSERT_NE(BoundsRange::AddWithOverflowCheck(UINT64_MAX / 2U, UINT64_MAX / 2U), std::nullopt);
    ASSERT_NE(BoundsRange::AddWithOverflowCheck(UINT64_MAX - 1U, 1U), std::nullopt);
    ASSERT_NE(BoundsRange::AddWithOverflowCheck(1U, UINT64_MAX - 1U), std::nullopt);

    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(0U, UINT64_MAX), 0U);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(UINT64_MAX, 0U), 0U);

    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(UINT64_MAX, UINT64_MAX), std::nullopt);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(UINT64_MAX / 2U + 1, 2U), std::nullopt);
    ASSERT_NE(BoundsRange::MulWithOverflowCheck(UINT64_MAX / 2U, 2U), std::nullopt);
}

TEST_F(BoundsAnalysisTest, BoundsNarrowing1)
{
    // case 1
    CCTest(ConditionCode::CC_GT, BR(10U, 50U), BR(20U, 60U), BR(21U, 50U), BR(20U, 49U));
    CCTest(ConditionCode::CC_A, BR(10U, 50U), BR(20U, 60U), BR(21U, 50U), BR(20U, 49U));
    CCTest(ConditionCode::CC_GE, BR(10U, 50U), BR(20U, 60U), BR(20U, 50U), BR(20U, 50U));
    CCTest(ConditionCode::CC_AE, BR(10U, 50U), BR(20U, 60U), BR(20U, 50U), BR(20U, 50U));
    CCTest(ConditionCode::CC_LT, BR(10U, 50U), BR(20U, 60U), BR(10U, 50U), BR(20U, 60U));
    CCTest(ConditionCode::CC_B, BR(10U, 50U), BR(20U, 60U), BR(10U, 50U), BR(20U, 60U));
    CCTest(ConditionCode::CC_LE, BR(10U, 50U), BR(20U, 60U), BR(10U, 50U), BR(20U, 60U));
    CCTest(ConditionCode::CC_BE, BR(10U, 50U), BR(20U, 60U), BR(10U, 50U), BR(20U, 60U));
    CCTest(ConditionCode::CC_EQ, BR(10U, 50U), BR(20U, 60U), BR(20U, 50U), BR(20U, 50U));
    CCTest(ConditionCode::CC_NE, BR(10U, 50U), BR(20U, 60U), BR(10U, 50U), BR(20U, 60U));

    // case 2
    CCTest(ConditionCode::CC_GT, BR(10U, 20U), BR(50U, 60U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_A, BR(10U, 20U), BR(50U, 60U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_GE, BR(10U, 20U), BR(50U, 60U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_AE, BR(10U, 20U), BR(50U, 60U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_LT, BR(10U, 20U), BR(50U, 60U), BR(10U, 20U), BR(50U, 60U));
    CCTest(ConditionCode::CC_B, BR(10U, 20U), BR(50U, 60U), BR(10U, 20U), BR(50U, 60U));
    CCTest(ConditionCode::CC_LE, BR(10U, 20U), BR(50U, 60U), BR(10U, 20U), BR(50U, 60U));
    CCTest(ConditionCode::CC_BE, BR(10U, 20U), BR(50U, 60U), BR(10U, 20U), BR(50U, 60U));
    CCTest(ConditionCode::CC_EQ, BR(10U, 20U), BR(50U, 60U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_NE, BR(10U, 20U), BR(50U, 60U), BR(10U, 20U), BR(50U, 60U));

    // case 3
    CCTest(ConditionCode::CC_GT, BR(10U, 60U), BR(20U, 50U), BR(21U, 60U), BR(20U, 50U));
    CCTest(ConditionCode::CC_A, BR(10U, 60U), BR(20U, 50U), BR(21U, 60U), BR(20U, 50U));
    CCTest(ConditionCode::CC_GE, BR(10U, 60U), BR(20U, 50U), BR(20U, 60U), BR(20U, 50U));
    CCTest(ConditionCode::CC_AE, BR(10U, 60U), BR(20U, 50U), BR(20U, 60U), BR(20U, 50U));
    CCTest(ConditionCode::CC_LT, BR(10U, 60U), BR(20U, 50U), BR(10U, 49U), BR(20U, 50U));
    CCTest(ConditionCode::CC_B, BR(10U, 60U), BR(20U, 50U), BR(10U, 49U), BR(20U, 50U));
    CCTest(ConditionCode::CC_LE, BR(10U, 60U), BR(20U, 50U), BR(10U, 50U), BR(20U, 50U));
    CCTest(ConditionCode::CC_BE, BR(10U, 60U), BR(20U, 50U), BR(10U, 50U), BR(20U, 50U));
    CCTest(ConditionCode::CC_EQ, BR(10U, 60U), BR(20U, 50U), BR(20U, 50U), BR(20U, 50U));
    CCTest(ConditionCode::CC_NE, BR(10U, 60U), BR(20U, 50U), BR(10U, 60U), BR(20U, 50U));
}

TEST_F(BoundsAnalysisTest, BoundsNarrowing2)
{
    // case 4
    CCTest(ConditionCode::CC_GT, BR(20U, 60U), BR(10U, 50U), BR(20U, 60U), BR(10U, 50U));
    CCTest(ConditionCode::CC_A, BR(20U, 60U), BR(10U, 50U), BR(20U, 60U), BR(10U, 50U));
    CCTest(ConditionCode::CC_GE, BR(20U, 60U), BR(10U, 50U), BR(20U, 60U), BR(10U, 50U));
    CCTest(ConditionCode::CC_AE, BR(20U, 60U), BR(10U, 50U), BR(20U, 60U), BR(10U, 50U));
    CCTest(ConditionCode::CC_LT, BR(20U, 60U), BR(10U, 50U), BR(20U, 49U), BR(21U, 50U));
    CCTest(ConditionCode::CC_B, BR(20U, 60U), BR(10U, 50U), BR(20U, 49U), BR(21U, 50U));
    CCTest(ConditionCode::CC_LE, BR(20U, 60U), BR(10U, 50U), BR(20U, 50U), BR(20U, 50U));
    CCTest(ConditionCode::CC_BE, BR(20U, 60U), BR(10U, 50U), BR(20U, 50U), BR(20U, 50U));
    CCTest(ConditionCode::CC_EQ, BR(20U, 60U), BR(10U, 50U), BR(20U, 50U), BR(20U, 50U));
    CCTest(ConditionCode::CC_NE, BR(20U, 60U), BR(10U, 50U), BR(20U, 60U), BR(10U, 50U));

    // case 5
    CCTest(ConditionCode::CC_GT, BR(50U, 60U), BR(10U, 20U), BR(50U, 60U), BR(10U, 20U));
    CCTest(ConditionCode::CC_A, BR(50U, 60U), BR(10U, 20U), BR(50U, 60U), BR(10U, 20U));
    CCTest(ConditionCode::CC_GE, BR(50U, 60U), BR(10U, 20U), BR(50U, 60U), BR(10U, 20U));
    CCTest(ConditionCode::CC_AE, BR(50U, 60U), BR(10U, 20U), BR(50U, 60U), BR(10U, 20U));
    CCTest(ConditionCode::CC_LT, BR(50U, 60U), BR(10U, 20U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_B, BR(50U, 60U), BR(10U, 20U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_LE, BR(50U, 60U), BR(10U, 20U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_BE, BR(50U, 60U), BR(10U, 20U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_EQ, BR(50U, 60U), BR(10U, 20U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_NE, BR(50U, 60U), BR(10U, 20U), BR(50U, 60U), BR(10U, 20U));

    // case 6
    CCTest(ConditionCode::CC_GT, BR(20U, 50U), BR(10U, 60U), BR(20U, 50U), BR(10U, 49U));
    CCTest(ConditionCode::CC_A, BR(20U, 50U), BR(10U, 60U), BR(20U, 50U), BR(10U, 49U));
    CCTest(ConditionCode::CC_GE, BR(20U, 50U), BR(10U, 60U), BR(20U, 50U), BR(10U, 50U));
    CCTest(ConditionCode::CC_AE, BR(20U, 50U), BR(10U, 60U), BR(20U, 50U), BR(10U, 50U));
    CCTest(ConditionCode::CC_LT, BR(20U, 50U), BR(10U, 60U), BR(20U, 50U), BR(21U, 60U));
    CCTest(ConditionCode::CC_B, BR(20U, 50U), BR(10U, 60U), BR(20U, 50U), BR(21U, 60U));
    CCTest(ConditionCode::CC_LE, BR(20U, 50U), BR(10U, 60U), BR(20U, 50U), BR(20U, 60U));
    CCTest(ConditionCode::CC_BE, BR(20U, 50U), BR(10U, 60U), BR(20U, 50U), BR(20U, 60U));
    CCTest(ConditionCode::CC_EQ, BR(20U, 50U), BR(10U, 60U), BR(20U, 50U), BR(20U, 50U));
    CCTest(ConditionCode::CC_NE, BR(20U, 50U), BR(10U, 60U), BR(20U, 50U), BR(10U, 60U));
}

TEST_F(BoundsAnalysisTest, IntervalCollisions)
{
    // Bounds collision
    CCTest(ConditionCode::CC_GT, BR(-10L, -5L), BR(-5L, 0U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_LT, BR(-5L, 0U), BR(-10L, -5L), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    // Single value interval collision
    CCTest(ConditionCode::CC_GT, BR(0U, 20U), BR(20U, 20U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_LT, BR(0U, 20U), BR(0U, 0U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_GT, BR(16U, 16U), BR(16U, 32U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
    CCTest(ConditionCode::CC_LT, BR(32U, 32U), BR(16U, 32U), BR(INT64_MIN, INT64_MAX), BR(INT64_MIN, INT64_MAX));
}

TEST_F(BoundsAnalysisTest, UnionTest)
{
    ArenaVector<BoundsRange> ranges(GetGraph()->GetAllocator()->Adapter());
    ranges.emplace_back(BoundsRange(-10L, 100U));
    ranges.emplace_back(BoundsRange(-20L, 15U));
    auto range = BoundsRange::Union(ranges);
    ASSERT_EQ(range.GetLeft(), -20L);
    ASSERT_EQ(range.GetRight(), 100U);
    ranges.clear();

    ranges.emplace_back(BoundsRange(INT64_MIN, -10L));
    ranges.emplace_back(BoundsRange(10U, INT64_MAX));
    range = BoundsRange::Union(ranges);
    ASSERT_EQ(range.GetLeft(), INT64_MIN);
    ASSERT_EQ(range.GetRight(), INT64_MAX);
}

void BoundsAnalysisTest::TypeFittingBuildGraph()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u16();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).u32();
        CONSTANT(3U, 0x23U).s64();
        CONSTANT(4U, 0x30U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            // v0 -- ANYTHING
            // v1 -- ANYTHING
            // v2 -- ANYTHING
            INST(5U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 3U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U, 5U)
        {
            // v0 -- ANYTHING
            // v1 <= 0x23
            // v2 -- ANYTHING
            INST(8U, Opcode::Compare).b().CC(CC_GT).Inputs(2U, 4U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            // v0 -- ANYTHING
            // v1 <= 0x23
            // v2 <= 0x30
            INST(11U, Opcode::Return).u16().Inputs(0U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            // v0 -- ANYTHING
            // v1 -- ANYTHING
            // v2 -- ANYTHING
            INST(14U, Opcode::ReturnI).u16().Imm(0x23U);
        }
    }
}

TEST_F(BoundsAnalysisTest, TypeFitting)
{
    TypeFittingBuildGraph();
    auto rinfo = GetGraph()->GetBoundsRangeInfo();
    BoundsRange range;

    // BB2
    range = rinfo->FindBoundsRange(&BB(2U), &INS(0U));
    EXPECT_EQ(range.GetLeft(), 0U);
    EXPECT_EQ(range.GetRight(), UINT16_MAX);

    range = rinfo->FindBoundsRange(&BB(2U), &INS(1U));
    EXPECT_EQ(range.GetLeft(), INT64_MIN);
    EXPECT_EQ(range.GetRight(), INT64_MAX);

    range = rinfo->FindBoundsRange(&BB(2U), &INS(2U));
    EXPECT_EQ(range.GetLeft(), 0U);
    EXPECT_EQ(range.GetRight(), UINT32_MAX);

    // BB4
    range = rinfo->FindBoundsRange(&BB(2U), &INS(0U));
    EXPECT_EQ(range.GetLeft(), 0U);
    EXPECT_EQ(range.GetRight(), UINT16_MAX);

    range = rinfo->FindBoundsRange(&BB(4U), &INS(1U));
    EXPECT_EQ(range.GetLeft(), INT64_MIN);
    EXPECT_EQ(range.GetRight(), 0x23U);

    range = rinfo->FindBoundsRange(&BB(2U), &INS(2U));
    EXPECT_EQ(range.GetLeft(), 0U);
    EXPECT_EQ(range.GetRight(), UINT32_MAX);

    // BB5
    range = rinfo->FindBoundsRange(&BB(2U), &INS(0U));
    EXPECT_EQ(range.GetLeft(), 0U);
    EXPECT_EQ(range.GetRight(), UINT16_MAX);

    range = rinfo->FindBoundsRange(&BB(4U), &INS(1U));
    EXPECT_EQ(range.GetLeft(), INT64_MIN);
    EXPECT_EQ(range.GetRight(), 0x23U);

    range = rinfo->FindBoundsRange(&BB(5U), &INS(2U));
    EXPECT_EQ(range.GetLeft(), 0U);
    EXPECT_EQ(range.GetRight(), 0x30U);

    // BB3
    range = rinfo->FindBoundsRange(&BB(2U), &INS(0U));
    EXPECT_EQ(range.GetLeft(), 0U);
    EXPECT_EQ(range.GetRight(), UINT16_MAX);

    range = rinfo->FindBoundsRange(&BB(2U), &INS(1U));
    EXPECT_EQ(range.GetLeft(), INT64_MIN);
    EXPECT_EQ(range.GetRight(), INT64_MAX);

    range = rinfo->FindBoundsRange(&BB(2U), &INS(2U));
    EXPECT_EQ(range.GetLeft(), 0U);
    EXPECT_EQ(range.GetRight(), UINT32_MAX);
}

TEST_F(BoundsAnalysisTest, NullCompare)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(3U, nullptr);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().CC(CC_NE).Inputs(3U, 0U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 4U).SrcVregs({2U, 3U});
            INST(8U, Opcode::NullCheck).ref().Inputs(0U, 7U);
            INST(9U, Opcode::LoadObject).s64().Inputs(8U).TypeId(243U);
            INST(10U, Opcode::Return).s64().Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(13U, Opcode::ReturnI).s64().Imm(0x14U);
        }
    }
    auto rinfo = GetGraph()->GetBoundsRangeInfo();
    EXPECT_EQ(rinfo->FindBoundsRange(&BB(2U), &INS(0U)).GetLeft(), 0U);
    EXPECT_EQ(rinfo->FindBoundsRange(&BB(2U), &INS(0U)).GetRight(), BoundsRange::GetMax(INS(0U).GetType()));

    EXPECT_NE(rinfo->FindBoundsRange(&BB(3U), &INS(0U)).GetLeft(), 0U);
    EXPECT_EQ(rinfo->FindBoundsRange(&BB(3U), &INS(0U)).GetRight(), BoundsRange::GetMax(INS(0U).GetType()));

    EXPECT_EQ(rinfo->FindBoundsRange(&BB(4U), &INS(0U)).GetLeft(), 0U);
    EXPECT_EQ(rinfo->FindBoundsRange(&BB(4U), &INS(0U)).GetRight(), 0U);
}

TEST_F(BoundsAnalysisTest, InitMoreThenTest)
{
    // For (int i = 10, i < 0, i++) {}
    // this loop is counable, but init > test value.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 0U);
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);

            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
    auto rinfo = GetGraph()->GetBoundsRangeInfo();
    auto range = rinfo->FindBoundsRange(&BB(3U), &INS(4U));
    EXPECT_EQ(range.GetLeft(), INT32_MIN);
    EXPECT_EQ(range.GetRight(), INT32_MAX);
}

TEST_F(BoundsAnalysisTest, ModTest)
{
    auto res = BoundsRange(0U, 10U).Mod(BoundsRange(0U, 2U));
    EXPECT_EQ(res.GetLeft(), 0U);
    EXPECT_EQ(res.GetRight(), 1U);

    res = BoundsRange(0U, 10U).Mod(BoundsRange(0U));
    EXPECT_EQ(res.GetLeft(), INT64_MIN);
    EXPECT_EQ(res.GetRight(), INT64_MAX);

    // It's correct situation, when right value is 0, and left value isn't 0.
    res = BoundsRange(-179L, -179L).Mod(BoundsRange(INT64_MIN, 0U));
    EXPECT_EQ(res.GetLeft(), -179L);
    EXPECT_EQ(res.GetRight(), 0U);

    res = BoundsRange(5U, 10U).Mod(BoundsRange(-3L));
    EXPECT_EQ(res.GetLeft(), 0U);
    EXPECT_EQ(res.GetRight(), 2U);

    res = BoundsRange(-10L, -5L).Mod(BoundsRange(-3L));
    EXPECT_EQ(res.GetLeft(), -2L);
    EXPECT_EQ(res.GetRight(), 0U);

    res = BoundsRange(-10L, 10U).Mod(BoundsRange(3U));
    EXPECT_EQ(res.GetLeft(), -2L);
    EXPECT_EQ(res.GetRight(), 2U);

    res = BoundsRange(-3L, 3U).Mod(BoundsRange(-10L, 10U));
    EXPECT_EQ(res.GetLeft(), -3L);
    EXPECT_EQ(res.GetRight(), 3U);
}

TEST_F(BoundsAnalysisTest, LoopWithBigStep)
{
    // For (int i = 0, i < 5, i += 10) {}
    // this loop is countable, and init + step > test value.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 5U);
        CONSTANT(2U, 10U);
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);

            INST(10U, Opcode::Add).s32().Inputs(4U, 2U);
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 1U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
    auto rinfo = GetGraph()->GetBoundsRangeInfo();
    auto range = rinfo->FindBoundsRange(&BB(3U), &INS(4U));
    EXPECT_EQ(range.GetLeft(), 0U);
    EXPECT_EQ(range.GetRight(), 4U);
}

TEST_F(BoundsAnalysisTest, LoopWithBigStep2)
{
    // For (int i = 1, i < 6, i += 2) {}
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 6U);
        CONSTANT(2U, 2U);
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);

            INST(10U, Opcode::Add).s32().Inputs(4U, 2U);
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 1U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
    auto rinfo = GetGraph()->GetBoundsRangeInfo();
    auto range = rinfo->FindBoundsRange(&BB(3U), &INS(4U));
    EXPECT_EQ(range.GetLeft(), 0U);
    // Check that right bound is (upper - 1)
    EXPECT_EQ(range.GetRight(), 5U);
}

TEST_F(BoundsAnalysisTest, ShrTest)
{
    auto r1 = BoundsRange(2U);
    auto max = BoundsRange();
    auto r2 = BoundsRange(4U, 8U);
    auto neg = BoundsRange(-1L);

    EXPECT_EQ(r2.Shr(r2), max);
    EXPECT_EQ(r2.Shr(neg), max);
    EXPECT_EQ(r2.Shr(r1), BoundsRange(1U, 2U));

    EXPECT_EQ(BoundsRange(-1L).Shr(BoundsRange(4U)), BoundsRange(0x0FFFFFFFFFFFFFFFU));
    EXPECT_EQ(BoundsRange(-1L, 16U).Shr(BoundsRange(4U)), BoundsRange(0U, 0x0FFFFFFFFFFFFFFFU));
    EXPECT_EQ(BoundsRange(9U, 15U).Shr(BoundsRange(2U)), BoundsRange(2U, 3U));
    EXPECT_EQ(BoundsRange(-15L, -9L).Shr(BoundsRange(2U)), BoundsRange(0x3FFFFFFFFFFFFFFCU, 0x3FFFFFFFFFFFFFFDU));
    EXPECT_EQ(BoundsRange(-100L, 100U).Shr(BoundsRange(4U)), BoundsRange(0U, 0x0FFFFFFFFFFFFFFFU));
}

TEST_F(BoundsAnalysisTest, AShrTest)
{
    auto r1 = BoundsRange(2U);
    auto max = BoundsRange();
    auto r2 = BoundsRange(4U, 8U);
    auto neg = BoundsRange(-1L);

    EXPECT_EQ(r2.AShr(r2), max);
    EXPECT_EQ(r2.AShr(neg), max);
    EXPECT_EQ(r2.AShr(r1), BoundsRange(1U, 2U));

    EXPECT_EQ(BoundsRange(-1L).AShr(BoundsRange(4U)), BoundsRange(-1L));
    EXPECT_EQ(BoundsRange(-1L, 16U).AShr(BoundsRange(4U)), BoundsRange(-1L, 1U));
}

TEST_F(BoundsAnalysisTest, ShlTest)
{
    auto max = BoundsRange();
    auto r = BoundsRange(4U, 8U);
    auto neg = BoundsRange(-1L);

    EXPECT_EQ(r.Shl(r), max);
    EXPECT_EQ(r.Shl(neg), max);

    EXPECT_EQ(r.Shl(BoundsRange(2U)), BoundsRange(16U, 32U));
    EXPECT_EQ(BoundsRange(-1L).Shl(BoundsRange(4U)), BoundsRange(0xFFFFFFFFFFFFFFF0U));
    EXPECT_EQ(BoundsRange(-1L, 16U).Shl(BoundsRange(4U)), BoundsRange(0xFFFFFFFFFFFFFFF0U, 256U));
    EXPECT_EQ(BoundsRange(16U, 0x0FFFFFFFFFFFFFFFU).Shl(BoundsRange(4U)), BoundsRange());
}

TEST_F(BoundsAnalysisTest, SignedDivTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 4U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Div).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());

    auto bb = &BB(2U);

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 4U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Div).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));

    auto bri = GetGraph()->GetBoundsRangeInfo();
    auto range = bri->FindBoundsRange(bb, bb->GetLastInst()->GetInput(0U).GetInst());
    ASSERT_EQ(range, BoundsRange(INT32_MIN / 4L, INT32_MAX / 4L));
}

TEST_F(BoundsAnalysisTest, AndTest)
{
    EXPECT_EQ(BoundsRange().And(BoundsRange(1U, 2U)), BoundsRange());
    EXPECT_EQ(BoundsRange().And(BoundsRange(0x2U)), BoundsRange(0U, 0x2U));
    EXPECT_EQ(BoundsRange().And(BoundsRange(0x3U)), BoundsRange(0U, 0x3U));
    EXPECT_EQ(BoundsRange().And(BoundsRange(-1L)), BoundsRange());
    EXPECT_EQ(BoundsRange().And(BoundsRange(0x8000000000000000U)), BoundsRange());
}

TEST_F(BoundsAnalysisTest, UINT64_INPUTS)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadObject).u64().Inputs(0U);
            INST(2U, Opcode::LoadObject).u64().Inputs(0U);
            INST(3U, Opcode::Add).i64().Inputs(1U, 2U);
            INST(4U, Opcode::Return).u64().Inputs(3U);
        }
    }
    auto bri = GetGraph()->GetBoundsRangeInfo();
    ASSERT_EQ(bri->FindBoundsRange(&BB(2U), &INS(1U)), BoundsRange(0U, INT64_MAX));
    ASSERT_EQ(bri->FindBoundsRange(&BB(2U), &INS(3U)), BoundsRange(INT64_MIN, INT64_MAX));
    ASSERT_EQ(bri->FindBoundsRange(&BB(2U), &INS(4U)), BoundsRange(0U, INT64_MAX));
}

TEST_F(BoundsAnalysisTest, Different_types)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Div).i64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).i64().Inputs(2U);
        }
    }
    auto bri = GetGraph()->GetBoundsRangeInfo();
    ASSERT_EQ(bri->FindBoundsRange(&BB(0U), &INS(0U)), BoundsRange(0U, INT64_MAX));
    ASSERT_EQ(bri->FindBoundsRange(&BB(2U), &INS(2U)), BoundsRange(INT64_MIN, INT64_MAX));
    ASSERT_EQ(bri->FindBoundsRange(&BB(2U), &INS(3U)), BoundsRange(INT64_MIN, INT64_MAX));
}

/*
 *              [0]
 *               |
 *               v
 *      /------>[2]------\
 *      |        |       |
 *      |       [3]      |
 *      |       / \      |
 *      |      v   v     v
 *      |     [4] [5]   [7]
 *      |       \ /      |
 *      \-------[6]    [exit]
 *
 */
TEST_F(BoundsAnalysisTest, LoopWithBranch)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).b();
        CONSTANT(1U, 0U);
        CONSTANT(15U, 1U);
        CONSTANT(2U, 2U);
        CONSTANT(25U, 10U);
        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(3U, Opcode::Phi).Inputs(1U, 9U).u64();   // index
            INST(35U, Opcode::Phi).Inputs(1U, 8U).u64();  // init
            INST(4U, Opcode::Compare).CC(CC_LT).b().Inputs(3U, 25U);
            INST(45U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(6U, Opcode::Add).Inputs(35U, 2U).u64();
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(7U, Opcode::Add).Inputs(35U, 15U).u64();
        }
        BASIC_BLOCK(6U, 2U)
        {
            INST(8U, Opcode::Phi).Inputs(6U, 7U).u64();  // update
            INST(9U, Opcode::Add).Inputs(3U, 15U).u64();
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(10U, Opcode::Return).Inputs(35U).u64();
        }
    }
    auto bri = GetGraph()->GetBoundsRangeInfo();
    ASSERT_EQ(bri->FindBoundsRange(&BB(2U), &INS(3U)), BoundsRange(0U, 9U));
    ASSERT_EQ(bri->FindBoundsRange(&BB(2U), &INS(35U)), BoundsRange(0U, 20U));
    ASSERT_EQ(bri->FindBoundsRange(&BB(6U), &INS(8U)), BoundsRange(10U, 20U));
}

TEST_F(BoundsAnalysisTest, NegativePhi)
{
    GRAPH(GetGraph())
    {
        CONSTANT(2U, 100U);
        CONSTANT(3U, 5U);
        CONSTANT(4U, 0U);
        CONSTANT(5U, 4U);
        CONSTANT(16U, 1U);
        BASIC_BLOCK(3U, 4U, 2U)
        {
            INST(21U, Opcode::Compare).CC(CC_GE).b().Inputs(4U, 3U).SrcType(DataType::INT32);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(21U);
        }
        BASIC_BLOCK(2U, 4U, 2U)
        {
            INST(9U, Opcode::Phi).Inputs(4U, 15U).i32();
            INST(10U, Opcode::Phi).Inputs(5U, 14U).i32();
            INST(14U, Opcode::Sub).Inputs(2U, 10U).i32();
            INST(15U, Opcode::Add).Inputs(9U, 16U).i32();
            INST(12U, Opcode::Compare).CC(CC_GE).b().Inputs(15U, 3U);
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(12U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::Return).Inputs(16U).i32();
        }
    }
    auto bri = GetGraph()->GetBoundsRangeInfo();
    ASSERT_EQ(bri->FindBoundsRange(&BB(2U), &INS(9U)), BoundsRange(0U, 4U));
    ASSERT_EQ(bri->FindBoundsRange(&BB(2U), &INS(10U)), BoundsRange(INT32_MIN, INT32_MAX));
    ASSERT_EQ(bri->FindBoundsRange(&BB(2U), &INS(14U)), BoundsRange(INT32_MIN, INT32_MAX));
    ASSERT_EQ(bri->FindBoundsRange(&BB(2U), &INS(15U)), BoundsRange(1U, 5U));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
