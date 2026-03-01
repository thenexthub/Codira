/*
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

#include "unit_test.h"
#include "optimizer/code_generator/spill_fill_encoder.h"

namespace ark::compiler {
class SpillFillEncoderTest : public GraphTest {};

bool operator==(const SpillFillData &left, const SpillFillData &right)
{
    return left.SrcType() == right.SrcType() && left.SrcValue() == right.SrcValue() &&
           left.DstType() == right.DstType() && left.DstValue() == right.DstValue() &&
           left.GetType() == right.GetType();
}
// NOLINTBEGIN(readability-magic-numbers)
TEST_F(SpillFillEncoderTest, SpillFillDataSorting)
{
    ArenaVector<SpillFillData> spillFills {
        {{LocationType::REGISTER, LocationType::STACK, 1U, 0U, DataType::Type::INT64},
         {LocationType::REGISTER, LocationType::STACK, 0U, 2U, DataType::Type::INT64},
         {LocationType::REGISTER, LocationType::REGISTER, 0U, 1U, DataType::Type::INT64},
         {LocationType::REGISTER, LocationType::REGISTER, 1U, 2U, DataType::Type::INT64},
         {LocationType::IMMEDIATE, LocationType::REGISTER, 0U, 0U, DataType::Type::INT64},
         {LocationType::IMMEDIATE, LocationType::REGISTER, 0U, 1U, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 0U, 0U, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 1U, 1U, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 2U, 2U, DataType::Type::INT64},
         {LocationType::REGISTER, LocationType::REGISTER, 2U, 4U, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 3U, 2U, DataType::Type::INT64},
         {LocationType::STACK, LocationType::STACK, 7U, 9U, DataType::Type::INT64},
         {LocationType::STACK, LocationType::STACK, 8U, 10U, DataType::Type::INT64}},
        GetAllocator()->Adapter()};

    // first two: reorder spills
    ArenaVector<SpillFillData> expectedOrder {
        {{LocationType::REGISTER, LocationType::STACK, 0U, 2U, DataType::Type::INT64},
         {LocationType::REGISTER, LocationType::STACK, 1U, 0U, DataType::Type::INT64},
         // skip move
         // CC-OFFNXT(G.FMT.02) project code style
         {LocationType::REGISTER, LocationType::REGISTER, 0U, 1U, DataType::Type::INT64},
         {LocationType::REGISTER, LocationType::REGISTER, 1U, 2U, DataType::Type::INT64},
         // skip imm move
         // CC-OFFNXT(G.FMT.02) project code style
         {LocationType::IMMEDIATE, LocationType::REGISTER, 0U, 0U, DataType::Type::INT64},
         {LocationType::IMMEDIATE, LocationType::REGISTER, 0U, 1U, DataType::Type::INT64},
         // reorder fills
         // CC-OFFNXT(G.FMT.02) project code style
         {LocationType::STACK, LocationType::REGISTER, 2U, 2U, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 1U, 1U, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 0U, 0U, DataType::Type::INT64},
         // skip move
         // CC-OFFNXT(G.FMT.02) project code style
         {LocationType::REGISTER, LocationType::REGISTER, 2U, 4U, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 3U, 2U, DataType::Type::INT64},
         // skip mem move
         // CC-OFFNXT(G.FMT.02) project code style
         {LocationType::STACK, LocationType::STACK, 7U, 9U, DataType::Type::INT64},
         {LocationType::STACK, LocationType::STACK, 8U, 10U, DataType::Type::INT64}},
        GetAllocator()->Adapter()};

    SpillFillEncoder::SortSpillFillData(&spillFills);
    for (size_t i = 0; i < expectedOrder.size(); i++) {
        EXPECT_EQ(spillFills[i], expectedOrder[i]) << "Mismatch at " << i;
    }
}

TEST_F(SpillFillEncoderTest, CanCombineSpillFills)
{
    auto graph = GetGraph();
    if (graph->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "Supported only on Aarch64";
    }

    EXPECT_TRUE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 2U, 0U, DataType::Type::INT64},
        {LocationType::STACK, LocationType::REGISTER, 1U, 0U, DataType::Type::INT64}, graph));

    EXPECT_TRUE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::REGISTER, LocationType::STACK, 0U, 2U, DataType::Type::INT64},
        {LocationType::REGISTER, LocationType::STACK, 0U, 1U, DataType::Type::INT64}, graph));

    EXPECT_TRUE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 2U, 0U, DataType::Type::INT32},
        {LocationType::STACK, LocationType::REGISTER, 1U, 0U, DataType::Type::INT64}, graph));

    EXPECT_TRUE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 2U, 0U, DataType::Type::INT32},
        {LocationType::STACK, LocationType::REGISTER, 1U, 0U, DataType::Type::INT8}, graph));

    // different type of moves
    EXPECT_FALSE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::REGISTER, LocationType::STACK, 2U, 0U, DataType::Type::INT32},
        {LocationType::STACK, LocationType::REGISTER, 1U, 0U, DataType::Type::INT32}, graph));

    // illegal slots order
    EXPECT_FALSE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 1U, 0U, DataType::Type::INT32},
        {LocationType::STACK, LocationType::REGISTER, 2U, 0U, DataType::Type::INT32}, graph));

    EXPECT_FALSE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 4U, 0U, DataType::Type::INT32},
        {LocationType::STACK, LocationType::REGISTER, 0U, 0U, DataType::Type::INT32}, graph));

    // unaligned access
    EXPECT_TRUE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 1U, 0U, DataType::Type::INT32},
        {LocationType::STACK, LocationType::REGISTER, 0U, 0U, DataType::Type::INT32}, graph));

    // float 32 are unsupported
    EXPECT_FALSE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 2U, 0U, DataType::Type::FLOAT32},
        {LocationType::STACK, LocationType::REGISTER, 1U, 0U, DataType::Type::FLOAT32}, graph));

    // float 64 are supported
    EXPECT_TRUE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 2U, 0U, DataType::Type::FLOAT64},
        {LocationType::STACK, LocationType::REGISTER, 1U, 0U, DataType::Type::FLOAT64}, graph));

    // different types
    EXPECT_FALSE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 2U, 0U, DataType::Type::INT64},
        {LocationType::STACK, LocationType::REGISTER, 1U, 0U, DataType::Type::FLOAT64}, graph));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
