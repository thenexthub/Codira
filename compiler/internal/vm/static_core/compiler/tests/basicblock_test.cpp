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

#include "optimizer/analysis/linear_order.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "unit_test.h"
#include <set>

namespace ark::compiler {
class BasicBlockTest : public GraphTest {
public:
    template <typename T, typename Blocks>
    void CheckVectorEqualSet(Blocks blocks, std::set<T *> &&excepct)
    {
        ASSERT_EQ(blocks.size(), excepct.size());

        std::set<T *> result;
        for (auto block : blocks) {
            result.insert(block);
        }
        EXPECT_EQ(result, excepct);
    }

    template <typename Blocks>
    void CheckVectorEqualBlocksIdSet(Blocks blocks, std::vector<int> &&bbIds)
    {
        std::set<BasicBlock *> bbSet;
        for (auto id : bbIds) {
            bbSet.insert(&BB(id));
        }
        CheckVectorEqualSet(std::move(blocks), std::move(bbSet));
    }

    /*
     * Check if block's false-successor is placed in the next position of the rpo vector or the block `NeedsJump()`
     */
    void CheckBlockFalseSuccessorPosition(BasicBlock *block, const ArenaVector<BasicBlock *> &blocksVector)
    {
        auto blockRpoIt = std::find(blocksVector.begin(), blocksVector.end(), block);
        auto falseBlockIt = std::find(blocksVector.begin(), blocksVector.end(), block->GetFalseSuccessor());
        ASSERT_NE(blockRpoIt, blocksVector.end());
        ASSERT_NE(falseBlockIt, blocksVector.end());
        auto blockRpoIndex = std::distance(blocksVector.begin(), blockRpoIt);
        auto falseBlockRpoIndex = std::distance(blocksVector.begin(), falseBlockIt);
        EXPECT_TRUE((blockRpoIndex + 1 == falseBlockRpoIndex) || (block->NeedsJump()));
    }

    void RemoveBlocksBuildGraph();
    void SplitByPhi1BuildGraph();
    Graph *IfLikelyUnlikelyTestBuildGraph();
};

// NOLINTBEGIN(readability-magic-numbers)
/*
 * Test Graph:
 *                      [entry]
 *                         |
 *                         v
 *                /-------[2]-------\
 *                |                 |
 *                v                 v
 *               [3]               [4]
 *                |                 |
 *                |                 v
 *                |       /--------[5]
 *                |       |         |
 *                |       v         v
 *                |      [6]       [7]
 *                |       |         |
 *                |       v	        v
 *                \----->[9]<-------/
 *                        |
 *                        v
 *                      [exit]
 */

void BasicBlockTest::RemoveBlocksBuildGraph()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 12U);
        CONSTANT(1U, 13U);
        PARAMETER(20U, 0U).u64();
        PARAMETER(21U, 1U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(18U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(18U);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(22U, Opcode::Mul).u64().Inputs(20U, 20U);
            INST(3U, Opcode::Not).u64().Inputs(0U);
            INST(17U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
        }
        BASIC_BLOCK(3U, 9U)
        {
            INST(4U, Opcode::Add).u64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(6U, 9U)
        {
            INST(5U, Opcode::Sub).u64().Inputs(1U, 0U);
        }
        BASIC_BLOCK(7U, 9U)
        {
            INST(6U, Opcode::Div).u64().Inputs(22U, 21U);
        }
        BASIC_BLOCK(9U, -1L)
        {
            INST(8U, Opcode::Phi).u64().Inputs({{3U, 4U}, {6U, 5U}, {7U, 6U}});
            INST(16U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(BasicBlockTest, RemoveBlocks)
{
    // build graph
    RemoveBlocksBuildGraph();

    EXPECT_EQ(INS(8U).GetInputsCount(), BB(9U).GetPredsBlocks().size());
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_ENTRY_BB).GetPredsBlocks(), {});
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_ENTRY_BB).GetSuccsBlocks(), {2U});
    CheckVectorEqualBlocksIdSet(BB(2U).GetPredsBlocks(), {IrConstructor::ID_ENTRY_BB});
    CheckVectorEqualBlocksIdSet(BB(2U).GetSuccsBlocks(), {3U, 4U});
    CheckVectorEqualBlocksIdSet(BB(3U).GetPredsBlocks(), {2U});
    CheckVectorEqualBlocksIdSet(BB(3U).GetSuccsBlocks(), {9U});
    CheckVectorEqualBlocksIdSet(BB(4U).GetPredsBlocks(), {2U});
    CheckVectorEqualBlocksIdSet(BB(4U).GetSuccsBlocks(), {5U});
    CheckVectorEqualBlocksIdSet(BB(5U).GetPredsBlocks(), {4U});
    CheckVectorEqualBlocksIdSet(BB(5U).GetSuccsBlocks(), {6U, 7U});
    CheckVectorEqualBlocksIdSet(BB(6U).GetPredsBlocks(), {5U});
    CheckVectorEqualBlocksIdSet(BB(6U).GetSuccsBlocks(), {9U});
    CheckVectorEqualBlocksIdSet(BB(7U).GetPredsBlocks(), {5U});
    CheckVectorEqualBlocksIdSet(BB(7U).GetSuccsBlocks(), {9U});
    CheckVectorEqualBlocksIdSet(BB(9U).GetPredsBlocks(), {3U, 6U, 7U});
    CheckVectorEqualBlocksIdSet(BB(9U).GetSuccsBlocks(), {IrConstructor::ID_EXIT_BB});
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_EXIT_BB).GetPredsBlocks(), {9U});
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_EXIT_BB).GetSuccsBlocks(), {});
    EXPECT_TRUE(INS(22U).GetUsers().Front().GetInst() == &INS(6U));
    EXPECT_TRUE(INS(21U).GetUsers().Front().GetInst() == &INS(6U));

    GetGraph()->DisconnectBlock(&BB(7U));

    EXPECT_TRUE(INS(22U).GetUsers().Empty());
    EXPECT_TRUE(INS(21U).GetUsers().Empty());
    CheckVectorEqualBlocksIdSet(BB(5U).GetSuccsBlocks(), {6U});
    CheckVectorEqualBlocksIdSet(BB(9U).GetPredsBlocks(), {3U, 6U});
    EXPECT_EQ(INS(8U).GetInputsCount(), BB(9U).GetPredsBlocks().size());

    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->RunPass<LoopAnalyzer>();
    GraphChecker(GetGraph()).Check();
}

/*
 *            [2]
 *             |
 *        /---------\
 *       [3]       [4]
 *        \---------/
 *             |
 *            [5]
 */
TEST_F(BasicBlockTest, RemoveEmptyBlock)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 5U) {}
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, -1L)
        {
            INST(4U, Opcode::ReturnVoid);
        }
    }
    ASSERT_EQ(BB(2U).GetTrueSuccessor(), &BB(3U));
    ASSERT_EQ(BB(2U).GetFalseSuccessor(), &BB(4U));
    auto bb5Pred3Idx = BB(5U).GetPredBlockIndex(&BB(3U));
    auto bb5Pred4Idx = BB(5U).GetPredBlockIndex(&BB(4U));
    GetGraph()->RemoveEmptyBlockWithPhis(&BB(3U));
    ASSERT_EQ(BB(2U).GetTrueSuccessor(), &BB(5U));
    ASSERT_EQ(BB(2U).GetFalseSuccessor(), &BB(4U));
    ASSERT_TRUE(BB(3U).GetSuccsBlocks().empty());
    ASSERT_TRUE(BB(3U).GetPredsBlocks().empty());
    ASSERT_EQ(BB(5U).GetPredBlockIndex(&BB(2U)), bb5Pred3Idx);
    ASSERT_EQ(BB(5U).GetPredBlockIndex(&BB(4U)), bb5Pred4Idx);
}

TEST_F(BasicBlockTest, MissBBId)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(2U, Opcode::ReturnVoid);
        }
    }
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_ENTRY_BB).GetPredsBlocks(), {});
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_ENTRY_BB).GetSuccsBlocks(), {2U});
    CheckVectorEqualBlocksIdSet(BB(2U).GetPredsBlocks(), {IrConstructor::ID_ENTRY_BB});
    CheckVectorEqualBlocksIdSet(BB(2U).GetSuccsBlocks(), {4U});
    CheckVectorEqualBlocksIdSet(BB(4U).GetPredsBlocks(), {2U});
    CheckVectorEqualBlocksIdSet(BB(4U).GetSuccsBlocks(), {IrConstructor::ID_EXIT_BB});
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_EXIT_BB).GetPredsBlocks(), {4U});
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_EXIT_BB).GetSuccsBlocks(), {});
}

/*
 *            [entry]
 *               |
 *               v
 *              [2]-------->[3]
 *               |        /     \
 *               v       v       v
 *              [4]---->[5]---->[6]
 *                               |
 *                               v
 *                             [exit]
 */
TEST_F(BasicBlockTest, IfTrueSwapSuccessors)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_EQ).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_NE).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(6U, Opcode::Add).s64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(13U, Opcode::Phi).s64().Inputs({{4U, 6U}, {3U, 0U}});
            INST(8U, Opcode::Add).s64().Inputs(13U, 13U);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(14U, Opcode::Phi).s64().Inputs({{5U, 8U}, {3U, 0U}});
            INST(10U, Opcode::Add).s64().Inputs(14U, 14U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(12U, Opcode::Return).s64().Inputs(10U);
        }
    }
    // The arch isn`t supported
    if (GetGraph()->GetCallingConvention() == nullptr) {
        return;
    }

    RegAlloc(GetGraph());
    ASSERT_EQ(GetGraph()->GetAnalysis<LinearOrder>().IsValid(), false);
    const auto &blocks = GetGraph()->GetBlocksLinearOrder();
    ASSERT_EQ(GetGraph()->GetAnalysis<LinearOrder>().IsValid(), true);
    GraphChecker(GetGraph()).Check();
    CheckBlockFalseSuccessorPosition(&BB(2U), blocks);
    CheckBlockFalseSuccessorPosition(&BB(3U), blocks);
    ASSERT_EQ(BB(5U).GetLastInst(), &INS(8U));
}

/*
 *                      [entry]
 *                         |
 *                         v
 *                /-------[2]
 *                |        |
 *                |        v
 *               [3]      [4]
 *                |        |
 *                \--------\------>[5]------\
 *                |        |                |
 *                |        |                v
 *                \--------\------>[6]--->[exit]
 *
 */
TEST_F(BasicBlockTest, IfTrueInsertFalseBlock)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_EQ).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_NE).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(6U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_EQ).Inputs(0U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::Return).s64().Inputs(0U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(9U, Opcode::Return).s64().Inputs(1U);
        }
    }
    // The arch isn`t supported
    if (GetGraph()->GetCallingConvention() == nullptr) {
        return;
    }

    RegAlloc(GetGraph());
    ASSERT_EQ(GetGraph()->GetAnalysis<LinearOrder>().IsValid(), false);
    const auto &blocks = GetGraph()->GetBlocksLinearOrder();
    ASSERT_EQ(GetGraph()->GetAnalysis<LinearOrder>().IsValid(), true);
    GraphChecker(GetGraph()).Check();
    CheckBlockFalseSuccessorPosition(&BB(2U), blocks);
    CheckBlockFalseSuccessorPosition(&BB(3U), blocks);
    CheckBlockFalseSuccessorPosition(&BB(4U), blocks);
}

TEST_F(BasicBlockTest, Split)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Add).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Mul).s64().Inputs(0U, 1U);
            INST(4U, Opcode::Compare).b().CC(CC_EQ).Inputs(2U, 3U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::ReturnVoid);
        }
    }
    ASSERT_EQ(BB(2U).GetTrueSuccessor(), &BB(3U));
    ASSERT_EQ(BB(2U).GetFalseSuccessor(), &BB(4U));
    auto newBb = BB(2U).SplitBlockAfterInstruction(&INS(2U), true);
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(BB(2U).GetTrueSuccessor(), newBb);
    ASSERT_EQ(newBb->GetPredsBlocks().size(), 1U);
    ASSERT_EQ(newBb->GetPredsBlocks().front(), &BB(2U));
    ASSERT_EQ(newBb->GetFirstInst(), &INS(3U));
    ASSERT_EQ(newBb->GetFirstInst()->GetNext(), &INS(4U));
    ASSERT_EQ(newBb->GetFirstInst()->GetNext()->GetNext(), &INS(5U));
    ASSERT_EQ(BB(3U).GetPredsBlocks().front(), newBb);
    ASSERT_EQ(BB(4U).GetPredsBlocks().front(), newBb);
    ASSERT_EQ(newBb->GetGuestPc(), INS(3U).GetPc());
    ASSERT_EQ(newBb->GetTrueSuccessor(), &BB(3U));
    ASSERT_EQ(newBb->GetFalseSuccessor(), &BB(4U));
}

void BasicBlockTest::SplitByPhi1BuildGraph()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);          // initial
        CONSTANT(1U, 1U);          // increment
        CONSTANT(2U, 10U);         // len_array
        PARAMETER(13U, 0U).s32();  // X
        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U);
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0U, 13U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(20U, Opcode::Phi).s32().Inputs(0U, 21U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(2U, 4U, 7U);
            INST(9U, Opcode::LoadArray).s32().Inputs(3U, 8U);  // a[i]
            INST(21U, Opcode::Add).s32().Inputs(20U, 9U);
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i++
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10U, 13U);  // i < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(22U, Opcode::Phi).s32().Inputs(0U, 21U);
            INST(12U, Opcode::Return).s32().Inputs(22U);
        }
    }
}

TEST_F(BasicBlockTest, SplitByPhi1)
{
    SplitByPhi1BuildGraph();

    BB(3U).SplitBlockAfterInstruction(&INS(20U), true);
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 0U);          // initial
        CONSTANT(1U, 1U);          // increment
        CONSTANT(2U, 10U);         // len_array
        PARAMETER(13U, 0U).s32();  // X
        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U);
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0U, 13U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 7U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(20U, Opcode::Phi).s32().Inputs(0U, 21U);
        }
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(2U, 4U, 7U);
            INST(9U, Opcode::LoadArray).s32().Inputs(3U, 8U);  // a[i]
            INST(21U, Opcode::Add).s32().Inputs(20U, 9U);
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i++
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10U, 13U);  // i < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(22U, Opcode::Phi).s32().Inputs(0U, 21U);
            INST(12U, Opcode::Return).s32().Inputs(22U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(BasicBlockTest, DisconnectPhiWithInputItself)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(2U, 0U);
        BASIC_BLOCK(3U, 4U)
        {
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 4U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(3U, 4U);
            INST(5U, Opcode::Compare).CC(CC_GT).b().Inputs(4U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(10U, Opcode::Return).s32().Inputs(4U);
        }
    }

    GetGraph()->DisconnectBlock(&BB(3U));
    GetGraph()->DisconnectBlock(&BB(4U));
    GetGraph()->DisconnectBlock(&BB(5U));

    ASSERT_TRUE(GetGraph()->GetStartBlock()->GetSuccsBlocks().empty());
    ASSERT_TRUE(GetGraph()->GetEndBlock()->GetPredsBlocks().empty());
}
// NOLINTEND(readability-magic-numbers)

Graph *BasicBlockTest::IfLikelyUnlikelyTestBuildGraph()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        CONSTANT(1U, 10U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GT).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Inputs(2U).Likely();
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Sub).s64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::Sub).s64().Inputs(1U, 0U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).s64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).s64().Inputs(6U, 6U);
            INST(8U, Opcode::Return).s64().Inputs(7U);
        }
    }
    return graph;
}

TEST_F(BasicBlockTest, IfLikelyUnlikelyTest)
{
    // The arch isn`t supported
    if (GetGraph()->GetCallingConvention() == nullptr) {
        return;
    }
    auto *graph1 = IfLikelyUnlikelyTestBuildGraph();

    const auto &blocks1 = graph1->GetBlocksLinearOrder();
    ASSERT_EQ(&BB(2U), blocks1.at(1U));
    ASSERT_EQ(&BB(3U), blocks1.at(2U));
    ASSERT_EQ(&BB(5U), blocks1.at(3U));
    ASSERT_EQ(&BB(4U), blocks1.back());

    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).s64();
        CONSTANT(1U, 10U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GT).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Inputs(2U).Unlikely();
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Sub).s64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::Sub).s64().Inputs(1U, 0U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).s64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).s64().Inputs(6U, 6U);
            INST(8U, Opcode::Return).s64().Inputs(7U);
        }
    }

    const auto &blocks2 = graph2->GetBlocksLinearOrder();
    ASSERT_EQ(&BB(2U), blocks2.at(1U));
    ASSERT_EQ(&BB(4U), blocks2.at(2U));
    ASSERT_EQ(&BB(5U), blocks2.at(3U));
    ASSERT_EQ(&BB(3U), blocks2.back());
}

}  // namespace ark::compiler
