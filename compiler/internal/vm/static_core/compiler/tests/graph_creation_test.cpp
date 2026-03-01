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
#include "optimizer/optimizations/loop_unroll.h"
#include "optimizer/optimizations/loop_peeling.h"

namespace ark::compiler {
class GraphCreationTest : public CommonTest {};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(GraphCreationTest, CreateEmptyGraph)
{
    Graph *graph = CreateEmptyGraph();
    EXPECT_NE(graph, nullptr);
}

TEST_F(GraphCreationTest, CreateGraphStartEndBlocks)
{
    Graph *graph = CreateGraphStartEndBlocks();
    EXPECT_NE(graph, nullptr);
    EXPECT_NE(graph->GetStartBlock(), nullptr);
    EXPECT_NE(graph->GetEndBlock(), nullptr);
    EXPECT_EQ(graph->GetAliveBlocksCount(), 2U);
}

TEST_F(GraphCreationTest, OsrModeGraph)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
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
    BB(2U).SetOsrEntry(true);
    auto cloneGraph = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();

    graph->RunPass<LoopPeeling>();
    graph->RunPass<LoopUnroll>(1000U, 4U);
    EXPECT_TRUE(GraphComparator().Compare(graph, cloneGraph));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
