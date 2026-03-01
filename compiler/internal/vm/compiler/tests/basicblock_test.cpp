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

#include <set>
#include "optimizer/analysis/linear_order.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "unit_test.h"

namespace panda::compiler {
class BasicBlockTest : public GraphTest {
public:
    template <typename T>
    void CheckVectorEqualSet(ArenaVector<T *> blocks, std::set<T *> &&excepct)
    {
        ASSERT_EQ(blocks.size(), excepct.size());

        std::set<T *> result;
        for (auto block : blocks) {
            result.insert(block);
        }
        EXPECT_EQ(result, excepct);
    }

    void CheckVectorEqualBlocksIdSet(ArenaVector<BasicBlock *> blocks, std::vector<int> &&bb_ids)
    {
        std::set<BasicBlock *> bb_set;
        for (auto id : bb_ids) {
            bb_set.insert(&BB(id));
        }
        CheckVectorEqualSet(blocks, std::move(bb_set));
    }

    /*
     * Check if block's false-successor is placed in the next position of the rpo vector or the block `NeedsJump()`
     */
    void CheckBlockFalseSuccessorPosition(BasicBlock *block, const ArenaVector<BasicBlock *> &blocks_vector)
    {
        auto block_rpo_it = std::find(blocks_vector.begin(), blocks_vector.end(), block);
        auto false_block_it = std::find(blocks_vector.begin(), blocks_vector.end(), block->GetFalseSuccessor());
        ASSERT_NE(block_rpo_it, blocks_vector.end());
        ASSERT_NE(false_block_it, blocks_vector.end());
        auto block_rpo_index = std::distance(blocks_vector.begin(), block_rpo_it);
        auto false_block_rpo_index = std::distance(blocks_vector.begin(), false_block_it);
        EXPECT_TRUE((block_rpo_index + 1 == false_block_rpo_index) || (block->NeedsJump()));
    }
};

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
TEST_F(BasicBlockTest, RemoveBlocks)
{
    // build graph
    GRAPH(GetGraph())
    {
        CONSTANT(0, 12);
        CONSTANT(1, 13);
        PARAMETER(20, 0).u64();
        PARAMETER(21, 1).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(18, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(18);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 6, 7)
        {
            INST(22, Opcode::Mul).u64().Inputs(20, 20);
            INST(3, Opcode::Not).u64().Inputs(0);
            INST(17, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(17);
        }
        BASIC_BLOCK(3, 9)
        {
            INST(4, Opcode::Add).u64().Inputs(0, 1);
        }
        BASIC_BLOCK(6, 9)
        {
            INST(5, Opcode::Sub).u64().Inputs(1, 0);
        }
        BASIC_BLOCK(7, 9)
        {
            INST(6, Opcode::Div).u64().Inputs(22, 21);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(8, Opcode::Phi).u64().Inputs({{3, 4}, {6, 5}, {7, 6}});
            INST(16, Opcode::ReturnVoid);
        }
    }

    EXPECT_EQ(INS(8).GetInputsCount(), BB(9).GetPredsBlocks().size());
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_ENTRY_BB).GetPredsBlocks(), {});
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_ENTRY_BB).GetSuccsBlocks(), {2});
    CheckVectorEqualBlocksIdSet(BB(2).GetPredsBlocks(), {IrConstructor::ID_ENTRY_BB});
    CheckVectorEqualBlocksIdSet(BB(2).GetSuccsBlocks(), {3, 4});
    CheckVectorEqualBlocksIdSet(BB(3).GetPredsBlocks(), {2});
    CheckVectorEqualBlocksIdSet(BB(3).GetSuccsBlocks(), {9});
    CheckVectorEqualBlocksIdSet(BB(4).GetPredsBlocks(), {2});
    CheckVectorEqualBlocksIdSet(BB(4).GetSuccsBlocks(), {5});
    CheckVectorEqualBlocksIdSet(BB(5).GetPredsBlocks(), {4});
    CheckVectorEqualBlocksIdSet(BB(5).GetSuccsBlocks(), {6, 7});
    CheckVectorEqualBlocksIdSet(BB(6).GetPredsBlocks(), {5});
    CheckVectorEqualBlocksIdSet(BB(6).GetSuccsBlocks(), {9});
    CheckVectorEqualBlocksIdSet(BB(7).GetPredsBlocks(), {5});
    CheckVectorEqualBlocksIdSet(BB(7).GetSuccsBlocks(), {9});
    CheckVectorEqualBlocksIdSet(BB(9).GetPredsBlocks(), {3, 6, 7});
    CheckVectorEqualBlocksIdSet(BB(9).GetSuccsBlocks(), {IrConstructor::ID_EXIT_BB});
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_EXIT_BB).GetPredsBlocks(), {9});
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_EXIT_BB).GetSuccsBlocks(), {});
    EXPECT_TRUE(INS(22).GetUsers().Front().GetInst() == &INS(6));
    EXPECT_TRUE(INS(21).GetUsers().Front().GetInst() == &INS(6));

    GetGraph()->DisconnectBlock(&BB(7));

    EXPECT_TRUE(INS(22).GetUsers().Empty());
    EXPECT_TRUE(INS(21).GetUsers().Empty());
    CheckVectorEqualBlocksIdSet(BB(5).GetSuccsBlocks(), {6});
    CheckVectorEqualBlocksIdSet(BB(9).GetPredsBlocks(), {3, 6});
    EXPECT_EQ(INS(8).GetInputsCount(), BB(9).GetPredsBlocks().size());

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
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, -1)
        {
            INST(4, Opcode::ReturnVoid);
        }
    }
    ASSERT_EQ(BB(2).GetTrueSuccessor(), &BB(3));
    ASSERT_EQ(BB(2).GetFalseSuccessor(), &BB(4));
    auto bb5_pred3_idx = BB(5).GetPredBlockIndex(&BB(3));
    auto bb5_pred4_idx = BB(5).GetPredBlockIndex(&BB(4));
    GetGraph()->RemoveEmptyBlockWithPhis(&BB(3));
    ASSERT_EQ(BB(2).GetTrueSuccessor(), &BB(5));
    ASSERT_EQ(BB(2).GetFalseSuccessor(), &BB(4));
    ASSERT_TRUE(BB(3).GetSuccsBlocks().empty());
    ASSERT_TRUE(BB(3).GetPredsBlocks().empty());
    ASSERT_EQ(BB(5).GetPredBlockIndex(&BB(2)), bb5_pred3_idx);
    ASSERT_EQ(BB(5).GetPredBlockIndex(&BB(4)), bb5_pred4_idx);
}

TEST_F(BasicBlockTest, MissBBId)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(2, Opcode::ReturnVoid);
        }
    }
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_ENTRY_BB).GetPredsBlocks(), {});
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_ENTRY_BB).GetSuccsBlocks(), {2});
    CheckVectorEqualBlocksIdSet(BB(2).GetPredsBlocks(), {IrConstructor::ID_ENTRY_BB});
    CheckVectorEqualBlocksIdSet(BB(2).GetSuccsBlocks(), {4});
    CheckVectorEqualBlocksIdSet(BB(4).GetPredsBlocks(), {2});
    CheckVectorEqualBlocksIdSet(BB(4).GetSuccsBlocks(), {IrConstructor::ID_EXIT_BB});
    CheckVectorEqualBlocksIdSet(BB(IrConstructor::ID_EXIT_BB).GetPredsBlocks(), {4});
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
        CONSTANT(0, 0);
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_EQ).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 5, 6)
        {
            INST(4, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_NE).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(6, Opcode::Add).s64().Inputs(0, 1);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(13, Opcode::Phi).s64().Inputs({{4, 6}, {3, 0}});
            INST(8, Opcode::Add).s64().Inputs(13, 13);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(14, Opcode::Phi).s64().Inputs({{5, 8}, {3, 0}});
            INST(10, Opcode::Add).s64().Inputs(14, 14);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(12, Opcode::Return).s64().Inputs(10);
        }
    }
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
        CONSTANT(0, 0);
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_EQ).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 5, 6)
        {
            INST(4, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_NE).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_EQ).Inputs(0, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(8, Opcode::Return).s64().Inputs(0);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(9, Opcode::Return).s64().Inputs(1);
        }
    }
}

TEST_F(BasicBlockTest, Split)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Add).s64().Inputs(0, 1);
            INST(3, Opcode::Mul).s64().Inputs(0, 1);
            INST(4, Opcode::Compare).b().CC(CC_EQ).Inputs(2, 3);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(6, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(7, Opcode::ReturnVoid);
        }
    }
    ASSERT_EQ(BB(2).GetTrueSuccessor(), &BB(3));
    ASSERT_EQ(BB(2).GetFalseSuccessor(), &BB(4));
    auto new_bb = BB(2).SplitBlockAfterInstruction(&INS(2), true);
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(BB(2).GetTrueSuccessor(), new_bb);
    ASSERT_EQ(new_bb->GetPredsBlocks().size(), 1U);
    ASSERT_EQ(new_bb->GetPredsBlocks().front(), &BB(2));
    ASSERT_EQ(new_bb->GetFirstInst(), &INS(3));
    ASSERT_EQ(new_bb->GetFirstInst()->GetNext(), &INS(4));
    ASSERT_EQ(new_bb->GetFirstInst()->GetNext()->GetNext(), &INS(5));
    ASSERT_EQ(BB(3).GetPredsBlocks().front(), new_bb);
    ASSERT_EQ(BB(4).GetPredsBlocks().front(), new_bb);
    ASSERT_EQ(new_bb->GetGuestPc(), INS(3).GetPc());
    ASSERT_EQ(new_bb->GetTrueSuccessor(), &BB(3));
    ASSERT_EQ(new_bb->GetFalseSuccessor(), &BB(4));
}

TEST_F(BasicBlockTest, SplitByPhi1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);          // initial
        CONSTANT(1, 1);          // increment
        CONSTANT(2, 10);         // len_array
        PARAMETER(13, 0).s32();  // X
        BASIC_BLOCK(2, 3, 6)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2);
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0, 13);  // i < X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(20, Opcode::Phi).s32().Inputs(0, 21);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::BoundsCheck).s32().Inputs(2, 4, 7);
            INST(9, Opcode::LoadArray).s32().Inputs(3, 8);  // a[i]
            INST(21, Opcode::Add).s32().Inputs(20, 9);
            INST(10, Opcode::Add).s32().Inputs(4, 1);                              // i++
            INST(5, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10, 13);  // i < X
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(22, Opcode::Phi).s32().Inputs(0, 21);
            INST(12, Opcode::Return).s32().Inputs(22);
        }
    }
    BB(3).SplitBlockAfterInstruction(&INS(20), true);
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 0);          // initial
        CONSTANT(1, 1);          // increment
        CONSTANT(2, 10);         // len_array
        PARAMETER(13, 0).s32();  // X
        BASIC_BLOCK(2, 3, 6)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2);
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0, 13);  // i < X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(3, 7)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(20, Opcode::Phi).s32().Inputs(0, 21);
        }
        BASIC_BLOCK(7, 3, 6)
        {
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::BoundsCheck).s32().Inputs(2, 4, 7);
            INST(9, Opcode::LoadArray).s32().Inputs(3, 8);  // a[i]
            INST(21, Opcode::Add).s32().Inputs(20, 9);
            INST(10, Opcode::Add).s32().Inputs(4, 1);                              // i++
            INST(5, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10, 13);  // i < X
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(22, Opcode::Phi).s32().Inputs(0, 21);
            INST(12, Opcode::Return).s32().Inputs(22);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(BasicBlockTest, DisconnectPhiWithInputItself)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        CONSTANT(2, 0);
        BASIC_BLOCK(3, 4)
        {
            INST(3, Opcode::Add).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 4, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(3, 4);
            INST(5, Opcode::Compare).CC(CC_GT).b().Inputs(4, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(10, Opcode::Return).s32().Inputs(4);
        }
    }

    GetGraph()->DisconnectBlock(&BB(3));
    GetGraph()->DisconnectBlock(&BB(4));
    GetGraph()->DisconnectBlock(&BB(5));

    ASSERT_TRUE(GetGraph()->GetStartBlock()->GetSuccsBlocks().empty());
    ASSERT_TRUE(GetGraph()->GetEndBlock()->GetPredsBlocks().empty());
}
}  // namespace panda::compiler
