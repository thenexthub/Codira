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

#include "optimizer/optimizations/cleanup.h"

namespace panda::compiler {
class CleanupTest : public GraphTest {
};

TEST_F(CleanupTest, Simple)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, 3) {}
        BASIC_BLOCK(3, -1)
        {
            INST(0, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(3, -1)
        {
            INST(0, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, BothHasPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_LE).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_EQ).Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(4, Opcode::Add).s64().Inputs(0, 1);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(5, Opcode::Phi).s64().Inputs({{3, 1}, {4, 4}});
        }
        BASIC_BLOCK(6, -1)
        {
            INST(6, Opcode::Phi).s64().Inputs({{2, 0}, {5, 5}});
            INST(7, Opcode::Return).s64().Inputs(6);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_LE).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 4, 6)
        {
            INST(3, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_EQ).Inputs(0, 1);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(4, Opcode::Add).s64().Inputs(0, 1);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(5, Opcode::Phi).s64().Inputs({{2, 0}, {3, 1}, {4, 4}});
            INST(6, Opcode::Return).s64().Inputs(5);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, HasPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_LT).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(3, Opcode::Add).s64().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(4, Opcode::Phi).s64().Inputs({{2, 1}, {3, 3}});
        }
        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Return).s64().Inputs(4);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 5)
        {
            INST(2, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_LT).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(3, Opcode::Add).s64().Inputs(0, 1);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(4, Opcode::Phi).s64().Inputs({{2, 1}, {3, 3}});
            INST(6, Opcode::Return).s64().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, DeadPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_NE).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::If).SrcType(DataType::Type::INT64).CC(CC_LT).Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 6)
        {
            INST(4, Opcode::Phi).s64().Inputs({{3, 0}, {4, 1}});
        }
        BASIC_BLOCK(6, -1)
        {
            INST(5, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, LoopPreHeader)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3) {}
        BASIC_BLOCK(3, 3, -1)
        {
            INST(3, Opcode::Neg).s64().Inputs(0);
            INST(5, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1, 3);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
    }

    ASSERT_EQ(&BB(2), BB(3).GetLoop()->GetPreHeader());
    ASSERT_EQ(1U, BB(3).GetLoop()->GetBlocks().size());
    ASSERT_EQ(3U, BB(3).GetLoop()->GetOuterLoop()->GetBlocks().size());

    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3) {}
        BASIC_BLOCK(3, 3, -1)
        {
            INST(3, Opcode::Neg).s64().Inputs(0);
            INST(5, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1, 3);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, TwoPredecessors)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(4, Opcode::CallStatic).v0id().InputsAutoType(20);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(21, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(21);
        }
        BASIC_BLOCK(5, 6) {}
        BASIC_BLOCK(6, -1)
        {
            INST(6, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 6)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(4, Opcode::CallStatic).v0id().InputsAutoType(20);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(21, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(21);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(6, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, TwoPredecessorsPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 7)
        {
            INST(2, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Add).s64().Inputs(0, 1);
            INST(5, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).v0id().InputsAutoType(20);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(21, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(21);
        }
        BASIC_BLOCK(6, 7) {}
        BASIC_BLOCK(7, -1)
        {
            INST(9, Opcode::Phi).s64().Inputs({{2, 1}, {6, 4}});
            INST(10, Opcode::Return).s64().Inputs(9);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 7)
        {
            INST(2, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Add).s64().Inputs(0, 1);
            INST(5, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 7)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).v0id().InputsAutoType(20);
        }
        BASIC_BLOCK(5, 7)
        {
            INST(21, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(21);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(9, Opcode::Phi).s64().Inputs({{2, 1}, {4, 4}, {5, 4}});
            INST(10, Opcode::Return).s64().Inputs(9);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, TriangleNoPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(4, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(4, -1)
        {
            INST(4, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, DiamondSamePhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, -1)
        {
            INST(4, Opcode::Phi).s64().Inputs({{3, 1}, {4, 1}});
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(5, -1)
        {
            INST(5, Opcode::Return).s64().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, JointTriangle)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 5)
        {
            INST(2, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).s64().Inputs({{2, 1}, {3, 0}, {4, 0}});
            INST(7, Opcode::Return).s64().Inputs(6);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 5)
        {
            INST(2, Opcode::Compare).b().CC(CC_LE).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).s64().Inputs({{2, 1}, {3, 0}});
            INST(7, Opcode::Return).s64().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, TriangleProhibited)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(4, Opcode::Phi).s64().Inputs({{2, 1}, {3, 0}});
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(4, Opcode::Phi).s64().Inputs({{2, 1}, {3, 0}});
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, InfiniteLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 3) {}
        BASIC_BLOCK(4, -1)
        {
            INST(4, Opcode::ReturnVoid);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 3) {}
        BASIC_BLOCK(4, -1)
        {
            INST(4, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, JointDiamond)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::If).SrcType(DataType::UINT64).CC(CC_GE).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 0);
            INST(4, Opcode::Mul).u64().Inputs(1, 1);
            INST(5, Opcode::If).SrcType(DataType::UINT64).CC(CC_GT).Inputs(4, 1);
        }
        BASIC_BLOCK(4, 7) {}
        BASIC_BLOCK(5, 7) {}
        BASIC_BLOCK(6, 7)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(20);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(7, Opcode::Phi).u64().Inputs({{6, 0}, {4, 3}, {5, 3}});
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::If).SrcType(DataType::UINT64).CC(CC_GE).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 7)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 0);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(20);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(7, Opcode::Phi).u64().Inputs({{6, 0}, {3, 3}});
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, JoinLoopBackEdgeWithMultiPreds)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        BASIC_BLOCK(2, 3, 7)
        {
            INST(4, Opcode::Phi).u64().Inputs(0, 9);
            INST(5, Opcode::If).SrcType(DataType::UINT64).CC(CC_LE).Inputs(4, 1);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::If).SrcType(DataType::UINT64).CC(CC_LE).Inputs(4, 2);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(7, Opcode::Mul).u64().Inputs(4, 1);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(8, Opcode::Mul).u64().Inputs(4, 2);
        }
        BASIC_BLOCK(6, 2)
        {
            INST(9, Opcode::Phi).u64().Inputs(7, 8);
            // loop back-edge
        }
        BASIC_BLOCK(7, -1)
        {
            INST(11, Opcode::Return).u64().Inputs(4);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    ASSERT_TRUE(GetGraph()->GetAnalysis<LoopAnalyzer>().IsValid());

    auto loop = BB(2).GetLoop();
    ASSERT_EQ(loop->GetHeader(), &BB(2));
    ASSERT_EQ(loop->GetBackEdges().size(), 2);
    ASSERT_TRUE(loop->HasBackEdge(&BB(4)));
    ASSERT_TRUE(loop->HasBackEdge(&BB(5)));
}

TEST_F(CleanupTest, DiamondBecomeEmpty)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(1, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, -1)
        {
            INST(2, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(5, -1)
        {
            INST(2, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, CarefulDCE)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, 3, 4)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(0, Opcode::CallStatic).b().InputsAutoType(20);
            INST(1, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(2, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(0, Opcode::CallStatic).b().InputsAutoType(20);
            INST(1, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, CarefulDCE2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 40)
        {
            INST(2, Opcode::If).SrcType(DataType::UINT64).CC(CC_GT).Inputs(0, 1);
        }
        BASIC_BLOCK(40, 5) {}
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(5, 6, 7)
        {
            INST(3, Opcode::Phi).u64().Inputs({{40, 0}, {3, 1}});
            INST(4, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(6, 7) {}
        BASIC_BLOCK(7, -1)
        {
            INST(5, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, Delete)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 12);
        CONSTANT(1, 13);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Neg).u64().Inputs(1);
            INST(3, Opcode::Add).u64().Inputs(0, 2);
            INST(4, Opcode::Return).u64().Inputs(2);
        }
    }

    // Delete Insts 0 and 3
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    ASSERT_TRUE(CheckInputs(INS(2), {1}));
    ASSERT_TRUE(CheckInputs(INS(4), {2}));

    ASSERT_TRUE(CheckUsers(INS(1), {2}));
    ASSERT_TRUE(CheckUsers(INS(2), {4}));
    ASSERT_TRUE(CheckUsers(INS(4), {}));
}

TEST_F(CleanupTest, NotRemoved)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 11);
        CONSTANT(1, 12);
        CONSTANT(2, 13);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).u64().Inputs(0, 1);
            INST(4, Opcode::Neg).u64().Inputs(3);
            INST(5, Opcode::Sub).u64().Inputs(4, 2);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());

    ASSERT_TRUE(CheckInputs(INS(3), {0, 1}));
    ASSERT_TRUE(CheckInputs(INS(4), {3}));
    ASSERT_TRUE(CheckInputs(INS(5), {4, 2}));
    ASSERT_TRUE(CheckInputs(INS(6), {5}));

    ASSERT_TRUE(CheckUsers(INS(0), {3}));
    ASSERT_TRUE(CheckUsers(INS(1), {3}));
    ASSERT_TRUE(CheckUsers(INS(2), {5}));
    ASSERT_TRUE(CheckUsers(INS(3), {4}));
    ASSERT_TRUE(CheckUsers(INS(4), {5}));
    ASSERT_TRUE(CheckUsers(INS(5), {6}));
}

TEST_F(CleanupTest, Loop)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::Phi).u64().Inputs(0, 5);
            INST(3, Opcode::Phi).u64().Inputs(1, 6);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Add).u64().Inputs(2, 3);
            INST(11, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(4, 2)
        {
            INST(5, Opcode::Neg).u64().Inputs(3);
            INST(6, Opcode::Neg).u64().Inputs(4);
            // Must be removed
            INST(7, Opcode::Add).u64().Inputs(5, 6);
        }
        BASIC_BLOCK(5, -1)
        {
            // Unused instruction
            INST(8, Opcode::Neg).u64().Inputs(4);
            INST(9, Opcode::Return).u64().Inputs(4);
        }
    }

    // Delete Insts 7 and 8
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    ASSERT_TRUE(CheckInputs(INS(2), {0, 5}));
    ASSERT_TRUE(CheckInputs(INS(3), {1, 6}));
    ASSERT_TRUE(CheckInputs(INS(4), {2, 3}));
    ASSERT_TRUE(CheckInputs(INS(5), {3}));
    ASSERT_TRUE(CheckInputs(INS(6), {4}));
    ASSERT_TRUE(CheckInputs(INS(9), {4}));

    ASSERT_TRUE(CheckUsers(INS(0), {2, 11}));
    ASSERT_TRUE(CheckUsers(INS(1), {3, 11}));
    ASSERT_TRUE(CheckUsers(INS(2), {4}));
    ASSERT_TRUE(CheckUsers(INS(3), {4, 5}));
    ASSERT_TRUE(CheckUsers(INS(4), {6, 9}));
    ASSERT_TRUE(CheckUsers(INS(5), {2}));
    ASSERT_TRUE(CheckUsers(INS(6), {3}));
}

TEST_F(CleanupTest, Loop1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 2, 3)
        {
            // Unused instructions
            INST(2, Opcode::Phi).u64().Inputs(1, 3);
            INST(3, Opcode::Add).u64().Inputs(2, 1);
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 0);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(3, -1)
        {
            // Unused instruction
            INST(4, Opcode::Neg).u64().Inputs(0);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }

    // Delete Insts 1, 2 and 3
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    ASSERT_EQ(INS(0).GetNext(), nullptr);
    ASSERT_FALSE(BB(2).IsEmpty());
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

TEST_F(CleanupTest, PhiInputs1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(18, 2);
        CONSTANT(1, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(18, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 7, 5)
        {
            INST(4, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(18, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(18, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }

        BASIC_BLOCK(5, 6)
        {
            INST(8, Opcode::Phi).u64().Inputs({{4, 0}, {3, 0}});
        }
        BASIC_BLOCK(6, 9)
        {
            INST(10, Opcode::Phi).u64().Inputs({{4, 0}, {5, 8}});
        }
        BASIC_BLOCK(7, 8, 9)
        {
            INST(12, Opcode::Phi).u64().Inputs({{8, 12}, {3, 0}});
            INST(13, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(18, 1);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(8, 7) {}
        BASIC_BLOCK(9, -1)
        {
            INST(16, Opcode::Phi).u64().Inputs({{6, 10}, {7, 12}});
            INST(17, Opcode::Return).u64().Inputs(16);
        }
    }

    // Delete all phi instructions
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    ASSERT_TRUE(CheckInputs(INS(17), {0}));
    ASSERT_TRUE(CheckUsers(INS(0), {17}));
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

TEST_F(CleanupTest, PhiInputs2)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(6, 2);
        CONSTANT(7, 3);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(6, 7);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 6, 7)
        {
            INST(2, Opcode::Phi).u64().Inputs(0, 1);
            INST(12, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(6, 7);
            INST(13, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(12);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(3, Opcode::Phi).u64().Inputs({{7, 4}, {5, 2}});
        }
        BASIC_BLOCK(7, 6, 8)
        {
            INST(4, Opcode::Phi).u64().Inputs({{6, 3}, {5, 2}});
            INST(15, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(6, 7);
            INST(16, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(15);
        }
        BASIC_BLOCK(8, -1)
        {
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }

    // Delete 3 and 4 phi (replace with v2p in return)
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    ASSERT_TRUE(CheckInputs(INS(2), {0, 1}));
    ASSERT_TRUE(CheckInputs(INS(5), {2}));

    ASSERT_TRUE(CheckUsers(INS(0), {2}));
    ASSERT_TRUE(CheckUsers(INS(1), {2}));
    ASSERT_TRUE(CheckUsers(INS(2), {5}));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(6, 2);
        CONSTANT(7, 3);
        BASIC_BLOCK(2, 5, 4)
        {
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(6, 7);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 7)
        {
            INST(2, Opcode::Phi).u64().Inputs(0, 1);
        }
        BASIC_BLOCK(7, 7, 8)
        {
            INST(15, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(6, 7);
            INST(16, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(15);
        }
        BASIC_BLOCK(8, -1)
        {
            INST(5, Opcode::Return).u64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, NullPtr)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, nullptr);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Return).u64().Inputs(0);
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
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 2);
        CONSTANT(3, 3);
        CONSTANT(4, 4);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Add).u64().Inputs(1, 4);
            INST(6, Opcode::Add).u64().Inputs(2, 3);
            INST(7, Opcode::Compare).b().Inputs(0, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs({{2, 6}, {3, 5}});
            INST(10, Opcode::Return).u64().Inputs(0);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(10, Opcode::Return).u64().Inputs(0);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, RemovingPhiFromTheSameBlock)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 2, 3)
        {
            INST(5, Opcode::Phi).u64().Inputs(1, 6);
            INST(6, Opcode::Phi).u64().Inputs(0, 6);
            INST(7, Opcode::Phi).u64().Inputs(0, 8);
            INST(8, Opcode::Add).u64().Inputs(7, 2);
            INST(9, Opcode::Compare).b().CC(CC_NE).Inputs(8, 1);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(11, Opcode::Add).u64().Inputs(5, 6);
            INST(12, Opcode::Return).u64().Inputs(11);
        }
    }
    ASSERT_TRUE(graph->RunPass<Cleanup>());
}

TEST_F(CleanupTest, CallDiscardReturnValue)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(0, Opcode::CallStatic).s32().InputsAutoType(20);
            INST(1, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_FALSE(INS(0).HasUsers());
    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());
    ASSERT_TRUE(INS(0).GetOpcode() == Opcode::CallStatic);
}

TEST_F(CleanupTest, CallReturnVoid)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(0, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(1, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_FALSE(INS(0).HasUsers());
    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());
    ASSERT_TRUE(INS(0).GetOpcode() == Opcode::CallStatic);
}

TEST_F(CleanupTest, StoreObject)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 1).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::StoreObject).s32().Inputs(3, 1);
            INST(5, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_FALSE(INS(4).HasUsers());
    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());
    ASSERT_TRUE(INS(4).GetOpcode() == Opcode::StoreObject);
}

TEST_F(CleanupTest, OneBlock)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::ReturnVoid);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, TwoBlocks)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 12);
        CONSTANT(1, 13);

        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::Neg).u64().Inputs(1);
            INST(3, Opcode::Add).u64().Inputs(0, 2);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(4, Opcode::Add).u64().Inputs(2, 3);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 12);
        CONSTANT(1, 13);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Neg).u64().Inputs(1);
            INST(3, Opcode::Add).u64().Inputs(0, 2);
            INST(4, Opcode::Add).u64().Inputs(2, 3);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, SameBlockPhiTwice)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 12).s64();
        CONSTANT(1, 13).s64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Neg).u64().Inputs(0);
            INST(5, Opcode::Neg).u64().Inputs(1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::Neg).u64().Inputs(1);
            INST(8, Opcode::Neg).u64().Inputs(0);
        }

        BASIC_BLOCK(5, 6)
        {
            INST(10, Opcode::Phi).u64().Inputs({{3, 4}, {4, 7}, {6, 15}});
            INST(11, Opcode::Phi).u64().Inputs({{3, 5}, {4, 8}, {6, 16}});
            INST(12, Opcode::Neg).u64().Inputs(10);
            INST(13, Opcode::Add).u64().Inputs(10, 11);
        }

        BASIC_BLOCK(6, 5, -1)
        {
            INST(15, Opcode::Add).u64().Inputs(0, 1);
            INST(16, Opcode::Neg).u64().Inputs(0);
            INST(17, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(17);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 12).s64();
        CONSTANT(1, 13).s64();

        BASIC_BLOCK(7, 6) {}

        BASIC_BLOCK(6, 6, -1)
        {
            INST(17, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(17);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, TwoBlocksLoop)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 12).s64();
        CONSTANT(1, 13).s64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Neg).u64().Inputs(0);
            INST(5, Opcode::Neg).u64().Inputs(1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::Neg).u64().Inputs(1);
            INST(8, Opcode::Neg).u64().Inputs(0);
        }

        BASIC_BLOCK(5, 6)
        {
            INST(10, Opcode::Phi).u64().Inputs({{3, 4}, {4, 7}, {6, 15}});
            INST(11, Opcode::Phi).u64().Inputs({{3, 5}, {4, 8}, {6, 16}});
            INST(12, Opcode::Neg).u64().Inputs(10);
            INST(13, Opcode::Add).u64().Inputs(10, 11);
        }

        BASIC_BLOCK(6, 5, -1)
        {
            INST(15, Opcode::Add).u64().Inputs(12, 13);
            INST(16, Opcode::Neg).u64().Inputs(12);
            INST(17, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(15, 16);
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(17);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 12).s64();
        CONSTANT(1, 13).s64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 7)
        {
            INST(4, Opcode::Neg).u64().Inputs(0);
            INST(5, Opcode::Neg).u64().Inputs(1);
        }

        BASIC_BLOCK(4, 7)
        {
            INST(7, Opcode::Neg).u64().Inputs(1);
            INST(8, Opcode::Neg).u64().Inputs(0);
        }

        BASIC_BLOCK(7, 5)
        {
            INST(19, Opcode::Phi).u64().Inputs({{3, 4}, {4, 7}});
            INST(20, Opcode::Phi).u64().Inputs({{3, 5}, {4, 8}});
        }

        BASIC_BLOCK(5, 5, -1)
        {
            INST(10, Opcode::Phi).u64().Inputs({{7, 19}, {5, 15}});
            INST(11, Opcode::Phi).u64().Inputs({{7, 20}, {5, 16}});
            INST(12, Opcode::Neg).u64().Inputs(10);
            INST(13, Opcode::Add).u64().Inputs(10, 11);
            INST(15, Opcode::Add).u64().Inputs(12, 13);
            INST(16, Opcode::Neg).u64().Inputs(12);
            INST(17, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(15, 16);
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(17);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, TwoLoopsPreHeader)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 12).s64();
        CONSTANT(1, 13).s64();
        PARAMETER(8, 0).b();

        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::Neg).u64().Inputs(0);
        }

        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Neg).u64().Inputs(0);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(4, 4, -1)
        {
            INST(6, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }

        BASIC_BLOCK(5, 5, -1)
        {
            INST(10, Opcode::Compare).b().CC(CC_GT).SrcType(DataType::Type::UINT64).Inputs(4, 2);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
    }

    ASSERT_EQ(&BB(3), BB(4).GetLoop()->GetPreHeader());
    ASSERT_EQ(&BB(3), BB(5).GetLoop()->GetPreHeader());
    ASSERT_EQ(1U, BB(4).GetLoop()->GetBlocks().size());
    ASSERT_EQ(1U, BB(5).GetLoop()->GetBlocks().size());
    ASSERT_EQ(4U, BB(4).GetLoop()->GetOuterLoop()->GetBlocks().size());
    ASSERT_EQ(4U, BB(5).GetLoop()->GetOuterLoop()->GetBlocks().size());

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 12).s64();
        CONSTANT(1, 13).s64();
        PARAMETER(8, 0).b();

        BASIC_BLOCK(2, 4, 5)
        {
            INST(2, Opcode::Neg).u64().Inputs(0);
            INST(4, Opcode::Neg).u64().Inputs(0);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(4, 4, -1)
        {
            INST(6, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }

        BASIC_BLOCK(5, 5, -1)
        {
            INST(10, Opcode::Compare).b().CC(CC_GT).SrcType(DataType::Type::UINT64).Inputs(4, 2);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));

    EXPECT_EQ(&BB(2), BB(4).GetLoop()->GetPreHeader());
    EXPECT_EQ(&BB(2), BB(5).GetLoop()->GetPreHeader());
    EXPECT_EQ(1U, BB(4).GetLoop()->GetBlocks().size());
    EXPECT_EQ(1U, BB(5).GetLoop()->GetBlocks().size());
    EXPECT_EQ(3U, BB(4).GetLoop()->GetOuterLoop()->GetBlocks().size());
    EXPECT_EQ(3U, BB(5).GetLoop()->GetOuterLoop()->GetBlocks().size());
}

TEST_F(CleanupTest, LoopBackEdge)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 12).s64();
        CONSTANT(1, 13).s64();

        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::Neg).u64().Inputs(0);
        }

        BASIC_BLOCK(3, 2, -1)
        {
            INST(4, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(1, 2);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
    }

    ASSERT_TRUE(BB(3).GetLoop()->HasBackEdge(&BB(3)));
    ASSERT_EQ(2U, BB(3).GetLoop()->GetBlocks().size());

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 12).s64();
        CONSTANT(1, 13).s64();

        BASIC_BLOCK(2, 2, -1)
        {
            INST(2, Opcode::Neg).u64().Inputs(0);
            INST(4, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(1, 2);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));

    EXPECT_TRUE(BB(2).GetLoop()->HasBackEdge(&BB(2)));
    EXPECT_EQ(1U, BB(2).GetLoop()->GetBlocks().size());
}

TEST_F(CleanupTest, LoopMiddleBlock)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 12).s64();
        CONSTANT(1, 13).s64();

        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::Neg).u64().Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(4, Opcode::Neg).u64().Inputs(1);
        }

        BASIC_BLOCK(4, 2, -1)
        {
            INST(6, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2, 4);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
    }

    ASSERT_EQ(3U, BB(2).GetLoop()->GetBlocks().size());
    ASSERT_EQ(3U, BB(2).GetLoop()->GetOuterLoop()->GetBlocks().size());

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 12).s64();
        CONSTANT(1, 13).s64();

        BASIC_BLOCK(2, 2, -1)
        {
            INST(2, Opcode::Neg).u64().Inputs(0);
            INST(4, Opcode::Neg).u64().Inputs(1);
            INST(6, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2, 4);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));

    EXPECT_EQ(1U, BB(2).GetLoop()->GetBlocks().size());
    EXPECT_EQ(3U, BB(2).GetLoop()->GetOuterLoop()->GetBlocks().size());
}

TEST_F(CleanupTest, ThreeBlocks)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 12);
        CONSTANT(1, 13);

        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::Neg).u64().Inputs(1);
            INST(3, Opcode::Add).u64().Inputs(0, 2);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(4, Opcode::Neg).u64().Inputs(0);
            INST(5, Opcode::Add).u64().Inputs(1, 4);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Add).u64().Inputs(3, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 12);
        CONSTANT(1, 13);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Neg).u64().Inputs(1);
            INST(3, Opcode::Add).u64().Inputs(0, 2);
            INST(4, Opcode::Neg).u64().Inputs(0);
            INST(5, Opcode::Add).u64().Inputs(1, 4);
            INST(6, Opcode::Add).u64().Inputs(3, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(CleanupTest, TwoBlocksAndPhi)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 12).s64();
        CONSTANT(1, 13).s64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Neg).u64().Inputs(0);
            INST(5, Opcode::Neg).u64().Inputs(1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::Neg).u64().Inputs(1);
            INST(8, Opcode::Neg).u64().Inputs(0);
        }

        BASIC_BLOCK(5, 6)
        {
            INST(10, Opcode::Phi).u64().Inputs({{3, 4}, {4, 7}});
            INST(11, Opcode::Phi).u64().Inputs({{3, 5}, {4, 8}});
            INST(12, Opcode::Neg).u64().Inputs(10);
            INST(13, Opcode::Add).u64().Inputs(10, 11);
        }

        BASIC_BLOCK(6, 7, 8)
        {
            INST(15, Opcode::Add).u64().Inputs(12, 13);
            INST(16, Opcode::Neg).u64().Inputs(12);
            INST(17, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(16, 15);
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(17);
        }

        BASIC_BLOCK(7, 8)
        {
            INST(19, Opcode::Add).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(8, -1)
        {
            INST(21, Opcode::Phi).u64().Inputs({{6, 16}, {7, 19}});
            INST(22, Opcode::Add).u64().Inputs(21, 1);
            INST(23, Opcode::Return).u64().Inputs(22);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 12).s64();
        CONSTANT(1, 13).s64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Neg).u64().Inputs(0);
            INST(5, Opcode::Neg).u64().Inputs(1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::Neg).u64().Inputs(1);
            INST(8, Opcode::Neg).u64().Inputs(0);
        }

        BASIC_BLOCK(5, 7, 8)
        {
            INST(10, Opcode::Phi).u64().Inputs({{3, 4}, {4, 7}});
            INST(11, Opcode::Phi).u64().Inputs({{3, 5}, {4, 8}});
            INST(12, Opcode::Neg).u64().Inputs(10);
            INST(13, Opcode::Add).u64().Inputs(10, 11);
            INST(15, Opcode::Add).u64().Inputs(12, 13);
            INST(16, Opcode::Neg).u64().Inputs(12);
            INST(17, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(16, 15);
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(17);
        }

        BASIC_BLOCK(7, 8)
        {
            INST(19, Opcode::Add).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(8, -1)
        {
            INST(21, Opcode::Phi).u64().Inputs(16, 19);
            INST(22, Opcode::Add).u64().Inputs(21, 1);
            INST(23, Opcode::Return).u64().Inputs(22);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
}  // namespace panda::compiler
