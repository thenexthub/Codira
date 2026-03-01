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
#include "optimizer/analysis/monitor_analysis.h"

namespace ark::compiler {
class MonitorAnalysisTest : public GraphTest {};

TEST_F(MonitorAnalysisTest, OneMonitorForOneBlock)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(1U, Opcode::Monitor).v0id().Entry().Inputs(0U, 4U);
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::Monitor).v0id().Exit().Inputs(0U, 5U);
            INST(3U, Opcode::ReturnVoid);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
    EXPECT_TRUE(BB(2U).GetMonitorEntryBlock());
    EXPECT_TRUE(BB(2U).GetMonitorExitBlock());
    EXPECT_TRUE(BB(2U).GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorBlock());
}

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(MonitorAnalysisTest, OneMonitorForSeveralBlocks)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).ref();
        CONSTANT(2U, 10U);
        CONSTANT(3U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(11U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(4U, Opcode::Monitor).v0id().Entry().Inputs(1U, 11U);
            INST(5U, Opcode::Compare).b().Inputs(0U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(7U, Opcode::Mul).u64().Inputs(0U, 3U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 7U}});
            INST(12U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(9U, Opcode::Monitor).v0id().Exit().Inputs(1U, 12U);
            INST(10U, Opcode::Return).u64().Inputs(8U);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
    EXPECT_TRUE(BB(2U).GetMonitorEntryBlock());
    EXPECT_FALSE(BB(2U).GetMonitorExitBlock());
    EXPECT_TRUE(BB(2U).GetMonitorBlock());

    EXPECT_FALSE(BB(3U).GetMonitorEntryBlock());
    EXPECT_FALSE(BB(3U).GetMonitorExitBlock());
    EXPECT_TRUE(BB(3U).GetMonitorBlock());

    EXPECT_FALSE(BB(4U).GetMonitorEntryBlock());
    EXPECT_TRUE(BB(4U).GetMonitorExitBlock());
    EXPECT_TRUE(BB(4U).GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorBlock());
}

SRC_GRAPH(TwoMonitorForSeveralBlocks, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).ref();
        PARAMETER(2U, 3U).ref();
        CONSTANT(3U, 10U);
        CONSTANT(4U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(14U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::Monitor).v0id().Entry().Inputs(1U, 14U);
            INST(15U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::Monitor).v0id().Entry().Inputs(2U, 15U);
            INST(16U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::Monitor).v0id().Exit().Inputs(1U, 16U);
            INST(8U, Opcode::Compare).b().Inputs(0U, 3U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(10U, Opcode::Mul).u64().Inputs(0U, 4U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 10U}});
            INST(17U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(12U, Opcode::Monitor).v0id().Exit().Inputs(2U, 17U);
            INST(13U, Opcode::Return).u64().Inputs(11U);
        }
    }
}

TEST_F(MonitorAnalysisTest, TwoMonitorForSeveralBlocks)
{
    src_graph::TwoMonitorForSeveralBlocks::CREATE(GetGraph());
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
    EXPECT_TRUE(BB(2U).GetMonitorEntryBlock());
    EXPECT_TRUE(BB(2U).GetMonitorExitBlock());
    EXPECT_TRUE(BB(2U).GetMonitorBlock());

    EXPECT_FALSE(BB(3U).GetMonitorEntryBlock());
    EXPECT_FALSE(BB(3U).GetMonitorExitBlock());
    EXPECT_TRUE(BB(3U).GetMonitorBlock());

    EXPECT_FALSE(BB(4U).GetMonitorEntryBlock());
    EXPECT_TRUE(BB(4U).GetMonitorExitBlock());
    EXPECT_TRUE(BB(4U).GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorBlock());
}

SRC_GRAPH(OneEntryMonitorAndTwoExitMonitors, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).ref();
        CONSTANT(2U, 10U);
        CONSTANT(3U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(12U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(4U, Opcode::Monitor).v0id().Entry().Inputs(1U, 12U);
            INST(5U, Opcode::Compare).b().Inputs(0U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(13U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(7U, Opcode::Monitor).v0id().Exit().Inputs(1U, 13U);
            INST(8U, Opcode::Mul).u64().Inputs(0U, 3U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(14U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(9U, Opcode::Monitor).v0id().Exit().Inputs(1U, 14U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::Phi).u64().Inputs({{4U, 0U}, {3U, 8U}});
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }
    }
}

TEST_F(MonitorAnalysisTest, OneEntryMonitorAndTwoExitMonitors)
{
    src_graph::OneEntryMonitorAndTwoExitMonitors::CREATE(GetGraph());
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
    EXPECT_TRUE(BB(2U).GetMonitorEntryBlock());
    EXPECT_FALSE(BB(2U).GetMonitorExitBlock());
    EXPECT_TRUE(BB(2U).GetMonitorBlock());

    EXPECT_FALSE(BB(3U).GetMonitorEntryBlock());
    EXPECT_TRUE(BB(3U).GetMonitorExitBlock());
    EXPECT_TRUE(BB(3U).GetMonitorBlock());

    EXPECT_FALSE(BB(4U).GetMonitorEntryBlock());
    EXPECT_TRUE(BB(4U).GetMonitorExitBlock());
    EXPECT_TRUE(BB(4U).GetMonitorBlock());

    EXPECT_FALSE(BB(5U).GetMonitorEntryBlock());
    EXPECT_FALSE(BB(5U).GetMonitorExitBlock());
    EXPECT_FALSE(BB(5U).GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorBlock());
}

/*
 *  Kernal case:
 *    bb 2
 *    |    \
 *    |      bb 3
 *    |    MonitorEntry
 *    |    /
 *    bb 4   - Conditional is equal to bb 2
 *    |    \
 *    |      bb 5
 *    |    MonitorExit
 *    |    /
 *    bb 6
 *
 *  The monitor analysis marks all blocks (excluding bb 2) as BlockMonitor
 */
TEST_F(MonitorAnalysisTest, KernalCase)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).ref();
        CONSTANT(2U, 10U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().Inputs(0U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(6U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::Monitor).v0id().Entry().Inputs(1U, 6U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(10U, Opcode::Compare).b().Inputs(0U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(12U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(13U, Opcode::Monitor).v0id().Exit().Inputs(1U, 12U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(15U, Opcode::ReturnVoid);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_FALSE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
}

/*
 *    bb 2
 *    |    \
 *    |      bb 3
 *    |    MonitorEntry
 *    |    /
 *    bb 4
 *    |
 *    bb 5
 *    MonitorExit
 *    |
 *    bb 6
 *
 *  MonitorAnalysis must return false because
 *  - MonitorEntry is optional
 *  - MonitorExit is not optional
 */
TEST_F(MonitorAnalysisTest, InconsistentMonitorsNumberCase1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).ref();
        CONSTANT(2U, 10U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().Inputs(0U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(6U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::Monitor).v0id().Entry().Inputs(1U, 6U);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 6U)
        {
            INST(12U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(13U, Opcode::Monitor).v0id().Exit().Inputs(1U, 12U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(15U, Opcode::ReturnVoid);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_FALSE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
}

/*
 *    bb 2
 *    MonitorEntry
 *    |   \
 *    |    bb 3
 *    |    MonitorExit
 *    |   /
 *    bb 4
 *    MonitorExit
 *    |
 *    bb 5
 *
 *  MonitorAnalysis must return false because two Exits can happen for the single monitor
 */
TEST_F(MonitorAnalysisTest, InconsistentMonitorsNumberCase2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).ref();
        CONSTANT(2U, 10U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(14U, Opcode::Monitor).v0id().Entry().Inputs(1U, 3U);
            INST(4U, Opcode::Compare).b().Inputs(0U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(6U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::Monitor).v0id().Exit().Inputs(1U, 6U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(9U, Opcode::Monitor).v0id().Exit().Inputs(1U, 8U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(15U, Opcode::ReturnVoid);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_FALSE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
}

/*
 *  MonitorAnalysis should be Ok about Monitor Entry/Exit conunter mismatch when Throw happens within
 *  the synchronized block
 */
TEST_F(MonitorAnalysisTest, MonitorAndThrow)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).ref();
        PARAMETER(1U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(4U, Opcode::Monitor).v0id().Entry().Inputs(1U, 3U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::Throw).Inputs(0U, 5U);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
