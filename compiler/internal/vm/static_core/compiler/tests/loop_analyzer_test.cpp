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

#include "libarkbase/macros.h"
#include "unit_test.h"

namespace ark::compiler {
class LoopAnalyzerTest : public CommonTest {
public:
    LoopAnalyzerTest() : graph_(CreateGraphStartEndBlocks()) {}
    ~LoopAnalyzerTest() override = default;

    NO_COPY_SEMANTIC(LoopAnalyzerTest);
    NO_MOVE_SEMANTIC(LoopAnalyzerTest);

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

    template <typename Blocks>
    void CheckVectorEqualBlocksIdSet(Blocks blocks, std::vector<int> &&bbIds)
    {
        std::set<BasicBlock *> bbSet;
        for (auto id : bbIds) {
            bbSet.insert(&BB(id));
        }
        CheckVectorEqualSet(std::move(blocks), std::move(bbSet));
    }

    void CheckPhiInputs(BasicBlock *block)
    {
        for (auto phi : block->PhiInsts()) {
            for (unsigned i = 0; i < phi->GetInputs().size(); i++) {
                EXPECT_EQ(block->GetPredsBlocks()[i], phi->GetInputs()[i].GetInst()->GetBasicBlock());
            }
        }
    }

    void CollectLoopsDFS(ArenaVector<Loop *> *loops, Loop *loop)
    {
        loops->push_back(loop);
        for (auto innerLoop : loop->GetInnerLoops()) {
            CollectLoopsDFS(loops, innerLoop);
        }
    }

    Graph *GetGraph()
    {
        return graph_;
    }

    void BuildGraphLoopAnalyzer();
    void BuildGraphPreheaderInsert();

private:
    Graph *graph_;
};

// NOLINTBEGIN(readability-magic-numbers)
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
void LoopAnalyzerTest::BuildGraphLoopAnalyzer()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 3U) {}
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 4U) {}
        BASIC_BLOCK(7U, 8U) {}
        BASIC_BLOCK(8U, 9U, 18U)
        {
            INST(9U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(9U, 10U, 15U)
        {
            INST(11U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(10U, 11U, 12U)
        {
            INST(13U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(11U, 8U) {}
        BASIC_BLOCK(12U, 13U) {}
        BASIC_BLOCK(13U, 14U, 20U)
        {
            INST(17U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
        }
        BASIC_BLOCK(14U, 3U) {}
        BASIC_BLOCK(15U, 16U) {}
        BASIC_BLOCK(16U, 17U) {}
        BASIC_BLOCK(17U, 3U) {}
        BASIC_BLOCK(18U, 19U) {}
        BASIC_BLOCK(19U, 3U) {}
        BASIC_BLOCK(20U, -1L)
        {
            INST(25U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(LoopAnalyzerTest, LoopAnalyzer)
{
    BuildGraphLoopAnalyzer();
    auto loop1 = BB(4U).GetLoop();
    auto loop2 = BB(8U).GetLoop();
    auto loop3 = BB(3U).GetLoop();
    auto rootLoop = GetGraph()->GetRootLoop();

    ASSERT_NE(loop1, nullptr);
    ASSERT_NE(loop2, nullptr);
    ASSERT_NE(loop3, nullptr);

    ASSERT_EQ(loop1->GetHeader(), &BB(4U));
    ASSERT_EQ(loop1->GetPreHeader(), &BB(3U));
    CheckVectorEqualBlocksIdSet(loop1->GetBackEdges(), {6U});
    CheckVectorEqualBlocksIdSet(loop1->GetBlocks(), {4U, 5U, 6U});
    CheckVectorEqualSet(loop1->GetInnerLoops(), {});
    EXPECT_EQ(loop1->GetOuterLoop(), loop3);
    EXPECT_EQ(loop1->IsIrreducible(), false);

    ASSERT_EQ(loop2->GetHeader(), &BB(8U));
    ASSERT_EQ(loop2->GetPreHeader(), &BB(7U));
    CheckVectorEqualBlocksIdSet(loop2->GetBackEdges(), {11U});
    CheckVectorEqualBlocksIdSet(loop2->GetBlocks(), {8U, 9U, 10U, 11U});
    CheckVectorEqualSet(loop2->GetInnerLoops(), {});
    EXPECT_EQ(loop2->GetOuterLoop(), loop3);
    EXPECT_EQ(loop2->IsIrreducible(), false);

    ASSERT_EQ(loop3->GetHeader(), &BB(3U));
    ASSERT_EQ(loop3->GetPreHeader(), &BB(2U));
    CheckVectorEqualBlocksIdSet(loop3->GetBackEdges(), {14U, 17U, 19U});
    CheckVectorEqualBlocksIdSet(loop3->GetBlocks(), {3U, 7U, 12U, 13U, 14U, 15U, 16U, 17U, 18U, 19U});
    CheckVectorEqualSet(loop3->GetInnerLoops(), {loop1, loop2});
    EXPECT_EQ(loop3->GetOuterLoop(), rootLoop);
    EXPECT_EQ(loop3->IsIrreducible(), false);

    EXPECT_EQ(BB(2U).GetLoop(), rootLoop);
    EXPECT_EQ(BB(20U).GetLoop(), rootLoop);
    CheckVectorEqualSet(rootLoop->GetInnerLoops(), {loop3});
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
void LoopAnalyzerTest::BuildGraphPreheaderInsert()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 5U).u64();
        PARAMETER(2U, 10U).u64();

        BASIC_BLOCK(2U, 4U, 5U)
        {
            INST(3U, Opcode::Not).u64().Inputs(0U);
            INST(19U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(20U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(19U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(5U, Opcode::Add).u64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(6U, Opcode::Add).u64().Inputs(0U, 2U);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(7U, Opcode::Phi).u64().Inputs({{4U, 5U}, {5U, 6U}, {8U, 12U}});
            INST(8U, Opcode::Phi).u64().Inputs({{4U, 5U}, {5U, 6U}, {8U, 12U}});
        }
        BASIC_BLOCK(7U, 8U, 9U)
        {
            INST(9U, Opcode::Mul).u64().Inputs(7U, 8U);
            INST(10U, Opcode::Compare).b().Inputs(9U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(8U, 6U)
        {
            INST(12U, Opcode::Mul).u64().Inputs(9U, 1U);
        }
        BASIC_BLOCK(9U, -1L)
        {
            INST(18U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(LoopAnalyzerTest, PreheaderInsert)
{
    BuildGraphPreheaderInsert();

    auto loop = BB(6U).GetLoop();
    ASSERT_NE(loop, nullptr);
    EXPECT_EQ(loop->GetHeader(), &BB(6U));
    CheckVectorEqualBlocksIdSet(loop->GetBackEdges(), {8U});

    auto preHeader = loop->GetPreHeader();
    ASSERT_EQ(preHeader->GetLoop(), loop->GetOuterLoop());
    CheckVectorEqualBlocksIdSet(preHeader->GetPredsBlocks(), {4U, 5U});
    CheckVectorEqualBlocksIdSet(preHeader->GetSuccsBlocks(), {6U});
    EXPECT_EQ(loop->GetHeader()->GetDominator(), preHeader);
    CheckVectorEqualBlocksIdSet(preHeader->GetDominatedBlocks(), {6U});

    CheckPhiInputs(&BB(6U));
    CheckPhiInputs(preHeader);
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
 *                                 [0]
 *                                  |
 *                                  v
 *                             [pre-header]
 *                                  |
 *                                  v
 *                                 [2]<------\
 *                                  |        |
 *                                  v        |
 *                                 [3]-------/
 *                                  |
 *                                  v
 *                             [pre-header]
 *                                  |
 *                                  v
 *                                 [4]<------\
 *                                  |        |
 *                                  v        |
 *                                 [5]-------/
 *                                  |
 *                                  V
 *                                 [6]----->[1]
 */
TEST_F(LoopAnalyzerTest, PreheaderInsert2)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 3U) {}
        BASIC_BLOCK(3U, 4U, 2U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 6U, 4U)
        {
            INST(6U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(8U, Opcode::ReturnVoid);
        }
    }

    auto loop1 = BB(2U).GetLoop();
    auto loop2 = BB(4U).GetLoop();
    CheckVectorEqualBlocksIdSet(loop1->GetPreHeader()->GetPredsBlocks(), {0U});
    CheckVectorEqualBlocksIdSet(loop1->GetPreHeader()->GetSuccsBlocks(), {2U});
    CheckVectorEqualBlocksIdSet(loop2->GetPreHeader()->GetPredsBlocks(), {3U});
    CheckVectorEqualBlocksIdSet(loop2->GetPreHeader()->GetSuccsBlocks(), {4U});
}

/*
 * Initial Graph:
 *                                 [0]
 *                                  |
 *                                  v
 *                                 [2]---------------\
 *                                  |                |
 *                                 [3]<------\      [5]<------\
 *                                  |        |       |        |
 *                                  v        |       v        |
 *                                 [4]-------/      [6]-------/
 *                                  |                |
 *                                  |----------------/
 *                                  |
 *                                  V
 *                                 [7]----->[1]
 *
 * After loop pre-headers insertion:
 *                                 [0]
 *                                  |
 *                                  v
 *                                 [2]---------------\
 *                                  |                |
 *                                 [3]<------\      [8]
 *                                  |        |       |
 *                                  v        |      [5]<------\
 *                                 [4]-------/       |        |
 *                                  |                v        |
 *                                  |               [6]-------/
 *                                  |----------------/
 *                                  |
 *                                  V
 *                                 [7]----->[1]
 *
 */
TEST_F(LoopAnalyzerTest, PreheaderInsert3)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, 7U, 3U)
        {
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5U);
        }

        BASIC_BLOCK(5U, 6U) {}
        BASIC_BLOCK(6U, 7U, 5U)
        {
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7U);
        }

        BASIC_BLOCK(7U, -1)
        {
            INST(9U, Opcode::ReturnVoid);
        }
    }

    auto loop1 = BB(3U).GetLoop();
    auto loop2 = BB(5U).GetLoop();
    ASSERT_EQ(loop1->GetPreHeader()->GetNextLoop(), loop1);
    ASSERT_EQ(loop2->GetPreHeader()->GetNextLoop(), loop2);
    ASSERT_NE(loop1->GetPreHeader(), loop2->GetPreHeader());
}

TEST_F(LoopAnalyzerTest, CountableLoopTest)
{
    // Loop isn't countable because const_step is negative and condition is CC_LT
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, -1L);
        PARAMETER(3U, 0U).ref();  // array
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(16U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 16U);  // 0 < len_array
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 2U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(16U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 8U, 0U);      // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 2U);                // i--
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 16U);  // i < len_array
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    auto loop = BB(3U).GetLoop();
    auto loopParser = CountableLoopParser(*loop);
    ASSERT_EQ(loopParser.Parse(), std::nullopt);
}

TEST_F(LoopAnalyzerTest, CountableLoopTestNegativeStep)
{
    // Countable because with negative const_step
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, -1L);
        PARAMETER(3U, 0U).ref();  // array
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(16U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 16U);  // 0 < len_array
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(16U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 2U});
            INST(10U, Opcode::Add).s32().Inputs(4U, 2U);  // i--
            INST(8U, Opcode::BoundsCheck).s32().Inputs(10U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 8U, 0U);     // a[i] = 0
            INST(13U, Opcode::Compare).CC(CC_GT).b().Inputs(10U, 0U);  // i > 0
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    auto loop = BB(3U).GetLoop();
    auto loopParser = CountableLoopParser(*loop);
    auto loopInfo = loopParser.Parse();
    ASSERT_NE(loopInfo, std::nullopt);
    auto loopInfoValue = loopInfo.value();
    ASSERT_EQ(loopInfoValue.ifImm, &INS(14U));
    ASSERT_EQ(loopInfoValue.init, &INS(16U));
    ASSERT_EQ(loopInfoValue.test, &INS(0U));
    ASSERT_EQ(loopInfoValue.update, &INS(10U));
    ASSERT_EQ(loopInfoValue.index, &INS(4U));
    ASSERT_EQ(loopInfoValue.constStep, 1U);
    ASSERT_EQ(loopInfoValue.normalizedCc, ConditionCode::CC_GT);
    ASSERT_FALSE(loopInfoValue.isInc);
}

TEST_F(LoopAnalyzerTest, CountableLoopTestIndexInInnerLoop)
{
    // Loop isn't countable because const_step is negative and condition is CC_LT
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, -1L);
        PARAMETER(3U, 0U).ref();  // array
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(16U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 16U);  // 0 < len_array
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 2U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(16U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 8U, 0U);      // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 2U);                // i--
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 16U);  // i < len_array
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    auto loop = BB(3U).GetLoop();
    auto loopParser = CountableLoopParser(*loop);
    ASSERT_EQ(loopParser.Parse(), std::nullopt);
}

TEST_F(LoopAnalyzerTest, CountableLoopTest1)
{
    // fix case when loop's update inst in inner loop.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(5U, 5U);
        CONSTANT(10U, 10U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Phi).s32().Inputs(0U, 7U);
            INST(3U, Opcode::Compare).b().CC(CC_GE).Inputs(2U, 10U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 2U)
        {
            INST(6U, Opcode::Phi).s32().Inputs(2U, 7U);
            INST(7U, Opcode::Add).s32().Inputs(6U, 1U);
            INST(8U, Opcode::Compare).b().CC(CC_GE).Inputs(7U, 5U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(11U, Opcode::Return).s32().Inputs(2U);
        }
    }
    auto loop = BB(2U).GetLoop();
    auto loopParser = CountableLoopParser(*loop);
    ASSERT_EQ(loopParser.Parse(), std::nullopt);
}

TEST_F(LoopAnalyzerTest, CountableLoopTest2)
{
    // fix case when, last backedge inst isn't IfImm.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(5U, 5U);
        CONSTANT(10U, 10U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Phi).s32().Inputs(0U, 7U);
            INST(3U, Opcode::Compare).b().CC(CC_LT).Inputs(2U, 10U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(7U, Opcode::Add).s32().Inputs(2U, 1U);
            INST(8U, Opcode::NOP);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(11U, Opcode::Return).s32().Inputs(2U);
        }
    }
    auto loop = BB(2U).GetLoop();
    auto loopParser = CountableLoopParser(*loop);
    auto loopInfo = loopParser.Parse();
    ASSERT_NE(loopInfo, std::nullopt);
    auto loopInfoValue = loopInfo.value();
    ASSERT_EQ(loopInfoValue.ifImm, &INS(4U));
    ASSERT_EQ(loopInfoValue.init, &INS(0U));
    ASSERT_EQ(loopInfoValue.test, &INS(10U));
    ASSERT_EQ(loopInfoValue.update, &INS(7U));
    ASSERT_EQ(loopInfoValue.index, &INS(2U));
    ASSERT_EQ(loopInfoValue.constStep, 1U);
    ASSERT_EQ(loopInfoValue.normalizedCc, ConditionCode::CC_LT);
    ASSERT_TRUE(loopInfoValue.isInc);
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
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(0U, 6U);
            INST(4U, Opcode::Compare).b().CC(CC_NE).Inputs(3U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(6U, Opcode::Add).s32().Inputs(3U, 2U);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 5U)
        {
            INST(7U, Opcode::Phi).s32().Inputs(3U, 8U);
            INST(8U, Opcode::Add).s32().Inputs(7U, 2U);
        }
    }

    auto loop1 = BB(2U).GetLoop();
    auto loop2 = BB(5U).GetLoop();
    ASSERT_FALSE(loop1->IsInfinite());
    ASSERT_TRUE(loop2->IsInfinite());
}

/**
 * Test checks the ASSERTION fail issue fix in a Loop::AppendBlock.
 *
 * Original program:
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
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);

        BASIC_BLOCK(10U, 11U, 12U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(12U, 13U, 11U)
        {
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(11U, 11U, 13U)
        {
            INST(6U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(13U, 11U, 14U)
        {
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(14U, -1L)
        {
            INST(10U, Opcode::ReturnVoid);
        }
    }

    // No asserts, just IrBuilder sanity check
}

/**
 * Check loops depth determination for the follwing pseudo-code:
 *   while(a > b)
 *       while(a > b)
 *           while(a > b) {}
 *           while(a > b) {}
 *           while(a > b) {}
 *   while(a > b) {}
 */
TEST_F(LoopAnalyzerTest, LoopDepth)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, 3U, 7U)  // loop depth 1
        {
            INST(3U, Opcode::If).CC(compiler::CC_GE).SrcType(DataType::INT32).Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, 4U, 2U)  // loop depth 2
        {
            INST(4U, Opcode::If).CC(compiler::CC_GE).SrcType(DataType::INT32).Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 4U, 5U)  // loop depth 3
        {
            INST(5U, Opcode::If).CC(compiler::CC_GE).SrcType(DataType::INT32).Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, 5U, 6U)  // loop depth 3
        {
            INST(6U, Opcode::If).CC(compiler::CC_GE).SrcType(DataType::INT32).Inputs(0U, 1U);
        }
        BASIC_BLOCK(6U, 6U, 3U)  // loop depth 3
        {
            INST(7U, Opcode::If).CC(compiler::CC_GE).SrcType(DataType::INT32).Inputs(0U, 1U);
        }
        BASIC_BLOCK(7U, 7U, 10U)  // loop depth 1
        {
            INST(8U, Opcode::If).CC(compiler::CC_GE).SrcType(DataType::INT32).Inputs(0U, 1U);
        }
        BASIC_BLOCK(10U, -1L)
        {
            INST(10U, Opcode::ReturnVoid);
        }
    }

    static constexpr unsigned MAX_DEPTH = 3;
    ArenaVector<Loop *> allLoops(GetGraph()->GetLocalAllocator()->Adapter());
    // Count loops number of each depth
    ArenaVector<unsigned> loopsDepthCounters(MAX_DEPTH + 1U, 0U, GetGraph()->GetLocalAllocator()->Adapter());

    CollectLoopsDFS(&allLoops, GetGraph()->GetRootLoop());
    for (auto loop : allLoops) {
        EXPECT_LE(loop->GetDepth(), MAX_DEPTH);
        loopsDepthCounters[loop->GetDepth()]++;
    }
    // Root loop should be zero-depth
    EXPECT_EQ(GetGraph()->GetRootLoop()->GetDepth(), 0U);
    EXPECT_EQ(loopsDepthCounters[0U], 1U);
    EXPECT_EQ(loopsDepthCounters[1U], 2U);
    EXPECT_EQ(loopsDepthCounters[2U], 1U);
    EXPECT_EQ(loopsDepthCounters[3U], 3U);
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
