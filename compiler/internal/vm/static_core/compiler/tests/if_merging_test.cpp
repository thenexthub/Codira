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
#include "optimizer/optimizations/if_merging.h"
#include "optimizer/optimizations/branch_elimination.h"

namespace ark::compiler {

// NOLINTBEGIN(readability-magic-numbers)

class IfMergingTest : public GraphTest {
public:
    Graph *CreateExpectedSameIfs(bool inverse);
    Graph *CreateExpectedCheckInstsSplit();
    Graph *CreateExpectedConstPhiIf();
    Graph *CreateExpConstantEqualToAllPhiInputs();
    Graph *CreateExpConstantNotEqualToPhiInputs();
    void CreateInitialGraphDuplicatePhiInputs();
    Graph *CreateExpectedSplitPhiIntoTwo();
    Graph *CreateExpectedConstantPhiLoopBackEdge(bool inverse);
    void CreateNestedIfGraph();
};

Graph *IfMergingTest::CreateExpectedSameIfs(bool inverse)
{
    auto graphExpected = CreateEmptyGraph();
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(8U, inverse ? Opcode::Sub : Opcode::Add).u64().Inputs(1U, 0U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(9U, inverse ? Opcode::Add : Opcode::Sub).u64().Inputs(2U, 0U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::Phi).u64().Inputs({{3U, 8U}, {4U, 9U}});
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }
    }
    return graphExpected;
}

// Duplicate Ifs dominating each other can be merged
TEST_F(IfMergingTest, SameIfs)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            CONSTANT(1U, 0U);
            CONSTANT(2U, 1U);
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
                INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
            }
            BASIC_BLOCK(3U, 4U) {}
            BASIC_BLOCK(4U, 5U, 6U)
            {
                INST(5U, Opcode::Phi).u64().Inputs({{3U, 1U}, {2U, 2U}});
                INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(inverse ? CC_EQ : CC_NE).Imm(0U).Inputs(3U);
            }
            BASIC_BLOCK(5U, 7U)
            {
                INST(8U, Opcode::Add).u64().Inputs(5U, 0U);
            }
            BASIC_BLOCK(6U, 7U)
            {
                INST(9U, Opcode::Sub).u64().Inputs(5U, 0U);
            }
            BASIC_BLOCK(7U, -1L)
            {
                INST(10U, Opcode::Phi).u64().Inputs({{5U, 8U}, {6U, 9U}});
                INST(11U, Opcode::Return).u64().Inputs(10U);
            }
        }

        ASSERT_TRUE(graph->RunPass<IfMerging>());
        ASSERT_TRUE(graph->RunPass<Cleanup>());

        auto graphExpected = CreateExpectedSameIfs(inverse);
        ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
    }
}

// If inputs are different, not applied
TEST_F(IfMergingTest, DifferentIfs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(5U, Opcode::Phi).u64().Inputs({{3U, 1U}, {2U, 2U}});
            INST(12U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 2U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(12U);
        }
        BASIC_BLOCK(5U, 7U)
        {
            INST(8U, Opcode::Add).u64().Inputs(5U, 0U);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(9U, Opcode::Sub).u64().Inputs(5U, 0U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(10U, Opcode::Phi).u64().Inputs({{5U, 8U}, {6U, 9U}});
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// If an instruction from second If block is used in both branches, we would need to copy it.
// Not applied
TEST_F(IfMergingTest, InstUsedInBothBranches)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(5U, Opcode::Phi).u64().Inputs({{3U, 1U}, {2U, 2U}});
            INST(6U, Opcode::Mul).u64().Inputs(5U, 5U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(5U, 7U)
        {
            INST(8U, Opcode::Add).u64().Inputs(6U, 0U);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(9U, Opcode::Sub).u64().Inputs(6U, 0U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(10U, Opcode::Phi).u64().Inputs({{5U, 8U}, {6U, 9U}});
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

Graph *IfMergingTest::CreateExpectedCheckInstsSplit()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(1U, 5U);
            INST(10U, Opcode::Mul).u64().Inputs(2U, 2U);
            INST(12U, Opcode::Add).u64().Inputs(10U, 2U);
            INST(15U, Opcode::Return).u64().Inputs(12U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(2U, 7U);
            INST(11U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(14U, Opcode::Return).u64().Inputs(11U);
        }
    }
    return graph;
}

// Instruction in If block can be split to true and false branches
TEST_F(IfMergingTest, CheckInstsSplit)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(1U, 5U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(2U, 7U);
        }
        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{4U, 1U}, {3U, 2U}});
            INST(10U, Opcode::Mul).u64().Inputs(9U, 9U);
            INST(11U, Opcode::Add).u64().Inputs(10U, 9U);
            INST(12U, Opcode::Sub).u64().Inputs(0U, 9U);
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(14U, Opcode::Return).u64().Inputs(11U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(15U, Opcode::Return).u64().Inputs(12U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateExpectedCheckInstsSplit();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *        [0]
 *         |
 *         v
 *  [3]<--[2]<--\
 *   |     |    |
 *   |     v    |
 *   \--->[4]---/
 *         |
 *         v
 *        [5]
 *
 * Transform to (before cleanup):
 *        [0]
 *         |
 *         v
 *  [3]<--[2]<--\
 *   |     |    |
 *   v     v    |
 *  [4']  [4]---/
 *   |
 *   v
 *  [5]
 *
 */
TEST_F(IfMergingTest, SameIfsLoopBackEdge)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            CONSTANT(1U, 0U);
            CONSTANT(2U, 1U);
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(3U, Opcode::Phi).u64().Inputs({{0U, 0U}, {4U, 4U}});
                INST(4U, Opcode::Sub).u64().Inputs(3U, 2U);
                INST(5U, Opcode::Compare).b().CC(CC_EQ).Inputs(4U, 1U);
                INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
            }
            BASIC_BLOCK(3U, 4U) {}
            BASIC_BLOCK(4U, 5U, 2U)
            {
                INST(7U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 1U}});
                INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(inverse ? CC_EQ : CC_NE).Imm(0U).Inputs(5U);
            }
            BASIC_BLOCK(5U, -1L)
            {
                INST(9U, Opcode::Return).u64().Inputs(7U);
            }
        }

        ASSERT_TRUE(graph->RunPass<IfMerging>());
        ASSERT_TRUE(graph->RunPass<Cleanup>());

        auto graphExpected = CreateEmptyGraph();
        GRAPH(graphExpected)
        {
            PARAMETER(0U, 0U).u64();
            CONSTANT(1U, 0U);
            CONSTANT(2U, 1U);
            BASIC_BLOCK(2U, 3U, 2U)
            {
                INST(3U, Opcode::Phi).u64().Inputs({{0U, 0U}, {2U, 4U}});
                INST(4U, Opcode::Sub).u64().Inputs(3U, 2U);
                INST(5U, Opcode::Compare).b().CC(CC_EQ).Inputs(4U, 1U);
                INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(9U, Opcode::Return).u64().Inputs(inverse ? 0U : 1U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
    }
}

/*
 * We cannot remove the second If because branches of the first If intersect
 *         [entry]
 *            |
 *            v
 *           [if]-->[4]
 *            |      |
 *            v      v
 *           [3]--->[5]
 *            |      |
 *            v      |
 *   [7]<----[if]<---/
 *    |       |
 *    v       v
 *  [exit]<--[8]
 */
TEST_F(IfMergingTest, SameIfsMixedBranches)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_NE).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }

        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(6U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_NE).Inputs(0U, 2U);
            INST(7U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(4U, 5U) {}

        BASIC_BLOCK(5U, 6U) {}

        BASIC_BLOCK(6U, 7U, 8U)
        {
            INST(8U, Opcode::Phi).b().Inputs(1U, 2U);
            INST(9U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(10U, Opcode::Return).u64().Inputs(1U);
        }

        BASIC_BLOCK(8U, -1L)
        {
            INST(11U, Opcode::Return).u64().Inputs(0U);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// Equivalent If instructions dominate each other, but block with the second If has
// more than two predecessors. Not applied
TEST_F(IfMergingTest, BlockWithThreePredecessors)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(6U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0U, 2U);
            INST(7U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(4U, 5U) {}

        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(8U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 1U}, {4U, 2U}});
            INST(9U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(10U, Opcode::Return).u64().Inputs(8U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(11U, Opcode::Return).u64().Inputs(1U);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// Equivalent If instructions dominate each other, but block with the second If has
// only one predecessor. Not applied. This is a case for branch elimination
TEST_F(IfMergingTest, BlockWithOnePredecessor)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Return).u64().Inputs(2U);
        }

        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(6U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0U, 1U);
            INST(7U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::Return).u64().Inputs(1U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(9U, Opcode::Return).u64().Inputs(1U);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

Graph *IfMergingTest::CreateExpectedConstPhiIf()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(1U, 5U);
            INST(13U, Opcode::Return).b().Inputs(2U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(2U, 7U);
            INST(12U, Opcode::Return).b().Inputs(1U);
        }
    }
    return graph;
}

TEST_F(IfMergingTest, ConstPhiIf)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(1U, 5U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(2U, 7U);
        }
        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{3U, 1U}, {4U, 2U}});
            INST(10U, Opcode::Compare).b().CC(CC_EQ).Inputs(9U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(12U, Opcode::Return).b().Inputs(1U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(13U, Opcode::Return).b().Inputs(2U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateExpectedConstPhiIf();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

Graph *IfMergingTest::CreateExpConstantEqualToAllPhiInputs()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(1U, 5U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(2U, 7U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(12U, Opcode::Return).b().Inputs(1U);
        }
    }
    return graph;
}

TEST_F(IfMergingTest, ConstantEqualToAllPhiInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(1U, 5U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(2U, 7U);
        }
        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{3U, 1U}, {4U, 1U}});
            INST(10U, Opcode::Compare).b().CC(CC_EQ).Inputs(9U, 1U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(12U, Opcode::Return).b().Inputs(1U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(13U, Opcode::Return).b().Inputs(2U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateExpConstantEqualToAllPhiInputs();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

Graph *IfMergingTest::CreateExpConstantNotEqualToPhiInputs()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(1U, 5U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(2U, 7U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(13U, Opcode::Return).b().Inputs(2U);
        }
    }
    return graph;
}

TEST_F(IfMergingTest, ConstantNotEqualToPhiInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(14U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(1U, 5U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(2U, 7U);
        }
        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{3U, 1U}, {4U, 2U}});
            INST(10U, Opcode::Compare).b().CC(CC_EQ).Inputs(9U, 14U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(12U, Opcode::Return).b().Inputs(1U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(13U, Opcode::Return).b().Inputs(2U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateExpConstantNotEqualToPhiInputs();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IfMergingTest, InstInPhiBlock)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(1U, 5U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(2U, 7U);
        }
        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{3U, 1U}, {4U, 2U}});
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(0U, 10U);
            INST(12U, Opcode::Compare).b().CC(CC_EQ).Inputs(9U, 2U);
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(12U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(14U, Opcode::Return).b().Inputs(1U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(15U, Opcode::Return).b().Inputs(2U);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation, solid logic
void IfMergingTest::CreateNestedIfGraph()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(3U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 7U)
        {
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(1U, 6U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(8U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 2U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(5U, 7U)
        {
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(2U, 10U);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(12U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).v0id().InputsAutoType(3U, 12U);
        }
        BASIC_BLOCK(7U, 8U, 9U)
        {
            INST(14U, Opcode::Phi).u64().Inputs({{3U, 1U}, {5U, 2U}, {6U, 3U}});
            INST(15U, Opcode::Compare).b().CC(CC_EQ).Inputs(14U, 2U);
            INST(16U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(15U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(17U, Opcode::Return).b().Inputs(2U);
        }
        BASIC_BLOCK(9U, 10U, 11U)
        {
            INST(18U, Opcode::Compare).b().CC(CC_EQ).Inputs(14U, 1U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(18U);
        }
        BASIC_BLOCK(10U, -1L)
        {
            INST(20U, Opcode::Return).b().Inputs(1U);
        }
        BASIC_BLOCK(11U, -1L)
        {
            INST(21U, Opcode::Return).b().Inputs(3U);
        }
    }
}

/*
 *        [0]
 *         |
 *         v
 *        [2]--->[4]----\
 *         |      |     |
 *         v      v     v
 *        [3]    [5]   [6]
 *         |      |     |
 *         |<-----+-----/
 *         v
 *        [7]--->[9]----\
 *         |      |     |
 *         v      v     v
 *        [8]    [10]  [11]
 *
 * Transform to:
 *        [0]
 *         |
 *         v
 *        [2]--->[4]----\
 *         |      |     |
 *         v      v     v
 *        [3]    [5]   [6]
 *         |      |     |
 *         v      v     v
 *        [8]    [10]  [11]
 */
TEST_F(IfMergingTest, NestedIf)
{
    CreateNestedIfGraph();
    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(3U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(1U, 6U);
            INST(20U, Opcode::Return).b().Inputs(1U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(8U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 2U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(2U, 10U);
            INST(17U, Opcode::Return).b().Inputs(2U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(12U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).v0id().InputsAutoType(3U, 12U);
            INST(21U, Opcode::Return).b().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void IfMergingTest::CreateInitialGraphDuplicatePhiInputs()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(3U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 7U)
        {
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(1U, 6U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(8U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 2U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(5U, 7U)
        {
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(2U, 10U);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(12U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).v0id().InputsAutoType(3U, 12U);
        }
        BASIC_BLOCK(7U, 8U, 9U)
        {
            INST(14U, Opcode::Phi).u64().Inputs({{3U, 1U}, {5U, 2U}, {6U, 1U}});
            INST(15U, Opcode::Compare).b().CC(CC_EQ).Inputs(14U, 1U);
            INST(16U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(15U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(17U, Opcode::Return).b().Inputs(1U);
        }
        BASIC_BLOCK(9U, -1L)
        {
            INST(18U, Opcode::Return).b().Inputs(2U);
        }
    }
}

TEST_F(IfMergingTest, DuplicatePhiInputs)
{
    CreateInitialGraphDuplicatePhiInputs();

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(3U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 7U)
        {
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(1U, 6U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(8U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 2U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(2U, 10U);
            INST(18U, Opcode::Return).b().Inputs(2U);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(12U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).v0id().InputsAutoType(3U, 12U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(17U, Opcode::Return).b().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

Graph *IfMergingTest::CreateExpectedSplitPhiIntoTwo()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(3U, 2U);
        CONSTANT(4U, 3U);
        BASIC_BLOCK(2U, 7U, 3U)
        {
            INST(5U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 8U, 4U)
        {
            INST(7U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 7U, 8U)
        {
            INST(9U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 3U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(17U, Opcode::Phi).u64().Inputs({{2U, 1U}, {4U, 2U}});
            INST(15U, Opcode::Return).u64().Inputs(17U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(18U, Opcode::Phi).u64().Inputs({{3U, 3U}, {4U, 4U}});
            INST(12U, Opcode::Add).u64().Inputs(18U, 0U);
            INST(16U, Opcode::Return).u64().Inputs(12U);
        }
    }
    return graph;
}

/*
 *        [0]
 *         |
 *         v   0
 *        [2]-----\
 *         |      |
 *         v    2 |
 *        [3]-----+
 *         |      |
 *         v    1 |
 *        [4]-----+
 *         |      |
 *         v   3  v       F
 *        [5]--->[Phi < 2]-->[8]
 *                  T|
 *                   v
 *                  [7]
 *
 * Transform to:
 *           [0]
 *            |
 *            v   0
 *           [2]-----\
 *            |      |
 *        2   v      |
 *      /----[3]     |
 *      |     |      |
 *      |     v   1  v
 *      |    [4]--->[7]
 *      |     |
 *      v  3  v
 *     [8]<--[5]
 */
TEST_F(IfMergingTest, SplitPhiIntoTwo)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(3U, 2U);
        CONSTANT(4U, 3U);
        BASIC_BLOCK(2U, 6U, 3U)
        {
            INST(5U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 6U, 4U)
        {
            INST(7U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(9U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 3U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(5U, 6U) {}
        BASIC_BLOCK(6U, 7U, 8U)
        {
            INST(11U, Opcode::Phi).u64().Inputs({{2U, 1U}, {3U, 3U}, {4U, 2U}, {5U, 4U}});
            INST(12U, Opcode::Add).u64().Inputs(11U, 0U);
            INST(13U, Opcode::Compare).b().CC(CC_LT).Inputs(11U, 3U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(15U, Opcode::Return).u64().Inputs(11U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(16U, Opcode::Return).u64().Inputs(12U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateExpectedSplitPhiIntoTwo();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

Graph *IfMergingTest::CreateExpectedConstantPhiLoopBackEdge(bool inverse)
{
    auto graphExpected = CreateEmptyGraph();
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        if (!inverse) {
            CONSTANT(2U, 2U);
        }
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, 3U, 2U)
        {
            INST(4U, Opcode::Phi).u64().Inputs({{0U, 0U}, {2U, 5U}});
            INST(5U, Opcode::Sub).u64().Inputs(4U, 3U);
            INST(6U, Opcode::Compare).b().CC(CC_EQ).Inputs(5U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(11U, Opcode::Return).u64().Inputs(inverse ? 1U : 2U);
        }
    }
    return graphExpected;
}

/*
 *        [0]
 *         |
 *         v
 *  [3]<--[2]<--\
 *   |     |    |
 *   |     v    |
 *   \--->[4]---/
 *         |
 *         v
 *        [5]
 *
 * Transform to (before cleanup):
 *        [0]
 *         |
 *         v
 *  [3]<--[2]<--\
 *   |     |    |
 *   v     v    |
 *  [4']  [4]---/
 *   |
 *   v
 *  [5]
 *
 */
TEST_F(IfMergingTest, ConstantPhiLoopBackEdge)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            CONSTANT(1U, 0U);
            CONSTANT(2U, 2U);
            CONSTANT(3U, 1U);
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::Phi).u64().Inputs({{0U, 0U}, {4U, 5U}});
                INST(5U, Opcode::Sub).u64().Inputs(4U, 3U);
                INST(6U, Opcode::Compare).b().CC(CC_EQ).Inputs(5U, 1U);
                INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
            }
            BASIC_BLOCK(3U, 4U) {}
            BASIC_BLOCK(4U, 5U, 2U)
            {
                INST(8U, Opcode::Phi).u64().Inputs({{2U, 1U}, {3U, 2U}});
                INST(9U, Opcode::Compare).b().CC(CC_LE).Inputs(4U, 3U);
                INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(inverse ? CC_EQ : CC_NE).Imm(0U).Inputs(6U);
            }
            BASIC_BLOCK(5U, -1L)
            {
                INST(11U, Opcode::Return).u64().Inputs(8U);
            }
        }

        ASSERT_TRUE(graph->RunPass<IfMerging>());
        ASSERT_TRUE(graph->RunPass<Cleanup>());

        auto graphExpected = CreateExpectedConstantPhiLoopBackEdge(inverse);
        ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
    }
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
 * Transform to:
 * [0] -> [3] -> [exit]
 *
 */
TEST_F(IfMergingTest, ConstantPhiUnrollLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Phi).u64().Inputs({{0U, 1U}, {3U, 2U}});
            INST(5U, Opcode::Compare).b().CC(CC_EQ).Inputs(4U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(7U);
            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
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
 * Transform to:
 *           [0]
 *            |
 *            v
 *      /--->[2]
 *      |     |
 *      |     v
 *      \----[3]
 *
 */
TEST_F(IfMergingTest, ConstantPhiRemoveLoopExit)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Phi).u64().Inputs({{0U, 1U}, {3U, 2U}});
            INST(5U, Opcode::Compare).b().CC(CC_GE).Inputs(4U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, 3U) {}

        BASIC_BLOCK(3U, 3U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(7U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
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
 * Transform to:
 * [0] -> [2] -> [exit]
 *
 */
TEST_F(IfMergingTest, ConstantPhiRemoveLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Phi).u64().Inputs({{0U, 1U}, {3U, 2U}});
            INST(5U, Opcode::Compare).b().CC(CC_EQ).Inputs(4U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *           [0]
 *            |
 *            v
 *      /--->[2]----\
 *      |     |     |
 *      |     |     v
 *      \-----/  [exit]
 *
 * Transform to:
 * [0] -> [2] -> [exit]
 *
 */
TEST_F(IfMergingTest, ConstantPhiRemoveEmptyLoop)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            CONSTANT(1U, 1U);
            CONSTANT(2U, 2U);

            BASIC_BLOCK(2U, 2U, 3U)
            {
                INST(4U, Opcode::Phi).u64().Inputs({{0U, 1U}, {2U, 2U}});
                INST(5U, Opcode::Compare).b().CC(inverse ? CC_NE : CC_EQ).Inputs(4U, 2U);
                INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(inverse ? CC_EQ : CC_NE).Imm(0U).Inputs(5U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(7U, Opcode::ReturnVoid).v0id();
            }
        }

        ASSERT_TRUE(graph->RunPass<IfMerging>());
        ASSERT_TRUE(graph->RunPass<Cleanup>());

        auto graphExpected = CreateEmptyGraph();
        GRAPH(graphExpected)
        {
            BASIC_BLOCK(2U, -1L)
            {
                INST(7U, Opcode::ReturnVoid).v0id();
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
    }
}

/*
 *           [0]
 *            |
 *            v
 *      /--->[2]----\
 *      |     |     |
 *      |     |     v
 *      \-----/  [exit]
 *
 * Could be transformed to:
 * [0] -> [2] -> [2] -> [exit]
 *
 * but basic block 2 would be duplicated. Not applied
 *
 */
TEST_F(IfMergingTest, ConstantPhiDontRemoveLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(4U, Opcode::Phi).u64().Inputs({{0U, 1U}, {2U, 2U}});
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(5U);
            INST(7U, Opcode::Compare).b().CC(CC_EQ).Inputs(4U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
