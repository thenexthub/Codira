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

namespace panda::compiler {
class LoopAnalyzerTest : public CommonTest {
public:
    LoopAnalyzerTest() : graph_(CreateGraphStartEndBlocks()) {}
    ~LoopAnalyzerTest() override {}

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

    void CheckPhiInputs(BasicBlock *block)
    {
        for (auto phi : block->PhiInsts()) {
            for (unsigned i = 0; i < phi->GetInputs().size(); i++) {
                EXPECT_EQ(block->GetPredsBlocks()[i], phi->GetInputs()[i].GetInst()->GetBasicBlock());
            }
        }
    }

    Graph *GetGraph()
    {
        return graph_;
    }

private:
    Graph *graph_;
};

/*
 * Test Graph:
 *                             [2]
 *                              |
 *                              v
 *       /------/------------->[3]<---------------------\
 *       |      |               |                       |
 *       |      |               v                       |
 *       |      |              [4]<---------\           |
 *       |      |               |           |           |
 *       |      |               |          [6]          |
 *       |      |               |           ^           |
 *       |      |               v           |           |
 *      [17]   [19]            [5]----------/           |
 *       |      |               |                       |
 *       |      |               v                       |
 *       |     [18]            [7]                     [14]
 *       |      ^               |                       ^
 *       |      |               v                       |
 *      [16]    \--------------[8]<-------------\       |
 *       ^                      |               |       |
 *       |                      v               |       |
 *       |                     [9]             [11]     |
 *       |                   /     \            ^       |
 *       |                  v       v           |       |
 *       \----------------[15]      [10]---------/       |
 *                                   |                  |
 *                                   v                  |
 *                                  [12]                |
 *                                   |                  |
 *                                   v                  |
 *                                  [13]----------------/
 *                                   |
 *                                   V
 *                                  [20]
 *                                   |
 *                                   V
 *                                 [exit]
 *
 * Loop1:
 *      header: B4
 *      back_edges: B5
 *      blocks: B4, B5, B6
 *      outer_loop: Loop3
 * Loop2:
 *      header: B8
 *      back_edges: B11
 *      blocks: B8, B9, B10, B11
 *      outer_loop: Loop3
 * Loop3:
 *      header: B3
 *      back_edges: B14, B17, B19
 *      blocks: B3, B7, B12, B13, B14, B15, B16, B17, B18, B19
 *      inner_loops: Loop1, Loop2
 *
 */
TEST_F(LoopAnalyzerTest, LoopAnalyzer)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        BASIC_BLOCK(2, 3) {}
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 6, 7)
        {
            INST(5, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(6, 4) {}
        BASIC_BLOCK(7, 8) {}
        BASIC_BLOCK(8, 9, 18)
        {
            INST(9, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(9, 10, 15)
        {
            INST(11, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(10, 11, 12)
        {
            INST(13, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(11, 8) {}
        BASIC_BLOCK(12, 13) {}
        BASIC_BLOCK(13, 14, 20)
        {
            INST(17, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(17);
        }
        BASIC_BLOCK(14, 3) {}
        BASIC_BLOCK(15, 16) {}
        BASIC_BLOCK(16, 17) {}
        BASIC_BLOCK(17, 3) {}
        BASIC_BLOCK(18, 19) {}
        BASIC_BLOCK(19, 3) {}
        BASIC_BLOCK(20, -1)
        {
            INST(25, Opcode::ReturnVoid);
        }
    }

    auto loop1 = BB(4).GetLoop();
    auto loop2 = BB(8).GetLoop();
    auto loop3 = BB(3).GetLoop();
    auto root_loop = GetGraph()->GetRootLoop();

    ASSERT_NE(loop1, nullptr);
    ASSERT_NE(loop2, nullptr);
    ASSERT_NE(loop3, nullptr);

    ASSERT_EQ(loop1->GetHeader(), &BB(4));
    ASSERT_EQ(loop1->GetPreHeader(), &BB(3));
    CheckVectorEqualBlocksIdSet(loop1->GetBackEdges(), {6});
    CheckVectorEqualBlocksIdSet(loop1->GetBlocks(), {4, 5, 6});
    CheckVectorEqualSet(loop1->GetInnerLoops(), {});
    EXPECT_EQ(loop1->GetOuterLoop(), loop3);
    EXPECT_EQ(loop1->IsIrreducible(), false);

    ASSERT_EQ(loop2->GetHeader(), &BB(8));
    ASSERT_EQ(loop2->GetPreHeader(), &BB(7));
    CheckVectorEqualBlocksIdSet(loop2->GetBackEdges(), {11});
    CheckVectorEqualBlocksIdSet(loop2->GetBlocks(), {8, 9, 10, 11});
    CheckVectorEqualSet(loop2->GetInnerLoops(), {});
    EXPECT_EQ(loop2->GetOuterLoop(), loop3);
    EXPECT_EQ(loop2->IsIrreducible(), false);

    ASSERT_EQ(loop3->GetHeader(), &BB(3));
    ASSERT_EQ(loop3->GetPreHeader(), &BB(2));
    CheckVectorEqualBlocksIdSet(loop3->GetBackEdges(), {14, 17, 19});
    CheckVectorEqualBlocksIdSet(loop3->GetBlocks(), {3, 7, 12, 13, 14, 15, 16, 17, 18, 19});
    CheckVectorEqualSet(loop3->GetInnerLoops(), {loop1, loop2});
    EXPECT_EQ(loop3->GetOuterLoop(), root_loop);
    EXPECT_EQ(loop3->IsIrreducible(), false);

    EXPECT_EQ(BB(2).GetLoop(), root_loop);
    EXPECT_EQ(BB(20).GetLoop(), root_loop);
    CheckVectorEqualSet(root_loop->GetInnerLoops(), {loop3});
}

/*
 * Initial Graph:
 *                               [entry]
 *                                  |
 *                                  v
 *                           /-----[2]-----\
 *                           |      |      |
 *                           v      v      v
 *                          [3]    [4]    [5]
 *                           |      |      |
 *                           |      v      |
 *                           \---->[6]<----/<----------\
 *                                  | PHI(3,4,5,8)     |
 *                                  | PHI(3,4,5,8)     |
 *                                  v                  |
 *                                 [7]----->[8]--------/
 *                                  |
 *                                  |
 *                                  v
 *                                [exit]
 *
 * After loop pre-header insertion:
 *
 *                                 [entry]
 *                                    |
 *                                    v
 *                         /---------[2]----------\
 *                         |          |           |
 *                         v          v           v
 *                        [3]        [4]         [5]
 *                         |          |           |
 *                         |          v           |
 *                         \---->[pre-header]<----/
 *                                    | PHI(3,4,5)
 *                                    | PHI(3,4,5)
 *                                    V
 *                                   [6]<-------------------\
 *                                    | PHI(8,pre-header)   |
 *                                    | PHI(8,pre-header)   |
 *                                    v                     |
 *                                   [7]-------->[8]--------/
 *                                    |
 *                                    |
 *                                    v
 *                                  [exit]
 *
 *
 */
TEST_F(LoopAnalyzerTest, PreheaderInsert)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 5).u64();
        PARAMETER(2, 10).u64();

        BASIC_BLOCK(2, 4, 5)
        {
            INST(3, Opcode::Not).u64().Inputs(0);
            INST(19, Opcode::Compare).b().Inputs(0, 1);
            INST(20, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(19);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 1);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(7, Opcode::Phi).u64().Inputs({{4, 5}, {5, 6}, {8, 12}});
            INST(8, Opcode::Phi).u64().Inputs({{4, 5}, {5, 6}, {8, 12}});
        }
        BASIC_BLOCK(7, 8, 9)
        {
            INST(9, Opcode::Mul).u64().Inputs(7, 8);
            INST(10, Opcode::Compare).b().Inputs(9, 2);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(8, 6)
        {
            INST(12, Opcode::Mul).u64().Inputs(9, 1);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(18, Opcode::ReturnVoid);
        }
    }

    auto loop = BB(6).GetLoop();
    ASSERT_NE(loop, nullptr);
    EXPECT_EQ(loop->GetHeader(), &BB(6));
    CheckVectorEqualBlocksIdSet(loop->GetBackEdges(), {8});

    auto pre_header = loop->GetPreHeader();
    ASSERT_EQ(pre_header->GetLoop(), loop->GetOuterLoop());
    CheckVectorEqualBlocksIdSet(pre_header->GetPredsBlocks(), {4, 5});
    CheckVectorEqualBlocksIdSet(pre_header->GetSuccsBlocks(), {6});
    EXPECT_EQ(loop->GetHeader()->GetDominator(), pre_header);
    CheckVectorEqualBlocksIdSet(pre_header->GetDominatedBlocks(), {6});

    CheckPhiInputs(&BB(6));
    CheckPhiInputs(pre_header);
}

/*
 * Initial Graph:
 *                                 [0]
 *                                  |
 *                                  v
 *                                 [2]<------\
 *                                  |        |
 *                                  v        |
 *                                 [3]-------/
 *                                  |
 *                                  v
 *                                 [4]<------\
 *                                  |        |
 *                                  v        |
 *                                 [5]-------/
 *                                  |
 *                                  V
 *                                 [6]----->[1]
 *
 * After loop pre-headers insertion:
 *
 */
TEST_F(LoopAnalyzerTest, PreheaderInsert2)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        BASIC_BLOCK(2, 3) {}
        BASIC_BLOCK(3, 4, 2)
        {
            INST(3, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 6, 4)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(8, Opcode::ReturnVoid);
        }
    }

    auto loop1 = BB(2).GetLoop();
    auto loop2 = BB(4).GetLoop();
    CheckVectorEqualBlocksIdSet(loop1->GetPreHeader()->GetPredsBlocks(), {0});
    CheckVectorEqualBlocksIdSet(loop1->GetPreHeader()->GetSuccsBlocks(), {2});
    CheckVectorEqualBlocksIdSet(loop2->GetPreHeader()->GetPredsBlocks(), {3});
    CheckVectorEqualBlocksIdSet(loop2->GetPreHeader()->GetSuccsBlocks(), {4});
}

/**
 * Check infinite loop
 *
 *         [begin]
 *            |
 *           [2]<------\
 *          /    \     |
 *       [4]     [3]---/
 *        |
 *       [5]<---\
 *        |     |
 *       [6]----/
 */
TEST_F(LoopAnalyzerTest, InfiniteLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        CONSTANT(2, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).s32().Inputs(0, 6);
            INST(4, Opcode::Compare).b().CC(CC_NE).Inputs(3, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(6, Opcode::Add).s32().Inputs(3, 2);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 5)
        {
            INST(7, Opcode::Phi).s32().Inputs(3, 8);
            INST(8, Opcode::Add).s32().Inputs(7, 2);
        }
    }

    auto loop1 = BB(2).GetLoop();
    auto loop2 = BB(5).GetLoop();
    ASSERT_FALSE(loop1->IsInfinite());
    ASSERT_TRUE(loop2->IsInfinite());
}

/**
 * Test checks the ASSERTION fail issue fix in a Loop::AppendBlock.
 *
 * Original progam:
 *
 *   .function u1 main(){
 *       ldai 0
 *       jltz loop1
 *       jltz loop2
 *     loop1:
 *       jltz loop1
 *     loop2:
 *       jltz loop1
 *       return
 *   }
 *
 * Graph dump:
 *
 *   BB 5
 *   prop: start
 *   succs: [bb 0]
 *
 *   BB 0  preds: [bb 5]
 *   succs: [bb 1, bb 2]
 *
 *   BB 2  preds: [bb 0]
 *   succs: [bb 3, bb 1]
 *
 *   BB 1  preds: [bb 0, bb 2, bb 1, bb 3]
 *   succs: [bb 1, bb 3]
 *
 *   BB 3  preds: [bb 2, bb 1]
 *   succs: [bb 1, bb 4]
 *
 *   BB 4  preds: [bb 3]
 *   succs: [bb 6]
 *
 *   BB 6  preds: [bb 4]
 *   prop: end
 *
 * Test Graph:
 *
 *      [5-start]
 *          |
 *         [0] --> [2]
 *           \    /
 *   [3] <--> [1] <-- ┐
 *    |        |      |
 *   [4]       └ ---> ┘
 *    |
 *  [6-end]
 *
 */
TEST_F(LoopAnalyzerTest, LoopTest1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);

        BASIC_BLOCK(10, 11, 12)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(12, 13, 11)
        {
            INST(4, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(11, 11, 13)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(13, 11, 14)
        {
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(14, -1)
        {
            INST(10, Opcode::ReturnVoid);
        }
    }

    // No asserts, just IrBuilder sanity check
}
}  // namespace panda::compiler
