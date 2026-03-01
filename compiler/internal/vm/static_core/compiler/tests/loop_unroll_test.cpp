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
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "optimizer/optimizations/loop_unroll.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/code_generator/codegen.h"
#include "optimizer/ir/graph_cloner.h"

#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_AMD64)
#include "vixl_exec_module.h"
#endif

namespace ark::compiler {
// NOLINTBEGIN(readability-magic-numbers)
class LoopUnrollTest : public GraphTest {
public:
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_AMD64)
    LoopUnrollTest() : opcodesCount_(GetAllocator()->Adapter()), execModule_(GetAllocator(), GetGraph()->GetRuntime())
#else
    LoopUnrollTest() : opcodesCount_(GetAllocator()->Adapter())
#endif
    {
    }

    template <typename T, typename... Args>
    bool CheckRetOnVixlSimulator([[maybe_unused]] Graph *graph, [[maybe_unused]] T returnValue,
                                 [[maybe_unused]] Args... args)
    {
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_AMD64)
#ifndef NDEBUG
        // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
        graph->SetLowLevelInstructionsEnabled();
#endif
        graph->RunPass<Cleanup>();
        EXPECT_TRUE(RegAlloc(graph));
        EXPECT_TRUE(graph->RunPass<Codegen>());
        auto entry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto exit = entry + graph->GetCode().Size();  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ASSERT(entry != nullptr && exit != nullptr);
        execModule_.SetInstructions(entry, exit);
        execModule_.SetParameters(args...);
        execModule_.Execute();
        return execModule_.GetRetValue<T>() == returnValue;
#else
        return true;
#endif
    }

    size_t CountOpcodes(const ArenaVector<BasicBlock *> &blocks)
    {
        size_t countInst = 0;
        opcodesCount_.clear();
        for (auto block : blocks) {
            for (auto inst : block->AllInsts()) {
                opcodesCount_[inst->GetOpcode()]++;
                countInst++;
            }
        }
        return countInst;
    }

    size_t GetOpcodeCount(Opcode opcode)
    {
        return opcodesCount_.at(opcode);
    }

    Graph *BuildGraphSimpleLoop()
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();    // a = 0
            PARAMETER(1U, 1U).u64();    // b = 1
            PARAMETER(2U, 100U).u64();  // c = 100
            PARAMETER(3U, 101U).u64();

            BASIC_BLOCK(2U, 2U, 3U)
            {
                INST(4U, Opcode::Phi).u64().Inputs(0U, 6U);
                INST(5U, Opcode::Phi).u64().Inputs(1U, 7U);
                INST(6U, Opcode::Mul).u64().Inputs(4U, 4U);  // a = a * a
                INST(7U, Opcode::Add).u64().Inputs(5U, 3U);  // b = b + 1

                INST(8U, Opcode::Compare).CC(CC_LT).b().Inputs(7U, 2U);  // while b < c
                INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Sub).u64().Inputs(6U, 7U);
                INST(11U, Opcode::Return).u64().Inputs(10U);  // return (a - b)
            }
        }
        return graph;
    }

    void CheckSimpleLoop(uint32_t instLimit, uint32_t unrollFactor, uint32_t expectedFactor)
    {
        auto graph = BuildGraphSimpleLoop();
        graph->RunPass<LoopUnroll>(instLimit, unrollFactor);
        graph->RunPass<Cleanup>();
        // Check number of instructions
        CountOpcodes(graph->GetBlocksRPO());
        EXPECT_EQ(GetOpcodeCount(Opcode::Add), expectedFactor);
        EXPECT_EQ(GetOpcodeCount(Opcode::Mul), expectedFactor);
        EXPECT_EQ(GetOpcodeCount(Opcode::Compare), expectedFactor);
        EXPECT_EQ(GetOpcodeCount(Opcode::IfImm), expectedFactor);
        EXPECT_EQ(GetOpcodeCount(Opcode::Sub), 1U);
        EXPECT_EQ(GetOpcodeCount(Opcode::Parameter), 4U);

        if (expectedFactor > 1U) {
            // Check control-flow
            EXPECT_EQ(BB(3U).GetSuccsBlocks().size(), 1U);
            EXPECT_EQ(BB(3U).GetSuccessor(0U), graph->GetEndBlock());
            EXPECT_EQ(BB(3U).GetPredsBlocks().size(), expectedFactor);

            // phi1 [INST(6, Mul), INST(6', Mul), INST(6'', Mul)]
            auto phi1 = INS(10U).GetInput(0U).GetInst();
            EXPECT_TRUE(phi1->IsPhi() && phi1->GetInputsCount() == expectedFactor);
            EXPECT_TRUE(phi1->GetInput(0U) == &INS(6U));
            for (auto input : phi1->GetInputs()) {
                EXPECT_TRUE(input.GetInst()->GetOpcode() == Opcode::Mul);
            }

            // phi2 [INST(7, Add), INST(7', Add), INST(7'', Add)]
            auto phi2 = INS(10U).GetInput(1U).GetInst();
            EXPECT_TRUE(phi2->IsPhi() && phi2->GetInputsCount() == expectedFactor);
            EXPECT_TRUE(phi2->GetInput(0U) == &INS(7U));
            for (auto input : phi2->GetInputs()) {
                EXPECT_TRUE(input.GetInst()->GetOpcode() == Opcode::Add);
            }

            // Check cloned `Mul` instruction inputs
            for (size_t i = 1; i < phi1->GetInputsCount(); i++) {
                auto clonedMul = phi1->GetInput(i).GetInst();
                auto prevMul = phi1->GetInput(i - 1L).GetInst();
                EXPECT_TRUE(clonedMul->GetInput(0U) == prevMul);
                EXPECT_TRUE(clonedMul->GetInput(1U) == prevMul);
            }

            // Check cloned `Add` instruction inputs
            for (size_t i = 1; i < phi2->GetInputsCount(); i++) {
                auto clonedAdd = phi2->GetInput(i).GetInst();
                auto prevAdd = phi2->GetInput(i - 1L).GetInst();
                EXPECT_TRUE(clonedAdd->GetInput(0U) == prevAdd);
                EXPECT_TRUE(clonedAdd->GetInput(1U) == &INS(3U));
            }
        } else {
            EXPECT_EQ(INS(10U).GetInput(0U).GetInst(), &INS(6U));
            EXPECT_EQ(INS(10U).GetInput(1U).GetInst(), &INS(7U));
            EXPECT_EQ(BB(3U).GetPredsBlocks().size(), 1U);
            EXPECT_EQ(BB(3U).GetPredsBlocks()[0U], &BB(2U));
        }
    }

    Graph *BuildGraphCheckLoopWithPhiAndSafePoint()
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();  // a = 26
            PARAMETER(1U, 1U).u64();  // b = 0
            CONSTANT(2U, 0U);         // const 0
            CONSTANT(3U, 1UL);        // const 1
            CONSTANT(4U, 2UL);        // const 2
            CONSTANT(5U, 10UL);       // const 10

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(6U, Opcode::Phi).u64().Inputs(0U, 15U);
                INST(7U, Opcode::Phi).u64().Inputs(1U, 14U);
                INST(20U, Opcode::SafePoint).Inputs(0U, 1U).SrcVregs({0U, 1U});
                INST(8U, Opcode::Mod).u64().Inputs(6U, 4U);              // mod = a % 2
                INST(9U, Opcode::Compare).CC(CC_EQ).b().Inputs(8U, 3U);  // if mod == 1
                INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
            }
            BASIC_BLOCK(3U, 5U)
            {
                INST(11U, Opcode::Add).u64().Inputs(7U, 2U);  // b = b + 0
            }
            BASIC_BLOCK(4U, 5U)
            {
                INST(12U, Opcode::Sub).u64().Inputs(7U, 3U);  // b = b + 1
            }
            BASIC_BLOCK(5U, 6U, 2U)
            {
                INST(13U, Opcode::Phi).u64().Inputs(11U, 12U);
                INST(14U, Opcode::Mul).u64().Inputs(13U, 5U);             // b = b * 10
                INST(15U, Opcode::Div).u64().Inputs(6U, 4U);              // a = a / 2
                INST(16U, Opcode::Compare).CC(CC_EQ).b().Inputs(6U, 2U);  // if a = 0
                INST(17U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(16U);
            }
            BASIC_BLOCK(6U, -1L)
            {
                INST(18U, Opcode::Div).u64().Inputs(14U, 5U);  // b = b / 10
                INST(19U, Opcode::Return).u64().Inputs(18U);   // return b
            }
        }
        return graph;
    }

    void CheckLoopWithPhiAndSafePoint(uint32_t instLimit, uint32_t unrollFactor, uint32_t expectedFactor)
    {
        auto graph = BuildGraphCheckLoopWithPhiAndSafePoint();
        graph->RunPass<LoopUnroll>(instLimit, unrollFactor);
        GraphChecker(graph).Check();

        // Check number of instructions
        CountOpcodes(graph->GetBlocksRPO());
        EXPECT_EQ(GetOpcodeCount(Opcode::Add), expectedFactor);
        EXPECT_EQ(GetOpcodeCount(Opcode::Sub), expectedFactor);
        EXPECT_EQ(GetOpcodeCount(Opcode::Mul), expectedFactor);
        EXPECT_EQ(GetOpcodeCount(Opcode::Mod), expectedFactor);
        EXPECT_EQ(GetOpcodeCount(Opcode::Div), expectedFactor + 1U);
        EXPECT_EQ(GetOpcodeCount(Opcode::IfImm), 2U * expectedFactor);
        EXPECT_EQ(GetOpcodeCount(Opcode::Compare), 2U * expectedFactor);
        size_t extraPhi = (expectedFactor > 1U) ? 1U : 0U;
        EXPECT_EQ(GetOpcodeCount(Opcode::Phi),
                  2U + expectedFactor + extraPhi);         // 2 in the front-block + N unrolled + 1 in the outer-block
        EXPECT_EQ(GetOpcodeCount(Opcode::SafePoint), 1U);  // SafePoint isn't unrolled

        if (expectedFactor > 1U) {
            // Check control-flow
            auto outerBlock = BB(5U).GetTrueSuccessor();
            EXPECT_EQ(outerBlock->GetSuccsBlocks().size(), 1U);
            EXPECT_EQ(outerBlock->GetSuccessor(0U), &BB(6U));
            EXPECT_EQ(outerBlock->GetPredsBlocks().size(), expectedFactor);

            // phi [INST(14, Mul), INST(14', Mul)]
            auto phi = INS(18U).GetInput(0U).GetInst();
            EXPECT_TRUE(phi->IsPhi() && phi->GetInputsCount() == expectedFactor);
            EXPECT_TRUE(phi->GetInput(0U) == &INS(14U));
            for (auto input : phi->GetInputs()) {
                EXPECT_TRUE(input.GetInst()->GetOpcode() == Opcode::Mul);
            }

            // Check cloned `Mul` instruction inputs
            for (size_t i = 1; i < phi->GetInputsCount(); i++) {
                auto clonedMul = phi->GetInput(i).GetInst();
                auto prevMul = phi->GetInput(i - 1L).GetInst();
                EXPECT_TRUE(clonedMul->GetInput(0U).GetInst()->IsPhi());
                EXPECT_TRUE(prevMul->GetInput(0U).GetInst()->IsPhi());
                EXPECT_NE(clonedMul->GetInput(0U).GetInst(), prevMul->GetInput(0U).GetInst());
                EXPECT_TRUE(clonedMul->GetInput(1U) == &INS(5U));
            }
        } else {
            EXPECT_EQ(INS(18U).GetInput(0U).GetInst(), &INS(14U));
            EXPECT_EQ(INS(18U).GetInput(1U).GetInst(), &INS(5U));
            EXPECT_EQ(BB(6U).GetPredsBlocks().size(), 1U);
            EXPECT_EQ(BB(6U).GetPredsBlocks()[0U], &BB(5U));
        }
    };

    Graph *BuildGraphInversedCompares1();
    Graph *BuildGraphInversedCompares2();
    Graph *BuildGraphPredsInversedOrder();
    Graph *BuildExpectedPredsInversedOrder();
    Graph *BuildGraphAddOverflowUnroll();
    Graph *BuildGraphUnrollNeedSaveStateBridge();
    Graph *BuildGraphPhiInputOfAnotherPhi();
    Graph *BuildLoopWithIncrement(ConditionCode cc, std::optional<uint64_t> start, uint64_t stop, uint64_t step,
                                  DataType::Type type = DataType::INT32);
    Graph *BuildLoopWithDecrement(ConditionCode cc, std::optional<uint64_t> start, std::optional<uint64_t> stop,
                                  uint64_t step, DataType::Type type = DataType::INT32);
    Graph *BuildLoopWithShiftLeftLe(DataType::Type type = DataType::INT32);
    Graph *BuildLoopWithShiftLeftLt(DataType::Type type = DataType::INT32);
    Graph *BuildLoopWithShiftRightGe(DataType::Type type = DataType::INT32);
    Graph *BuildLoopWithShiftRightGt(DataType::Type type = DataType::INT32);

protected:
    static constexpr uint32_t INST_LIMIT = 1000;
    static constexpr uint32_t ZERO_START = 0;
    static constexpr uint32_t ZERO_STOP = 0;

private:
    ArenaUnorderedMap<Opcode, size_t> opcodesCount_;
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_AMD64)
    VixlExecModule execModule_;
#endif
};

/*
 * Test Graph:
 *              [0]
 *               |
 *               v
 *              [2]<----\
 *               |      |
 *               v      |
 *              [3]-----/
 *               |
 *               v
 *             [exit]
 *
 *
 * After unroll with FACTOR = 3
 *
 *              [0]
 *               |
 *               v
 *              [2]<----\
 *               |      |
 *               v      |
 *         /----[3]     |
 *         |     |      |
 *         |     v      |
 *         |    [2']    |
 *         |     |      |
 *         |     v      |
 *         |<---[3']    |
 *         |     |      |
 *         |     v      |
 *         |    [2'']   |
 *         |     |      |
 *         |     v      |
 *         |<---[3'']---/
 *         |
 *         |
 *         \-->[outer]
 *                |
 *                v
 *              [exit]
 *
 */

/**
 * There are 6 instructions in the loop [bb2, bb3], 4 of them are cloneable
 * So we have the following mapping form unroll factor to number on unrolled instructions:
 *
 * factor | unrolled inst count
 * 1        6
 * 2        10
 * 3        14
 * 4        18
 * ...
 * 100      402
 *
 * unrolled_inst_count = (factor * cloneable_inst) + (not_cloneable_inst)
 */
TEST_F(LoopUnrollTest, SimpleLoop)
{
    CheckSimpleLoop(0U, 4U, 1U);
    CheckSimpleLoop(6U, 4U, 1U);
    CheckSimpleLoop(9U, 4U, 1U);
    CheckSimpleLoop(10U, 4U, 2U);
    CheckSimpleLoop(14U, 4U, 3U);
    CheckSimpleLoop(100U, 4U, 4U);
    CheckSimpleLoop(100U, 10U, 10U);
    CheckSimpleLoop(400U, 100U, 99U);
    CheckSimpleLoop(1000U, 100U, 100U);
}

/*
 * Test Graph:
 *              [0]
 *               |
 *               v
 *              [2]<--------\
 *             /   \        |
 *            v     v       |
 *           [3]    [4]     |
 *            \      /      |
 *             v    v       |
 *              [5]---------/
 *               |
 *               v
 *             [exit]
 *
 * After unroll with FACTOR = 2
 *
 *              [0]
 *               |
 *               v
 *              [2]<--------\
 *             /   \        |
 *            v     v       |
 *           [3]    [4]     |
 *            \      /      |
 *             v    v       |
 *  /-----------[5]         |
 *  |            |          |
 *  |            v          |
 *  |           [2']        |
 *  |          /   \        |
 *  |         v     v       |
 *  |       [3']    [4']    |
 *  |         \      /      |
 *  |          v    v       |
 *  |           [5']--------/
 *  |            |
 *  |            v
 *  \--------->[outer]
 *               |
 *               v
 *             [exit]
 */

/**
 * There are 13 instructions in the loop [bb2, bb3, bb4, bb5], 10 of them are cloneable
 * So we have the following mapping form unroll factor to number on unrolled instructions:
 *
 * factor | unrolled inst count
 * 1        13
 * 2        23
 * 3        33
 * 4        43
 * ...
 * 100      1003
 *
 * unrolled_inst_count = (factor * cloneable_inst) + (not_cloneable_inst)
 */
TEST_F(LoopUnrollTest, LoopWithPhisAndSafePoint)
{
    CheckLoopWithPhiAndSafePoint(0U, 4U, 1U);
    CheckLoopWithPhiAndSafePoint(13U, 4U, 1U);
    CheckLoopWithPhiAndSafePoint(22U, 4U, 1U);
    CheckLoopWithPhiAndSafePoint(23U, 4U, 2U);
    CheckLoopWithPhiAndSafePoint(33U, 4U, 3U);
    CheckLoopWithPhiAndSafePoint(100U, 4U, 4U);
    CheckLoopWithPhiAndSafePoint(1000U, 10U, 10U);
    CheckLoopWithPhiAndSafePoint(1003U, 100U, 100U);
}

/*
 * Test Graph:
 *              [0]
 *               |
 *               v
 *         /----[2]<----\
 *         |     |      |
 *         |     v      |
 *         |    [3]-----/
 *         |
 *         |
 *         \--->[4]
 *               |
 *               v
 *       /----->[5]<-----\
 *       |       |       |
 *       |       v       |
 *       \------[6]      |
 *               |       |
 *               v       |
 *              [7]------/
 *               |
 *               v
 *             [exit]
 */
TEST_F(LoopUnrollTest, UnrollNotApplied)
{
    constexpr uint32_t UNROLL_FACTOR = 2;
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).v0id().InputsAutoType(20U);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 6U) {}
        BASIC_BLOCK(6U, 7U, 5U)
        {
            INST(13U, Opcode::Add).u64().Inputs(1U, 2U);
            INST(8U, Opcode::Compare).b().Inputs(13U, 0U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(7U, 5U, 8U)
        {
            INST(10U, Opcode::Compare).b().Inputs(1U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(12U, Opcode::ReturnVoid);
        }
    }
    auto instCount = CountOpcodes(GetGraph()->GetBlocksRPO());
    auto cmpCount = GetOpcodeCount(Opcode::Compare);
    auto ifCount = GetOpcodeCount(Opcode::IfImm);
    auto addCount = GetOpcodeCount(Opcode::Add);

    GetGraph()->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    GraphChecker(GetGraph()).Check();
    auto unrolledCount = CountOpcodes(GetGraph()->GetBlocksRPO());
    EXPECT_EQ(GetOpcodeCount(Opcode::Compare), cmpCount);
    EXPECT_EQ(GetOpcodeCount(Opcode::IfImm), ifCount);
    EXPECT_EQ(GetOpcodeCount(Opcode::Add), addCount);
    EXPECT_EQ(unrolledCount, instCount);

    EXPECT_EQ(BB(8U).GetPredsBlocks().size(), 1U);
    EXPECT_EQ(BB(8U).GetPredsBlocks()[0U], &BB(7U));
}

/**
 *  a, b, c = 0, 1, 2
 *  while c < 100:
 *      a, b, c = b, c, c + a
 *  return c
 */
Graph *LoopUnrollTest::BuildGraphPhiInputOfAnotherPhi()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(4U, Opcode::Phi).u64().Inputs(0U, 5U);
            INST(5U, Opcode::Phi).u64().Inputs(1U, 6U);
            INST(6U, Opcode::Phi).u64().Inputs(2U, 7U);
            INST(7U, Opcode::Add).u64().Inputs(4U, 6U);
            INST(8U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_LT).Imm(100U).Inputs(7U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(10U, Opcode::Return).u64().Inputs(7U);
        }
    }
    return graph;
}

TEST_F(LoopUnrollTest, PhiInputOfAnotherPhi2)
{
    // Test with UNROLL_FACTOR = 2
    auto graph = BuildGraphPhiInputOfAnotherPhi();

    auto graphUnrollFactor2 = CreateEmptyGraph();
    GRAPH(graphUnrollFactor2)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(4U, Opcode::Phi).u64().Inputs(0U, 6U);
            INST(5U, Opcode::Phi).u64().Inputs(1U, 7U);
            INST(6U, Opcode::Phi).u64().Inputs(2U, 11U);
            INST(7U, Opcode::Add).u64().Inputs(4U, 6U);
            INST(8U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_LT).Imm(100U).Inputs(7U);
        }

        BASIC_BLOCK(4U, 2U, 3U)
        {
            INST(11U, Opcode::Add).u64().Inputs(5U, 7U);
            INST(12U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_LT).Imm(100U).Inputs(11U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs(7U, 11U);
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }
    static constexpr uint64_t PROGRAM_RESULT = 101;
    graph->RunPass<LoopUnroll>(INST_LIMIT, 2U);
    graph->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnrollFactor2));
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graphUnrollFactor2, PROGRAM_RESULT));
}

TEST_F(LoopUnrollTest, PhiInputOfAnotherPhi4)
{
    // Test with UNROLL_FACTOR = 4
    auto graph = BuildGraphPhiInputOfAnotherPhi();

    auto graphUnrollFactor4 = CreateEmptyGraph();
    GRAPH(graphUnrollFactor4)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(4U, Opcode::Phi).u64().Inputs(0U, 11U);
            INST(5U, Opcode::Phi).u64().Inputs(1U, 13U);
            INST(6U, Opcode::Phi).u64().Inputs(2U, 15U);
            INST(7U, Opcode::Add).u64().Inputs(4U, 6U);
            INST(8U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_LT).Imm(100U).Inputs(7U);
        }

        BASIC_BLOCK(4U, 5U, 3U)
        {
            INST(11U, Opcode::Add).u64().Inputs(5U, 7U);
            INST(12U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_LT).Imm(100U).Inputs(11U);
        }

        BASIC_BLOCK(5U, 6U, 3U)
        {
            INST(13U, Opcode::Add).u64().Inputs(6U, 11U);
            INST(14U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_LT).Imm(100U).Inputs(13U);
        }

        BASIC_BLOCK(6U, 2U, 3U)
        {
            INST(15U, Opcode::Add).u64().Inputs(7U, 13U);
            INST(16U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_LT).Imm(100U).Inputs(15U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs(7U, 11U, 13U, 15U);
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }

    static constexpr uint64_t PROGRAM_RESULT = 101;
    graph->RunPass<LoopUnroll>(INST_LIMIT, 4U);
    graph->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnrollFactor4));
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graphUnrollFactor4, PROGRAM_RESULT));
}

TEST_F(LoopUnrollTest, PhiInputsOutsideLoop)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(4U, Opcode::Phi).u64().Inputs(1U, 2U);
            INST(5U, Opcode::Phi).u64().Inputs(0U, 6U);
            INST(6U, Opcode::Add).u64().Inputs(4U, 5U);
            INST(7U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_LT).Imm(100U).Inputs(6U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(10U, Opcode::Return).u64().Inputs(5U);
        }
    }

    graph->RunPass<LoopUnroll>(INST_LIMIT, 2U);
    graph->RunPass<Cleanup>();

    auto expectedGraph = CreateEmptyGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);
        BASIC_BLOCK(2U, 3U)
        {
            // preheader
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Phi).u64().Inputs(1U, 2U);
            INST(5U, Opcode::Phi).u64().Inputs(0U, 8U);
            INST(6U, Opcode::Add).u64().Inputs(4U, 5U);
            INST(7U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_LT).Imm(100U).Inputs(6U);
        }

        BASIC_BLOCK(4U, 3U, 5U)
        {
            INST(8U, Opcode::Add).u64().Inputs(2U, 6U);
            INST(9U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_LT).Imm(100U).Inputs(8U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::Phi).u64().Inputs(5U, 6U);
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

Graph *LoopUnrollTest::BuildLoopWithIncrement(ConditionCode cc, std::optional<uint64_t> start, uint64_t stop,
                                              uint64_t step, DataType::Type type)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, stop);
        CONSTANT(1U, 0U);  // b = 0
        CONSTANT(2U, step);
        if (start) {
            CONSTANT(13U, *start);
        } else {
            PARAMETER(13U, 0U).SetType(type);  // initial `a` to test unroll with non-const boundaries
        }
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().SrcType(type).CC(cc).Inputs(13U, 0U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);  // if a [cc] stop
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).SetType(type).Inputs(13U, 7U);  // a
            INST(6U, Opcode::Phi).SetType(type).Inputs(1U, 8U);   // b
            INST(7U, Opcode::Add).SetType(type).Inputs(5U, 2U);   // a += step
            INST(8U, Opcode::Add).SetType(type).Inputs(6U, 7U);   // b += a
            INST(9U, Opcode::Compare).b().SrcType(type).CC(cc).Inputs(7U, 0U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);  // if a [cc] stop
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).SetType(type).Inputs(1U, 6U);
            INST(12U, Opcode::Return).SetType(type).Inputs(11U);  // return b
        }
    }
    return graph;
}

Graph *LoopUnrollTest::BuildLoopWithDecrement(ConditionCode cc, std::optional<uint64_t> start,
                                              std::optional<uint64_t> stop, uint64_t step, DataType::Type type)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        size_t param = 0;
        if (start) {
            CONSTANT(0U, *start);
        } else {
            PARAMETER(0U, param++).SetType(type);  // initial `a` to test unroll with non-const boundaries
        }
        if (stop) {
            CONSTANT(13U, *stop);
        } else {
            PARAMETER(13U, param++).SetType(type);
        }
        CONSTANT(1U, 0U);  // b = 0
        CONSTANT(2U, step);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().SrcType(type).CC(cc).Inputs(0U, 13U);  // if a [cc] stop
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).SetType(type).Inputs(0U, 8U);                  // a
            INST(6U, Opcode::Phi).SetType(type).Inputs(1U, 7U);                  // b
            INST(7U, Opcode::Add).SetType(type).Inputs(6U, 5U);                  // b += a
            INST(8U, Opcode::Sub).SetType(type).Inputs(5U, 2U);                  // a -= 1
            INST(9U, Opcode::Compare).b().SrcType(type).CC(cc).Inputs(8U, 13U);  // if a [cc] stop
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).SetType(type).Inputs(1U, 7U);
            INST(12U, Opcode::Return).SetType(type).Inputs(11U);  // return b
        }
    }
    return graph;
}

Graph *LoopUnrollTest::BuildLoopWithShiftLeftLe(DataType::Type type)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 0xfU);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().SrcType(type).CC(CC_LE).Inputs(1U, 2U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);  // if a [cc] stop
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).SetType(type).Inputs(1U, 8U);  // a
            INST(6U, Opcode::Phi).SetType(type).Inputs(0U, 7U);  // b
            INST(7U, Opcode::Add).SetType(type).Inputs(6U, 5U);  // b += a
            INST(8U, Opcode::Shl).SetType(type).Inputs(5U, 1U);  // a <<= step
            INST(9U, Opcode::Compare).b().SrcType(type).CC(CC_LE).Inputs(8U, 2U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);  // if a [cc] stop
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).SetType(type).Inputs(1U, 7U);
            INST(12U, Opcode::Return).SetType(type).Inputs(11U);  // return b
        }
    }
    return graph;
}

Graph *LoopUnrollTest::BuildLoopWithShiftLeftLt(DataType::Type type)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 0x10U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().SrcType(type).CC(CC_LT).Inputs(1U, 2U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);  // if a [cc] stop
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).SetType(type).Inputs(1U, 8U);  // a
            INST(6U, Opcode::Phi).SetType(type).Inputs(0U, 7U);  // b
            INST(7U, Opcode::Add).SetType(type).Inputs(6U, 5U);  // b += a
            INST(8U, Opcode::Shl).SetType(type).Inputs(5U, 1U);  // a <<= step
            INST(9U, Opcode::Compare).b().SrcType(type).CC(CC_LT).Inputs(8U, 2U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);  // if a [cc] stop
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).SetType(type).Inputs(1U, 7U);
            INST(12U, Opcode::Return).SetType(type).Inputs(11U);  // return b
        }
    }
    return graph;
}

Graph *LoopUnrollTest::BuildLoopWithShiftRightGe(DataType::Type type)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 0xfU);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().SrcType(type).CC(CC_GE).Inputs(2U, 1U);  // if a [cc] stop
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).SetType(type).Inputs(2U, 8U);                    // a
            INST(6U, Opcode::Phi).SetType(type).Inputs(0U, 7U);                    // b
            INST(7U, Opcode::Add).SetType(type).Inputs(6U, 5U);                    // b += a
            INST(8U, Opcode::Shr).SetType(type).Inputs(5U, 1U);                    // a >>= 1
            INST(9U, Opcode::Compare).b().SrcType(type).CC(CC_GE).Inputs(8U, 1U);  // if a [cc] stop
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).SetType(type).Inputs(1U, 7U);
            INST(12U, Opcode::Return).SetType(type).Inputs(11U);  // return b
        }
    }
    return graph;
}

Graph *LoopUnrollTest::BuildLoopWithShiftRightGt(DataType::Type type)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 0xfU);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().SrcType(type).CC(CC_GT).Inputs(2U, 0U);  // if a [cc] stop
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).SetType(type).Inputs(2U, 8U);                    // a
            INST(6U, Opcode::Phi).SetType(type).Inputs(0U, 7U);                    // b
            INST(7U, Opcode::Add).SetType(type).Inputs(6U, 5U);                    // b += a
            INST(8U, Opcode::Shr).SetType(type).Inputs(5U, 1U);                    // a >>= 1
            INST(9U, Opcode::Compare).b().SrcType(type).CC(CC_GT).Inputs(8U, 0U);  // if a [cc] stop
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).SetType(type).Inputs(1U, 7U);
            INST(12U, Opcode::Return).SetType(type).Inputs(11U);  // return b
        }
    }
    return graph;
}

TEST_F(LoopUnrollTest, ConstCountableLoopWithIncrement)
{
    static constexpr uint32_t INC_STEP = 1;
    static constexpr uint32_t INC_STOP = 10;
    for (size_t unrollFactor = 1; unrollFactor <= 10U; unrollFactor++) {
        auto graph = BuildLoopWithIncrement(CC_LT, ZERO_START, INC_STOP, INC_STEP);
        graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor);
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 45U));

        graph = BuildLoopWithIncrement(CC_LE, ZERO_START, INC_STOP, INC_STEP);
        graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor);
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 55U));
    }

    static constexpr uint32_t UNROLL_FACTOR = 2;
    auto graph = BuildLoopWithIncrement(CC_LT, ZERO_START, INC_STOP, INC_STEP);
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0U);  // a = 0, b = 0
        CONSTANT(2U, 1U);  // UNROLL_FACTOR - 1 = 1
                           // NB: add a new constant if UNROLL_FACTOR is changed and fix INST(20, Opcode::Sub).

        BASIC_BLOCK(2U, 3U, 5U)
        {
            // NB: replace the second input if UNROLL_FACTOR is changed:
            INST(20U, Opcode::Sub).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(1U, 20U);  // if (a < 10 -
                                                                                               // (UNROLL_FACTOR - 1))
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(1U, 21U);   // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 22U);   // b
            INST(7U, Opcode::Add).s32().Inputs(5U, 2U);    // a + 1
            INST(8U, Opcode::Add).s32().Inputs(6U, 7U);    // b + a
            INST(21U, Opcode::Add).s32().Inputs(7U, 2U);   // a + 1
            INST(22U, Opcode::Add).s32().Inputs(8U, 21U);  // b + a
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(21U, 20U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);  // if a < 10 -
                                                                                            // (UNROLL_FACTOR - 1)
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(11U, Opcode::Phi).s32().Inputs(1U, 8U);
            INST(12U, Opcode::Return).s32().Inputs(11U);  // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
}

TEST_F(LoopUnrollTest, ConstCountableLoopWithIncrementLeftoverIterations)
{
    static constexpr uint32_t INC_STEP = 1;
    static constexpr uint32_t INC_STOP = 11;
    static constexpr uint32_t UNROLL_FACTOR = 3;
    auto graph = BuildLoopWithIncrement(CC_LT, ZERO_START, INC_STOP, INC_STEP);
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        CONSTANT(0U, 11U);
        CONSTANT(1U, 0U);  // a = 0, b = 0
        CONSTANT(2U, 1U);
        CONSTANT(32U, 2U);  // UNROLL_FACTOR - 1 = 2
                            // NB: update constant if UNROLL_FACTOR is changed

        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(20U, Opcode::Sub).s32().Inputs(0U, 32U);
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(1U, 20U);  // if (a < 11 -
                                                                                               // (UNROLL_FACTOR - 1))
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(1U, 23U);    // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 24U);    // b
            INST(7U, Opcode::Add).s32().Inputs(5U, 2U);     // a + 1
            INST(8U, Opcode::Add).s32().Inputs(6U, 7U);     // b + a
            INST(21U, Opcode::Add).s32().Inputs(7U, 2U);    // a + 1
            INST(22U, Opcode::Add).s32().Inputs(8U, 21U);   // b + a
            INST(23U, Opcode::Add).s32().Inputs(21U, 2U);   // a + 1
            INST(24U, Opcode::Add).s32().Inputs(22U, 23U);  // b + a
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(23U, 20U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);  // if a < 11 -
                                                                                            // (UNROLL_FACTOR - 1)
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(25U, Opcode::Phi).s32().Inputs(1U, 23U);   // a
            INST(26U, Opcode::Phi).s32().Inputs(1U, 24U);   // b
            INST(27U, Opcode::Add).s32().Inputs(25U, 2U);   // a + 1
            INST(28U, Opcode::Add).s32().Inputs(26U, 27U);  // b + a
            INST(12U, Opcode::Return).s32().Inputs(28U);    // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
}

// LoopUnroll used to fail by assertion on this test
TEST_F(LoopUnrollTest, ConstCountableLoopAlreadySplitBackEdge)
{
    static constexpr uint32_t INC_STEP = 1;
    static constexpr uint32_t INC_STOP = 11;
    static constexpr uint32_t UNROLL_FACTOR = 3;
    auto graph = BuildLoopWithIncrement(CC_LT, ZERO_START, INC_STOP, INC_STEP);

    auto clone = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();
    clone->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    clone->RunPass<Cleanup>();

    auto splitAfter = &INS(8U);
    ASSERT(splitAfter->GetOpcode() == Opcode::Add);
    BB(3U).SplitBlockAfterInstruction(splitAfter, true);
    graph->InvalidateAnalysis<LoopAnalyzer>();
    GraphChecker(graph).Check();
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();

    EXPECT_TRUE(GraphComparator().Compare(graph, clone));
}

TEST_F(LoopUnrollTest, SmallCountableLoopWithIncrement)
{
    static constexpr uint32_t INC_STEP = 1;
    static constexpr uint32_t INC_STOP = 4;
    static constexpr uint32_t UNROLL_FACTOR = 2;

    auto graph = BuildLoopWithIncrement(CC_LT, ZERO_START, INC_STOP, INC_STEP);
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 6U));

    auto graph2 = BuildLoopWithIncrement(CC_LE, ZERO_START, INC_STOP - 1L, INC_STEP);
    graph2->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph2->RunPass<Cleanup>();
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph2, 6U));

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        CONSTANT(1U, 0U);  // a = 0, b = 0
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::Add).s32().Inputs(1U, 2U);     // a + 1
            INST(8U, Opcode::Add).s32().Inputs(1U, 7U);     // b + a
            INST(13U, Opcode::Add).s32().Inputs(7U, 2U);    // a + 1
            INST(14U, Opcode::Add).s32().Inputs(8U, 13U);   // b + a
            INST(15U, Opcode::Add).s32().Inputs(13U, 2U);   // a + 1
            INST(16U, Opcode::Add).s32().Inputs(14U, 15U);  // b + a
            INST(12U, Opcode::Return).s32().Inputs(16U);    // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
    EXPECT_TRUE(GraphComparator().Compare(graph2, graphUnroll));
}

TEST_F(LoopUnrollTest, SmallCountableLoopWithShiftLeft)
{
    static constexpr uint32_t UNROLL_FACTOR = 4;

    auto graph = BuildLoopWithShiftLeftLt();
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 15U));

    auto graph2 = BuildLoopWithShiftLeftLe();
    graph2->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph2->RunPass<Cleanup>();
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph2, 15U));

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        CONSTANT(0U, 0U);  // b = 0
        CONSTANT(1U, 1U);  // a = 1

        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::Add).s32().Inputs(0U, 1U);     // b += a
            INST(8U, Opcode::Shl).s32().Inputs(1U, 1U);     // a <<= 1
            INST(14U, Opcode::Add).s32().Inputs(7U, 8U);    // b += a
            INST(15U, Opcode::Shl).s32().Inputs(8U, 1U);    // a <<= 1
            INST(16U, Opcode::Add).s32().Inputs(14U, 15U);  // b += a
            INST(17U, Opcode::Shl).s32().Inputs(15U, 1U);   // a <<= 1
            INST(18U, Opcode::Add).s32().Inputs(16U, 17U);  // b += a
            INST(12U, Opcode::Return).s32().Inputs(18U);    // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
    EXPECT_TRUE(GraphComparator().Compare(graph2, graphUnroll));
}

TEST_F(LoopUnrollTest, SmallCountableLoopWithShiftRight)
{
    static constexpr uint32_t UNROLL_FACTOR = 4;

    auto graph = BuildLoopWithShiftRightGt();
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 26U));

    auto graph2 = BuildLoopWithShiftRightGe();
    graph2->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph2->RunPass<Cleanup>();
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph2, 26U));

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        CONSTANT(0U, 0U);  // a = 0xf
        CONSTANT(1U, 1U);  // b = 0
        CONSTANT(2U, 0xfU);

        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::Add).s32().Inputs(0U, 2U);     // b += a
            INST(8U, Opcode::Shr).s32().Inputs(2U, 1U);     // a >>= 1
            INST(14U, Opcode::Add).s32().Inputs(7U, 8U);    // b += a
            INST(15U, Opcode::Shr).s32().Inputs(8U, 1U);    // a >>= 1
            INST(16U, Opcode::Add).s32().Inputs(14U, 15U);  // b += a
            INST(17U, Opcode::Shr).s32().Inputs(15U, 1U);   // a >>= 1
            INST(18U, Opcode::Add).s32().Inputs(16U, 17U);  // b += a
            INST(12U, Opcode::Return).s32().Inputs(18U);    // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
    EXPECT_TRUE(GraphComparator().Compare(graph2, graphUnroll));
}

// empty loop: for (i = -1; i >= 0; i += -1)
TEST_F(LoopUnrollTest, EmptyCountableLoopWithNegativeIncrement)
{
    static constexpr uint64_t INC_STEP = -1;
    static constexpr uint64_t INC_START = -1;
    static constexpr uint64_t INC_STOP = 0;
    static constexpr uint32_t UNROLL_FACTOR = 2;

    auto graph = BuildLoopWithIncrement(CC_GE, INC_START, INC_STOP, INC_STEP);
    // unroll with side exits; such loop will be removed after BranchElimination
    EXPECT_TRUE(graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR));
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 0U));
}

// loop with two iterations: for (i = 0 + -1; i >= -2; i += -1)
TEST_F(LoopUnrollTest, SmallCountableLoopWithNegativeIncrement)
{
    static constexpr uint64_t INC_STEP = -1;
    static constexpr uint64_t INC_STOP = -2;
    static constexpr uint32_t UNROLL_FACTOR = 2;

    auto graph = BuildLoopWithIncrement(CC_GE, ZERO_START, INC_STOP, INC_STEP);
    EXPECT_TRUE(graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR));
    EXPECT_TRUE(graph->RunPass<Cleanup>());
    EXPECT_TRUE(CheckRetOnVixlSimulator<int32_t>(graph, -3L));

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        CONSTANT(1U, 0U);  // a = 0, b = 0
        CONSTANT(2U, -1L);

        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::Add).s32().Inputs(1U, 2U);    // a - 1
            INST(8U, Opcode::Add).s32().Inputs(1U, 7U);    // b + a
            INST(13U, Opcode::Add).s32().Inputs(7U, 2U);   // a - 1
            INST(14U, Opcode::Add).s32().Inputs(8U, 13U);  // b + a
            INST(12U, Opcode::Return).s32().Inputs(14U);   // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
}

TEST_F(LoopUnrollTest, EmptyCountableLoop)
{
    static constexpr uint32_t INC_STEP = 1;
    static constexpr uint32_t INC_STOP = 1;
    static constexpr uint32_t UNROLL_FACTOR = 2;

    auto graph = BuildLoopWithIncrement(CC_LT, ZERO_START, INC_STOP, INC_STEP);
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        CONSTANT(1U, 0U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(12U, Opcode::Return).s32().Inputs(1U);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
}

TEST_F(LoopUnrollTest, CountableLoopOverflow)
{
    static constexpr uint32_t INC_STEP = 2;
    static constexpr uint32_t INC_STOP = std::numeric_limits<int32_t>::max();
    static constexpr uint32_t UNROLL_FACTOR = 2;

    auto graph = BuildLoopWithIncrement(CC_LT, INC_STOP - 4L, INC_STOP, INC_STEP);
    auto clone = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();
    // small INST_LIMIT allows only unrolling with constant iterations, but such unrolling
    // is impossible due to index overflow
    graph->RunPass<LoopUnroll>(6U, UNROLL_FACTOR);

    EXPECT_TRUE(GraphComparator().Compare(graph, clone));
}

TEST_F(LoopUnrollTest, NonConstCountableLoopWithIncrement)
{
    static constexpr uint32_t INC_STEP = 1;
    static constexpr uint32_t INC_STOP = 10;
    for (size_t unrollFactor = 1; unrollFactor <= 10U; unrollFactor++) {
        auto graph = BuildLoopWithIncrement(CC_LT, std::nullopt, INC_STOP, INC_STEP);
        graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor);
        // 3 + 4 + ... + 9
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 42U, 2U));

        graph = BuildLoopWithIncrement(CC_LE, std::nullopt, INC_STOP, INC_STEP);
        graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor);
        // 3 + 4 + ... + 10
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 52U, 2U));
    }
}

TEST_F(LoopUnrollTest, NonConstCountableLoopWithIncrementCheckGraph)
{
    static constexpr uint32_t INC_STEP = 1;
    static constexpr uint32_t INC_STOP = 10;
    static constexpr uint32_t UNROLL_FACTOR = 2;
    auto graph = BuildLoopWithIncrement(CC_LT, std::nullopt, INC_STOP, INC_STEP);
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0U);          // b = 0
        CONSTANT(2U, 1U);          // UNROLL_FACTOR - 1 = 1
        PARAMETER(29U, 0U).s32();  // a
                                   // NB: add a new constant if UNROLL_FACTOR is changed and fix INST(20, Opcode::Sub).

        BASIC_BLOCK(2U, 3U, 5U)
        {
            // NB: replace the second input if UNROLL_FACTOR is changed:
            INST(20U, Opcode::Sub).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(29U, 20U);  // if (a < 10 -
                                                                                                // (UNROLL_FACTOR - 1))
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(29U, 21U);  // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 22U);   // b
            INST(7U, Opcode::Add).s32().Inputs(5U, 2U);    // a + 1
            INST(8U, Opcode::Add).s32().Inputs(6U, 7U);    // b + 1
            INST(21U, Opcode::Add).s32().Inputs(7U, 2U);   // a + 1
            INST(22U, Opcode::Add).s32().Inputs(8U, 21U);  // b + 1
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(21U, 20U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);  // if a < 10 -
                                                                                            // (UNROLL_FACTOR - 1)
        }
        BASIC_BLOCK(5U, 6U, 4U)
        {
            INST(11U, Opcode::Phi).s32().Inputs(1U, 8U);
            INST(25U, Opcode::Phi).s32().Inputs(29U, 21U);                                      // a
            INST(26U, Opcode::Phi).s32().Inputs(1U, 22U);                                       // b
            INST(27U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(25U, 0U);  // if (a < 10)
            INST(28U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(27U);
        }
        BASIC_BLOCK(6U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(31U, Opcode::Phi).s32().Inputs(11U, 26U);
            INST(12U, Opcode::Return).s32().Inputs(31U);  // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
}

TEST_F(LoopUnrollTest, ConstCountableLoopWithDecrement)
{
    static constexpr uint32_t DEC_STEP = 1;
    static constexpr uint32_t DEC_START = 10;
    for (size_t unrollFactor = 1; unrollFactor <= 10U; unrollFactor++) {
        auto graph = BuildLoopWithDecrement(CC_GT, DEC_START, ZERO_STOP, DEC_STEP);
        graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor);
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 55U));

        graph = BuildLoopWithDecrement(CC_GE, DEC_START, ZERO_STOP, DEC_STEP);
        graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor);
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 55U));
    }

    static constexpr uint32_t UNROLL_FACTOR = 2;
    auto graph = BuildLoopWithDecrement(CC_GT, DEC_START, ZERO_STOP, DEC_STEP);
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        CONSTANT(0U, 10U);  // a = 10
        CONSTANT(1U, 0U);   // b = 0
        CONSTANT(2U, 1U);   // UNROLL_FACTOR - 1 = 1
                            // NB: add a new constant if UNROLL_FACTOR is changed and fix INST(20, Opcode::Add).

        BASIC_BLOCK(2U, 3U, 5U)
        {
            // NB: replace the second input if UNROLL_FACTOR is changed:
            INST(20U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GT).Inputs(0U, 20U);  // if (a > UNROLL_FACTOR
                                                                                               // - 1)
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 22U);                                        // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 21U);                                        // b
            INST(7U, Opcode::Add).s32().Inputs(6U, 5U);                                         // b += a
            INST(8U, Opcode::Sub).s32().Inputs(5U, 2U);                                         // a -= 1
            INST(21U, Opcode::Add).s32().Inputs(7U, 8U);                                        // b += a
            INST(22U, Opcode::Sub).s32().Inputs(8U, 2U);                                        // a -= 1
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GT).Inputs(22U, 20U);  // if (a > UNROLL_FACTOR
                                                                                                // - 1)
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(25U, Opcode::Phi).s32().Inputs(1U, 21U);  // b
            INST(12U, Opcode::Return).s32().Inputs(25U);   // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
}

TEST_F(LoopUnrollTest, SmallCountableLoopWithDecrement)
{
    static constexpr uint32_t DEC_STEP = 1;
    static constexpr uint32_t DEC_START = 4;
    static constexpr uint32_t UNROLL_FACTOR = 2;

    auto graph = BuildLoopWithDecrement(CC_GT, DEC_START, ZERO_STOP, DEC_STEP);
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 10U));

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        CONSTANT(0U, 4U);  // a = 4
        CONSTANT(1U, 0U);  // b = 0
        CONSTANT(2U, 1U);
        // NB: add a new constant if UNROLL_FACTOR is changed and fix INST(20, Opcode::Sub).

        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::Add).s32().Inputs(1U, 0U);     // b + a
            INST(8U, Opcode::Sub).s32().Inputs(0U, 2U);     // a - 1
            INST(13U, Opcode::Add).s32().Inputs(7U, 8U);    // b + a
            INST(14U, Opcode::Sub).s32().Inputs(8U, 2U);    // a - 1
            INST(15U, Opcode::Add).s32().Inputs(13U, 14U);  // b + a
            INST(16U, Opcode::Sub).s32().Inputs(14U, 2U);   // a - 1
            INST(17U, Opcode::Add).s32().Inputs(15U, 16U);  // b + a
            INST(12U, Opcode::Return).s32().Inputs(17U);    // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
}

TEST_F(LoopUnrollTest, NonConstCountableLoopWithDecrement)
{
    static constexpr uint32_t DEC_STEP = 1;
    for (size_t unrollFactor = 1; unrollFactor <= 10U; unrollFactor++) {
        auto graph = BuildLoopWithDecrement(CC_GT, std::nullopt, ZERO_STOP, DEC_STEP);
        graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor);
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 55U, 10U));

        graph = BuildLoopWithDecrement(CC_GE, std::nullopt, ZERO_STOP, DEC_STEP);
        graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor);
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 55U, 10U));
    }
}

TEST_F(LoopUnrollTest, NonConstCountableLoopWithDecrementCheckGraph)
{
    static constexpr uint32_t DEC_STEP = 1;
    static constexpr uint32_t UNROLL_FACTOR = 2;
    auto graph = BuildLoopWithDecrement(CC_GT, std::nullopt, ZERO_STOP, DEC_STEP);
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        PARAMETER(0U, 0U).s32();  // a
        CONSTANT(1U, 0U);         // b = 0
        CONSTANT(2U, 1U);         // UNROLL_FACTOR - 1 = 1
                                  // NB: add a new constant if UNROLL_FACTOR is changed and fix INST(20, Opcode::Add).

        BASIC_BLOCK(2U, 3U, 5U)
        {
            // NB: replace the second input if UNROLL_FACTOR is changed:
            INST(20U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GT).Inputs(0U, 20U);  // if (a > UNROLL_FACTOR
                                                                                               // - 1)
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 22U);                                        // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 21U);                                        // b
            INST(7U, Opcode::Add).s32().Inputs(6U, 5U);                                         // b += a
            INST(8U, Opcode::Sub).s32().Inputs(5U, 2U);                                         // a -= 1
            INST(21U, Opcode::Add).s32().Inputs(7U, 8U);                                        // b += a
            INST(22U, Opcode::Sub).s32().Inputs(8U, 2U);                                        // a -= 1
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GT).Inputs(22U, 20U);  // if (a > UNROLL_FACTOR
                                                                                                // - 1)
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(5U, 6U, 4U)
        {
            INST(25U, Opcode::Phi).s32().Inputs(1U, 21U);                                       // b
            INST(26U, Opcode::Phi).s32().Inputs(0U, 22U);                                       // a
            INST(27U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GT).Inputs(26U, 1U);  // if (a > 0)
            INST(28U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(27U);
        }
        BASIC_BLOCK(6U, 4U)
        {
            INST(29U, Opcode::Add).s32().Inputs(25U, 26U);  // b += a
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(31U, Opcode::Phi).s32().Inputs(25U, 29U);
            INST(12U, Opcode::Return).s32().Inputs(31U);  // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
}

TEST_F(LoopUnrollTest, UnsignedCountableLoopWithIncrementConditionOverflow)
{
    static constexpr uint32_t INC_STOP = 3;
    static constexpr uint32_t INC_STEP = 1;
    for (size_t unrollFactor = 4U; unrollFactor <= 8U; unrollFactor++) {
        auto graph = BuildLoopWithIncrement(CC_LE, std::nullopt, INC_STOP, INC_STEP, DataType::UINT32);
        EXPECT_EQ(5U, graph->GetAliveBlocksCount());
        EXPECT_TRUE(graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor));
        if (unrollFactor == 4U) {
            // unroll without side exits
            EXPECT_EQ(18U, graph->GetAliveBlocksCount());
        } else {
            // do not unroll without side exits because new condition in header would overflow
            EXPECT_EQ(6U + unrollFactor, graph->GetAliveBlocksCount());
        }
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 6U, ZERO_START));
    }
}

TEST_F(LoopUnrollTest, UnsignedCountableLoopWithDecrementConditionOverflow)
{
    static constexpr uint32_t DEC_STOP = std::numeric_limits<uint32_t>::max() - 3L;
    static constexpr uint32_t DEC_STEP = 2;
    for (size_t unrollFactor = 3U; unrollFactor <= 6U; unrollFactor++) {
        auto graph = BuildLoopWithDecrement(CC_GT, std::nullopt, DEC_STOP, DEC_STEP, DataType::UINT32);
        EXPECT_EQ(5U, graph->GetAliveBlocksCount());
        EXPECT_TRUE(graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor));
        // do not unroll without side exits because new condition in header would overflow
        EXPECT_EQ(6U + unrollFactor, graph->GetAliveBlocksCount());
    }
}

TEST_F(LoopUnrollTest, UnsignedCountableLoopWithDecrementSmallValues)
{
    static constexpr uint32_t DEC_START = 10;
    static constexpr uint32_t DEC_STOP = 2;
    static constexpr uint32_t DEC_STEP = 1;
    for (size_t unrollFactor = 1U; unrollFactor <= 10U; unrollFactor++) {
        bool unrolled = unrollFactor > 1U;
        auto graph = BuildLoopWithDecrement(CC_GT, std::nullopt, std::nullopt, DEC_STEP, DataType::UINT32);
        EXPECT_EQ(unrolled, graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor));
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 52U, DEC_START, DEC_STOP));

        graph = BuildLoopWithDecrement(CC_GE, std::nullopt, std::nullopt, DEC_STEP, DataType::UINT32);
        EXPECT_EQ(unrolled, graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor));
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 54U, DEC_START, DEC_STOP));
    }
}

TEST_F(LoopUnrollTest, UnsignedCountableLoopWithDecrementLargeValues)
{
    static constexpr uint32_t DEC_STEP = 1;
    static constexpr uint32_t DEC_START = std::numeric_limits<uint32_t>::max();
    static constexpr uint32_t DEC_STOP = DEC_START - 3L;
    uint32_t resultGt = 0;
    for (auto i = DEC_START; i > DEC_STOP; i--) {
        resultGt += i;
    }
    uint32_t resultGe = resultGt + DEC_STOP;

    for (size_t unrollFactor = 1U; unrollFactor <= 10U; unrollFactor++) {
        bool unrolled = unrollFactor > 1U;
        auto graph = BuildLoopWithDecrement(CC_GT, std::nullopt, std::nullopt, DEC_STEP, DataType::UINT32);
        EXPECT_EQ(unrolled, graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor));
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, resultGt, DEC_START, DEC_STOP));

        graph = BuildLoopWithDecrement(CC_GE, std::nullopt, std::nullopt, DEC_STEP, DataType::UINT32);
        EXPECT_EQ(unrolled, graph->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor));
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, resultGe, DEC_START, DEC_STOP));
    }
}

Graph *LoopUnrollTest::BuildGraphInversedCompares1()
{
    // Case 1: if (a < 10 is false) goto exit
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(13U, 0U).s32();  // a; pass as a parameter to check for unknown number of iterations
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0U);  // b = 0
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(13U, 0U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(3U);  // if a < 10 goto loop
        }
        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(13U, 7U);  // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 8U);   // b
            INST(7U, Opcode::Add).s32().Inputs(5U, 2U);   // a += 1
            INST(8U, Opcode::Add).s32().Inputs(6U, 7U);   // b += a
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(7U, 0U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(9U);  // if a < 10 goto loop
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).s32().Inputs(1U, 6U);
            INST(12U, Opcode::Return).s32().Inputs(11U);  // return b
        }
    }
    return graph;
}

TEST_F(LoopUnrollTest, InversedCompares1)
{
    auto graph = BuildGraphInversedCompares1();
    static constexpr uint32_t UNROLL_FACTOR = 2;
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        PARAMETER(13U, 0U).s32();  // a
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0U);  // b = 0
        CONSTANT(2U, 1U);  // UNROLL_FACTOR - 1 = 1
                           // NB: add a new constant if UNROLL_FACTOR is changed and fix INST(20, Opcode::Sub).

        BASIC_BLOCK(2U, 3U, 5U)
        {
            // NB: replace the second input if UNROLL_FACTOR is changed:
            INST(20U, Opcode::Sub).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(13U, 20U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);  // if (a <= 10 -
                                                                                           // UNROLL_FACTOR)
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(13U, 21U);  // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 22U);   // b
            INST(7U, Opcode::Add).s32().Inputs(5U, 2U);    // a + 1
            INST(8U, Opcode::Add).s32().Inputs(6U, 7U);    // b + 1
            INST(21U, Opcode::Add).s32().Inputs(7U, 2U);   // a + 1
            INST(22U, Opcode::Add).s32().Inputs(8U, 21U);  // b + 1
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(21U, 20U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);  // if (a <= 10 -
                                                                                            // UNROLL_FACTOR)
        }
        BASIC_BLOCK(5U, 4U, 6U)
        {
            INST(11U, Opcode::Phi).s32().Inputs(1U, 8U);
            INST(25U, Opcode::Phi).s32().Inputs(13U, 21U);                                      // a
            INST(26U, Opcode::Phi).s32().Inputs(1U, 22U);                                       // b
            INST(27U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(25U, 0U);  // if (a < 10)
            INST(28U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(27U);
        }
        BASIC_BLOCK(6U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(31U, Opcode::Phi).s32().Inputs(11U, 26U);
            INST(12U, Opcode::Return).s32().Inputs(31U);  // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 45U, 0U));
}

Graph *LoopUnrollTest::BuildGraphInversedCompares2()
{
    // Case 2: if (a >= 10 is false) goto loop
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(13U, 0U).s32();  // a
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0U);  // b = 0
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(13U, 0U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(3U);  // if a < 10 goto loop
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(13U, 7U);  // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 8U);   // b
            INST(7U, Opcode::Add).s32().Inputs(5U, 2U);   // a += 1
            INST(8U, Opcode::Add).s32().Inputs(6U, 7U);   // b += a
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(7U, 0U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(9U);  // if a < 10 goto loop
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).s32().Inputs(1U, 6U);
            INST(12U, Opcode::Return).s32().Inputs(11U);  // return b
        }
    }
    return graph;
}

TEST_F(LoopUnrollTest, InversedCompares2)
{
    auto graph = BuildGraphInversedCompares2();
    static constexpr uint32_t UNROLL_FACTOR = 2;
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        PARAMETER(13U, 0U).s32();  // a
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0U);  // b = 0
        CONSTANT(2U, 1U);  // UNROLL_FACTOR - 1 = 1
                           // NB: add a new constant if UNROLL_FACTOR is changed and fix INST(20, Opcode::Sub).

        BASIC_BLOCK(2U, 3U, 5U)
        {
            // NB: replace the second input if UNROLL_FACTOR is changed:
            INST(20U, Opcode::Sub).s32().Inputs(0U, 2U);
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(13U, 20U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);  // if (a <= 10 -
                                                                                           // UNROLL_FACTOR)
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(13U, 21U);  // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 22U);   // b
            INST(7U, Opcode::Add).s32().Inputs(5U, 2U);    // a + 1
            INST(8U, Opcode::Add).s32().Inputs(6U, 7U);    // b + 1
            INST(21U, Opcode::Add).s32().Inputs(7U, 2U);   // a + 1
            INST(22U, Opcode::Add).s32().Inputs(8U, 21U);  // b + 1
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(21U, 20U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);  // if (a <= 10 -
                                                                                            // UNROLL_FACTOR)
        }
        BASIC_BLOCK(5U, 6U, 4U)
        {
            INST(11U, Opcode::Phi).s32().Inputs(1U, 8U);
            INST(25U, Opcode::Phi).s32().Inputs(13U, 21U);                                      // a
            INST(26U, Opcode::Phi).s32().Inputs(1U, 22U);                                       // b
            INST(27U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(25U, 0U);  // if (a < 10)
            INST(28U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(27U);
        }
        BASIC_BLOCK(6U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(31U, Opcode::Phi).s32().Inputs(11U, 26U);
            INST(12U, Opcode::Return).s32().Inputs(31U);  // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, 45U, 0U));
}

TEST_F(LoopUnrollTest, InversedCompares3)
{
    // Case 3 - if (10 != a) goto loop
    auto graph3 = CreateEmptyGraph();
    GRAPH(graph3)
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0U);  // a = 0, b = 0
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_NE).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(3U);  // if 10 != a goto loop
        }
        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(1U, 7U);  // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 8U);  // b
            INST(7U, Opcode::Add).s32().Inputs(5U, 2U);  // a += 1
            INST(8U, Opcode::Add).s32().Inputs(6U, 7U);  // b += a
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_NE).Inputs(0U, 7U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(9U);  // if 10 != a goto loop
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).s32().Inputs(1U, 6U);
            INST(12U, Opcode::Return).s32().Inputs(11U);  // return b
        }
    }
    static constexpr uint32_t UNROLL_FACTOR = 2;
    graph3->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph3, 45U));
}

TEST_F(LoopUnrollTest, InversedCompares4)
{
    // Case 4 (decrement): if (0 == a) goto out_loop
    auto graph4 = CreateEmptyGraph();
    GRAPH(graph4)
    {
        CONSTANT(0U, 9U);  // a = 9
        CONSTANT(1U, 0U);  // b = 0
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_EQ).Inputs(1U, 0U);  // if 0 == a goto out_loop
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 8U);                                       // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 7U);                                       // b
            INST(7U, Opcode::Add).s32().Inputs(6U, 5U);                                       // b += a
            INST(8U, Opcode::Sub).s32().Inputs(5U, 2U);                                       // a -= 1
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_EQ).Inputs(1U, 8U);  // if 0 == a goto out_loop
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).s32().Inputs(1U, 7U);
            INST(12U, Opcode::Return).s32().Inputs(11U);  // return b
        }
    }
    static constexpr uint32_t UNROLL_FACTOR = 2;
    graph4->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph4, 45U));
}

TEST_F(LoopUnrollTest, LoopWithDifferentConstantsIncrement)
{
    static constexpr uint32_t UNROLL_FACTOR = 2;
    static constexpr uint32_t INC_STOP = 100;

    for (size_t incStep = 1U; incStep <= 10U; incStep++) {
        // CC_LT
        size_t result = 0;
        for (size_t i = 0; i < INC_STOP; i += incStep) {
            result += i;
        }
        auto graph = BuildLoopWithIncrement(CC_LT, ZERO_START, INC_STOP, incStep);
        EXPECT_TRUE(graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR));
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, result));

        // CC_LE
        result = 0;
        for (size_t i = 0; i <= INC_STOP; i += incStep) {
            result += i;
        }
        graph = BuildLoopWithIncrement(CC_LE, ZERO_START, INC_STOP, incStep);
        EXPECT_TRUE(graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR));
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, result));

        // CC_NE
        if (INC_STOP % incStep != 0U) {
            // Otherwise test loop with CC_NE will be infinite
            continue;
        }
        result = 0;
        for (size_t i = 0; i != INC_STOP; i += incStep) {
            result += i;
        }
        graph = BuildLoopWithIncrement(CC_NE, ZERO_START, INC_STOP, incStep);
        EXPECT_TRUE(graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR));
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, result));
    }
}

TEST_F(LoopUnrollTest, LoopWithDifferentConstantsDecrement)
{
    static constexpr uint32_t UNROLL_FACTOR = 2;
    static constexpr uint32_t DEC_START = 100;

    for (size_t decStep = 1U; decStep <= 10U; decStep++) {
        // CC_GT
        int result = 0;
        for (int i = DEC_START; i > 0; i -= decStep) {
            result += i;
        }
        auto graph = BuildLoopWithDecrement(CC_GT, DEC_START, ZERO_STOP, decStep);
        EXPECT_TRUE(graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR));
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, result));

        // CC_GE
        result = 0;
        for (int i = DEC_START; i >= 0; i -= decStep) {
            result += i;
        }
        graph = BuildLoopWithDecrement(CC_GE, DEC_START, ZERO_STOP, decStep);
        EXPECT_TRUE(graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR));
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, result));

        // CC_NE
        if (DEC_START % decStep != 0U) {
            // Otherwise test loop with CC_NE will be infinite
            continue;
        }
        result = 0;
        for (int i = DEC_START; i != 0; i -= decStep) {
            result += i;
        }
        graph = BuildLoopWithDecrement(CC_NE, DEC_START, ZERO_STOP, decStep);
        EXPECT_TRUE(graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR));
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, result));
    }
}

Graph *LoopUnrollTest::BuildGraphPredsInversedOrder()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();  // a
        PARAMETER(1U, 1U).s64();  // b
        CONSTANT(2U, 1U);
        CONSTANT(3U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::Phi).s64().Inputs(1U, 12U);                             // b
            INST(7U, Opcode::Mod).s64().Inputs(6U, 3U);                              // b % 2
            INST(8U, Opcode::If).SrcType(DataType::INT64).CC(CC_EQ).Inputs(7U, 2U);  // if b % 2 == 1
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(10U, Opcode::Mul).s64().Inputs(6U, 6U);  // b = b * b
        }
        BASIC_BLOCK(4U, 2U, 5U)
        {
            INST(12U, Opcode::Phi).s64().Inputs({{3U, 10U}, {2U, 6U}});
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(12U, 0U);  // if b < a
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(15U, Opcode::Return).s64().Inputs(12U);  // return b
        }
    }
    return graph;
}

Graph *LoopUnrollTest::BuildExpectedPredsInversedOrder()
{
    auto expectedGraph = CreateEmptyGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s64();  // a
        PARAMETER(1U, 1U).s64();  // b
        CONSTANT(2U, 1U);
        CONSTANT(3U, 2U);
        BASIC_BLOCK(6U, 2U) {}
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::Phi).s64().Inputs(1U, 19U);                             // b
            INST(7U, Opcode::Mod).s64().Inputs(6U, 3U);                              // b % 2
            INST(8U, Opcode::If).SrcType(DataType::INT64).CC(CC_EQ).Inputs(7U, 2U);  // if b % 2 == 1
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(10U, Opcode::Mul).s64().Inputs(6U, 6U);  // b = b * b
        }
        BASIC_BLOCK(4U, 9U, 8U)
        {
            INST(12U, Opcode::Phi).s64().Inputs({{3U, 10U}, {2U, 6U}});
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(12U, 0U);  // if b < a
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(9U, 10U, 11U)
        {
            INST(16U, Opcode::Mod).s64().Inputs(12U, 3U);                              // b % 2
            INST(17U, Opcode::If).SrcType(DataType::INT64).CC(CC_EQ).Inputs(16U, 2U);  // if b % 2 == 1
        }
        BASIC_BLOCK(10U, 11U)
        {
            INST(18U, Opcode::Mul).s64().Inputs(12U, 12U);  // b = b * b
        }
        BASIC_BLOCK(11U, 2U, 8U)
        {
            INST(19U, Opcode::Phi).s64().Inputs({{9U, 12U}, {10U, 18U}});
            INST(20U, Opcode::Compare).CC(CC_LT).b().Inputs(19U, 0U);  // if b < a
            INST(21U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(20U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(22U, Opcode::Phi).s64().Inputs({{4U, 12U}, {11U, 19U}});
            INST(15U, Opcode::Return).s64().Inputs(22U);  // return b
        }
    }
    return expectedGraph;
}

TEST_F(LoopUnrollTest, PredsInversedOrder)
{
    auto graph = BuildGraphPredsInversedOrder();
    // Swap BB4 preds
    std::swap(BB(4U).GetPredsBlocks()[0U], BB(4U).GetPredsBlocks()[1U]);
    INS(12U).SetInput(0U, &INS(6U));
    INS(12U).SetInput(1U, &INS(10U));
    graph->RunPass<LoopUnroll>(INST_LIMIT, 2U);
    graph->RunPass<Cleanup>();

    auto expectedGraph = BuildExpectedPredsInversedOrder();
    EXPECT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

// NOTE (a.popov) Fix after supporting infinite loops unrolling
TEST_F(LoopUnrollTest, InfiniteLoop)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 2U)
        {
            INST(2U, Opcode::Phi).s32().Inputs(0U, 3U);
            INST(3U, Opcode::Add).s32().Inputs(2U, 1U);
        }
    }
    EXPECT_FALSE(graph->RunPass<LoopUnroll>(1000U, 2U));
}

TEST_F(LoopUnrollTest, PhiDominatesItsPhiInput)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 100U);

        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 7U);
            INST(6U, Opcode::Phi).s32().Inputs(1U, 5U);
            INST(7U, Opcode::Add).s32().Inputs(5U, 6U);  // Fibonacci Sequence
            INST(8U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(7U, 2U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(10U, Opcode::Return).s32().Inputs(7U);
        }
    }

    static constexpr uint64_t PROGRAM_RESULT = 144;
    for (size_t unrollFactor = 2; unrollFactor < 10U; ++unrollFactor) {
        auto clone = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();
        clone->RunPass<LoopUnroll>(INST_LIMIT, unrollFactor);
        EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(clone, PROGRAM_RESULT));
    }
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, PROGRAM_RESULT));
}

TEST_F(LoopUnrollTest, BackEdgeWithoutCompare)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0U);  // a = 0, b = 0
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(1U, 0U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);  // if a < 10
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(1U, 7U);   // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 12U);  // b
            INST(7U, Opcode::Add).s32().Inputs(5U, 2U);   // a++
            INST(8U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(7U, 0U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);  // if a < 10
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(10U, Opcode::Add).s32().Inputs(6U, 2U);  // b++
        }
        BASIC_BLOCK(5U, 3U, 6U)
        {
            INST(11U, Opcode::Phi).s32().Inputs(6U, 10U);                                   // b
            INST(12U, Opcode::Add).s32().Inputs(11U, 7U);                                   // b += a
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);  // if a < 10
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(14U, Opcode::Phi).s32().Inputs(1U, 12U);
            INST(15U, Opcode::Return).s32().Inputs(14U);  // return b
        }
    }
    auto unrolledGraph = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();
    EXPECT_TRUE(unrolledGraph->RunPass<LoopUnroll>(INST_LIMIT, 2U));

    static constexpr uint64_t PROGRAM_RESULT = 64;
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(graph, PROGRAM_RESULT));
    EXPECT_TRUE(CheckRetOnVixlSimulator<uint64_t>(unrolledGraph, PROGRAM_RESULT));
}

TEST_F(LoopUnrollTest, UnrollWithCalls)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();  // a
        PARAMETER(1U, 1U).u64();  // b

        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(4U, Opcode::Phi).u64().Inputs(0U, 6U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).u64().InputsAutoType(20U);
            INST(6U, Opcode::Add).u64().Inputs(4U, 5U);              // a += call()
            INST(7U, Opcode::Compare).CC(CC_LT).b().Inputs(6U, 1U);  // while a < b
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(11U, Opcode::Return).u64().Inputs(6U);  // return a
        }
    }
    static constexpr auto UNROLL_FACTOR = 5U;
    auto defaultIsUnrollWithCalls = g_options.IsCompilerUnrollLoopWithCalls();

    // Enable loop unroll with calls
    g_options.SetCompilerUnrollLoopWithCalls(true);
    auto clone = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();
    EXPECT_TRUE(clone->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR));
    CountOpcodes(clone->GetBlocksRPO());
    EXPECT_EQ(GetOpcodeCount(Opcode::CallStatic), UNROLL_FACTOR);

    // Disable loop unroll with calls
    g_options.SetCompilerUnrollLoopWithCalls(false);
    EXPECT_FALSE(graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR));
    CountOpcodes(graph->GetBlocksRPO());
    EXPECT_EQ(GetOpcodeCount(Opcode::CallStatic), 1U);

    // Return default option
    g_options.SetCompilerUnrollLoopWithCalls(defaultIsUnrollWithCalls);
}

Graph *LoopUnrollTest::BuildGraphAddOverflowUnroll()
{
    // Alwayes make unroll with side exits for AddOverflowCheck instructions
    // Case 1: if (a < 10 is false) goto exit
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0U);  // a = 0, b = 0
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(1U, 0U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(3U);  // if a < 10 goto loop
        }
        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(1U, 7U);  // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 8U);  // b
            INST(20U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::AddOverflowCheck).s32().Inputs(5U, 2U, 20U);  // a += 1
            INST(21U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::AddOverflowCheck).s32().Inputs(6U, 7U, 21U);  // b += a
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(7U, 0U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(9U);  // if a < 10 goto loop
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).s32().Inputs(1U, 6U);
            INST(12U, Opcode::Return).s32().Inputs(11U);  // return b
        }
    }
    return graph;
}

TEST_F(LoopUnrollTest, AddOverflowUnroll)
{
    auto graph = BuildGraphAddOverflowUnroll();
    static constexpr uint32_t UNROLL_FACTOR = 2;
    graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR);
    graph->RunPass<Cleanup>();

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0U);  // a = 0, b = 0
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(1U, 0U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(3U);  // if a < 10 goto loop
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(1U, 17U);  // a
            INST(6U, Opcode::Phi).s32().Inputs(1U, 18U);  // b
            INST(20U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::AddOverflowCheck).s32().Inputs(5U, 2U, 20U);  // a += 1
            INST(21U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::AddOverflowCheck).s32().Inputs(6U, 7U, 21U);  // b += a
            INST(9U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(7U, 0U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(9U);  // if a < 10 goto loop
        }
        BASIC_BLOCK(5U, 4U, 3U)
        {
            INST(30U, Opcode::SaveState).NoVregs();
            INST(17U, Opcode::AddOverflowCheck).s32().Inputs(7U, 2U, 30U);  // a += 1
            INST(31U, Opcode::SaveState).NoVregs();
            INST(18U, Opcode::AddOverflowCheck).s32().Inputs(8U, 17U, 31U);  // b += a
            INST(19U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(17U, 0U);
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(19U);  // if a < 10 goto loop
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).s32().Inputs(1U, 6U, 8U);
            INST(12U, Opcode::Return).s32().Inputs(11U);  // return b
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
}

Graph *LoopUnrollTest::BuildGraphUnrollNeedSaveStateBridge()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();  // a
        PARAMETER(1U, 1U).u64();  // b
        PARAMETER(2U, 2U).ref();  // arr
        CONSTANT(3U, 1U);

        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(4U, Opcode::Phi).u64().Inputs(0U, 6U);
            INST(20U, Opcode::SaveState).Inputs(2U).SrcVregs({0U});
            INST(9U, Opcode::NullCheck).ref().Inputs(2U, 20U);
            INST(5U, Opcode::StoreArray).u64().Inputs(9U, 4U, 4U);   // arr[a] = a
            INST(6U, Opcode::Add).u64().Inputs(4U, 3U);              // a++
            INST(7U, Opcode::Compare).CC(CC_LT).b().Inputs(6U, 1U);  // while a < b
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(12U, Opcode::SaveState).Inputs(2U).SrcVregs({0U});
            INST(13U, Opcode::CallStatic).v0id().InputsAutoType(12U);
            INST(10U, Opcode::LoadArray).u64().Inputs(9U, 1U);
            INST(11U, Opcode::Return).u64().Inputs(10U);  // return arr[b]
        }
    }
    return graph;
}

TEST_F(LoopUnrollTest, UnrollNeedSaveStateBridge)
{
    auto graph = BuildGraphUnrollNeedSaveStateBridge();
    static constexpr auto UNROLL_FACTOR = 2U;
    EXPECT_TRUE(graph->RunPass<LoopUnroll>(INST_LIMIT, UNROLL_FACTOR));
    graph->RunPass<Cleanup>();

    auto graphUnroll = CreateEmptyGraph();
    GRAPH(graphUnroll)
    {
        PARAMETER(0U, 0U).u64();  // a
        PARAMETER(1U, 1U).u64();  // b
        PARAMETER(2U, 2U).ref();  // arr
        CONSTANT(3U, 1U);

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(4U, Opcode::Phi).u64().Inputs(0U, 26U);
            INST(20U, Opcode::SaveState).Inputs(2U).SrcVregs({0U});
            INST(9U, Opcode::NullCheck).ref().Inputs(2U, 20U);
            INST(5U, Opcode::StoreArray).u64().Inputs(9U, 4U, 4U);   // arr[a] = a
            INST(6U, Opcode::Add).u64().Inputs(4U, 3U);              // a++
            INST(7U, Opcode::Compare).CC(CC_LT).b().Inputs(6U, 1U);  // while a < b
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(4U, 2U, 3U)
        {
            INST(23U, Opcode::SaveState).Inputs(2U).SrcVregs({0U});
            INST(24U, Opcode::NullCheck).ref().Inputs(2U, 23U);
            INST(25U, Opcode::StoreArray).u64().Inputs(24U, 6U, 6U);   // arr[a] = a
            INST(26U, Opcode::Add).u64().Inputs(6U, 3U);               // a++
            INST(27U, Opcode::Compare).CC(CC_LT).b().Inputs(26U, 1U);  // while a < b
            INST(28U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(27U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(21U, Opcode::Phi).ref().Inputs(9U, 24U);
            // Phi with check inputs should be added to SaveState
            INST(12U, Opcode::SaveState).Inputs(2U, 21U).SrcVregs({0U, VirtualRegister::BRIDGE});
            INST(13U, Opcode::CallStatic).v0id().InputsAutoType(12U);
            INST(10U, Opcode::LoadArray).u64().Inputs(21U, 1U);
            INST(11U, Opcode::Return).u64().Inputs(10U);  // return arr[b]
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, graphUnroll));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
