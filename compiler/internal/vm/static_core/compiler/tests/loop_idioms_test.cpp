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
#include "optimizer/optimizations/loop_idioms.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/graph_cloner.h"

namespace ark::compiler {
class LoopIdiomsTest : public GraphTest {
protected:
    Graph *CheckFillArrayFullInitial(DataType::Type arrayType)
    {
        auto initial = CreateEmptyGraph();
        GRAPH(initial)
        {
            // NOLINTBEGIN(readability-magic-numbers)
            PARAMETER(0U, 0U).ref();
            PARAMETER(1U, 1U).type(arrayType);
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
                INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
                INST(6U, Opcode::LenArray).i32().Inputs(5U);
                INST(7U, Opcode::Compare).b().Inputs(2U, 6U).CC(CC_LT).SrcType(DataType::INT32);
                INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
            }

            BASIC_BLOCK(3U, 4U, 3U)
            {
                INST(9U, Opcode::Phi).i32().Inputs(2U, 11U);
                INST(10U, Opcode::StoreArray).type(arrayType).Inputs(5U, 9U, 1U);
                INST(11U, Opcode::Add).i32().Inputs(9U, 3U);
                INST(12U, Opcode::Compare).b().Inputs(6U, 11U).CC(CC_LE).SrcType(DataType::INT32);
                INST(13U, Opcode::IfImm).Inputs(12U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
            }

            BASIC_BLOCK(4U, -1L)
            {
                INST(14U, Opcode::ReturnVoid).v0id();
            }
            // NOLINTEND(readability-magic-numbers)
        }
        return initial;
    }

    void BuildExpectedFillArrayFull(Graph *expected, DataType::Type arrayType,
                                    RuntimeInterface::IntrinsicId expectedIntrinsic)
    {
        GRAPH(expected)
        {
            // NOLINTBEGIN(readability-magic-numbers)
            PARAMETER(0U, 0U).ref();
            PARAMETER(1U, 1U).type(arrayType);
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);
            CONSTANT(20U, 6U);  // LoopIdioms::ITERATIONS_THRESHOLD

            BASIC_BLOCK(2U, 5U, 4U)
            {
                INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
                INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
                INST(6U, Opcode::LenArray).i32().Inputs(5U);
                INST(7U, Opcode::Compare).b().Inputs(2U, 6U).CC(CC_LT).SrcType(DataType::INT32);
                INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
            }

            BASIC_BLOCK(5U, 3U, 6U)
            {
                INST(15U, Opcode::Sub).i32().Inputs(6U, 2U);
                INST(16U, Opcode::Compare).b().Inputs(15U, 20U).SrcType(DataType::INT32).CC(CC_LE);
                INST(17U, Opcode::IfImm).Inputs(16U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
            }

            BASIC_BLOCK(3U, 4U, 3U)
            {
                INST(9U, Opcode::Phi).i32().Inputs(2U, 11U);
                INST(10U, Opcode::StoreArray).type(arrayType).Inputs(5U, 9U, 1U);
                INST(11U, Opcode::Add).i32().Inputs(9U, 3U);
                INST(12U, Opcode::Compare).b().Inputs(6U, 11U).CC(CC_LE).SrcType(DataType::INT32);
                INST(13U, Opcode::IfImm).Inputs(12U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
            }

            BASIC_BLOCK(6U, 4U)
            {
                INST(18U, Opcode::Intrinsic)
                    .v0id()
                    .Inputs({{DataType::REFERENCE, 5U}, {arrayType, 1U}, {DataType::INT32, 2U}, {DataType::INT32, 6U}})
                    .IntrinsicId(expectedIntrinsic)
                    .SetFlag(compiler::inst_flags::NO_HOIST)
                    .SetFlag(compiler::inst_flags::NO_DCE)
                    .SetFlag(compiler::inst_flags::NO_CSE)
                    .SetFlag(compiler::inst_flags::BARRIER)
                    .ClearFlag(compiler::inst_flags::REQUIRE_STATE)
                    .ClearFlag(compiler::inst_flags::CAN_THROW)
                    .ClearFlag(compiler::inst_flags::RUNTIME_CALL);
            }

            BASIC_BLOCK(4U, -1L)
            {
                INST(14U, Opcode::ReturnVoid).v0id();
            }
            // NOLINTEND(readability-magic-numbers)
        }
    }

    bool CheckFillArrayFull(DataType::Type arrayType, RuntimeInterface::IntrinsicId expectedIntrinsic)
    {
        auto initial = CheckFillArrayFullInitial(arrayType);
        if (!initial->RunPass<LoopIdioms>()) {
            return false;
        }
        initial->RunPass<Cleanup>();

        auto expected = CreateEmptyGraph();
        BuildExpectedFillArrayFull(expected, arrayType, expectedIntrinsic);
        return GraphComparator().Compare(initial, expected);
    }

    void FillLargeArrayWithConstantIterationsCount();
    Graph *FillLargeArrayWithConstantIterationsCountExpected();
};

TEST_F(LoopIdiomsTest, FillArray)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    EXPECT_TRUE(CheckFillArrayFull(DataType::BOOL, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_8));
    EXPECT_TRUE(CheckFillArrayFull(DataType::UINT8, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_8));
    EXPECT_TRUE(CheckFillArrayFull(DataType::INT8, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_8));
    EXPECT_TRUE(CheckFillArrayFull(DataType::INT16, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_16));
    EXPECT_TRUE(CheckFillArrayFull(DataType::UINT16, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_16));
    EXPECT_TRUE(CheckFillArrayFull(DataType::INT32, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_32));
    EXPECT_TRUE(CheckFillArrayFull(DataType::UINT32, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_32));
    EXPECT_TRUE(CheckFillArrayFull(DataType::INT64, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_64));
    EXPECT_TRUE(CheckFillArrayFull(DataType::UINT64, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_64));
    EXPECT_TRUE(CheckFillArrayFull(DataType::FLOAT32, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_F32));
    EXPECT_TRUE(CheckFillArrayFull(DataType::FLOAT64, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_F64));

    EXPECT_FALSE(CheckFillArrayFull(DataType::REFERENCE, RuntimeInterface::IntrinsicId::COUNT));
    EXPECT_FALSE(CheckFillArrayFull(DataType::POINTER, RuntimeInterface::IntrinsicId::COUNT));
}

TEST_F(LoopIdiomsTest, IncorrectStep)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 10U);  // incorrect step

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LenArray).i32().Inputs(5U);
            INST(7U, Opcode::Compare).b().Inputs(2U, 6U).CC(CC_LT).SrcType(DataType::INT32);
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(9U, Opcode::Phi).i32().Inputs(2U, 11U);
            INST(10U, Opcode::StoreArray).i32().Inputs(5U, 9U, 1U);
            INST(11U, Opcode::Add).i32().Inputs(9U, 3U);
            INST(12U, Opcode::Compare).b().Inputs(6U, 11U).CC(CC_LE).SrcType(DataType::INT32);
            INST(13U, Opcode::IfImm).Inputs(12U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(14U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
}

TEST_F(LoopIdiomsTest, OtherInstructionsWithinLoop)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LenArray).i32().Inputs(5U);
            INST(7U, Opcode::Compare).b().Inputs(2U, 6U).CC(CC_LT).SrcType(DataType::INT32);
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(9U, Opcode::Phi).i32().Inputs(2U, 11U);
            INST(10U, Opcode::StoreArray).i32().Inputs(5U, 9U, 1U);
            // it should prevent optimization
            INST(15U, Opcode::LoadArray).i32().Inputs(5U, 9U);
            INST(11U, Opcode::Add).i32().Inputs(9U, 3U);
            INST(12U, Opcode::Compare).b().Inputs(6U, 11U).CC(CC_LE).SrcType(DataType::INT32);
            INST(13U, Opcode::IfImm).Inputs(12U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(14U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
}

TEST_F(LoopIdiomsTest, UseLoopInstructionsOutstideOfLoop)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LenArray).i32().Inputs(5U);
            INST(7U, Opcode::Compare).b().Inputs(2U, 6U).CC(CC_LT).SrcType(DataType::INT32);
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(9U, Opcode::Phi).i32().Inputs(2U, 11U);
            INST(10U, Opcode::StoreArray).i32().Inputs(5U, 9U, 1U);
            INST(11U, Opcode::Add).i32().Inputs(9U, 3U);
            INST(12U, Opcode::Compare).b().Inputs(6U, 11U).CC(CC_LE).SrcType(DataType::INT32);
            INST(13U, Opcode::IfImm).Inputs(12U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(14U, Opcode::Phi).i32().Inputs(2U, 9U);
            INST(15U, Opcode::Return).i32().Inputs(14U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
}

TEST_F(LoopIdiomsTest, FillTinyArray)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        CONSTANT(15U, 3U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LenArray).i32().Inputs(5U);
            INST(7U, Opcode::Compare).b().Inputs(15U, 6U).CC(CC_LT).SrcType(DataType::INT32);
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(9U, Opcode::Phi).i32().Inputs(2U, 11U);
            INST(10U, Opcode::StoreArray).i32().Inputs(5U, 9U, 1U);
            INST(11U, Opcode::Add).i32().Inputs(9U, 3U);
            INST(12U, Opcode::Compare).b().Inputs(15U, 11U).CC(CC_LE).SrcType(DataType::INT32);
            INST(13U, Opcode::IfImm).Inputs(12U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(14U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
}

void LoopIdiomsTest::FillLargeArrayWithConstantIterationsCount()
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        CONSTANT(15U, 42U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LenArray).i32().Inputs(5U);
            INST(7U, Opcode::Compare).b().Inputs(15U, 6U).CC(CC_LT).SrcType(DataType::INT32);
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(9U, Opcode::Phi).i32().Inputs(2U, 11U);
            INST(10U, Opcode::StoreArray).i32().Inputs(5U, 9U, 1U);
            INST(11U, Opcode::Add).i32().Inputs(9U, 3U);
            INST(12U, Opcode::Compare).b().Inputs(15U, 11U).CC(CC_LE).SrcType(DataType::INT32);
            INST(13U, Opcode::IfImm).Inputs(12U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(14U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

Graph *LoopIdiomsTest::FillLargeArrayWithConstantIterationsCountExpected()
{
    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U);
        CONSTANT(15U, 42U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LenArray).i32().Inputs(5U);
            INST(7U, Opcode::Compare).b().Inputs(15U, 6U).CC(CC_LT).SrcType(DataType::INT32);
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U)
        {
            std::initializer_list<std::pair<int, int>> inputs = {
                {DataType::REFERENCE, 5U}, {DataType::INT32, 1U}, {DataType::INT32, 2U}, {DataType::INT32, 15U}};
            INST(17U, Opcode::Intrinsic)
                .v0id()
                .Inputs(inputs)
                .IntrinsicId(RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_32)
                .SetFlag(compiler::inst_flags::NO_HOIST)
                .SetFlag(compiler::inst_flags::NO_DCE)
                .SetFlag(compiler::inst_flags::NO_CSE)
                .SetFlag(compiler::inst_flags::BARRIER)
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE)
                .ClearFlag(compiler::inst_flags::CAN_THROW)
                .ClearFlag(compiler::inst_flags::RUNTIME_CALL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(14U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    return expected;
}

// NOLINTEND(readability-magic-numbers)

TEST_F(LoopIdiomsTest, FillLargeArrayWithConstantIterationsCount)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }
    FillLargeArrayWithConstantIterationsCount();
    ASSERT_TRUE(GetGraph()->RunPass<LoopIdioms>());
    GetGraph()->RunPass<Cleanup>();

    auto expected = FillLargeArrayWithConstantIterationsCountExpected();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected));
}

TEST_F(LoopIdiomsTest, MultipleAdds)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LenArray).i32().Inputs(5U);
            INST(7U, Opcode::Compare).b().Inputs(2U, 6U).CC(CC_LT).SrcType(DataType::INT32);
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(9U, Opcode::Phi).i32().Inputs(2U, 12U);
            INST(10U, Opcode::StoreArray).i32().Inputs(5U, 9U, 1U);
            INST(11U, Opcode::Add).i32().Inputs(9U, 3U);
            INST(12U, Opcode::Add).i32().Inputs(11U, 3U);
            INST(13U, Opcode::Compare).b().Inputs(6U, 12U).CC(CC_LE).SrcType(DataType::INT32);
            INST(14U, Opcode::IfImm).Inputs(13U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
}

TEST_F(LoopIdiomsTest, PreIncrementIndex)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LenArray).i32().Inputs(5U);
            INST(7U, Opcode::Compare).b().Inputs(2U, 6U).CC(CC_LT).SrcType(DataType::INT32);
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(9U, Opcode::Phi).i32().Inputs(2U, 10U);
            INST(10U, Opcode::Add).i32().Inputs(9U, 3U);
            INST(11U, Opcode::StoreArray).i32().Inputs(5U, 10U, 1U);
            INST(12U, Opcode::Compare).b().Inputs(6U, 10U).CC(CC_LE).SrcType(DataType::INT32);
            INST(13U, Opcode::IfImm).Inputs(12U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(14U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
}

TEST_F(LoopIdiomsTest, MismatchingConditions)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LenArray).i32().Inputs(5U);
            // This condition don't make any sensce, but it is so intentionally.
            INST(7U, Opcode::Compare).b().Inputs(3U, 2U).CC(CC_LT).SrcType(DataType::INT32);
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(9U, Opcode::Phi).i32().Inputs(2U, 11U);
            INST(10U, Opcode::StoreArray).i32().Inputs(5U, 9U, 1U);
            INST(11U, Opcode::Add).i32().Inputs(9U, 3U);
            INST(12U, Opcode::Compare).b().Inputs(6U, 11U).CC(CC_LE).SrcType(DataType::INT32);
            INST(13U, Opcode::IfImm).Inputs(12U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(14U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    // Different condition before and within the loop should not prevent idiom recognition.
    ASSERT_TRUE(GetGraph()->RunPass<LoopIdioms>());
}

}  // namespace ark::compiler
