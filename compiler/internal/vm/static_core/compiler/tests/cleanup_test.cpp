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
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/regalloc/cleanup_empty_blocks.h"

namespace ark::compiler {
class CleanupTest : public GraphTest {
public:
    bool RunCleanupEmptyBlocks()
    {
        auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
        auto applied = GetGraph()->RunPass<Cleanup>();
        EXPECT_EQ(applied, CleanupEmptyBlocks(clone));
        EXPECT_TRUE(GraphComparator().Compare(GetGraph(), clone));
        return applied;
    }
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(CleanupTest, Simple)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, 3U) {}
        BASIC_BLOCK(3U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(RunCleanupEmptyBlocks());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(3U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(BothHasPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(2U, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_LE).Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(3U, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_EQ).Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(4U, Opcode::Add).s64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(5U, Opcode::Phi).s64().Inputs({{3U, 1U}, {4U, 4U}});
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(6U, Opcode::Phi).s64().Inputs({{2U, 0U}, {5U, 5U}});
            INST(7U, Opcode::Return).s64().Inputs(6U);
        }
    }
}

OUT_GRAPH(BothHasPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(2U, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_LE).Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, 4U, 6U)
        {
            INST(3U, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_EQ).Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(4U, Opcode::Add).s64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(5U, Opcode::Phi).s64().Inputs({{2U, 0U}, {3U, 1U}, {4U, 4U}});
            INST(6U, Opcode::Return).s64().Inputs(5U);
        }
    }
}

TEST_F(CleanupTest, BothHasPhi)
{
    src_graph::BothHasPhi::CREATE(GetGraph());

    ASSERT_TRUE(RunCleanupEmptyBlocks());

    auto graph = CreateEmptyGraph();
    out_graph::BothHasPhi::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, HasPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_LT).Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(3U, Opcode::Add).s64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(4U, Opcode::Phi).s64().Inputs({{2U, 1U}, {3U, 3U}});
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Return).s64().Inputs(4U);
        }
    }

    ASSERT_TRUE(RunCleanupEmptyBlocks());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(2U, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_LT).Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(3U, Opcode::Add).s64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(4U, Opcode::Phi).s64().Inputs({{2U, 1U}, {3U, 3U}});
            INST(6U, Opcode::Return).s64().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, DeadPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(2U, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_NE).Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(3U, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_LT).Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 6U)
        {
            INST(4U, Opcode::Phi).s64().Inputs({{3U, 0U}, {4U, 1U}});
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(5U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, LoopPreHeader)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U) {}
        BASIC_BLOCK(3U, 3U, -1L)
        {
            INST(3U, Opcode::Neg).s64().Inputs(0U);
            INST(5U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1U, 3U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
    }

    ASSERT_EQ(&BB(2U), BB(3U).GetLoop()->GetPreHeader());
    ASSERT_EQ(1U, BB(3U).GetLoop()->GetBlocks().size());
    ASSERT_EQ(3U, BB(3U).GetLoop()->GetOuterLoop()->GetBlocks().size());

    ASSERT_FALSE(RunCleanupEmptyBlocks());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U) {}
        BASIC_BLOCK(3U, 3U, -1L)
        {
            INST(3U, Opcode::Neg).s64().Inputs(0U);
            INST(5U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1U, 3U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(TwoPredecessors, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::CallStatic).v0id().InputsAutoType(20U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(21U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(21U);
        }
        BASIC_BLOCK(5U, 6U) {}
        BASIC_BLOCK(6U, -1L)
        {
            INST(6U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(TwoPredecessors, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 6U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::CallStatic).v0id().InputsAutoType(20U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(21U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(21U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(6U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(CleanupTest, TwoPredecessors)
{
    src_graph::TwoPredecessors::CREATE(GetGraph());

    ASSERT_TRUE(RunCleanupEmptyBlocks());

    auto graph = CreateEmptyGraph();
    out_graph::TwoPredecessors::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(TwoPredecessorsPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Add).s64().Inputs(0U, 1U);
            INST(5U, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(20U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(21U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(21U);
        }
        BASIC_BLOCK(6U, 7U) {}
        BASIC_BLOCK(7U, -1L)
        {
            INST(9U, Opcode::Phi).s64().Inputs({{2U, 1U}, {6U, 4U}});
            INST(10U, Opcode::Return).s64().Inputs(9U);
        }
    }
}

OUT_GRAPH(TwoPredecessorsPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Add).s64().Inputs(0U, 1U);
            INST(5U, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 7U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(20U);
        }
        BASIC_BLOCK(5U, 7U)
        {
            INST(21U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(21U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(9U, Opcode::Phi).s64().Inputs({{2U, 1U}, {4U, 4U}, {5U, 4U}});
            INST(10U, Opcode::Return).s64().Inputs(9U);
        }
    }
}

TEST_F(CleanupTest, TwoPredecessorsPhi)
{
    src_graph::TwoPredecessorsPhi::CREATE(GetGraph());

    ASSERT_TRUE(RunCleanupEmptyBlocks());

    auto graph = CreateEmptyGraph();
    out_graph::TwoPredecessorsPhi::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, TriangleNoPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(4U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(4U, -1L)
        {
            INST(4U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, DiamondSamePhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 5U) {}
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, -1L)
        {
            INST(4U, Opcode::Phi).s64().Inputs({{3U, 1U}, {4U, 1U}});
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(5U, -1L)
        {
            INST(5U, Opcode::Return).s64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, JointTriangle)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).s64().Inputs({{2U, 1U}, {3U, 0U}, {4U, 0U}});
            INST(7U, Opcode::Return).s64().Inputs(6U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 5U) {}
        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).s64().Inputs({{2U, 1U}, {3U, 0U}});
            INST(7U, Opcode::Return).s64().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, TriangleProhibited)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(4U, Opcode::Phi).s64().Inputs({{2U, 1U}, {3U, 0U}});
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }

    ASSERT_FALSE(RunCleanupEmptyBlocks());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(4U, Opcode::Phi).s64().Inputs({{2U, 1U}, {3U, 0U}});
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, InfiniteLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 3U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(4U, Opcode::ReturnVoid);
        }
    }

    ASSERT_FALSE(RunCleanupEmptyBlocks());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 3U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(4U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(JointDiamond, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(2U, Opcode::If).SrcType(DataType::UINT64).CC(CC_GE).Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(3U, Opcode::Mul).u64().Inputs(0U, 0U);
            INST(4U, Opcode::Mul).u64().Inputs(1U, 1U);
            INST(5U, Opcode::If).SrcType(DataType::UINT64).CC(CC_GT).Inputs(4U, 1U);
        }
        BASIC_BLOCK(4U, 7U) {}
        BASIC_BLOCK(5U, 7U) {}
        BASIC_BLOCK(6U, 7U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(20U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(7U, Opcode::Phi).u64().Inputs({{6U, 0U}, {4U, 3U}, {5U, 3U}});
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }
}

OUT_GRAPH(JointDiamond, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(2U, Opcode::If).SrcType(DataType::UINT64).CC(CC_GE).Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, 7U)
        {
            INST(3U, Opcode::Mul).u64().Inputs(0U, 0U);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(20U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(7U, Opcode::Phi).u64().Inputs({{6U, 0U}, {3U, 3U}});
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }
}

TEST_F(CleanupTest, JointDiamond)
{
    src_graph::JointDiamond::CREATE(GetGraph());

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    out_graph::JointDiamond::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, JoinLoopBackEdgeWithMultiPreds)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(4U, Opcode::Phi).u64().Inputs(0U, 9U);
            INST(5U, Opcode::If).SrcType(DataType::UINT64).CC(CC_LE).Inputs(4U, 1U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::If).SrcType(DataType::UINT64).CC(CC_LE).Inputs(4U, 2U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(7U, Opcode::Mul).u64().Inputs(4U, 1U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(8U, Opcode::Mul).u64().Inputs(4U, 2U);
        }
        BASIC_BLOCK(6U, 2U)
        {
            INST(9U, Opcode::Phi).u64().Inputs(7U, 8U);
            // loop back-edge
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(11U, Opcode::Return).u64().Inputs(4U);
        }
    }

    ASSERT_TRUE(RunCleanupEmptyBlocks());
    ASSERT_TRUE(GetGraph()->GetAnalysis<LoopAnalyzer>().IsValid());

    auto loop = BB(2U).GetLoop();
    ASSERT_EQ(loop->GetHeader(), &BB(2U));
    ASSERT_EQ(loop->GetBackEdges().size(), 2U);
    ASSERT_TRUE(loop->HasBackEdge(&BB(4U)));
    ASSERT_TRUE(loop->HasBackEdge(&BB(5U)));
}

TEST_F(CleanupTest, DiamondBecomeEmpty)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).b();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 5U) {}
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, -1L)
        {
            INST(2U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(5U, -1L)
        {
            INST(2U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, CarefulDCE)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(0U, Opcode::CallStatic).b().InputsAutoType(20U);
            INST(1U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(2U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(0U, Opcode::CallStatic).b().InputsAutoType(20U);
            INST(1U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, CarefulDCE2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        BASIC_BLOCK(2U, 3U, 40U)
        {
            INST(2U, Opcode::If).SrcType(DataType::UINT64).CC(CC_GT).Inputs(0U, 1U);
        }
        BASIC_BLOCK(40U, 5U) {}
        BASIC_BLOCK(3U, 5U) {}
        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(3U, Opcode::Phi).u64().Inputs({{40U, 0U}, {3U, 1U}});
            INST(4U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(6U, 7U) {}
        BASIC_BLOCK(7U, -1L)
        {
            INST(5U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, Delete)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 12U);
        CONSTANT(1U, 13U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Neg).u64().Inputs(1U);
            INST(3U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(4U, Opcode::Return).u64().Inputs(2U);
        }
    }

    // Delete Insts 0 and 3
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    ASSERT_TRUE(CheckInputs(INS(2U), {1U}));
    ASSERT_TRUE(CheckInputs(INS(4U), {2U}));

    ASSERT_TRUE(CheckUsers(INS(1U), {2U}));
    ASSERT_TRUE(CheckUsers(INS(2U), {4U}));
    ASSERT_TRUE(CheckUsers(INS(4U), {}));
}

TEST_F(CleanupTest, NotRemoved)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 11U);
        CONSTANT(1U, 12U);
        CONSTANT(2U, 13U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Neg).u64().Inputs(3U);
            INST(5U, Opcode::Sub).u64().Inputs(4U, 2U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());

    ASSERT_TRUE(CheckInputs(INS(3U), {0U, 1U}));
    ASSERT_TRUE(CheckInputs(INS(4U), {3U}));
    ASSERT_TRUE(CheckInputs(INS(5U), {4U, 2U}));
    ASSERT_TRUE(CheckInputs(INS(6U), {5U}));

    ASSERT_TRUE(CheckUsers(INS(0U), {3U}));
    ASSERT_TRUE(CheckUsers(INS(1U), {3U}));
    ASSERT_TRUE(CheckUsers(INS(2U), {5U}));
    ASSERT_TRUE(CheckUsers(INS(3U), {4U}));
    ASSERT_TRUE(CheckUsers(INS(4U), {5U}));
    ASSERT_TRUE(CheckUsers(INS(5U), {6U}));
}

TEST_F(CleanupTest, Loop)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::Phi).u64().Inputs(0U, 5U);
            INST(3U, Opcode::Phi).u64().Inputs(1U, 6U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(11U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(4U, 2U)
        {
            INST(5U, Opcode::Neg).u64().Inputs(3U);
            INST(6U, Opcode::Neg).u64().Inputs(4U);
            // Must be removed
            INST(7U, Opcode::Add).u64().Inputs(5U, 6U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            // Unused instruction
            INST(8U, Opcode::Neg).u64().Inputs(4U);
            INST(9U, Opcode::Return).u64().Inputs(4U);
        }
    }

    // Delete Insts 7 and 8
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    ASSERT_TRUE(CheckInputs(INS(2U), {0U, 5U}));
    ASSERT_TRUE(CheckInputs(INS(3U), {1U, 6U}));
    ASSERT_TRUE(CheckInputs(INS(4U), {2U, 3U}));
    ASSERT_TRUE(CheckInputs(INS(5U), {3U}));
    ASSERT_TRUE(CheckInputs(INS(6U), {4U}));
    ASSERT_TRUE(CheckInputs(INS(9U), {4U}));

    ASSERT_TRUE(CheckUsers(INS(0U), {2U, 11U}));
    ASSERT_TRUE(CheckUsers(INS(1U), {3U, 11U}));
    ASSERT_TRUE(CheckUsers(INS(2U), {4U}));
    ASSERT_TRUE(CheckUsers(INS(3U), {4U, 5U}));
    ASSERT_TRUE(CheckUsers(INS(4U), {6U, 9U}));
    ASSERT_TRUE(CheckUsers(INS(5U), {2U}));
    ASSERT_TRUE(CheckUsers(INS(6U), {3U}));
}

TEST_F(CleanupTest, Loop1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 2U, 3U)
        {
            // Unused instructions
            INST(2U, Opcode::Phi).u64().Inputs(1U, 3U);
            INST(3U, Opcode::Add).u64().Inputs(2U, 1U);
            INST(6U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 0U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            // Unused instruction
            INST(4U, Opcode::Neg).u64().Inputs(0U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }

    // Delete Insts 1, 2 and 3
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    ASSERT_EQ(INS(0U).GetNext(), nullptr);
    ASSERT_FALSE(BB(2U).IsEmpty());
}

/*
 * Test Graph:
 *                        [0]
 *                         |
 *                         v
 *                /-------[2]-------\
 *                |                 |
 *                v                 v
 *               [3]               [4]
 *             /     \           /     \
 *            v       \         /       |
 *  [8]<---->[7]       \->[5]<-/        |
 *            |             |           |
 *            |             \--->[6]<---/
 *            |                   |
 *            \---------->[9]<----/
 *                         |
 *                         v
 *                        [1]
 */

SRC_GRAPH(PhiInputs1, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(18U, 2U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(18U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 7U, 5U)
        {
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(18U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(6U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(18U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }

        BASIC_BLOCK(5U, 6U)
        {
            INST(8U, Opcode::Phi).u64().Inputs({{4U, 0U}, {3U, 0U}});
        }
        BASIC_BLOCK(6U, 9U)
        {
            INST(10U, Opcode::Phi).u64().Inputs({{4U, 0U}, {5U, 8U}});
        }
        BASIC_BLOCK(7U, 8U, 9U)
        {
            INST(12U, Opcode::Phi).u64().Inputs({{8U, 12U}, {3U, 0U}});
            INST(13U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(18U, 1U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(8U, 7U) {}
        BASIC_BLOCK(9U, -1L)
        {
            INST(16U, Opcode::Phi).u64().Inputs({{6U, 10U}, {7U, 12U}});
            INST(17U, Opcode::Return).u64().Inputs(16U);
        }
    }
}

TEST_F(CleanupTest, PhiInputs1)
{
    src_graph::PhiInputs1::CREATE(GetGraph());

    // Delete all phi instructions
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>(false));

    ASSERT_TRUE(CheckInputs(INS(17U), {0U}));
    ASSERT_TRUE(CheckUsers(INS(0U), {17U}));
}

/*
 * Test Graph:
 *                                   [0]
 *                                    |
 *                                    v
 *                         /---------[2]---------\
 *                         |                     |
 *                         v                     v
 *                        [3]                   [4]
 *                         |                     |
 *                         \-------->[5]<--------/
 *                                    |
 *                                    |->[6]<----\
 *                                    |   |      |
 *                                    |   v      |
 *                                    \->[7]-----/
 *                                        |
 *                                        v
 *                                       [8]
 *                                        |
 *                                        v
 *                                       [1]
 */

SRC_GRAPH(PhiInputs2, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(6U, 2U);
        CONSTANT(7U, 3U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(6U, 7U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(3U, 5U) {}
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(2U, Opcode::Phi).u64().Inputs(0U, 1U);
            INST(12U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(6U, 7U);
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(12U);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(3U, Opcode::Phi).u64().Inputs({{7U, 4U}, {5U, 2U}});
        }
        BASIC_BLOCK(7U, 6U, 8U)
        {
            INST(4U, Opcode::Phi).u64().Inputs({{6U, 3U}, {5U, 2U}});
            INST(15U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(6U, 7U);
            INST(16U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(15U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }
}

OUT_GRAPH(PhiInputs2, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(6U, 2U);
        CONSTANT(7U, 3U);
        BASIC_BLOCK(2U, 5U, 4U)
        {
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(6U, 7U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 7U)
        {
            INST(2U, Opcode::Phi).u64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(7U, 7U, 8U)
        {
            INST(15U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(6U, 7U);
            INST(16U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(15U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(5U, Opcode::Return).u64().Inputs(2U);
        }
    }
}

TEST_F(CleanupTest, PhiInputs2)
{
    src_graph::PhiInputs2::CREATE(GetGraph());

    // Delete 3 and 4 phi (replace with v2p in return)
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>(false));

    ASSERT_TRUE(CheckInputs(INS(2U), {0U, 1U}));
    ASSERT_TRUE(CheckInputs(INS(5U), {2U}));

    ASSERT_TRUE(CheckUsers(INS(0U), {2U}));
    ASSERT_TRUE(CheckUsers(INS(1U), {2U}));
    ASSERT_TRUE(CheckUsers(INS(2U), {5U}));

    auto graph = CreateEmptyGraph();
    out_graph::PhiInputs2::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, NullPtr)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, nullptr);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Return).u64().Inputs(0U);
        }
    }

    ASSERT_TRUE(GetGraph()->HasNullPtrInst());
    // Delete NullPtr
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    ASSERT_FALSE(GetGraph()->HasNullPtrInst());
}

TEST_F(CleanupTest, JustReturnLeft)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);
        CONSTANT(3U, 3U);
        CONSTANT(4U, 4U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Add).u64().Inputs(1U, 4U);
            INST(6U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(7U, Opcode::Compare).b().Inputs(0U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{2U, 6U}, {3U, 5U}});
            INST(10U, Opcode::Return).u64().Inputs(0U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(10U, Opcode::Return).u64().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, RemovingPhiFromTheSameBlock)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(5U, Opcode::Phi).u64().Inputs(1U, 6U);
            INST(6U, Opcode::Phi).u64().Inputs(0U, 6U);
            INST(7U, Opcode::Phi).u64().Inputs(0U, 8U);
            INST(8U, Opcode::Add).u64().Inputs(7U, 2U);
            INST(9U, Opcode::Compare).b().CC(CC_NE).Inputs(8U, 1U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(11U, Opcode::Add).u64().Inputs(5U, 6U);
            INST(12U, Opcode::Return).u64().Inputs(11U);
        }
    }
    ASSERT_TRUE(graph->RunPass<Cleanup>(false));
}

TEST_F(CleanupTest, CallDiscardReturnValue)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(0U, Opcode::CallStatic).s32().InputsAutoType(20U);
            INST(1U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_FALSE(INS(0U).HasUsers());
    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());
    ASSERT_TRUE(INS(0U).GetOpcode() == Opcode::CallStatic);
}

TEST_F(CleanupTest, CallReturnVoid)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(0U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(1U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_FALSE(INS(0U).HasUsers());
    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());
    ASSERT_TRUE(INS(0U).GetOpcode() == Opcode::CallStatic);
}

TEST_F(CleanupTest, StoreObject)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 1U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::StoreObject).s32().Inputs(3U, 1U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_FALSE(INS(4U).HasUsers());
    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());
    ASSERT_TRUE(INS(4U).GetOpcode() == Opcode::StoreObject);
}

TEST_F(CleanupTest, OneBlock)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
        }
    }

    ASSERT_FALSE(RunCleanupEmptyBlocks());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, TwoBlocks)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 12U);
        CONSTANT(1U, 13U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::Neg).u64().Inputs(1U);
            INST(3U, Opcode::Add).u64().Inputs(0U, 2U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(4U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 12U);
        CONSTANT(1U, 13U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Neg).u64().Inputs(1U);
            INST(3U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(4U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(SameBlockPhiTwice, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 12U).s64();
        CONSTANT(1U, 13U).s64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Neg).u64().Inputs(0U);
            INST(5U, Opcode::Neg).u64().Inputs(1U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::Neg).u64().Inputs(1U);
            INST(8U, Opcode::Neg).u64().Inputs(0U);
        }

        BASIC_BLOCK(5U, 6U)
        {
            INST(10U, Opcode::Phi).u64().Inputs({{3U, 4U}, {4U, 7U}, {6U, 15U}});
            INST(11U, Opcode::Phi).u64().Inputs({{3U, 5U}, {4U, 8U}, {6U, 16U}});
            INST(12U, Opcode::Neg).u64().Inputs(10U);
            INST(13U, Opcode::Add).u64().Inputs(10U, 11U);
        }

        BASIC_BLOCK(6U, 5U, -1L)
        {
            INST(15U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(16U, Opcode::Neg).u64().Inputs(0U);
            INST(17U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
        }
    }
}

OUT_GRAPH(SameBlockPhiTwice, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 12U).s64();
        CONSTANT(1U, 13U).s64();

        BASIC_BLOCK(7U, 6U) {}

        BASIC_BLOCK(6U, 6U, -1L)
        {
            INST(17U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
        }
    }
}

TEST_F(CleanupTest, SameBlockPhiTwice)
{
    src_graph::SameBlockPhiTwice::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    out_graph::SameBlockPhiTwice::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(TwoBlocksLoop, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 12U).s64();
        CONSTANT(1U, 13U).s64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Neg).u64().Inputs(0U);
            INST(5U, Opcode::Neg).u64().Inputs(1U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::Neg).u64().Inputs(1U);
            INST(8U, Opcode::Neg).u64().Inputs(0U);
        }

        BASIC_BLOCK(5U, 6U)
        {
            INST(10U, Opcode::Phi).u64().Inputs({{3U, 4U}, {4U, 7U}, {6U, 15U}});
            INST(11U, Opcode::Phi).u64().Inputs({{3U, 5U}, {4U, 8U}, {6U, 16U}});
            INST(12U, Opcode::Neg).u64().Inputs(10U);
            INST(13U, Opcode::Add).u64().Inputs(10U, 11U);
        }

        BASIC_BLOCK(6U, 5U, -1L)
        {
            INST(15U, Opcode::Add).u64().Inputs(12U, 13U);
            INST(16U, Opcode::Neg).u64().Inputs(12U);
            INST(17U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(15U, 16U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
        }
    }
}

OUT_GRAPH(TwoBlocksLoop, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 12U).s64();
        CONSTANT(1U, 13U).s64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 7U)
        {
            INST(4U, Opcode::Neg).u64().Inputs(0U);
            INST(5U, Opcode::Neg).u64().Inputs(1U);
        }

        BASIC_BLOCK(4U, 7U)
        {
            INST(7U, Opcode::Neg).u64().Inputs(1U);
            INST(8U, Opcode::Neg).u64().Inputs(0U);
        }

        BASIC_BLOCK(7U, 5U)
        {
            INST(19U, Opcode::Phi).u64().Inputs({{3U, 4U}, {4U, 7U}});
            INST(20U, Opcode::Phi).u64().Inputs({{3U, 5U}, {4U, 8U}});
        }

        BASIC_BLOCK(5U, 5U, -1L)
        {
            INST(10U, Opcode::Phi).u64().Inputs({{7U, 19U}, {5U, 15U}});
            INST(11U, Opcode::Phi).u64().Inputs({{7U, 20U}, {5U, 16U}});
            INST(12U, Opcode::Neg).u64().Inputs(10U);
            INST(13U, Opcode::Add).u64().Inputs(10U, 11U);
            INST(15U, Opcode::Add).u64().Inputs(12U, 13U);
            INST(16U, Opcode::Neg).u64().Inputs(12U);
            INST(17U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(15U, 16U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
        }
    }
}

TEST_F(CleanupTest, TwoBlocksLoop)
{
    src_graph::TwoBlocksLoop::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    out_graph::TwoBlocksLoop::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(TwoLoopsPreHeaders, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 12U).s64();
        CONSTANT(1U, 13U).s64();
        PARAMETER(8U, 0U).b();

        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::Neg).u64().Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Neg).u64().Inputs(0U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }

        BASIC_BLOCK(4U, 4U, -1L)
        {
            INST(6U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }

        BASIC_BLOCK(5U, 6U) {}

        BASIC_BLOCK(6U, 6U, -1L)
        {
            INST(10U, Opcode::Compare).b().CC(CC_GT).SrcType(DataType::Type::UINT64).Inputs(4U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
    }
}

OUT_GRAPH(TwoLoopsPreHeaders, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 12U).s64();
        CONSTANT(1U, 13U).s64();
        PARAMETER(8U, 0U).b();

        BASIC_BLOCK(2U, 4U, 5U)
        {
            INST(2U, Opcode::Neg).u64().Inputs(0U);
            INST(4U, Opcode::Neg).u64().Inputs(0U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }

        BASIC_BLOCK(4U, 4U, -1L)
        {
            INST(6U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }

        BASIC_BLOCK(5U, 6U) {}

        BASIC_BLOCK(6U, 6U, -1L)
        {
            INST(10U, Opcode::Compare).b().CC(CC_GT).SrcType(DataType::Type::UINT64).Inputs(4U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
    }
}

TEST_F(CleanupTest, TwoLoopsPreHeaders)
{
    src_graph::TwoLoopsPreHeaders::CREATE(GetGraph());

    ASSERT_EQ(&BB(3U), BB(4U).GetLoop()->GetPreHeader());
    ASSERT_EQ(&BB(5U), BB(6U).GetLoop()->GetPreHeader());
    ASSERT_EQ(1U, BB(4U).GetLoop()->GetBlocks().size());
    ASSERT_EQ(1U, BB(6U).GetLoop()->GetBlocks().size());
    ASSERT_EQ(5U, BB(4U).GetLoop()->GetOuterLoop()->GetBlocks().size());
    ASSERT_EQ(5U, BB(6U).GetLoop()->GetOuterLoop()->GetBlocks().size());

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    out_graph::TwoLoopsPreHeaders::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));

    EXPECT_EQ(&BB(2U), BB(4U).GetLoop()->GetPreHeader());
    EXPECT_EQ(&BB(5U), BB(6U).GetLoop()->GetPreHeader());
    EXPECT_EQ(1U, BB(4U).GetLoop()->GetBlocks().size());
    EXPECT_EQ(1U, BB(6U).GetLoop()->GetBlocks().size());
    EXPECT_EQ(4U, BB(4U).GetLoop()->GetOuterLoop()->GetBlocks().size());
    EXPECT_EQ(4U, BB(6U).GetLoop()->GetOuterLoop()->GetBlocks().size());
}

TEST_F(CleanupTest, LoopBackEdge)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 12U).s64();
        CONSTANT(1U, 13U).s64();

        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::Neg).u64().Inputs(0U);
        }

        BASIC_BLOCK(3U, 2U, -1L)
        {
            INST(4U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(1U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
    }

    ASSERT_TRUE(BB(3U).GetLoop()->HasBackEdge(&BB(3U)));
    ASSERT_EQ(2U, BB(3U).GetLoop()->GetBlocks().size());

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 12U).s64();
        CONSTANT(1U, 13U).s64();

        BASIC_BLOCK(2U, 2U, -1L)
        {
            INST(2U, Opcode::Neg).u64().Inputs(0U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(1U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));

    EXPECT_TRUE(BB(2U).GetLoop()->HasBackEdge(&BB(2U)));
    EXPECT_EQ(1U, BB(2U).GetLoop()->GetBlocks().size());
}

TEST_F(CleanupTest, LoopMiddleBlock)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 12U).s64();
        CONSTANT(1U, 13U).s64();

        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::Neg).u64().Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(4U, Opcode::Neg).u64().Inputs(1U);
        }

        BASIC_BLOCK(4U, 2U, -1L)
        {
            INST(6U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2U, 4U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
    }

    ASSERT_EQ(3U, BB(2U).GetLoop()->GetBlocks().size());
    ASSERT_EQ(3U, BB(2U).GetLoop()->GetOuterLoop()->GetBlocks().size());

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 12U).s64();
        CONSTANT(1U, 13U).s64();

        BASIC_BLOCK(2U, 2U, -1L)
        {
            INST(2U, Opcode::Neg).u64().Inputs(0U);
            INST(4U, Opcode::Neg).u64().Inputs(1U);
            INST(6U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2U, 4U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));

    EXPECT_EQ(1U, BB(2U).GetLoop()->GetBlocks().size());
    EXPECT_EQ(3U, BB(2U).GetLoop()->GetOuterLoop()->GetBlocks().size());
}

TEST_F(CleanupTest, ThreeBlocks)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 12U);
        CONSTANT(1U, 13U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::Neg).u64().Inputs(1U);
            INST(3U, Opcode::Add).u64().Inputs(0U, 2U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(4U, Opcode::Neg).u64().Inputs(0U);
            INST(5U, Opcode::Add).u64().Inputs(1U, 4U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Add).u64().Inputs(3U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 12U);
        CONSTANT(1U, 13U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Neg).u64().Inputs(1U);
            INST(3U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(4U, Opcode::Neg).u64().Inputs(0U);
            INST(5U, Opcode::Add).u64().Inputs(1U, 4U);
            INST(6U, Opcode::Add).u64().Inputs(3U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(TwoBlocksAndPhi, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 12U).s64();
        CONSTANT(1U, 13U).s64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Neg).u64().Inputs(0U);
            INST(5U, Opcode::Neg).u64().Inputs(1U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::Neg).u64().Inputs(1U);
            INST(8U, Opcode::Neg).u64().Inputs(0U);
        }

        BASIC_BLOCK(5U, 6U)
        {
            INST(10U, Opcode::Phi).u64().Inputs({{3U, 4U}, {4U, 7U}});
            INST(11U, Opcode::Phi).u64().Inputs({{3U, 5U}, {4U, 8U}});
            INST(12U, Opcode::Neg).u64().Inputs(10U);
            INST(13U, Opcode::Add).u64().Inputs(10U, 11U);
        }

        BASIC_BLOCK(6U, 7U, 8U)
        {
            INST(15U, Opcode::Add).u64().Inputs(12U, 13U);
            INST(16U, Opcode::Neg).u64().Inputs(12U);
            INST(17U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(16U, 15U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
        }

        BASIC_BLOCK(7U, 8U)
        {
            INST(19U, Opcode::Add).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(8U, -1L)
        {
            INST(21U, Opcode::Phi).u64().Inputs({{6U, 16U}, {7U, 19U}});
            INST(22U, Opcode::Add).u64().Inputs(21U, 1U);
            INST(23U, Opcode::Return).u64().Inputs(22U);
        }
    }
}

OUT_GRAPH(TwoBlocksAndPhi, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 12U).s64();
        CONSTANT(1U, 13U).s64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Neg).u64().Inputs(0U);
            INST(5U, Opcode::Neg).u64().Inputs(1U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::Neg).u64().Inputs(1U);
            INST(8U, Opcode::Neg).u64().Inputs(0U);
        }

        BASIC_BLOCK(5U, 7U, 8U)
        {
            INST(10U, Opcode::Phi).u64().Inputs({{3U, 4U}, {4U, 7U}});
            INST(11U, Opcode::Phi).u64().Inputs({{3U, 5U}, {4U, 8U}});
            INST(12U, Opcode::Neg).u64().Inputs(10U);
            INST(13U, Opcode::Add).u64().Inputs(10U, 11U);
            INST(15U, Opcode::Add).u64().Inputs(12U, 13U);
            INST(16U, Opcode::Neg).u64().Inputs(12U);
            INST(17U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(16U, 15U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
        }

        BASIC_BLOCK(7U, 8U)
        {
            INST(19U, Opcode::Add).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(8U, -1L)
        {
            INST(21U, Opcode::Phi).u64().Inputs(16U, 19U);
            INST(22U, Opcode::Add).u64().Inputs(21U, 1U);
            INST(23U, Opcode::Return).u64().Inputs(22U);
        }
    }
}

TEST_F(CleanupTest, TwoBlocksAndPhi)
{
    src_graph::TwoBlocksAndPhi::CREATE(GetGraph());

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    out_graph::TwoBlocksAndPhi::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, RemoveCallReturnInlined)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::CallStatic).u64().Inlined().InputsAutoType(0U, 1U, 2U);
            INST(6U, Opcode::SaveState).Inputs(0U).SrcVregs({0U}).Caller(5U);
            INST(7U, Opcode::Add).s32().Inputs(1U, 1U);
            INST(8U, Opcode::ReturnInlined).Inputs(2U);

            INST(9U, Opcode::Return).s32().Inputs(7U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>(false));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::Add).s32().Inputs(1U, 1U);
            INST(9U, Opcode::Return).s32().Inputs(7U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, RemoveNestedCallInlined)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::CallStatic).u64().Inlined().InputsAutoType(0U, 2U);
            INST(6U, Opcode::SaveState).Inputs(0U).SrcVregs({0U}).Caller(5U);
            INST(7U, Opcode::NullCheck).ref().Inputs(0U, 6U);

            INST(10U, Opcode::CallStatic).u64().Inlined().InputsAutoType(7U, 6U);
            INST(11U, Opcode::SaveState).Inputs(0U).SrcVregs({0U}).Caller(10U);
            INST(12U, Opcode::LoadObject).u64().Inputs(7U);

            INST(13U, Opcode::SaveState).Inputs(0U).SrcVregs({0U}).Caller(10U);
            INST(14U, Opcode::CallStatic).v0id().Inlined().InputsAutoType(12U, 13U);

            INST(15U, Opcode::ReturnInlined).Inputs(13U);
            INST(16U, Opcode::ReturnInlined).Inputs(6U);
            INST(8U, Opcode::ReturnInlined).Inputs(2U);
            INST(9U, Opcode::Return).u64().Inputs(12U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>(false));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::CallStatic).u64().Inlined().InputsAutoType(0U, 2U);

            INST(6U, Opcode::SaveState).Inputs(0U).SrcVregs({0U}).Caller(5U);
            INST(7U, Opcode::NullCheck).ref().Inputs(0U, 6U);
            INST(12U, Opcode::LoadObject).u64().Inputs(7U);

            INST(8U, Opcode::ReturnInlined).Inputs(2U);
            INST(9U, Opcode::Return).u64().Inputs(12U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(InlinedCallsWithCommonSaveState, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::IfImm).Inputs(1U).Imm(0U).CC(CC_NE);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(13U, Opcode::CallStatic).ref().Inlined().InputsAutoType(0U, 2U);
            INST(14U, Opcode::ReturnInlined).Inputs(2U);
            INST(15U, Opcode::Return).u64().Inputs(1U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(5U, Opcode::CallStatic).ref().Inlined().InputsAutoType(0U, 2U);
            INST(6U, Opcode::SaveState).Inputs(0U).SrcVregs({0U}).Caller(5U);
            INST(7U, Opcode::NullCheck).ref().Inputs(0U, 6U);
            INST(12U, Opcode::LoadObject).u64().Inputs(7U);
            INST(8U, Opcode::ReturnInlined).Inputs(2U);
            INST(9U, Opcode::Return).u64().Inputs(12U);
        }
    }
}

OUT_GRAPH(InlinedCallsWithCommonSaveState, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::IfImm).Inputs(1U).Imm(0U).CC(CC_NE);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(15U, Opcode::Return).u64().Inputs(1U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(5U, Opcode::CallStatic).ref().Inlined().InputsAutoType(0U, 2U);
            INST(6U, Opcode::SaveState).Inputs(0U).SrcVregs({0U}).Caller(5U);
            INST(7U, Opcode::NullCheck).ref().Inputs(0U, 6U);
            INST(12U, Opcode::LoadObject).u64().Inputs(7U);
            INST(8U, Opcode::ReturnInlined).Inputs(2U);
            INST(9U, Opcode::Return).u64().Inputs(12U);
        }
    }
}

TEST_F(CleanupTest, InlinedCallsWithCommonSaveState)
{
    src_graph::InlinedCallsWithCommonSaveState::CREATE(GetGraph());

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>(false));

    auto graph = CreateEmptyGraph();
    out_graph::InlinedCallsWithCommonSaveState::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, DontRemoveCallInlined)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::CallStatic).u64().Inlined().InputsAutoType(0U, 2U);
            INST(6U, Opcode::SaveState).Inputs(0U).SrcVregs({0U}).Caller(5U);
            INST(7U, Opcode::NullCheck).ref().Inputs(0U, 6U);
            INST(8U, Opcode::ReturnInlined).Inputs(2U);

            INST(9U, Opcode::Return).ref().Inputs(7U);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>(false));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(CleanupTest, DontRemoveCallInlinedWithBarrier)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::CallStatic).u64().Inlined().InputsAutoType(0U, 2U);
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 1U);
            INST(8U, Opcode::ReturnInlined).Inputs(2U).SetFlag(compiler::inst_flags::MEM_BARRIER);

            INST(9U, Opcode::Return).ref().Inputs(0U);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>(false));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
