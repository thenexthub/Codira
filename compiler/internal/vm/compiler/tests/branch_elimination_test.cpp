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
#include "optimizer/optimizations/branch_elimination.h"
#include "optimizer/optimizations/cleanup.h"

namespace panda::compiler {
enum class RemainedSuccessor { TRUE_SUCCESSOR, FALSE_SUCCESSOR, BOTH };
enum class DominantCondResult { FALSE, TRUE };
enum class SwapInputs { FALSE, TRUE };
enum class SwapCC { FALSE, TRUE };

class BranchEliminationTest : public GraphTest {
public:
    template <uint64_t CONST_CONDITION_BLOCK_ID, uint64_t CONST_VALUE, SwapCC swap_cc = SwapCC::FALSE>
    void BuildTestGraph(Graph *graph);

    template <uint64_t CONST_CONDITION_BLOCK_ID, uint64_t CONST_VALUE>
    void BuildTestGraph2(Graph *graph);

    template <DominantCondResult dom_result, RemainedSuccessor elim_succ, SwapInputs swap_inputs = SwapInputs::FALSE>
    void BuildGraphAndCheckElimination(ConditionCode dominant_code, ConditionCode code);

    template <DominantCondResult dom_result, SwapInputs swap_inputs>
    void BuildContitionsCheckGraph(Graph *graph, ConditionCode dominant_code, ConditionCode code);
    template <DominantCondResult dom_result, SwapInputs swap_inputs>
    void BuildContitionsCheckGraphElimTrueSucc(Graph *graph, ConditionCode dominant_code, ConditionCode code);
    template <DominantCondResult dom_result, SwapInputs swap_inputs>
    void BuildContitionsCheckGraphElimFalseSucc(Graph *graph, ConditionCode dominant_code, ConditionCode code);

protected:
    void InitBlockToBeDisconnected(std::vector<BasicBlock *> &&blocks)
    {
        disconnected_blocks_ = std::move(blocks);
        removed_instructions_.clear();
        for (const auto &block : disconnected_blocks_) {
            for (auto inst : block->AllInsts()) {
                removed_instructions_.push_back(inst);
            }
        }
    }

    void CheckBlocksDisconnected()
    {
        for (const auto &block : disconnected_blocks_) {
            EXPECT_TRUE(block->GetGraph() == nullptr);
            EXPECT_TRUE(block->IsEmpty());
            EXPECT_EQ(block->GetSuccsBlocks().size(), 0U);
            EXPECT_EQ(block->GetPredsBlocks().size(), 0U);
            for (auto inst : block->AllInsts()) {
                EXPECT_TRUE(inst->GetBasicBlock() == nullptr);
                EXPECT_TRUE(inst->GetUsers().Empty());
                for (auto input : inst->GetInputs()) {
                    EXPECT_TRUE(input.GetInst() == nullptr);
                }
            }
        }
    }

private:
    std::vector<BasicBlock *> disconnected_blocks_;
    std::vector<Inst *> removed_instructions_;
};

/*
 *             [0]
 *              |
 *        /----[2]----\
 *        |           |
 *        v           v
 *       [3]    /----[4]----\
 *        |     |           |
 *        |    [5]         [6]
 *        |     |           |
 *        v     v           |
 *      [exit]<-------------/
 */
template <uint64_t CONST_CONDITION_BLOCK_ID, uint64_t CONST_VALUE, SwapCC swap_cc>
void BranchEliminationTest::BuildTestGraph(Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        CONSTANT(3, CONST_VALUE);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(19, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(19);
        }
        BASIC_BLOCK(3, 7)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 1);
            INST(6, Opcode::Add).u64().Inputs(5, 2);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(9, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 2);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(5, 7)
        {
            INST(11, Opcode::Sub).u64().Inputs(0, 1);
            INST(12, Opcode::Sub).u64().Inputs(11, 2);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(14, Opcode::Mul).u64().Inputs(0, 1);
            INST(15, Opcode::Mul).u64().Inputs(14, 2);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(17, Opcode::Phi).u64().Inputs(6, 12, 15);
            INST(18, Opcode::Return).u64().Inputs(17);
        }
    }

    auto inst_if = BB(CONST_CONDITION_BLOCK_ID).GetLastInst();
    ASSERT_TRUE(inst_if->GetOpcode() == Opcode::IfImm);
    inst_if->SetInput(0, &INS(3));

    if constexpr (swap_cc == SwapCC::TRUE) {
        INS(4).CastToIfImm()->SetCc(CC_EQ);
        INS(10).CastToIfImm()->SetCc(CC_EQ);
    }
}

/*
 *              [0]
 *               |
 *        /-----[2]-----\
 *        |             |
 *        |             v
 *        |       /----[4]----\
 *        |       |           |
 *       [3]---->[5]         [6]
 *        |       |           |
 *        v       v           |
 *      [exit]<---------------/
 */
template <uint64_t CONST_CONDITION_BLOCK_ID, uint64_t CONST_VALUE>
void BranchEliminationTest::BuildTestGraph2(Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        CONSTANT(3, CONST_VALUE);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(19, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(19);
        }
        BASIC_BLOCK(3, 7, 5)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 1);
            INST(6, Opcode::Add).u64().Inputs(5, 2);
            INST(20, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 2);
            INST(21, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(20);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(9, Opcode::Compare).b().CC(CC_EQ).Inputs(1, 2);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(5, 7)
        {
            INST(11, Opcode::Phi).u64().Inputs(0, 1);
            INST(12, Opcode::Sub).u64().Inputs(11, 2);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(14, Opcode::Mul).u64().Inputs(0, 1);
            INST(15, Opcode::Mul).u64().Inputs(14, 2);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(17, Opcode::Phi).u64().Inputs(6, 12, 15);
            INST(18, Opcode::Return).u64().Inputs(17);
        }
    }
    auto inst_if = BB(CONST_CONDITION_BLOCK_ID).GetLastInst();
    ASSERT_TRUE(inst_if->GetOpcode() == Opcode::IfImm);
    inst_if->SetInput(0, &INS(3));
}

/*
 *             [0]
 *              |
 *        /----[2]----\
 *        |           |
 *        v           v
 *       [3]    /----[4]----\
 *        |     |           |
 *        |    [5]         [6]
 *        |     |           |
 *        v     v           |
 *      [exit]<-------------/
 *
 *  Disconnect branch: [4, 5, 6]
 */
TEST_F(BranchEliminationTest, DisconnectFalseBranch)
{
    static constexpr uint64_t CONST_CONDITION_BLOCK_ID = 2;
    static constexpr uint64_t CONSTANT_VALUE = 1;
    BuildTestGraph<CONST_CONDITION_BLOCK_ID, CONSTANT_VALUE>(GetGraph());
    InitBlockToBeDisconnected({&BB(4), &BB(5), &BB(6)});

    GetGraph()->RunPass<BranchElimination>();
    CheckBlocksDisconnected();
    EXPECT_EQ(BB(2).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(2).GetSuccessor(0), &BB(3));
    EXPECT_FALSE(INS(17).HasUsers());
    EXPECT_EQ(INS(18).GetInput(0).GetInst(), &INS(6));

    auto graph = CreateEmptyGraph();
    BuildTestGraph<CONST_CONDITION_BLOCK_ID, CONSTANT_VALUE, SwapCC::TRUE>(graph);
    InitBlockToBeDisconnected({&BB(3)});
    graph->RunPass<BranchElimination>();
    CheckBlocksDisconnected();
    EXPECT_EQ(BB(2).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(2).GetSuccessor(0), &BB(4));

    auto phi = &INS(17);
    EXPECT_TRUE(phi->HasUsers());
    EXPECT_EQ(phi->GetInputsCount(), 2U);
    EXPECT_EQ(INS(12).GetUsers().Front().GetInst(), phi);
    EXPECT_EQ(INS(15).GetUsers().Front().GetInst(), phi);
}

/*
 *             [0]
 *              |
 *        /----[2]----\
 *        |           |
 *        v           v
 *       [3]    /----[4]----\
 *        |     |           |
 *        |    [5]         [6]
 *        |     |           |
 *        v     v           |
 *      [exit]<-------------/
 *
 *  Disconnect branch: [3]
 */
TEST_F(BranchEliminationTest, DisconnectTrueBranch)
{
    static constexpr uint64_t CONST_CONDITION_BLOCK_ID = 2;
    static constexpr uint64_t CONSTANT_VALUE = 0;
    BuildTestGraph<CONST_CONDITION_BLOCK_ID, CONSTANT_VALUE>(GetGraph());
    InitBlockToBeDisconnected({&BB(3)});

    GetGraph()->RunPass<BranchElimination>();
    CheckBlocksDisconnected();
    EXPECT_EQ(BB(2).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(2).GetSuccessor(0), &BB(4));

    auto phi = &INS(17);
    EXPECT_TRUE(phi->HasUsers());
    EXPECT_EQ(phi->GetInputsCount(), 2U);
    EXPECT_EQ(INS(12).GetUsers().Front().GetInst(), phi);
    EXPECT_EQ(INS(15).GetUsers().Front().GetInst(), phi);

    auto graph = CreateEmptyGraph();
    BuildTestGraph<CONST_CONDITION_BLOCK_ID, CONSTANT_VALUE, SwapCC::TRUE>(graph);
    InitBlockToBeDisconnected({&BB(4), &BB(5), &BB(6)});

    graph->RunPass<BranchElimination>();
    CheckBlocksDisconnected();
    EXPECT_EQ(BB(2).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(2).GetSuccessor(0), &BB(3));
    EXPECT_FALSE(INS(17).HasUsers());
    EXPECT_EQ(INS(18).GetInput(0).GetInst(), &INS(6));
}

/*
 *             [0]
 *              |
 *        /----[2]----\
 *        |           |
 *        v           v
 *       [3]    /----[4]----\
 *        |     |           |
 *        |    [5]         [6]
 *        |     |           |
 *        v     v           |
 *      [exit]<-------------/
 *
 *  Disconnect branch: [6]
 */
TEST_F(BranchEliminationTest, DisconnectInnerFalseBranch)
{
    static constexpr uint64_t CONST_CONDITION_BLOCK_ID = 4;
    static constexpr uint64_t CONSTANT_VALUE = 1;
    BuildTestGraph<CONST_CONDITION_BLOCK_ID, CONSTANT_VALUE>(GetGraph());
    InitBlockToBeDisconnected({&BB(6)});

    GetGraph()->RunPass<BranchElimination>();
    CheckBlocksDisconnected();
    EXPECT_EQ(BB(4).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(4).GetSuccessor(0), &BB(5));

    auto phi = &INS(17);
    EXPECT_TRUE(phi->HasUsers());
    EXPECT_EQ(phi->GetInputsCount(), 2U);
    EXPECT_EQ(INS(6).GetUsers().Front().GetInst(), phi);
    EXPECT_EQ(INS(12).GetUsers().Front().GetInst(), phi);
}

/*
 *             [0]
 *              |
 *        /----[2]----\
 *        |           |
 *        v           v
 *       [3]    /----[4]----\
 *        |     |           |
 *        |    [5]         [6]
 *        |     |           |
 *        v     v           |
 *      [exit]<-------------/
 *
 *  Disconnect branch: [5]
 */
TEST_F(BranchEliminationTest, DisconnectInnerTrueBranch)
{
    static constexpr uint64_t CONST_CONDITION_BLOCK_ID = 4;
    static constexpr uint64_t CONSTANT_VALUE = 0;
    BuildTestGraph<CONST_CONDITION_BLOCK_ID, CONSTANT_VALUE>(GetGraph());
    InitBlockToBeDisconnected({&BB(5)});

    GetGraph()->RunPass<BranchElimination>();
    CheckBlocksDisconnected();
    EXPECT_EQ(BB(4).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(4).GetSuccessor(0), &BB(6));

    auto phi = &INS(17);
    EXPECT_TRUE(phi->HasUsers());
    EXPECT_EQ(phi->GetInputsCount(), 2U);
    EXPECT_EQ(INS(6).GetUsers().Front().GetInst(), phi);
    EXPECT_EQ(INS(15).GetUsers().Front().GetInst(), phi);
}

/*
 *              [0]
 *               |
 *        /-----[2]-----\
 *        |             |
 *        |             v
 *        |       /----[4]----\
 *        |       |           |
 *       [3]---->[5]         [6]
 *        |       |           |
 *        v       v           |
 *      [exit]<---------------/
 *
 *  Disconnect branch: [4, 6]
 */
TEST_F(BranchEliminationTest, RemoveBranchPart)
{
    static constexpr uint64_t CONST_CONDITION_BLOCK_ID = 2;
    static constexpr uint64_t CONSTANT_VALUE = 1;
    BuildTestGraph2<CONST_CONDITION_BLOCK_ID, CONSTANT_VALUE>(GetGraph());
    InitBlockToBeDisconnected({&BB(4), &BB(6)});

    GetGraph()->RunPass<BranchElimination>();
    CheckBlocksDisconnected();
    EXPECT_EQ(BB(2).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(2).GetSuccessor(0), &BB(3));
    EXPECT_EQ(BB(5).GetPredsBlocks().size(), 1U);
    EXPECT_EQ(BB(5).GetPredBlockByIndex(0), &BB(3));

    auto phi = &INS(17);
    EXPECT_TRUE(phi->HasUsers());
    EXPECT_EQ(phi->GetInputsCount(), 2U);
    EXPECT_EQ(INS(6).GetUsers().Front().GetInst(), phi);
    EXPECT_EQ(INS(12).GetUsers().Front().GetInst(), phi);
}

/*
 *              [0]
 *               |
 *        /-----[2]-----\
 *        |             |
 *        |             v
 *        |       /----[4]----\
 *        |       |           |
 *       [3]---->[5]         [6]
 *        |       |           |
 *        v       v           |
 *      [exit]<---------------/
 *
 *  Remove edge between [4] and [5]
 */
TEST_F(BranchEliminationTest, RemoveEdge)
{
    static constexpr uint64_t CONST_CONDITION_BLOCK_ID = 4;
    static constexpr uint64_t CONSTANT_VALUE = 0;
    BuildTestGraph2<CONST_CONDITION_BLOCK_ID, CONSTANT_VALUE>(GetGraph());

    GetGraph()->RunPass<BranchElimination>();

    EXPECT_EQ(BB(4).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(4).GetSuccessor(0), &BB(6));
    EXPECT_EQ(BB(5).GetPredsBlocks().size(), 1U);
    EXPECT_EQ(BB(5).GetPredBlockByIndex(0), &BB(3));

    auto phi = &INS(17);
    EXPECT_TRUE(phi->HasUsers());
    EXPECT_EQ(phi->GetInputsCount(), 3U);
    EXPECT_EQ(INS(6).GetUsers().Front().GetInst(), phi);
    EXPECT_EQ(INS(12).GetUsers().Front().GetInst(), phi);
    EXPECT_EQ(INS(15).GetUsers().Front().GetInst(), phi);
}

/*
 *              [0]
 *               |
 *        /-----[2]-----\
 *        |             |
 *        |             v
 *        |       /----[4]----\
 *        |       |           |
 *       [3]---->[5]         [6]
 *        |       |           |
 *        v       v           |
 *      [exit]<---------------/
 *
 *  Remove branches [3-5] and [4-5]
 *
 *              [0]
 *               |
 *              [2]
 *               |
 *              [4]
 *               |
 *              [6]
 *               |
 *             [exit]
 */
TEST_F(BranchEliminationTest, RemoveEdgeAndWholeBlock)
{
    static constexpr uint64_t CONST_CONDITION_BLOCK_ID = 2;
    static constexpr uint64_t CONSTANT_VALUE = 0;
    BuildTestGraph2<CONST_CONDITION_BLOCK_ID, CONSTANT_VALUE>(GetGraph());
    BB(4).GetLastInst()->SetInput(0, &INS(3));
    InitBlockToBeDisconnected({&BB(3), &BB(5)});

    GetGraph()->RunPass<BranchElimination>();
    CheckBlocksDisconnected();

    EXPECT_EQ(BB(0).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(2).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(4).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(6).GetSuccsBlocks().size(), 1U);

    auto phi = &INS(17);
    EXPECT_FALSE(phi->HasUsers());
}

/*
 *                  [0]
 *                   |
 *            /-----[2]-----\
 *            |             |
 *            v             v
 *      /----[3]----\  /----[4]----\
 *      |           |  |           |
 *     [5]         [6][7]         [8]
 *      |           |  |           |
 *      |           v  v           |
 *      |           [9]            |
 *      |            v             |
 *      \--------->[exit]<---------/
 *
 * Remove branches [3-6], [4-7] and as a result remove block [9]
 */
TEST_F(BranchEliminationTest, DisconnectPredecessors)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 0);
        CONSTANT(4, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 5, 6)
        {
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(4, 7, 8)
        {
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(6, 9) {}
        BASIC_BLOCK(7, 9) {}
        BASIC_BLOCK(5, 10)
        {
            INST(10, Opcode::Add).u64().Inputs(1, 2);
        }
        BASIC_BLOCK(8, 10)
        {
            INST(12, Opcode::Sub).u64().Inputs(1, 2);
        }
        BASIC_BLOCK(9, 10)
        {
            INST(14, Opcode::Mul).u64().Inputs(1, 2);
        }
        BASIC_BLOCK(10, -1)
        {
            INST(16, Opcode::Phi).u64().Inputs(10, 12, 14);
            INST(17, Opcode::Return).u64().Inputs(16);
        }
    }

    InitBlockToBeDisconnected({&BB(6), &BB(7), &BB(9)});

    GetGraph()->RunPass<BranchElimination>();
    CheckBlocksDisconnected();

    EXPECT_EQ(BB(3).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(3).GetSuccessor(0), &BB(5));
    EXPECT_EQ(BB(4).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(4).GetSuccessor(0), &BB(8));
    EXPECT_EQ(BB(10).GetPredsBlocks().size(), 2U);

    auto phi = &INS(16);
    EXPECT_EQ(phi->GetInputsCount(), 2U);
    EXPECT_EQ(INS(10).GetUsers().Front().GetInst(), phi);
    EXPECT_EQ(INS(12).GetUsers().Front().GetInst(), phi);
}

/*
 *           [0]
 *            |
 *            v
 *      /--->[2]----\
 *      |     |     |
 *      |     v     v
 *      \----[3]  [exit]
 *
 * Remove [3]
 *
 * [0] -> [2] -> [exit]
 *
 */
TEST_F(BranchEliminationTest, RemoveLoopBackEdge)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 0);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Phi).u64().Inputs(1, 9);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(8, Opcode::Add).u64().Inputs(5, 2);
            INST(9, Opcode::Add).u64().Inputs(8, 2);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Return).u64().Inputs(5);
        }
    }

    InitBlockToBeDisconnected({&BB(3)});

    GetGraph()->RunPass<BranchElimination>();
    CheckBlocksDisconnected();
    EXPECT_EQ(BB(0).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(2).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(INS(11).GetInput(0).GetInst(), &INS(1));
}

/*
 *           [0]
 *            |
 *            v
 *      /--->[2]
 *      |     | \
 *      |     |  \
 *      \----/    \
 *                 |
 *                 v
 *               [exit]
 *
 * Remove loop edge [2->2]
 *
 * [0] -> [2] -> [exit]
 */
TEST_F(BranchEliminationTest, RemoveOneBlockLoopExit)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 0);

        BASIC_BLOCK(2, 2, 4)
        {
            INST(5, Opcode::Phi).u64().Inputs(1, 9);
            INST(8, Opcode::Add).u64().Inputs(5, 2);
            INST(9, Opcode::Add).u64().Inputs(8, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Return).u64().Inputs(9);
        }
    }

    GetGraph()->RunPass<BranchElimination>();

    EXPECT_EQ(BB(0).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(2).GetSuccsBlocks().size(), 1U);

    EXPECT_FALSE(INS(5).HasUsers());
    EXPECT_TRUE(INS(8).HasUsers());
    EXPECT_TRUE(INS(9).HasUsers());
    EXPECT_EQ(INS(8).GetInput(0).GetInst(), &INS(1));
}

/*
 *           [0]
 *            |
 *            v
 *      /--->[2]
 *      |     |
 *      |     v
 *      \----[3]----\
 *                  |
 *                  v
 *                [exit]
 *
 * Remove edge [3-2]
 *
 * [0] -> [2] -> [3] -> [exit]
 */
TEST_F(BranchEliminationTest, RemoveLoopExit)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 0);

        BASIC_BLOCK(2, 5, 6)
        {
            INST(5, Opcode::Phi).u64().Inputs(1, 9);
            INST(20, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(4, Opcode::If).SrcType(DataType::Type::UINT64).CC(CC_NE).Inputs(1, 2);
        }
        BASIC_BLOCK(5, 3)
        {
            INST(21, Opcode::SaveState).NoVregs();
            INST(10, Opcode::CallStatic).v0id().InputsAutoType(21);
        }
        BASIC_BLOCK(6, 3)
        {
            INST(22, Opcode::SaveState).NoVregs();
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(22);
        }
        BASIC_BLOCK(3, 2, 4)
        {
            INST(8, Opcode::Add).u64().Inputs(5, 2);
            INST(9, Opcode::Add).u64().Inputs(8, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(12, Opcode::Return).u64().Inputs(9);
        }
    }

    GetGraph()->RunPass<BranchElimination>();

    EXPECT_EQ(BB(0).GetSuccsBlocks().size(), 1U);
    EXPECT_EQ(BB(2).GetSuccsBlocks().size(), 2U);
    EXPECT_EQ(BB(3).GetSuccsBlocks().size(), 1U);

    EXPECT_FALSE(INS(5).HasUsers());
    EXPECT_TRUE(INS(8).HasUsers());
    EXPECT_TRUE(INS(9).HasUsers());
    EXPECT_EQ(INS(8).GetInput(0).GetInst(), &INS(1));
}

/*
 *              [0]
 *            T  |  F
 *        /-----[2]-----\
 *        |             |
 *        v             v
 *       [3]<----------[4]<----\
 *        |             |      |
 *        v             v      |
 *      [exit]         [5]-----/
 *
 * Transform to [0]-->[2]-->[exit]
 */
TEST_F(BranchEliminationTest, RemoveEdgeToLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).b();
        CONSTANT(3, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::Phi).u64().Inputs(0, 7);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
        BASIC_BLOCK(4, 5, 3)
        {
            INST(7, Opcode::Phi).u64().Inputs({{2, 0}, {5, 9}});
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(5, 4)
        {
            INST(9, Opcode::Add).u64().Inputs(7, 0);
        }
    }
    GetGraph()->RunPass<BranchElimination>();
    GetGraph()->RunPass<Cleanup>();

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).u64().Inputs(0);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected_graph));
}

/*
 *             [0]
 *           T  |  F
 *        /----[2]----\
 *        |           |
 *        v        T  v  F
 *       [3]    /----[4]----\
 *        |     |           |
 *        |    [5]         [6]
 *        |     |           |
 *        v     v           |
 *      [exit]<-------------/
 */
template <DominantCondResult dom_result, SwapInputs swap_inputs>
void BranchEliminationTest::BuildContitionsCheckGraph(Graph *graph, ConditionCode dominant_code, ConditionCode code)
{
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(19, Opcode::Compare).b().CC(dominant_code).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(19);
        }
        BASIC_BLOCK(3, 7)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 1);
            INST(6, Opcode::Add).u64().Inputs(5, 2);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(9, Opcode::Compare).b().CC(code).Inputs(0, 1);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(5, 7)
        {
            INST(11, Opcode::Sub).u64().Inputs(0, 1);
            INST(12, Opcode::Sub).u64().Inputs(11, 2);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(14, Opcode::Mul).u64().Inputs(0, 1);
            INST(15, Opcode::Mul).u64().Inputs(14, 2);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(17, Opcode::Phi).u64().Inputs(6, 12, 15);
            INST(18, Opcode::Return).u64().Inputs(17);
        }
    }
    if constexpr (dom_result == DominantCondResult::TRUE) {
        BB(2).SwapTrueFalseSuccessors();
    }
    if constexpr (swap_inputs == SwapInputs::TRUE) {
        INS(19).SwapInputs();
    }
}

/*
 *             [0]
 *              |
 *        /----[2]----\
 *        |           |
 *        v           v
 *       [3]         [4]
 *        |           |
 *        |          [5]
 *        |           |
 *        v           |
 *      [exit]<-------/
 */
template <DominantCondResult dom_result, SwapInputs swap_inputs>
void BranchEliminationTest::BuildContitionsCheckGraphElimFalseSucc(Graph *graph, ConditionCode dominant_code,
                                                                   ConditionCode code)
{
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(19, Opcode::Compare).b().CC(dominant_code).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(19);
        }
        BASIC_BLOCK(3, 7)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 1);
            INST(6, Opcode::Add).u64().Inputs(5, 2);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(9, Opcode::Compare).b().CC(code).Inputs(0, 1);
        }
        BASIC_BLOCK(5, 7)
        {
            INST(11, Opcode::Sub).u64().Inputs(0, 1);
            INST(12, Opcode::Sub).u64().Inputs(11, 2);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(17, Opcode::Phi).u64().Inputs(6, 12);
            INST(18, Opcode::Return).u64().Inputs(17);
        }
    }
    if constexpr (dom_result == DominantCondResult::TRUE) {
        BB(2).SwapTrueFalseSuccessors();
    }
    if constexpr (swap_inputs == SwapInputs::TRUE) {
        INS(19).SwapInputs();
    }
}

/*
 *             [0]
 *              |
 *        /----[2]----\
 *        |           |
 *        v           v
 *       [3]         [4]----\
 *        |                 |
 *        |                [6]
 *        |                 |
 *        v                 |
 *      [exit]<-------------/
 */
template <DominantCondResult dom_result, SwapInputs swap_inputs>
void BranchEliminationTest::BuildContitionsCheckGraphElimTrueSucc(Graph *graph, ConditionCode dominant_code,
                                                                  ConditionCode code)
{
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(19, Opcode::Compare).b().CC(dominant_code).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(19);
        }
        BASIC_BLOCK(3, 7)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 1);
            INST(6, Opcode::Add).u64().Inputs(5, 2);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(9, Opcode::Compare).b().CC(code).Inputs(0, 1);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(14, Opcode::Mul).u64().Inputs(0, 1);
            INST(15, Opcode::Mul).u64().Inputs(14, 2);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(17, Opcode::Phi).u64().Inputs(6, 15);
            INST(18, Opcode::Return).u64().Inputs(17);
        }
    }
    if constexpr (dom_result == DominantCondResult::TRUE) {
        BB(2).SwapTrueFalseSuccessors();
    }
    if constexpr (swap_inputs == SwapInputs::TRUE) {
        INS(19).SwapInputs();
    }
}

template <DominantCondResult dom_result, RemainedSuccessor remained_succ, SwapInputs swap_inputs>
void BranchEliminationTest::BuildGraphAndCheckElimination(ConditionCode dominant_code, ConditionCode code)
{
    auto graph = CreateEmptyGraph();
    BuildContitionsCheckGraph<dom_result, swap_inputs>(graph, dominant_code, code);
    auto expected_graph = CreateEmptyGraph();
    if constexpr (remained_succ == RemainedSuccessor::FALSE_SUCCESSOR) {
        BuildContitionsCheckGraphElimTrueSucc<dom_result, swap_inputs>(expected_graph, dominant_code, code);
    } else if constexpr (remained_succ == RemainedSuccessor::TRUE_SUCCESSOR) {
        BuildContitionsCheckGraphElimFalseSucc<dom_result, swap_inputs>(expected_graph, dominant_code, code);
    } else {
        BuildContitionsCheckGraph<dom_result, swap_inputs>(expected_graph, dominant_code, code);
    }

    graph->RunPass<BranchElimination>();
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
}

TEST_F(BranchEliminationTest, EliminateByDominatedCondition)
{
    // dominant condition:  a op1 b,
    // dominated condition: a op2 b (reached from false successor)
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_EQ,
                                                                                               ConditionCode::CC_EQ);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_EQ,
                                                                                                ConditionCode::CC_NE);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_EQ,
                                                                                                ConditionCode::CC_LT);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_EQ,
                                                                                               ConditionCode::CC_LE);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_EQ,
                                                                                                ConditionCode::CC_GT);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_EQ,
                                                                                               ConditionCode::CC_GE);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_EQ,
                                                                                                ConditionCode::CC_B);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_EQ,
                                                                                               ConditionCode::CC_BE);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_EQ,
                                                                                                ConditionCode::CC_A);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_EQ,
                                                                                               ConditionCode::CC_AE);

    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_EQ,
                                                                                                 ConditionCode::CC_EQ);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_EQ,
                                                                                                ConditionCode::CC_NE);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_EQ,
                                                                                      ConditionCode::CC_LT);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_EQ,
                                                                                      ConditionCode::CC_LE);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_EQ,
                                                                                      ConditionCode::CC_GT);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_EQ,
                                                                                      ConditionCode::CC_GE);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_EQ,
                                                                                      ConditionCode::CC_B);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_EQ,
                                                                                      ConditionCode::CC_BE);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_EQ,
                                                                                      ConditionCode::CC_A);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_EQ,
                                                                                      ConditionCode::CC_AE);

    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_LT,
                                                                                                ConditionCode::CC_EQ);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_LT,
                                                                                               ConditionCode::CC_NE);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_LT,
                                                                                               ConditionCode::CC_LT);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_LT,
                                                                                               ConditionCode::CC_LE);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_LT,
                                                                                                ConditionCode::CC_GT);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_LT,
                                                                                                ConditionCode::CC_GE);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::BOTH>(ConditionCode::CC_LT,
                                                                                     ConditionCode::CC_B);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::BOTH>(ConditionCode::CC_LT,
                                                                                     ConditionCode::CC_BE);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::BOTH>(ConditionCode::CC_LT,
                                                                                     ConditionCode::CC_A);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::BOTH>(ConditionCode::CC_LT,
                                                                                     ConditionCode::CC_AE);

    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_LT,
                                                                                      ConditionCode::CC_EQ);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_LT,
                                                                                      ConditionCode::CC_NE);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_LT,
                                                                                                 ConditionCode::CC_LT);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_LT,
                                                                                      ConditionCode::CC_LE);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_LT,
                                                                                      ConditionCode::CC_GT);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_LT,
                                                                                                ConditionCode::CC_GE);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_LT,
                                                                                      ConditionCode::CC_B);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_LT,
                                                                                      ConditionCode::CC_BE);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_LT,
                                                                                      ConditionCode::CC_A);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_LT,
                                                                                      ConditionCode::CC_AE);

    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::BOTH>(ConditionCode::CC_LE,
                                                                                     ConditionCode::CC_EQ);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::BOTH>(ConditionCode::CC_LE,
                                                                                     ConditionCode::CC_NE);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::BOTH>(ConditionCode::CC_LE,
                                                                                     ConditionCode::CC_LT);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_LE,
                                                                                               ConditionCode::CC_LE);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_LE,
                                                                                                ConditionCode::CC_GT);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::BOTH>(ConditionCode::CC_LE,
                                                                                     ConditionCode::CC_GE);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::BOTH>(ConditionCode::CC_LE,
                                                                                     ConditionCode::CC_B);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::BOTH>(ConditionCode::CC_LE,
                                                                                     ConditionCode::CC_BE);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::BOTH>(ConditionCode::CC_LE,
                                                                                     ConditionCode::CC_A);
    BuildGraphAndCheckElimination<DominantCondResult::TRUE, RemainedSuccessor::BOTH>(ConditionCode::CC_LE,
                                                                                     ConditionCode::CC_AE);

    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_LE,
                                                                                                 ConditionCode::CC_EQ);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_LE,
                                                                                                ConditionCode::CC_NE);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_LE,
                                                                                                 ConditionCode::CC_LT);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::FALSE_SUCCESSOR>(ConditionCode::CC_LE,
                                                                                                 ConditionCode::CC_LE);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_LE,
                                                                                                ConditionCode::CC_GT);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::TRUE_SUCCESSOR>(ConditionCode::CC_LE,
                                                                                                ConditionCode::CC_GE);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_LE,
                                                                                      ConditionCode::CC_B);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_LE,
                                                                                      ConditionCode::CC_BE);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_LE,
                                                                                      ConditionCode::CC_A);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH>(ConditionCode::CC_LE,
                                                                                      ConditionCode::CC_AE);

    // dominant condition:  b op1 a,
    // dominated condition: a op2 b (reached from false successor)
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH, SwapInputs::TRUE>(
        ConditionCode::CC_GT, ConditionCode::CC_EQ);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::FALSE_SUCCESSOR, SwapInputs::TRUE>(
        ConditionCode::CC_GE, ConditionCode::CC_EQ);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::BOTH, SwapInputs::TRUE>(
        ConditionCode::CC_LT, ConditionCode::CC_EQ);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::FALSE_SUCCESSOR, SwapInputs::TRUE>(
        ConditionCode::CC_LE, ConditionCode::CC_EQ);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::FALSE_SUCCESSOR, SwapInputs::TRUE>(
        ConditionCode::CC_EQ, ConditionCode::CC_EQ);
    BuildGraphAndCheckElimination<DominantCondResult::FALSE, RemainedSuccessor::TRUE_SUCCESSOR, SwapInputs::TRUE>(
        ConditionCode::CC_NE, ConditionCode::CC_EQ);
}

TEST_F(BranchEliminationTest, CascadeElimination)
{
    /*
     * Case 1
     *
     *             [0]
     *          T   |  F
     *        /----[2]----\
     *        |           |
     *        v        T  v  F
     *       [3]    /----[4]----\
     *        |     |       T   |  F
     *        |    [5]    /----[6]----\
     *        |     |     |           |
     *        |     |    [7]         [8]
     *        v     v     v           v
     *      [exit]<-------------------/
     *
     *  ---->
     *
     *             [0]
     *              |
     *        /----[2]----\
     *        |           |
     *        v           v
     *       [3]         [4]
     *        |           |
     *        |          [6]
     *        |           |
     *        |          [8]
     *        v           |
     *      [exit]<-------/
     */

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 9)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(7, Opcode::Compare).b().CC(CC_EQ).Inputs(1, 0);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(5, 9)
        {
            INST(9, Opcode::Sub).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(6, 7, 8)
        {
            INST(11, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(7, 9)
        {
            INST(13, Opcode::Mul).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(8, 9)
        {
            INST(15, Opcode::Div).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(17, Opcode::Phi).u64().Inputs(5, 9, 13, 15);
            INST(18, Opcode::Return).u64().Inputs(17);
        }
    }
    graph->RunPass<BranchElimination>();
    graph->RunPass<Cleanup>();

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, 3, 8)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 9)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(8, 9)
        {
            INST(15, Opcode::Div).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(17, Opcode::Phi).u64().Inputs(5, 15);
            INST(18, Opcode::Return).u64().Inputs(17);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));

    /*
     * Case 2
     *
     *             [0]
     *          F   |  T
     *        /----[2]----\
     *        |           |
     *        v        T  v  F
     *       [3]    /----[4]----\
     *        |     |       T   |  F
     *        |    [5]    /----[6]----\
     *        |     |     |           |
     *        |     |    [7]         [8]
     *        v     v     v           v
     *      [exit]<-------------------/
     *
     * ---->
     *
     *             [0]
     *          T   |  F
     *        /----[2]----\
     *        |           |
     *        v           v
     *       [3]         [4]
     *        |           |
     *        |          [5]
     *        |           |
     *        |           |
     *        v           |
     *      [exit]<-------/
     *
     */
    auto graph_case2 = CreateEmptyGraph();
    GRAPH(graph_case2)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, 4, 3)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 9)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(7, Opcode::Compare).b().CC(CC_EQ).Inputs(1, 0);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(5, 9)
        {
            INST(9, Opcode::Sub).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(6, 7, 8)
        {
            INST(11, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(7, 9)
        {
            INST(13, Opcode::Mul).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(8, 9)
        {
            INST(15, Opcode::Div).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(17, Opcode::Phi).u64().Inputs(5, 9, 13, 15);
            INST(18, Opcode::Return).u64().Inputs(17);
        }
    }
    graph_case2->RunPass<BranchElimination>();
    graph_case2->RunPass<Cleanup>();

    auto expected_graph2 = CreateEmptyGraph();
    GRAPH(expected_graph2)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, 5, 3)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 9)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(5, 9)
        {
            INST(9, Opcode::Sub).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(17, Opcode::Phi).u64().Inputs(5, 9);
            INST(18, Opcode::Return).u64().Inputs(17);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph_case2, expected_graph2));
}

TEST_F(BranchEliminationTest, ConditionEliminationNotApplied)
{
    /*
     * Case 1:
     *             [0]
     *              |
     *        /----[2]----\
     *        |           |
     *        v           v
     *       [3]         [4]
     *        |           |
     *        \-----------/
     *              |
     *        /----[6]----\
     *        |           |
     *       [7]         [8]
     *        |           |
     *        v           |
     *      [exit]<-------/
     */
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 6)
        {
            INST(5, Opcode::Add).u64().Inputs(1, 2);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(7, Opcode::Sub).u64().Inputs(1, 2);
        }
        BASIC_BLOCK(6, 7, 8)
        {
            INST(9, Opcode::Phi).Inputs(5, 7).u64();
            INST(10, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(7, 9)
        {
            INST(12, Opcode::Add).u64().Inputs(9, 1);
        }
        BASIC_BLOCK(8, 9)
        {
            INST(14, Opcode::Sub).u64().Inputs(9, 1);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(16, Opcode::Phi).u64().Inputs(12, 14);
            INST(17, Opcode::Return).u64().Inputs(16);
        }
    }
    graph->RunPass<BranchElimination>();
    EXPECT_EQ(BB(6).GetSuccsBlocks().size(), 2U);

    /*
     * Case 2
     *             [0]
     *              |
     *        /----[2]----\
     *        |           |
     *        |           |
     *       [3]--------->|
     *                    v
     *              /----[4]----\
     *              |           |
     *             [5]         [6]
     *              |           |
     *              v           |
     *            [exit]<-------/
     */
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(5, Opcode::Add).u64().Inputs(1, 2);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(7, Opcode::Phi).Inputs(1, 5).u64();
            INST(8, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(5, 7)
        {
            INST(10, Opcode::Add).u64().Inputs(7, 1);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(12, Opcode::Sub).u64().Inputs(7, 1);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(14, Opcode::Phi).u64().Inputs(10, 12);
            INST(15, Opcode::Return).u64().Inputs(14);
        }
    }
    graph2->RunPass<BranchElimination>();
    EXPECT_EQ(BB(4).GetSuccsBlocks().size(), 2U);
}

/*
 *             [0]
 *              |
 *        /----[2]----\
 *        |           |
 *        |           v
 *        |          [3]
 *        |           |
 *        |           v
 *        \--------->[4]
 *                    |
 *                    v
 *              /----[5]----\
 *              |           |
 *             [6]         [7]
 *              |           |
 *              v           |
 *            [exit]<-------/
 *
 */
TEST_F(BranchEliminationTest, DomBothSuccessorsReachTargetBlock)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 0);
        }
        BASIC_BLOCK(4, 6, 7)
        {
            INST(7, Opcode::Phi).u64().Inputs({{2, 0}, {3, 5}});
            INST(8, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(6, 8)
        {
            INST(10, Opcode::Add).u64().Inputs(7, 1);
        }
        BASIC_BLOCK(7, 8)
        {
            INST(12, Opcode::Sub).u64().Inputs(7, 1);
        }
        BASIC_BLOCK(8, -1)
        {
            INST(14, Opcode::Phi).u64().Inputs(10, 12);
            INST(15, Opcode::Return).u64().Inputs(14);
        }
    }
    graph->RunPass<BranchElimination>();
    // Elimination NOT applied
    EXPECT_EQ(BB(4).GetGraph(), graph);
    EXPECT_EQ(BB(4).GetSuccsBlocks().size(), 2U);
}

TEST_F(BranchEliminationTest, CreateInfiniteLoop)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 2, 3)
        {
            INST(2, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(4, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(graph->HasEndBlock());
    graph->RunPass<BranchElimination>();
    ASSERT_FALSE(graph->HasEndBlock());

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        CONSTANT(1, 1);
        BASIC_BLOCK(2, 3)
        {  // pre-header
        }
        BASIC_BLOCK(3, 3)
        {  // infinite loop
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
}

/**
 *          compare1
 *            ...
 *        if_imm(compare1)
 */
TEST_F(BranchEliminationTest, CompareAndIfNotSameBlock)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 3)
        {
            INST(4, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(5, Opcode::Phi).s64().Inputs(2, 8);
            INST(6, Opcode::Compare).b().CC(CC_LT).Inputs(5, 0);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(8, Opcode::Add).s64().Inputs(5, 3);
        }
        BASIC_BLOCK(5, 6, 7)
        {
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }  // if INS(4) != 0 goto BB(6) else goto BB(7)
        BASIC_BLOCK(6, 12)
        {
            INST(10, Opcode::Mul).u64().Inputs(0, 1);
        }
        BASIC_BLOCK(7, 8, 9)
        {
            INST(11, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);  // equal to INS(4) -> INS(11) == 0
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(11);
        }  // INS(11) == 0 is true -> goto BB(8), remove BB(9)
        BASIC_BLOCK(8, 10, 11)
        {
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }  // INS(11) == 1 is false -> goto BB(11), remove BB(10)
        BASIC_BLOCK(9, 12)
        {
            INST(15, Opcode::Sub).u64().Inputs(0, 1);
        }
        BASIC_BLOCK(10, 12)
        {
            INST(16, Opcode::Mul).u64().Inputs(1, 1);
        }
        BASIC_BLOCK(11, 12)
        {
            INST(17, Opcode::Add).u64().Inputs(2, 2);
        }
        BASIC_BLOCK(12, -1)
        {
            INST(18, Opcode::Phi).u64().Inputs(10, 15, 16, 17);
            INST(19, Opcode::Return).u64().Inputs(18);
        }
    }
    graph->RunPass<BranchElimination>();
    graph->RunPass<Cleanup>();

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        CONSTANT(3, 0);

        BASIC_BLOCK(2, 3)
        {
            INST(4, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(5, Opcode::Phi).s64().Inputs(2, 8);
            INST(6, Opcode::Compare).b().CC(CC_LT).Inputs(5, 0);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(8, Opcode::Add).s64().Inputs(5, 3);
        }
        BASIC_BLOCK(5, 6, 11)
        {
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(6, 12)
        {
            INST(10, Opcode::Mul).u64().Inputs(0, 1);
        }
        BASIC_BLOCK(11, 12)
        {
            INST(17, Opcode::Add).u64().Inputs(2, 2);
        }
        BASIC_BLOCK(12, -1)
        {
            INST(18, Opcode::Phi).u64().Inputs(10, 17);
            INST(19, Opcode::Return).u64().Inputs(18);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
}

TEST_F(BranchEliminationTest, DisconnectPhiWithInputItself)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(2, 10);
        PARAMETER(3, 0).s64();
        BASIC_BLOCK(2, 10, 4)
        {
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(4, 7)
        {
            INST(5, Opcode::Mul).s64().Inputs(2, 2);
        }
        BASIC_BLOCK(7, 8, 10)
        {
            INST(6, Opcode::Phi).s64().Inputs(0, 12, 13);
            INST(7, Opcode::Phi).s64().Inputs(5, 6, 7);
            INST(8, Opcode::Compare).b().CC(CC_LT).Inputs(6, 7);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(8, 5, 6)
        {
            INST(10, Opcode::Compare).b().CC(CC_LT).Inputs(6, 3);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(5, 7)
        {
            INST(12, Opcode::Add).s64().Inputs(6, 1);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(13, Opcode::Add).s64().Inputs(6, 2);
        }
        BASIC_BLOCK(10, -1)
        {
            INST(20, Opcode::Phi).s64().Inputs(0, 6);
            INST(21, Opcode::Return).s64().Inputs(20);
        }
    }
    graph->RunPass<BranchElimination>();
    graph->RunPass<Cleanup>();

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        CONSTANT(0, 0);
        BASIC_BLOCK(2, -1)
        {
            INST(21, Opcode::Return).s64().Inputs(0);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
}
}  // namespace panda::compiler
