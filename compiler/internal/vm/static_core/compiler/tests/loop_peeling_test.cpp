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
#include "optimizer/optimizations/loop_peeling.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/graph_cloner.h"

namespace ark::compiler {

// NOLINTBEGIN(readability-magic-numbers)
class LoopPeelingTest : public GraphTest {
protected:
    void BuildGraphTwoBackEdges(Graph *graph)
    {
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).b();
            PARAMETER(1U, 1U).u64();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(2U, Opcode::Phi).Inputs(1U, 4U, 6U).u64();
                INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
            }
            BASIC_BLOCK(3U, 2U, 5U)
            {
                INST(4U, Opcode::Add).Inputs(1U, 1U).u64();
                INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
            }
            BASIC_BLOCK(5U, 2U)
            {
                INST(6U, Opcode::Add).Inputs(4U, 4U).u64();
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(7U, Opcode::Return).u64().Inputs(2U);
            }
        }
    }

    void BuildGraphNotHeaderExit(Graph *graph)
    {
        GRAPH(graph)
        {
            PARAMETER(1U, 1U).u64();
            BASIC_BLOCK(2U, 6U, 7U)
            {
                INST(2U, Opcode::Phi).Inputs(1U, 6U).u64();
                INST(20U, Opcode::SaveState).NoVregs();
                INST(8U, Opcode::CallStatic).v0id().InputsAutoType(20U);
                INST(0U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_NE).Imm(0U).Inputs(1U);
            }
            BASIC_BLOCK(6U, 3U)
            {
                INST(3U, Opcode::Add).Inputs(1U, 2U).u64();
            }
            BASIC_BLOCK(7U, 3U)
            {
                INST(9U, Opcode::Add).Inputs(2U, 1U).u64();
            }
            BASIC_BLOCK(3U, 4U, 5U)
            {
                INST(10U, Opcode::Phi).Inputs(3U, 9U).u64();
                INST(4U, Opcode::Add).Inputs(1U, 10U).u64();
                INST(5U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_NE).Imm(0U).Inputs(2U);
            }
            BASIC_BLOCK(4U, 2U)
            {
                INST(6U, Opcode::Add).Inputs(4U, 4U).u64();
            }
            BASIC_BLOCK(5U, -1L)
            {
                INST(7U, Opcode::Return).u64().Inputs(4U);
            }
        }
    }

    void BuildGraphHeaderAndBackEdgeExit(Graph *graph)
    {
        GRAPH(graph)
        {
            CONSTANT(0U, 10U);
            CONSTANT(1U, 0U);
            CONSTANT(2U, 1U);
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(3U, Opcode::Phi).u64().Inputs(1U, 9U);
                INST(4U, Opcode::Phi).u64().Inputs(1U, 10U);
                INST(6U, Opcode::SafePoint).Inputs(0U, 3U, 4U, 4U).SrcVregs({0U, 1U, 2U, 3U});
                INST(7U, Opcode::Compare).CC(CC_EQ).b().Inputs(4U, 0U);
                INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
            }
            BASIC_BLOCK(3U, 2U, 4U)
            {
                INST(9U, Opcode::And).u64().Inputs(4U, 3U);
                INST(10U, Opcode::Add).u64().Inputs(4U, 2U);
                INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(12U, Opcode::Phi).u64().Inputs(4U, 10U);
                INST(13U, Opcode::Return).u64().Inputs(12U);
            }
        }
    }

    void BuildGraphMultiExit(Graph *graph)
    {
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).b();
            PARAMETER(1U, 1U).u64();
            BASIC_BLOCK(2U, 3U, 5U)
            {
                INST(2U, Opcode::Phi).Inputs(1U, 6U).u64();
                INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
            }
            BASIC_BLOCK(3U, 4U, 5U)
            {
                INST(4U, Opcode::Add).Inputs(1U, 1U).u64();
                INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
            }
            BASIC_BLOCK(4U, 2U)
            {
                INST(6U, Opcode::Add).Inputs(4U, 4U).u64();
            }
            BASIC_BLOCK(5U, -1L)
            {
                INST(7U, Opcode::Phi).Inputs(2U, 4U).u64();
                INST(8U, Opcode::Return).u64().Inputs(7U);
            }
        }
    }

    void BuildGraphCloneBlock();
    void BuildGraphSingleLoop();
    void BuildGraphInnerLoop();
    void BuildGraphLoopWithBranch();
    void BuildGraphSingleBlockLoop();
    void BuildGraphRepeatedCloneableInputs();
    void BuildGraphMultiSafePointsLoop();
    void BuildGraphNeedSaveStateBridge();
};

/*
 *              [0]
 *               |
 *               v
 *        /---->[2]-----\
 *        |      |      |
 *        |      v      v
 *        \-----[3]    [4]
 *                      |
 *                    [exit]
 */
void LoopPeelingTest::BuildGraphCloneBlock()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs(1U, 9U).u64();
            INST(4U, Opcode::Phi).u64().Inputs(2U, 10U).u64();
            INST(6U, Opcode::SafePoint).Inputs(0U, 3U, 4U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::Compare).CC(CC_EQ).b().Inputs(0U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(9U, Opcode::And).u64().Inputs(4U, 3U);
            INST(10U, Opcode::Add).u64().Inputs(4U, 2U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(13U, Opcode::Div).Inputs(3U, 4U).u64();
            INST(14U, Opcode::Return).u64().Inputs(13U);
        }
    }
}

TEST_F(LoopPeelingTest, CloneBlock)
{
    BuildGraphCloneBlock();
    GetGraph()->RunPass<LoopAnalyzer>();
    auto graphCloner = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator());
    graphCloner.CloneLoopHeader(&BB(2U), &BB(4U), BB(2U).GetLoop()->GetPreHeader());
    GetGraph()->RunPass<Cleanup>();

    auto expectedGraph = CreateEmptyGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        BASIC_BLOCK(8U, 2U, 4U)
        {
            INST(15U, Opcode::Compare).CC(CC_EQ).b().Inputs(0U, 1U);
            INST(16U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(15U);
        }
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs({{8U, 1U}, {3U, 9U}}).u64();
            INST(4U, Opcode::Phi).u64().Inputs({{8U, 2U}, {3U, 10U}}).u64();
            INST(6U, Opcode::SafePoint).Inputs(0U, 3U, 4U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::Compare).CC(CC_EQ).b().Inputs(0U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(9U, Opcode::And).u64().Inputs(4U, 3U);
            INST(10U, Opcode::Add).u64().Inputs(4U, 2U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(17U, Opcode::Phi).u64().Inputs({{2U, 3U}, {8U, 1U}}).u64();
            INST(18U, Opcode::Phi).u64().Inputs({{2U, 4U}, {8U, 2U}}).u64();
            INST(13U, Opcode::Div).Inputs(17U, 18U).u64();
            INST(14U, Opcode::Return).u64().Inputs(13U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

/*
 *              [0]
 *               |
 *               v
 *        /---->[2]-----\
 *        |      |      |
 *        |      v      v
 *        \-----[3]    [4]
 *                      |
 *                    [exit]

 * Transform to:
 *              [0]
 *               |
 *               v
 *           [pre-loop]---------\
 *               |              |
 *        /---->[2]             |
 *        |      |              |
 *        |      v              |
 *        |     [3]             |
 *        |      |              |
 *        |      v              v
 *        \--[loop-exit]--->[loop-outer]
 *                              |
 *                              v
 *                             [4]
 *                              |
 *                              v
 *                            [exit]
 */
void LoopPeelingTest::BuildGraphSingleLoop()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs(1U, 5U);
            INST(4U, Opcode::Phi).u64().Inputs(2U, 10U);
            INST(5U, Opcode::Sub).u64().Inputs(3U, 2U);
            INST(6U, Opcode::SafePoint).Inputs(0U, 3U, 4U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::Compare).CC(CC_EQ).b().Inputs(5U, 0U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(9U, Opcode::And).u64().Inputs(4U, 5U);
            INST(10U, Opcode::Add).u64().Inputs(9U, 4U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Return).u64().Inputs(4U);
        }
    }
}

TEST_F(LoopPeelingTest, SingleLoop)
{
    BuildGraphSingleLoop();
    auto expectedGraph = CreateEmptyGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        BASIC_BLOCK(5U, 2U, 4U)
        {
            INST(12U, Opcode::Sub).u64().Inputs(1U, 2U);
            INST(13U, Opcode::Compare).CC(CC_EQ).b().Inputs(12U, 0U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(2U, 2U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs({{5U, 12U}, {2U, 5U}});
            INST(4U, Opcode::Phi).u64().Inputs({{5U, 2U}, {2U, 10U}});
            INST(9U, Opcode::And).u64().Inputs(4U, 3U);
            INST(10U, Opcode::Add).u64().Inputs(9U, 4U);
            INST(5U, Opcode::Sub).u64().Inputs(3U, 2U);
            INST(6U, Opcode::SafePoint).Inputs(0U, 3U, 10U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::Compare).CC(CC_EQ).b().Inputs(5U, 0U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(16U, Opcode::Phi).u64().Inputs({{5U, 2U}, {2U, 10U}});
            INST(11U, Opcode::Return).u64().Inputs(16U);
        }
    }

    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    // NOTE(a.popov): remove these calls, see NOTE in LoopPeeling pass constructor
    GetGraph()->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

/*
 *              [0]
 *               |
 *               v
 *   /--------->[2]------------\
 *   |           |             |
 *   |           v             |
 *   |    /---->[3]-----\     [6]
 *   |    |      |      |      |
 *   |    |      v      v      |
 *   |    \-----[4]    [5]   [exit]
 *   |                  |
 *   |                  |
 *   \------------------/
 *
 * Transform to:
 *              [0]
 *               |
 *               v
 *   /--------->[2]-------------------------\
 *   |           |                          |
 *   |           v                         [6]
 *   |       [pre-loop]---------\           |
 *   |           |              |           v
 *   |           v              |         [exit]
 *   |    /---->[3]             |
 *   |    |      |              |
 *   |    |      v              |
 *   |    |     [4]             |
 *   |    |      |              |
 *   |    |      v              v
 *   |    \--[loop-exit]-->[loop-outer]
 *   |                          |
 *   |                          v
 *   \-------------------------[5]
 *
 */
void LoopPeelingTest::BuildGraphInnerLoop()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();  // count
        PARAMETER(1U, 1U).u64();  // i
        CONSTANT(2U, 100U);
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(4U, Opcode::Phi).u64().Inputs(0U, 8U);              // count
            INST(5U, Opcode::Phi).u64().Inputs(1U, 14U);             // i
            INST(6U, Opcode::Compare).CC(CC_LT).b().Inputs(5U, 2U);  // while i < 100
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(8U, Opcode::Phi).u64().Inputs(4U, 12U);              // count
            INST(9U, Opcode::Phi).u64().Inputs(5U, 13U);              // j = i
            INST(10U, Opcode::Compare).CC(CC_LT).b().Inputs(9U, 2U);  // while j < 100
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(12U, Opcode::Add).u64().Inputs(8U, 3U);  // j++
            INST(13U, Opcode::Add).u64().Inputs(9U, 3U);  // count++
        }
        BASIC_BLOCK(5U, 2U)
        {
            INST(14U, Opcode::Add).u64().Inputs(5U, 3U);  // i++
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(15U, Opcode::Return).u64().Inputs(4U);  // return count
        }
    }
}

TEST_F(LoopPeelingTest, InnerLoop)
{
    BuildGraphInnerLoop();
    auto expectedGraph = CreateEmptyGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 100U);
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, 7U, 6U)
        {
            INST(4U, Opcode::Phi).u64().Inputs(0U, 18U);
            INST(5U, Opcode::Phi).u64().Inputs(1U, 14U);
            INST(6U, Opcode::Compare).CC(CC_LT).b().Inputs(5U, 2U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(7U, 3U, 5U)
        {
            INST(16U, Opcode::Compare).CC(CC_LT).b().Inputs(5U, 2U);
            INST(17U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(16U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(8U, Opcode::Phi).u64().Inputs(4U, 12U);
            INST(9U, Opcode::Phi).u64().Inputs(5U, 13U);
            INST(12U, Opcode::Add).u64().Inputs(8U, 3U);
            INST(13U, Opcode::Add).u64().Inputs(9U, 3U);
            INST(10U, Opcode::Compare).CC(CC_LT).b().Inputs(13U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }

        BASIC_BLOCK(5U, 2U)
        {
            INST(18U, Opcode::Phi).u64().Inputs({{7U, 4U}, {3U, 12U}});
            INST(14U, Opcode::Add).u64().Inputs(5U, 3U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(15U, Opcode::Return).u64().Inputs(4U);
        }
    }

    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    GetGraph()->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
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
 *  Transform to:
 *              [0]
 *               |
 *               v
 *           [pre-loop]---------\
 *               |              |
 *               v              |
 *      /------>[2]             |
 *      |        |              |
 *      |       [3]             |
 *      |       / \             |
 *      |      v   v            |
 *      |     [4] [5]           |
 *      |       \ /             |
 *      |       [6]             |
 *      |        |              v
 *      \---[loop-exit]-->[loop-outer]
 *                             |
 *                            [7]
 *                             |
 *                           [exit]
 */
void LoopPeelingTest::BuildGraphLoopWithBranch()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 2U);
        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(3U, Opcode::Phi).Inputs(1U, 8U).u64();
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(6U, Opcode::Add).Inputs(3U, 2U).u64();
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(7U, Opcode::Mul).Inputs(3U, 2U).u64();
        }
        BASIC_BLOCK(6U, 2U)
        {
            INST(8U, Opcode::Phi).Inputs(6U, 7U).u64();
            INST(11U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::CallStatic).v0id().InputsAutoType(11U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(10U, Opcode::Return).Inputs(3U).u64();
        }
    }
}

TEST_F(LoopPeelingTest, LoopWithBranch)
{
    BuildGraphLoopWithBranch();
    auto expectedGraph = CreateEmptyGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 2U);
        BASIC_BLOCK(8U, 3U, 7U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(3U, Opcode::Phi).Inputs(1U, 8U).u64();
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(6U, Opcode::Add).Inputs(3U, 2U).u64();
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(7U, Opcode::Mul).Inputs(3U, 2U).u64();
        }
        BASIC_BLOCK(6U, 3U, 7U)
        {
            INST(8U, Opcode::Phi).Inputs(6U, 7U).u64();
            INST(13U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::CallStatic).v0id().InputsAutoType(13U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(12U, Opcode::Phi).Inputs({{8U, 1U}, {6U, 8U}}).u64();
            INST(10U, Opcode::Return).Inputs(12U).u64();
        }
    }

    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    GetGraph()->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

/*
 *              [0]
 *               |
 *               v
 *      --/---->[2]-----\
 *      | |      |      |
 *      | |      v      v
 *      | \-----[3]    [4]
 *      |        |      |
 *      \-------[5]   [exit]
 *
 * NotApplied
 */
TEST_F(LoopPeelingTest, TwoBackEdges)
{
    BuildGraphTwoBackEdges(GetGraph());
    auto graph = CreateEmptyGraph();
    BuildGraphTwoBackEdges(graph);

    EXPECT_FALSE(GetGraph()->RunPass<LoopPeeling>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *              [0]
 *               |
 *               v
 *      /------>[2]
 *      |        |
 *      |        v
 *      |       [3]--->[5]
 *      |        |      |
 *      \-------[4]   [exit]
 *
 * NotApplied
 */
TEST_F(LoopPeelingTest, NotHeaderExit)
{
    BuildGraphNotHeaderExit(GetGraph());
    auto graph = CreateEmptyGraph();
    BuildGraphNotHeaderExit(graph);

    EXPECT_FALSE(GetGraph()->RunPass<LoopPeeling>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *              [0]
 *               |
 *               v
 *        /---->[2]-----\
 *        |      |      |
 *        |      v      v
 *        \-----[3]--->[4]
 *                      |
 *                      v
 *                    [exit]
 *
 * NotApplied
 */
TEST_F(LoopPeelingTest, HeaderAndBackEdgeExit)
{
    BuildGraphHeaderAndBackEdgeExit(GetGraph());
    auto graph = CreateEmptyGraph();
    BuildGraphHeaderAndBackEdgeExit(graph);

    EXPECT_FALSE(GetGraph()->RunPass<LoopPeeling>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *              [0]
 *               |
 *               v
 *      /------>[2]----\
 *      |        |      |
 *      |        v      v
 *      |       [3]--->[5]
 *      |        |      |
 *      \-------[4]   [exit]
 *
 * NotApplied
 */
TEST_F(LoopPeelingTest, MultiExit)
{
    BuildGraphMultiExit(GetGraph());
    auto graph = CreateEmptyGraph();
    BuildGraphMultiExit(graph);

    EXPECT_FALSE(GetGraph()->RunPass<LoopPeeling>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LoopPeelingTest, RemoveDeadPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Phi).u64().Inputs(2U, 10U);
            INST(6U, Opcode::SafePoint).Inputs(3U, 4U).SrcVregs({0U, 1U});
            INST(7U, Opcode::Compare).CC(CC_EQ).b().Inputs(4U, 0U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(10U, Opcode::Add).u64().Inputs(4U, 2U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Return).u64().Inputs(4U);
        }
    }

    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    EXPECT_FALSE(INS(3U).HasUsers());
    EXPECT_EQ(INS(3U).GetBasicBlock(), nullptr);
}

void LoopPeelingTest::BuildGraphSingleBlockLoop()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(3U, Opcode::Phi).u64().Inputs(0U, 6U);
            INST(4U, Opcode::SafePoint).Inputs(3U).SrcVregs({0U});
            INST(5U, Opcode::Add).u64().Inputs(1U, 3U);
            INST(6U, Opcode::Add).u64().Inputs(0U, 3U);
            INST(7U, Opcode::Compare).CC(CC_EQ).b().Inputs(6U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(9U, Opcode::Return).u64().Inputs(5U);
        }
    }
}

TEST_F(LoopPeelingTest, SingleBlockLoop)
{
    BuildGraphSingleBlockLoop();
    auto expectedGraph = CreateEmptyGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(6U, 2U, 7U)
        {
            INST(12U, Opcode::Add).u64().Inputs(1U, 0U);
            INST(13U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(14U, Opcode::Compare).CC(CC_EQ).b().Inputs(13U, 2U);
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }

        BASIC_BLOCK(2U, 2U, 7U)
        {
            INST(3U, Opcode::Phi).u64().Inputs(13U, 6U);
            INST(4U, Opcode::SafePoint).Inputs(3U).SrcVregs({0U});
            INST(5U, Opcode::Add).u64().Inputs(1U, 3U);
            INST(6U, Opcode::Add).u64().Inputs(0U, 3U);
            INST(7U, Opcode::Compare).CC(CC_EQ).b().Inputs(6U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(17U, Opcode::Phi).u64().Inputs({{2U, 5U}, {6U, 12U}});
            INST(9U, Opcode::Return).u64().Inputs(17U);
        }
    }

    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    GetGraph()->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

void LoopPeelingTest::BuildGraphRepeatedCloneableInputs()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).u32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::LoadObject).u32().Inputs(0U);
            INST(3U, Opcode::Sub).u32().Inputs(2U, 1U);
            INST(4U, Opcode::Compare).CC(CC_EQ).b().Inputs(3U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(6U, Opcode::SaveState).Inputs(1U, 2U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::Return).u32().Inputs(2U);
        }
    }
}

TEST_F(LoopPeelingTest, RepeatedCloneableInputs)
{
    BuildGraphRepeatedCloneableInputs();
    auto expectedGraph = CreateEmptyGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).u32();
        BASIC_BLOCK(6U, 2U, 7U)
        {
            INST(9U, Opcode::LoadObject).u32().Inputs(0U);
            INST(10U, Opcode::Sub).u32().Inputs(9U, 1U);
            INST(11U, Opcode::Compare).CC(CC_EQ).b().Inputs(10U, 1U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(2U, 2U, 7U)
        {
            INST(2U, Opcode::LoadObject).u32().Inputs(0U);
            INST(3U, Opcode::Sub).u32().Inputs(2U, 1U);
            INST(4U, Opcode::Compare).CC(CC_EQ).b().Inputs(3U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(14U, Opcode::Phi).u32().Inputs({{2U, 2U}, {6U, 9U}});
            INST(7U, Opcode::Return).u32().Inputs(14U);
        }
    }
    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    GetGraph()->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

TEST_F(LoopPeelingTest, InfiniteLoop)
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
    EXPECT_FALSE(graph->RunPass<LoopPeeling>());
}

void LoopPeelingTest::BuildGraphMultiSafePointsLoop()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs(1U, 5U);
            INST(4U, Opcode::Phi).u64().Inputs(2U, 10U);
            INST(5U, Opcode::Sub).u64().Inputs(3U, 2U);
            INST(6U, Opcode::SafePoint).Inputs(0U, 5U, 4U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::Compare).CC(CC_EQ).b().Inputs(5U, 0U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(9U, Opcode::And).u64().Inputs(4U, 5U);
            INST(10U, Opcode::Add).u64().Inputs(9U, 4U);
            INST(12U, Opcode::SafePoint).Inputs(0U, 5U, 10U).SrcVregs({0U, 1U, 2U});
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Return).u64().Inputs(4U);
        }
    }
}

TEST_F(LoopPeelingTest, MultiSafePointsLoop)
{
    BuildGraphMultiSafePointsLoop();
    auto expectedGraph = CreateEmptyGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        BASIC_BLOCK(5U, 2U, 4U)
        {
            INST(13U, Opcode::Sub).u64().Inputs(1U, 2U);
            INST(14U, Opcode::Compare).CC(CC_EQ).b().Inputs(13U, 0U);
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(2U, 2U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs(13U, 5U);
            INST(4U, Opcode::Phi).u64().Inputs(2U, 10U);
            INST(9U, Opcode::And).u64().Inputs(4U, 3U);
            INST(10U, Opcode::Add).u64().Inputs(9U, 4U);
            INST(12U, Opcode::SafePoint).Inputs(0U, 3U, 10U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::Sub).u64().Inputs(3U, 2U);
            INST(6U, Opcode::SafePoint).Inputs(0U, 5U, 10U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::Compare).CC(CC_EQ).b().Inputs(5U, 0U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(16U, Opcode::Phi).u64().Inputs({{2U, 10U}, {5U, 2U}});
            INST(11U, Opcode::Return).u64().Inputs(16U);
        }
    }
    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    GetGraph()->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

TEST_F(LoopPeelingTest, LoopWithInlinedCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(4U, Opcode::CallStatic).v0id().Inlined().InputsAutoType(3U);
            INST(5U, Opcode::IfImm).SrcType(DataType::UINT32).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(6U, Opcode::ReturnInlined).Inputs(3U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(8U, Opcode::Throw).Inputs(1U, 7U);
        }
    }
    INS(7U).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(4U)));
    ASSERT_FALSE(GetGraph()->RunPass<LoopPeeling>());
}

void LoopPeelingTest::BuildGraphNeedSaveStateBridge()
{
    GRAPH(GetGraph())
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
}

TEST_F(LoopPeelingTest, NeedSaveStateBridge)
{
    BuildGraphNeedSaveStateBridge();
    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    GetGraph()->RunPass<Cleanup>();

    auto expectedGraph = CreateEmptyGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).u64();  // a
        PARAMETER(1U, 1U).u64();  // b
        PARAMETER(2U, 2U).ref();  // arr
        CONSTANT(3U, 1U);

        BASIC_BLOCK(4U, 2U, 3U)
        {
            INST(23U, Opcode::SaveState).Inputs(2U).SrcVregs({0U});
            INST(24U, Opcode::NullCheck).ref().Inputs(2U, 23U);
            INST(25U, Opcode::StoreArray).u64().Inputs(24U, 0U, 0U);   // arr[a] = a
            INST(26U, Opcode::Add).u64().Inputs(0U, 3U);               // a++
            INST(27U, Opcode::Compare).CC(CC_LT).b().Inputs(26U, 1U);  // while a < b
            INST(28U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(27U);
        }

        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(4U, Opcode::Phi).u64().Inputs(26U, 6U);
            INST(20U, Opcode::SaveState).Inputs(2U).SrcVregs({0U});
            INST(9U, Opcode::NullCheck).ref().Inputs(2U, 20U);
            INST(5U, Opcode::StoreArray).u64().Inputs(9U, 4U, 4U);   // arr[a] = a
            INST(6U, Opcode::Add).u64().Inputs(4U, 3U);              // a++
            INST(7U, Opcode::Compare).CC(CC_LT).b().Inputs(6U, 1U);  // while a < b
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(21U, Opcode::Phi).ref().Inputs(24U, 9U);
            // Phi with check inputs should be added to SaveState
            INST(12U, Opcode::SaveState).Inputs(2U, 21U).SrcVregs({0U, VirtualRegister::BRIDGE});
            INST(13U, Opcode::CallStatic).v0id().InputsAutoType(12U);
            INST(10U, Opcode::LoadArray).u64().Inputs(21U, 1U);
            INST(11U, Opcode::Return).u64().Inputs(10U);  // return arr[b]
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
