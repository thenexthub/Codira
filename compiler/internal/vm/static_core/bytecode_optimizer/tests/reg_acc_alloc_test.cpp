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

#include "common.h"
#include "bytecode_optimizer/reg_acc_alloc.h"
#include <gtest/gtest.h>
#include "bytecode_optimizer/optimize_bytecode.h"

namespace ark::bytecodeopt::test {

// NOLINTBEGIN(readability-magic-numbers)

class RegAccAllocTest : public CommonTest {
public:
    void CheckInstructionsDestRegIsAcc(std::vector<int> &&instIds)
    {
        for (auto id : instIds) {
            ASSERT_EQ(INS(id).GetDstReg(), compiler::ACC_REG_ID);
        }
    }

    void CheckInstructionsSrcRegIsAcc(std::vector<int> &&instIds)
    {
        for (auto id : instIds) {
            uint8_t idx = 0;
            switch (INS(id).GetOpcode()) {
                case compiler::Opcode::LoadArray:
                case compiler::Opcode::StoreObject:
                case compiler::Opcode::StoreStatic:
                    idx = 1;
                    break;
                case compiler::Opcode::StoreArray:
                    idx = 2U;
                    break;
                default:
                    break;
            }

            ASSERT_EQ(INS(id).GetSrcReg(idx), compiler::ACC_REG_ID);
        }
    }
};

/*
 * Test if two arithmetic instructions follow each other.
 */
TEST_F(RegAccAllocTest, ArithmeticInstructions)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32();
        CONSTANT(1U, 10U).s32();
        CONSTANT(2U, 20U).s32();

        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::Mul).s32().Inputs(0U, 2U);
            INST(4U, Opcode::Add).s32().Inputs(3U, 1U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({3U, 4U});
    CheckInstructionsSrcRegIsAcc({4U, 5U});
}

/*
 * Test if two arithmetic instructions follow each other.
 * Take the advantage of commutativity.
 */
TEST_F(RegAccAllocTest, Commutativity1)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32();
        CONSTANT(1U, 10U).s32();
        CONSTANT(2U, 20U).s32();

        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::Mul).s32().Inputs(0U, 2U);
            INST(4U, Opcode::Add).s32().Inputs(1U, 3U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({3U, 4U});
    CheckInstructionsSrcRegIsAcc({4U, 5U});
}

/*
 * Test if two arithmetic instructions follow each other.
 * Cannot take the advantage of commutativity at the Sub instruction.
 */
TEST_F(RegAccAllocTest, Commutativity2)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32();
        CONSTANT(1U, 10U).s32();
        CONSTANT(2U, 20U).s32();

        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::Mul).s32().Inputs(0U, 2U);
            INST(4U, Opcode::Sub).s32().Inputs(1U, 3U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({4U});
    CheckInstructionsSrcRegIsAcc({5U});
}

/*
 * Test if the first arithmetic instruction has multiple users.
 * That (3, Opcode::Mul) is spilled out to register becasue the subsequent
 * instruction (4, Opcode::Add) makes the accumulator dirty.
 */
TEST_F(RegAccAllocTest, ArithmeticInstructionsWithDirtyAccumulator)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32();
        CONSTANT(1U, 10U).s32();
        CONSTANT(2U, 20U).s32();

        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::Mul).s32().Inputs(0U, 2U);
            INST(4U, Opcode::Add).s32().Inputs(1U, 3U);
            INST(5U, Opcode::Sub).s32().Inputs(3U, 4U);
            INST(6U, Opcode::Mul).s32().Inputs(5U, 3U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({5U, 6U});
    CheckInstructionsSrcRegIsAcc({6U, 7U});
}

/*
 * Test if arithmetic instructions are used as Phi inputs.
 *
 * Test Graph:
 *              [0]
 *               |
 *               v
 *         /----[2]----\
 *         |           |
 *         v           v
 *        [3]         [4]
 *         |           |
 *         \--->[4]<---/
 *               |
 *               v
 *             [exit]
 */
TEST_F(RegAccAllocTest, SimplePhi)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32();
        CONSTANT(1U, 10U).s32();
        CONSTANT(2U, 20U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(5U, Opcode::Compare).b().Inputs(4U, 0U);
            INST(6U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(5U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(8U, Opcode::Sub).s32().Inputs(4U, 0U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(9U, Opcode::Add).s32().Inputs(4U, 0U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::Phi).s32().Inputs({{3U, 8U}, {4U, 9U}});
            INST(11U, Opcode::Add).s32().Inputs(4U, 10U);
            INST(7U, Opcode::Return).s32().Inputs(11U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({5U, 8U, 9U, 11U});
    CheckInstructionsSrcRegIsAcc({6U, 7U, 11U});
}

// Case from GitCode issue 8073
TEST_F(RegAccAllocTest, PhiWithTwoInputsAsConstInAcc)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, 3U)
        {
            CONSTANT(8U, 0U).s32();
            CONSTANT(9U, 1U).s32();
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(5U, Opcode::Phi).s32().Inputs({{2U, 9U}, {3U, 8U}});
            INST(6U, Opcode::IfImm).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Imm(0U).Inputs(5U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::ReturnVoid).v0id();
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    ASSERT_EQ(INS(5U).GetDstReg(), compiler::GetInvalidReg());
    ASSERT_EQ(INS(8U).GetDstReg(), compiler::GetInvalidReg());
    ASSERT_EQ(INS(9U).GetDstReg(), compiler::GetInvalidReg());
    ASSERT_EQ(INS(6U).GetSrcReg(0), compiler::GetInvalidReg());
}

/*
 * Test Cast instructions which follow each other.
 * Each instruction refers to the previos one.
 */
TEST_F(RegAccAllocTest, CastInstructions)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f64().SrcType(compiler::DataType::UINT32).Inputs(0U);
            INST(2U, Opcode::Cast).s32().SrcType(compiler::DataType::FLOAT64).Inputs(1U);
            INST(3U, Opcode::Cast).s16().SrcType(compiler::DataType::INT32).Inputs(2U);
            INST(4U, Opcode::Return).s16().Inputs(3U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({1U, 2U, 3U});
    CheckInstructionsSrcRegIsAcc({2U, 3U, 4U});
}

/*
 * Test Abs and Sqrt instructions.
 * Each instruction refers to the previous instruction.
 */
TEST_F(RegAccAllocTest, AbsAndSqrtInstructions)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1.22_D).f64();
        PARAMETER(1U, 10U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Abs).f64().Inputs(2U);
            INST(4U, Opcode::Sqrt).f64().Inputs(3U);
            INST(5U, Opcode::Return).f64().Inputs(4U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({4U});
    CheckInstructionsSrcRegIsAcc({5U});
}

/*
 * Test LoadArray instruction that reads accumulator as second input.
 * Note: most instructions read accumulator as first input.
 */
TEST_F(RegAccAllocTest, LoadArrayInstruction)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).s32();
        PARAMETER(1U, 10U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 0U, 2U);
            INST(6U, Opcode::LoadArray).s32().Inputs(3U, 5U);
            INST(7U, Opcode::Cast).f64().SrcType(compiler::DataType::INT32).Inputs(6U);
            INST(8U, Opcode::Return).f64().Inputs(7U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({4U, 6U, 7U});
    CheckInstructionsSrcRegIsAcc({5U, 7U, 8U});
}

/*
 * Test throw instruction.
 * Currently, just linear block-flow is supported.
 * Nothing happens in this test.
 */
TEST_F(RegAccAllocTest, ThrowInstruction)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32();
        CONSTANT(1U, 0U).s32();

        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
        }
        BASIC_BLOCK(3U, 4U, 5U, 6U)
        {
            INST(5U, Opcode::Try).CatchTypeIds({0x0U, 0xE1U});
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(7U, Opcode::Throw).Inputs(4U, 6U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::Return).b().Inputs(1U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(9U, Opcode::Return).b().Inputs(0U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
}

/*
 * Test Phi instruction in loop.
 * This test is copied from reg_alloc_linear_scan_test.cpp file.
 */
TEST_F(RegAccAllocTest, PhiInstructionInLoop)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32();
        CONSTANT(1U, 10U).s32();
        CONSTANT(2U, 20U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).s32().Inputs({{0U, 0U}, {3U, 8U}});
            INST(4U, Opcode::Phi).s32().Inputs({{0U, 1U}, {3U, 9U}});
            INST(5U, Opcode::SafePoint).Inputs(0U, 3U, 4U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::Compare).b().Inputs(4U, 0U);
            INST(7U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(6U);
        }

        BASIC_BLOCK(3U, 2U)
        {
            INST(8U, Opcode::Mul).s32().Inputs(3U, 4U);
            INST(9U, Opcode::Sub).s32().Inputs(4U, 0U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Add).s32().Inputs(2U, 3U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({6U, 10U});
    CheckInstructionsSrcRegIsAcc({7U, 11U});
}

/*
 * Test multiple branches.
 * Test Graph:
 *            /---[2]---\
 *            |         |
 *            v         v
 *           [3]<------[4]
 *                      |
 *                      v
 *                     [5]
 */
TEST_F(RegAccAllocTest, MultipleBranches)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u16();
        PARAMETER(1U, 1U).s64();
        CONSTANT(2U, 0x20U).s64();
        CONSTANT(3U, 0x25U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Compare).b().CC(compiler::CC_GT).Inputs(0U, 2U);
            INST(6U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(5U);
        }
        // v0 <= 0x20
        BASIC_BLOCK(4U, 3U, 5U)
        {
            INST(8U, Opcode::Compare).b().CC(compiler::CC_GT).Inputs(1U, 3U);
            INST(9U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(8U);
        }
        // v0 <= 0x20 && v1 <= 0x25
        BASIC_BLOCK(5U, -1L)
        {
            INST(11U, Opcode::Return).u16().Inputs(0U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(14U, Opcode::Return).u16().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({5U, 8U});
    CheckInstructionsSrcRegIsAcc({6U, 9U});
}

/*
 * Test Phi with multiple inputs.
 * Phi cannot be optimized because one of the inputs is a CONSTANT.
 * Test Graph:
 *          /---[2]---\
 *          |         |
 *          v         |
 *      /--[3]--\     |
 *      |       |     |
 *      v       v     |
 *     [4]     [5]    |
 *      |       |     |
 *      |       v     |
 *      \----->[6]<---/
 *              |
 *            [exit]
 */
TEST_F(RegAccAllocTest, PhiWithMultipleInputs)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(2U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Mul).s32().Inputs(0U, 0U);
            INST(5U, Opcode::Compare).b().Inputs(4U, 1U);
            INST(6U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(7U, Opcode::Mul).s32().Inputs(4U, 1U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(8U, Opcode::Add).s32().Inputs(4U, 1U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(9U, Opcode::Phi).s32().Inputs({{2U, 0U}, {4U, 7U}, {5U, 8U}});
            INST(10U, Opcode::Return).s32().Inputs(9U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({2U, 5U});
    CheckInstructionsSrcRegIsAcc({3U, 6U});
}

/*
 * Test for Phi. Phi cannot be optimized because the subsequent
 * Compare instruction makes the accumulator dirty.
 */
TEST_F(RegAccAllocTest, PhiWithSubsequentCompareInstruction)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32();
        CONSTANT(1U, 10U).s32();
        CONSTANT(2U, 20U).s32();

        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs({{2U, 3U}, {4U, 8U}});
            INST(5U, Opcode::SafePoint).Inputs(0U, 4U).SrcVregs({0U, 1U});
            INST(6U, Opcode::Compare).b().Inputs(4U, 0U);
            INST(7U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(6U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(8U, Opcode::Mul).s32().Inputs(3U, 2U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::Add).s32().Inputs(2U, 4U);
            INST(10U, Opcode::Return).s32().Inputs(9U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({6U, 9U});
    CheckInstructionsSrcRegIsAcc({7U, 10U});
}

/*
 * A switch-case example. There are different arithmetic instructions
 * in the case blocks. These instructions are the inputs of a Phi.
 */
// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(RegAccAllocTest, SwitchCase)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0x1U).s32();
        CONSTANT(5U, 0xaU).s32();
        CONSTANT(8U, 0x2U).s32();
        CONSTANT(10U, 0x3U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::LoadArray).ref().Inputs(0U, 1U);
            INST(3U, Opcode::SaveState).Inputs(2U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(4U, Opcode::CallStatic)
                .s32()
                .Inputs({{compiler::DataType::REFERENCE, 2U}, {compiler::DataType::NO_TYPE, 3U}});
            INST(7U, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(1U, 4U);
        }

        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(9U, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(8U, 4U);
        }

        BASIC_BLOCK(6U, 7U, 9U)
        {
            INST(11U, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(10U, 4U);
        }

        BASIC_BLOCK(9U, 10U)
        {
            INST(12U, Opcode::Mod).s32().Inputs(5U, 4U);
        }

        BASIC_BLOCK(7U, 10U)
        {
            INST(13U, Opcode::Mul).s32().Inputs(4U, 5U);
        }

        BASIC_BLOCK(5U, 10U)
        {
            INST(14U, Opcode::Add).s32().Inputs(4U, 5U);
        }

        BASIC_BLOCK(3U, 10U)
        {
            INST(15U, Opcode::Sub).s32().Inputs(5U, 4U);
        }

        BASIC_BLOCK(10U, -1L)
        {
            INST(16U, Opcode::Phi).s32().Inputs({{3U, 15U}, {5U, 14U}, {7U, 13U}, {9U, 12U}});
            INST(17U, Opcode::Return).s32().Inputs(16U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({12U, 13U, 14U, 15U, 16U});
    CheckInstructionsSrcRegIsAcc({17U});
}

/*
 * This test creates an array and does modifications in that.
 */
TEST_F(RegAccAllocTest, CreateArray)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0x2U).s32();
        CONSTANT(4U, 0x1U).s32();
        CONSTANT(10U, 0x0U).s32();
        CONSTANT(11U, 0xaU).s32();
        CONSTANT(20U, 0xcU).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(1U, 0U).SrcVregs({0U, 6U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(2U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 1U, 2U);
            INST(5U, Opcode::LoadArray).ref().Inputs(0U, 4U);
            INST(6U, Opcode::SaveState).Inputs(5U, 4U, 3U, 0U).SrcVregs({0U, 1U, 4U, 6U});
            INST(7U, Opcode::CallStatic)
                .s32()
                .Inputs({{compiler::DataType::REFERENCE, 5U}, {compiler::DataType::NO_TYPE, 6U}});
            INST(9U, Opcode::Add).s32().Inputs(7U, 1U);
            INST(12U, Opcode::StoreArray).s32().Inputs(5U, 10U, 11U);
            INST(14U, Opcode::Add).s32().Inputs(7U, 20U);
            INST(15U, Opcode::StoreArray).s32().Inputs(3U, 4U, 14U);
            INST(17U, Opcode::Mul).s32().Inputs(14U, 9U);
            INST(18U, Opcode::Return).s32().Inputs(17U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({17U});
    CheckInstructionsSrcRegIsAcc({9U, 15U, 18U});
}

/*
 * Test StoreObject instruction that reads accumulator as second input.
 * Note: most instructions read accumulator as first input.
 */
TEST_F(RegAccAllocTest, StoreObjectInstruction)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, nullptr).ref();
        CONSTANT(1U, 0x2aU).s64();
        CONSTANT(2U, 0x1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 2U).SrcVregs({0U, 1U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::Add).s64().Inputs(1U, 2U);
            INST(6U, Opcode::StoreObject).s64().Inputs(4U, 5U);
            INST(7U, Opcode::Return).s64().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({5U});
    CheckInstructionsSrcRegIsAcc({6U});
}

/*
 * Test StoreStatic instruction that reads accumulator as second input.
 * Note: most instructions read accumulator as first input.
 */
TEST_F(RegAccAllocTest, StoreStaticInstruction)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(5U, Opcode::StoreStatic).s32().Inputs(3U, 4U).Volatile();
            INST(6U, Opcode::ReturnVoid).v0id();
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({4U});
    CheckInstructionsSrcRegIsAcc({5U});
}

/*
 * Test if Phi uses Phi as input.
 * This case is not supported right now.
 */
// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(RegAccAllocTest, PhiUsesPhiAsInput)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0x1U).s32();
        CONSTANT(5U, 0xaU).s32();
        CONSTANT(8U, 0x2U).s32();
        CONSTANT(10U, 0x3U).s32();

        BASIC_BLOCK(2U, 3U, 12U)
        {
            INST(2U, Opcode::LoadArray).ref().Inputs(0U, 1U);
            INST(3U, Opcode::SaveState).Inputs(2U, 1U, 0U).SrcVregs({0U, 1U, 2U});
            INST(4U, Opcode::CallStatic)
                .s32()
                .Inputs({{compiler::DataType::REFERENCE, 2U}, {compiler::DataType::NO_TYPE, 3U}});
            INST(23U, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(5U, 4U);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(7U, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(1U, 4U);
        }

        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(9U, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(8U, 4U);
        }

        BASIC_BLOCK(7U, 8U, 10U)
        {
            INST(11U, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(10U, 4U);
        }

        BASIC_BLOCK(10U, 11U)
        {
            INST(12U, Opcode::Mod).s32().Inputs(5U, 4U);
        }

        BASIC_BLOCK(8U, 11U)
        {
            INST(13U, Opcode::Mul).s32().Inputs(4U, 5U);
        }

        BASIC_BLOCK(6U, 11U)
        {
            INST(14U, Opcode::Add).s32().Inputs(4U, 5U);
        }

        BASIC_BLOCK(4U, 11U)
        {
            INST(15U, Opcode::Sub).s32().Inputs(5U, 4U);
        }

        BASIC_BLOCK(11U, 13U)
        {
            INST(16U, Opcode::Phi).s32().Inputs({{4U, 15U}, {6U, 14U}, {8U, 13U}, {10U, 12U}});
        }

        BASIC_BLOCK(12U, 13U)
        {
            INST(17U, Opcode::Sub).s32().Inputs(5U, 1U);
        }

        BASIC_BLOCK(13U, -1L)
        {
            INST(18U, Opcode::Phi).s32().Inputs({{11U, 16U}, {12U, 17U}});
            INST(19U, Opcode::Return).s32().Inputs(18U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
}

/*
 * This test covers case with LoadObject which user can not read from accumulator
 */
TEST_F(RegAccAllocTest, NotUseAccDstRegLoadObject)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::LoadObject).s32().Inputs(0U);
            INST(6U, Opcode::LoadObject).ref().Inputs(0U);
            INST(7U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 6U}, {NO_TYPE, 7U}});
            INST(24U, Opcode::If).CC(compiler::ConditionCode::CC_NE).SrcType(INT32).Inputs(9U, 3U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            CONSTANT(25U, 0xffffU).s32();
            INST(13U, Opcode::Return).u16().Inputs(25U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(16U, Opcode::LoadObject).ref().Inputs(0U);
            INST(19U, Opcode::LoadObject).s32().Inputs(0U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(22U, Opcode::CallVirtual).u16().Inputs({{REFERENCE, 16U}, {INT32, 19U}, {NO_TYPE, 20U}});
            INST(23U, Opcode::Return).u16().Inputs(22U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    ASSERT_NE(INS(3U).GetDstReg(), compiler::ACC_REG_ID);
    ASSERT_EQ(INS(6U).GetDstReg(), compiler::ACC_REG_ID);
    ASSERT_NE(INS(16U).GetDstReg(), compiler::ACC_REG_ID);
    ASSERT_EQ(INS(19U).GetDstReg(), compiler::ACC_REG_ID);
}

/*
 * Test checks if accumulator gets dirty between input and inst where input is LoadObject
 */
TEST_F(RegAccAllocTest, IsAccWriteBetweenLoadObject)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::LoadObject).s32().Inputs(0U);
            INST(6U, Opcode::IfImm).CC(compiler::ConditionCode::CC_NE).SrcType(INT32).Inputs(3U).Imm(0U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            CONSTANT(22U, 0xffffU).s32();
            INST(8U, Opcode::Return).u16().Inputs(22U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(21U, Opcode::SubI).s32().Imm(1U).Inputs(3U);
            INST(16U, Opcode::StoreObject).s32().Inputs(0U, 21U);
            INST(17U, Opcode::SaveState).NoVregs();
            INST(19U, Opcode::CallVirtual).u16().Inputs({{REFERENCE, 0U}, {NO_TYPE, 17U}});
            INST(20U, Opcode::Return).u16().Inputs(19U);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    ASSERT_NE(INS(3U).GetDstReg(), compiler::ACC_REG_ID);
}

/*
 * Test calls with accumulator
 */
TEST_F(RegAccAllocTest, CallWithAcc)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;

        CONSTANT(0U, 0U).s32();
        CONSTANT(1U, 1U).s32();
        CONSTANT(2U, 2U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::LoadString).ref().Inputs(3U).TypeId(42U);
            INST(5U, Opcode::CallStatic)
                .v0id()
                .Inputs({{INT32, 0U}, {INT32, 1U}, {INT32, 2U}, {REFERENCE, 4U}, {NO_TYPE, 3U}});
            INST(6U, Opcode::LoadString).ref().Inputs(3U).TypeId(43U);
            INST(7U, Opcode::CallStatic).v0id().Inputs({{INT32, 0U}, {INT32, 1U}, {REFERENCE, 6U}, {NO_TYPE, 3U}});
            INST(8U, Opcode::LoadString).ref().Inputs(3U).TypeId(44U);
            INST(9U, Opcode::CallStatic).v0id().Inputs({{INT32, 0U}, {REFERENCE, 8U}, {NO_TYPE, 3U}});
            INST(10U, Opcode::LoadString).ref().Inputs(3U).TypeId(45U);
            INST(11U, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 10U}, {NO_TYPE, 3U}});
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    EXPECT_EQ(INS(4U).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(6U).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(8U).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(10U).GetDstReg(), compiler::ACC_REG_ID);
}

TEST_F(RegAccAllocTest, Ldai)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            CONSTANT(1U, 3U).s32();
            INST(10U, Opcode::If).CC(compiler::CC_GE).SrcType(compiler::DataType::INT32).Inputs(1U, 0U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            CONSTANT(9U, 8U).s32();
            INST(7U, Opcode::Return).s32().Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Return).s32().Inputs(0U);
        }
    }
    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    EXPECT_EQ(INS(1U).GetDstReg(), compiler::ACC_REG_ID);
}

TEST_F(RegAccAllocTest, Ldai_Call)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        BASIC_BLOCK(2U, -1L)
        {
            CONSTANT(1U, 1U).s32();
            INST(2U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5U, Opcode::InitObject).ref().Inputs({{REFERENCE, 3U}, {INT32, 1U}, {NO_TYPE, 4U}});
            INST(6U, Opcode::SaveState).Inputs().SrcVregs({});
            CONSTANT(7U, 11U).s32();
            INST(8U, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 5U}, {INT32, 7U}, {NO_TYPE, 6U}});

            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    graph->RunPass<bytecodeopt::RegAccAlloc>();

    EXPECT_NE(INS(1U).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(5U).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_NE(INS(7U).GetDstReg(), compiler::ACC_REG_ID);
}

TEST_F(RegAccAllocTest, Ldai_Call_2)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        BASIC_BLOCK(2U, -1L)
        {
            CONSTANT(1U, 1U).s32();
            INST(2U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5U, Opcode::InitObject).ref().Inputs({{REFERENCE, 3U}, {INT32, 1U}, {NO_TYPE, 4U}});
            INST(6U, Opcode::SaveState).Inputs().SrcVregs({});
            CONSTANT(7U, 11U).s32();
            CONSTANT(8U, 11U).s32();
            INST(9U, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 5U}, {INT32, 7U}, {INT32, 8U}, {NO_TYPE, 6U}});

            INST(10U, Opcode::Return).ref().Inputs(5U);
        }
    }
    graph->RunPass<bytecodeopt::RegAccAlloc>();

    EXPECT_EQ(INS(7U).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_NE(INS(8U).GetDstReg(), compiler::ACC_REG_ID);
}

TEST_F(RegAccAllocTest, Ldai_Cast)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s32().SrcType(compiler::DataType::UINT64).Inputs(0U);
            CONSTANT(2U, 159U).s32();
            INST(3U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    graph->RunPass<bytecodeopt::RegAccAlloc>();

    EXPECT_EQ(INS(1U).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(3U).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(3U).GetInput(1U).GetInst()->GetOpcode(), Opcode::Constant);
}

TEST_F(RegAccAllocTest, Ldai_Cast2)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            CONSTANT(2U, 159U).s32();
            INST(1U, Opcode::Cast).s32().SrcType(compiler::DataType::INT64).Inputs(0U);
            INST(3U, Opcode::Add).s32().Inputs(2U, 1U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    graph->RunPass<bytecodeopt::RegAccAlloc>();

    EXPECT_NE(INS(2U).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(1U).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(3U).GetInput(0U).GetInst()->GetOpcode(), Opcode::Cast);
    EXPECT_EQ(INS(3U).GetInput(1U).GetInst()->GetOpcode(), Opcode::Constant);
}

TEST_F(RegAccAllocTest, Ldai_Exist)
{
    pandasm::Parser p;
    auto source = std::string(R"(
        .record A {
            f32 a1 <static>
        }
        .function i32 main() {
            fmovi v1, 0x42280000
            lda v1
            ststatic A.a1
            lda v1
            f32toi32
            return
        }
        )");

    auto res = p.Parse(source);
    auto &program = res.Value();
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    std::string fileName = "Ldai_Exist";
    auto pfile = pandasm::AsmEmitter::Emit(fileName, program, nullptr, &maps);
    ASSERT_NE(pfile, false);

    auto oldOptions = ark::bytecodeopt::g_options;
    ark::bytecodeopt::g_options = ark::bytecodeopt::Options("--opt-level=2");
    EXPECT_TRUE(OptimizeBytecode(&program, &maps, fileName, false, true));
    ark::bytecodeopt::g_options = oldOptions;
    bool fldaiExists = false;
    for (const auto &inst : program.functionStaticTable.find("main:()")->second.ins) {
        if (inst.opcode == ark::pandasm::Opcode::FLDAI) {
            fldaiExists = true;
            break;
        }
    }
    EXPECT_EQ(fldaiExists, true);
}

TEST_F(RegAccAllocTest, Lda_Extra1)
{
    pandasm::Parser p;
    auto source = std::string(R"(
        .function u1 main() {
            fldai 0x42280000
            f32toi32
            sta v1
            movi v2, 0x2a
            lda v1
            jeq v2, jump_label_0
            ldai 0x1
            return
        jump_label_0:
            ldai 0x0
            return
        }
        )");

    auto res = p.Parse(source);
    auto &program = res.Value();
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    std::string fileName = "Lda_Extra1";
    auto pfile = pandasm::AsmEmitter::Emit(fileName, program, nullptr, &maps);
    ASSERT_NE(pfile, false);

    auto oldOptions = ark::bytecodeopt::g_options;
    ark::bytecodeopt::g_options = ark::bytecodeopt::Options("--opt-level=2");
    EXPECT_TRUE(OptimizeBytecode(&program, &maps, fileName, false, true));
    ark::bytecodeopt::g_options = oldOptions;
    bool ldaExists = false;
    for (const auto &inst : program.functionStaticTable.find("main:()")->second.ins) {
        if (inst.opcode == ark::pandasm::Opcode::LDA) {
            ldaExists = true;
            break;
        }
    }
    EXPECT_EQ(ldaExists, false);
}

TEST_F(RegAccAllocTest, Lda_Extra2)
{
    pandasm::Parser p;
    auto source = std::string(R"(
        .function i32[] main(i32 a0) {
            movi v0, 0x4
            newarr v4, v0, i32[]
            mov v1, a0
            add v0, a0
            sta v0
            movi v2, 0x1
            lda v0
            starr v4, v2
            lda v1
            add2 a0
            sta v0
            movi v2, 0x2
            lda v0
            starr v4, v2
            lda v1
            add2 a0
            sta v0
            movi v1, 0x3
            lda v0
            starr v4, v1
            lda.obj v4
            return.obj
        }
        )");

    auto res = p.Parse(source);
    auto &program = res.Value();
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    std::string fileName = "Lda_Extra2";
    auto pfile = pandasm::AsmEmitter::Emit(fileName, program, nullptr, &maps);
    ASSERT_NE(pfile, false);

    auto oldOptions = ark::bytecodeopt::g_options;
    ark::bytecodeopt::g_options = ark::bytecodeopt::Options("--opt-level=2");
    EXPECT_TRUE(OptimizeBytecode(&program, &maps, fileName, false, true));
    ark::bytecodeopt::g_options = oldOptions;
    int ldaAmount = 0;
    for (const auto &inst : program.functionStaticTable.find("main:(i32)")->second.ins) {
        if (inst.opcode == ark::pandasm::Opcode::LDA) {
            ldaAmount += 1;
        }
    }
    EXPECT_EQ(ldaAmount, 1U);
}

TEST_F(RegAccAllocTest, Const_Phi)
{
    auto graph = GetAllocator()->New<compiler::Graph>(GetAllocator(), GetLocalAllocator(), Arch::X86_64, false, true);
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(8U, 0U).f64();
        BASIC_BLOCK(2U, 3U)
        {
            CONSTANT(1U, 6U).f64();
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(3U, Opcode::Phi).Inputs(0U, 7U).s32();
            INST(6U, Opcode::Phi).Inputs(8U, 1U).f64();
            INST(4U, Opcode::Cmp).s32().SrcType(compiler::DataType::FLOAT64).Fcmpg(true).Inputs(6U, 8U);
            INST(5U, Opcode::IfImm).SrcType(compiler::DataType::INT32).CC(compiler::CC_GE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::AddI).Imm(1U).Inputs(3U).s32();
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::Return).Inputs(3U).s32();
        }
    }
    graph->RunPass<bytecodeopt::RegAccAlloc>();

    EXPECT_NE(INS(6U).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(4U).GetDstReg(), compiler::ACC_REG_ID);
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::bytecodeopt::test
