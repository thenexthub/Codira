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
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/code_sink.h"

namespace ark::compiler {
class CodeSinkTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(CodeSinkTest, OperationPropagation)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Add).s64().Inputs(1U, 2U);
            INST(6U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 3U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Return).s64().Inputs(5U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(11U, Opcode::Return).s64().Inputs(3U);
        }
    }
    Graph *sunkGraph = CreateEmptyGraph();
    GRAPH(sunkGraph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 3U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(5U, Opcode::Add).s64().Inputs(1U, 2U);
            INST(9U, Opcode::Return).s64().Inputs(5U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(11U, Opcode::Return).s64().Inputs(3U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), sunkGraph));
}

/**
 * Move load but the NullCheck is still on its place:
 * exception should be thrown where it was initially.
 */
// NOTE(Kudriashov Evgenii) enable the test after fixing CodeSink
SRC_GRAPH(DISABLED_LoadWithOperationPropagation, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).ref();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(3U, 2U).SrcVregs({0U, 4U});
            INST(5U, Opcode::NullCheck).ref().Inputs(2U, 4U);
            INST(6U, Opcode::LoadObject).s64().Inputs(5U).TypeId(176U);
            INST(7U, Opcode::Add).s64().Inputs(6U, 1U);
            INST(8U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 3U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Return).s64().Inputs(7U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(13U, Opcode::Return).s64().Inputs(3U);
        }
    }
}

TEST_F(CodeSinkTest, DISABLED_LoadWithOperationPropagation)
{
    src_graph::DISABLED_LoadWithOperationPropagation::CREATE(GetGraph());
    Graph *sunkGraph = CreateEmptyGraph();
    GRAPH(sunkGraph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).ref();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(3U, 2U).SrcVregs({0U, 4U});
            INST(5U, Opcode::NullCheck).ref().Inputs(2U, 4U);
            INST(8U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 3U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::LoadObject).s64().Inputs(5U).TypeId(176U);
            INST(7U, Opcode::Add).s64().Inputs(6U, 1U);
            INST(11U, Opcode::Return).s64().Inputs(7U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(13U, Opcode::Return).s64().Inputs(3U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), sunkGraph));
}

/// Do not move anything
TEST_F(CodeSinkTest, NoDomination)
{
    std::vector<Graph *> equalGraphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equalGraphs) {
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s64();
            PARAMETER(1U, 1U).s64();
            PARAMETER(2U, 2U).s64();
            CONSTANT(3U, 0x0U).s64();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(5U, Opcode::Add).s64().Inputs(1U, 2U);
                INST(6U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 3U);
                INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(9U, Opcode::Return).s64().Inputs(5U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(11U, Opcode::Return).s64().Inputs(5U);
            }
        }
    }
    equalGraphs[0U]->RunPass<CodeSink>();
    GraphChecker(equalGraphs[0U]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equalGraphs[0U], equalGraphs[1U]));
}

/// Do not sink loads that may alias further stores in the block
TEST_F(CodeSinkTest, LoadStoreAliasing)
{
    std::vector<Graph *> equalGraphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equalGraphs) {
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).ref();
            PARAMETER(1U, 1U).ref();
            PARAMETER(2U, 2U).s64();
            CONSTANT(3U, 0x0U).s64();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(6U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
                INST(9U, Opcode::StoreObject).s64().Inputs(1U, 2U).TypeId(243U);
                INST(10U, Opcode::Compare).b().CC(CC_NE).Inputs(2U, 3U);
                INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(13U, Opcode::Return).s64().Inputs(6U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(15U, Opcode::Return).s64().Inputs(3U);
            }
        }
    }
    equalGraphs[0U]->RunPass<CodeSink>();
    GraphChecker(equalGraphs[0U]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equalGraphs[0U], equalGraphs[1U]));
}

SRC_GRAPH(LoopSinking, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).ref();
        CONSTANT(3U, 0x1U).s64();
        CONSTANT(4U, 0x0U).s64();
        CONSTANT(5U, 0x42U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            // Do not sink into loop
            INST(6U, Opcode::Add).s64().Inputs(1U, 5U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(10U, Opcode::Phi).s64().Inputs({{2U, 4U}, {3U, 22U}});
            INST(20U, Opcode::LoadArray).s64().Inputs(2U, 10U);
            INST(21U, Opcode::Add).s64().Inputs(20U, 6U);
            INST(22U, Opcode::Add).s64().Inputs(21U, 10U);
            INST(23U, Opcode::Add).s32().Inputs(10U, 3U);
            // Sink out of loop
            INST(26U, Opcode::Add).s64().Inputs(21U, 22U);
            INST(24U, Opcode::Compare).b().CC(CC_LT).Inputs(23U, 0U);
            INST(25U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(24U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(27U, Opcode::Return).s64().Inputs(26U);
        }
    }
}

TEST_F(CodeSinkTest, LoopSinking)
{
    src_graph::LoopSinking::CREATE(GetGraph());
    Graph *sunkGraph = CreateEmptyGraph();
    GRAPH(sunkGraph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).ref();
        CONSTANT(3U, 0x1U).s64();
        CONSTANT(4U, 0x0U).s64();
        CONSTANT(5U, 0x42U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(6U, Opcode::Add).s64().Inputs(1U, 5U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(10U, Opcode::Phi).s64().Inputs({{2U, 4U}, {3U, 22U}});
            INST(20U, Opcode::LoadArray).s64().Inputs(2U, 10U);
            INST(21U, Opcode::Add).s64().Inputs(20U, 6U);
            INST(22U, Opcode::Add).s64().Inputs(21U, 10U);
            INST(23U, Opcode::Add).s32().Inputs(10U, 3U);
            INST(24U, Opcode::Compare).b().CC(CC_LT).Inputs(23U, 0U);
            INST(25U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(24U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(26U, Opcode::Add).s64().Inputs(21U, 22U);
            INST(27U, Opcode::Return).s64().Inputs(26U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), sunkGraph));
}

/// Sink instruction over critical edge
SRC_GRAPH(CriticalEdgeSinking, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s64();
        PARAMETER(3U, 3U).s64();
        CONSTANT(4U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Add).s32().Inputs(3U, 2U);
            INST(8U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            INST(9U, Opcode::Compare).b().CC(CC_NE).Inputs(8U, 4U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(13U, Opcode::StoreObject).s64().Inputs(1U, 8U).TypeId(243U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(15U, Opcode::Return).s32().Inputs(5U);
        }
    }
}

TEST_F(CodeSinkTest, CriticalEdgeSinking)
{
    src_graph::CriticalEdgeSinking::CREATE(GetGraph());
    Graph *sunkGraph = CreateEmptyGraph();
    GRAPH(sunkGraph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s64();
        PARAMETER(3U, 3U).s64();
        CONSTANT(4U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(8U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            INST(9U, Opcode::Compare).b().CC(CC_NE).Inputs(8U, 4U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(13U, Opcode::StoreObject).s64().Inputs(1U, 8U).TypeId(243U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Add).s32().Inputs(3U, 2U);
            INST(15U, Opcode::Return).s32().Inputs(5U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), sunkGraph));
}

/// Do not sink loads over monitor
SRC_GRAPH(LoadOverMonitor, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).ref();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(2U, 1U, 0U, 3U).SrcVregs({4U, 3U, 2U, 0U});
            INST(5U, Opcode::NullCheck).ref().Inputs(2U, 4U);
            // Do not move load
            INST(6U, Opcode::LoadObject).s64().Inputs(5U).TypeId(243U);
            // Safely move arithmetic operations
            INST(7U, Opcode::Add).s64().Inputs(6U, 1U);
            INST(15U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(8U, Opcode::Monitor).v0id().Entry().Inputs(0U, 15U);
            INST(9U, Opcode::Compare).b().CC(CC_NE).Inputs(1U, 3U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(16U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(11U, Opcode::Monitor).v0id().Exit().Inputs(0U, 16U);
            INST(12U, Opcode::Return).s64().Inputs(7U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(17U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(13U, Opcode::Monitor).v0id().Exit().Inputs(0U, 17U);
            INST(14U, Opcode::Return).s64().Inputs(3U);
        }
    }
}

TEST_F(CodeSinkTest, LoadOverMonitor)
{
    src_graph::LoadOverMonitor::CREATE(GetGraph());
    Graph *sunkGraph = CreateEmptyGraph();
    GRAPH(sunkGraph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).ref();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(2U, 1U, 0U, 3U).SrcVregs({4U, 3U, 2U, 0U});
            INST(5U, Opcode::NullCheck).ref().Inputs(2U, 4U);
            INST(6U, Opcode::LoadObject).s64().Inputs(5U).TypeId(243U);
            INST(15U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(8U, Opcode::Monitor).v0id().Entry().Inputs(0U, 15U);
            INST(9U, Opcode::Compare).b().CC(CC_NE).Inputs(1U, 3U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::Add).s64().Inputs(6U, 1U);
            INST(16U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(11U, Opcode::Monitor).v0id().Exit().Inputs(0U, 16U);
            INST(12U, Opcode::Return).s64().Inputs(7U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(17U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(13U, Opcode::Monitor).v0id().Exit().Inputs(0U, 17U);
            INST(14U, Opcode::Return).s64().Inputs(3U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), sunkGraph));
}

/// Reordering of Normal Load and subsequent Volatile Load is allowed
// NOTE(Kudriashov Evgenii) enable the test after fixing CodeSink
SRC_GRAPH(DISABLED_LoadOverVolatileLoad, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(2U, 2U).ref();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(2U).SrcVregs({0U});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(0U);
            INST(6U, Opcode::LoadObject).s64().Inputs(2U).TypeId(176U);
            INST(7U, Opcode::LoadStatic).s64().Inputs(5U).Volatile().TypeId(103U);
            INST(8U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 7U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Return).s64().Inputs(6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(13U, Opcode::Return).s64().Inputs(3U);
        }
    }
}

TEST_F(CodeSinkTest, DISABLED_LoadOverVolatileLoad)
{
    src_graph::DISABLED_LoadOverVolatileLoad::CREATE(GetGraph());
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(2U, 2U).ref();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(2U).SrcVregs({0U});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(0U);
            INST(7U, Opcode::LoadStatic).s64().Inputs(5U).Volatile().TypeId(103U);
            INST(8U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 7U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::LoadObject).s64().Inputs(2U).TypeId(176U);
            INST(11U, Opcode::Return).s64().Inputs(6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(13U, Opcode::Return).s64().Inputs(3U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

/**
 *   /---[2]---\
 *   |         |
 *  [3]<------[4]
 *   |         |
 *   |        [5]
 *   |         |
 *   \---[6]---/
 *
 * Sink from BB 4 to BB 5
 */
SRC_GRAPH(IntermediateSinking, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        CONSTANT(4U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(1U, 4U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(4U, 5U, 3U)
        {
            INST(6U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(7U, Opcode::Compare).b().CC(CC_EQ).Inputs(2U, 4U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(10U, Opcode::SaveState).Inputs(6U, 2U, 1U, 0U, 6U).SrcVregs({5U, 4U, 3U, 2U, 1U});
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::StoreObject).s32().Inputs(11U, 6U).TypeId(271U);
        }
        BASIC_BLOCK(3U, 6U) {}
        BASIC_BLOCK(6U, -1L)
        {
            INST(18U, Opcode::Return).s32().Inputs(2U);
        }
    }
}

TEST_F(CodeSinkTest, IntermediateSinking)
{
    src_graph::IntermediateSinking::CREATE(GetGraph());
    Graph *sunkGraph = CreateEmptyGraph();
    GRAPH(sunkGraph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        CONSTANT(4U, 0x0U).s64();
        BASIC_BLOCK(2U, 6U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(1U, 4U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(7U, Opcode::Compare).b().CC(CC_EQ).Inputs(2U, 4U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(6U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(10U, Opcode::SaveState).Inputs(6U, 2U, 1U, 0U, 6U).SrcVregs({5U, 4U, 3U, 2U, 1U});
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::StoreObject).s32().Inputs(11U, 6U).TypeId(271U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(18U, Opcode::Return).s32().Inputs(2U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), sunkGraph));
}

/// Do not sink object allocations
TEST_F(CodeSinkTest, Allocations)
{
    std::vector<Graph *> equalGraphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equalGraphs) {
        GRAPH(graph)
        {
            PARAMETER(1U, 1U).s64();
            CONSTANT(4U, 0x0U).s64();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(18U, Opcode::SaveState).Inputs(1U).SrcVregs({3U});
                INST(19U, Opcode::LoadAndInitClass).TypeId(231U).ref().Inputs(18U);
                INST(2U, Opcode::NewObject).ref().TypeId(231U).Inputs(19U, 18U);
                INST(3U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 4U);
                INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
            }
            BASIC_BLOCK(4U, 5U)
            {
                INST(8U, Opcode::LoadObject).s64().Inputs(2U).TypeId(243U);
                INST(11U, Opcode::LoadObject).s64().Inputs(2U).TypeId(257U);
                INST(12U, Opcode::Add).s64().Inputs(11U, 8U);
            }
            BASIC_BLOCK(3U, 5U)
            {
                INST(14U, Opcode::Neg).s64().Inputs(1U);
            }
            BASIC_BLOCK(5U, -1L)
            {
                INST(16U, Opcode::Phi).s64().Inputs({{4U, 12U}, {3U, 14U}});
                INST(17U, Opcode::Return).s64().Inputs(16U);
            }
        }
    }
    equalGraphs[0U]->RunPass<CodeSink>();
    GraphChecker(equalGraphs[0U]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equalGraphs[0U], equalGraphs[1U]));
}

/// Do not sink over PHI statement
TEST_F(CodeSinkTest, PhiUsers)
{
    std::vector<Graph *> equalGraphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equalGraphs) {
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s64();
            CONSTANT(5U, 0x0U).s64();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::AddI).s64().Inputs(0U).Imm(0x3U);
                INST(4U, Opcode::Compare).b().CC(CC_GT).Inputs(0U, 5U);
                INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
            }
            BASIC_BLOCK(4U, 3U)
            {
                INST(8U, Opcode::Neg).s64().Inputs(0U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(9U, Opcode::Phi).s64().Inputs({{2U, 12U}, {4U, 8U}});
                INST(11U, Opcode::Return).s64().Inputs(9U);
            }
        }
    }
    equalGraphs[0U]->RunPass<CodeSink>();
    GraphChecker(equalGraphs[0U]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equalGraphs[0U], equalGraphs[1U]));
}

/// Do not sink volatile loads because other paths might be broken because of it
TEST_F(CodeSinkTest, SinkableVolatileLoad)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(2U, 2U).ref();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::LoadObject).s64().Volatile().Inputs(2U).TypeId(176U);
            INST(8U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 3U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Return).s64().Inputs(6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(13U, Opcode::Return).s64().Inputs(3U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Do not sink loads over volatile store
TEST_F(CodeSinkTest, LoadOverVolatileStore)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).ref();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(1U, 2U).SrcVregs({0U, 1U});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(0U);
            INST(6U, Opcode::LoadObject).s64().Inputs(2U).TypeId(176U);
            INST(7U, Opcode::StoreStatic).ref().Volatile().Inputs(5U, 1U).TypeId(103U);
            INST(8U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 3U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Return).s64().Inputs(6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(13U, Opcode::Return).s64().Inputs(3U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Do not sink loads over GC barriered store.
TEST_F(CodeSinkTest, LoadOverRefStore)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).ref();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::LoadArray).ref().Inputs(1U, 0U);
            INST(7U, Opcode::StoreArray).ref().Inputs(1U, 0U, 2U);
            INST(8U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 3U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Return).ref().Inputs(6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(13U, Opcode::Return).ref().Inputs(2U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Do not sink into irreducible loops.
TEST_F(CodeSinkTest, SinkIntoIrreducible)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        CONSTANT(5U, 0x2aU).s64();
        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(8U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 7U, 9U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(1U);
        }
        BASIC_BLOCK(6U, 10U, 9U)
        {
            INST(20U, Opcode::Phi).s32().Inputs({{2U, 5U}, {7U, 11U}});
            INST(26U, Opcode::Mul).s32().Inputs(20U, 5U);
            INST(28U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(2U);
        }
        // BB 10 and BB 7 represent an irreducible loop. Do not sink v26 into it.
        BASIC_BLOCK(10U, 7U)
        {
            INST(30U, Opcode::SaveState).NoVregs();
            INST(36U, Opcode::CallStatic).s64().InputsAutoType(26U, 30U);
        }
        BASIC_BLOCK(7U, 6U, 9U)
        {
            INST(11U, Opcode::Phi).s32().Inputs({{3U, 5U}, {10U, 26U}});
            INST(19U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(9U, -1L)
        {
            INST(34U, Opcode::Phi).s32().Inputs({{3U, 1U}, {6U, 2U}, {7U, 3U}});
            INST(35U, Opcode::Return).s32().Inputs(34U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Do not try to sink if an instruction has no uses. It's a waste of time.
TEST_F(CodeSinkTest, UselessSinking)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            // v5 is not used anywhere
            INST(5U, Opcode::Add).s64().Inputs(1U, 2U);
            INST(6U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 3U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Return).s64().Inputs(0U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(11U, Opcode::Return).s64().Inputs(3U);
        }
    }

    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
