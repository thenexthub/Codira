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
#include "optimizer/optimizations/redundant_loop_elimination.h"

namespace ark::compiler {
class RedundantLoopEliminationTest : public CommonTest {
public:
    RedundantLoopEliminationTest() : graph_(CreateGraphStartEndBlocks()) {}

    Graph *GetGraph()
    {
        return graph_;
    }

private:
    Graph *graph_ {nullptr};
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(RedundantLoopEliminationTest, SimpleLoopTest1)
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 10U);
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(20U, Opcode::SafePoint).Inputs().SrcVregs({});
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(5U, Opcode::Compare).CC(CC_LT).b().Inputs(4U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 10U);
        BASIC_BLOCK(2U, 5U) {}
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RedundantLoopEliminationTest, SimpleLoopTest2)
{
    // not applied, insts from loop have user outside the loop.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 10U);
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(5U, Opcode::Compare).CC(CC_LT).b().Inputs(4U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 10U);
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(5U, Opcode::Compare).CC(CC_LT).b().Inputs(4U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(SimpleLoopTest3, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 10U);
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(5U, Opcode::Compare).CC(CC_LT).b().Inputs(4U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).s32().InputsAutoType(20U);
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(SimpleLoopTest3, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 10U);
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(5U, Opcode::Compare).CC(CC_LT).b().Inputs(4U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).s32().InputsAutoType(20U);
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(RedundantLoopEliminationTest, SimpleLoopTest3)
{
    // not applied, loop contains not removable inst
    src_graph::SimpleLoopTest3::CREATE(GetGraph());
    ASSERT_FALSE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    out_graph::SimpleLoopTest3::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RedundantLoopEliminationTest, InnerLoopsTest1)
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        PARAMETER(3U, 0U).s32();
        PARAMETER(4U, 1U).s32();
        BASIC_BLOCK(2U, 5U, 4U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 8U);
            INST(6U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5U, 4U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(5U, 6U, 3U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(11U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(10U, 3U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(6U, 5U)
        {
            INST(13U, Opcode::Add).s32().Inputs(10U, 1U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(8U, Opcode::Add).s32().Inputs(5U, 1U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(9U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        PARAMETER(3U, 0U).s32();
        PARAMETER(4U, 1U).s32();
        BASIC_BLOCK(2U, 5U) {}
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(InnerLoopsTest2, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        PARAMETER(3U, 0U).s32();
        PARAMETER(4U, 1U).s32();
        BASIC_BLOCK(2U, 5U, 4U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 8U);
            INST(6U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5U, 4U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(5U, 6U, 3U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(11U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(10U, 3U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(6U, 5U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).s32().InputsAutoType(20U);
            INST(13U, Opcode::Add).s32().Inputs(10U, 1U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(8U, Opcode::Add).s32().Inputs(5U, 1U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(9U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(InnerLoopsTest2, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        PARAMETER(3U, 0U).s32();
        PARAMETER(4U, 1U).s32();
        BASIC_BLOCK(2U, 5U, 4U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 8U);
            INST(6U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5U, 4U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(5U, 6U, 3U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(11U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(10U, 3U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(6U, 5U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).s32().InputsAutoType(20U);
            INST(13U, Opcode::Add).s32().Inputs(10U, 1U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(8U, Opcode::Add).s32().Inputs(5U, 1U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(9U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(RedundantLoopEliminationTest, InnerLoopsTest2)
{
    // not applied, inner loop contains not removable inst
    src_graph::InnerLoopsTest2::CREATE(GetGraph());
    ASSERT_FALSE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    out_graph::InnerLoopsTest2::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(InnerLoopsTest3, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        PARAMETER(3U, 0U).s32();
        PARAMETER(4U, 1U).s32();
        BASIC_BLOCK(2U, 5U, 4U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 8U);
            INST(6U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5U, 4U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(5U, 6U, 3U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(11U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(10U, 3U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(6U, 5U)
        {
            INST(13U, Opcode::Add).s32().Inputs(10U, 1U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).s32().InputsAutoType(20U);
            INST(8U, Opcode::Add).s32().Inputs(5U, 1U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(9U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(InnerLoopsTest3, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        PARAMETER(3U, 0U).s32();
        PARAMETER(4U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 8U);
            INST(6U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5U, 4U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).s32().InputsAutoType(20U);
            INST(8U, Opcode::Add).s32().Inputs(5U, 1U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(9U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(RedundantLoopEliminationTest, InnerLoopsTest3)
{
    // applied, outher loop contains not removable inst, but inner loop
    src_graph::InnerLoopsTest3::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<RedundantLoopElimination>());
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    out_graph::InnerLoopsTest3::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(InnerLoopsTestPhiInOuterBlock, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        PARAMETER(3U, 0U).s32();
        PARAMETER(4U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 8U);
            INST(6U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5U, 4U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).s32().InputsAutoType(20U);
            INST(8U, Opcode::Add).s32().Inputs(5U, 1U);
        }
        BASIC_BLOCK(5U, 6U, 2U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(11U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(10U, 3U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(6U, 5U)
        {
            INST(13U, Opcode::Add).s32().Inputs(10U, 1U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(9U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(InnerLoopsTestPhiInOuterBlock, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        PARAMETER(3U, 0U).s32();
        PARAMETER(4U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 8U);
            INST(6U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5U, 4U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).s32().InputsAutoType(20U);
            INST(8U, Opcode::Add).s32().Inputs(5U, 1U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(9U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(RedundantLoopEliminationTest, InnerLoopsTestPhiInOuterBlock)
{
    // applied for inner loop
    // outer block of inner loop is head of outer loop, check that Phi inputs in it are updated correctly
    src_graph::InnerLoopsTestPhiInOuterBlock::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    out_graph::InnerLoopsTestPhiInOuterBlock::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RedundantLoopEliminationTest, SimpleLoopTestIncAfterPeeling)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 10U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < len_array
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(4U, Opcode::Phi).s32().Inputs({{2U, 0U}, {3U, 10U}});
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < len_array
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 10U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < len_array
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(TestLoopWithPhiInOuterBlock, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        PARAMETER(3U, 0U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(0U, 3U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(0U, 8U);
            INST(8U, Opcode::Add).s32().Inputs(9U, 1U);
        }
        BASIC_BLOCK(5U, 3U, 4U)
        {
            INST(10U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(8U, 3U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 0U}, {5U, 1U}});
            INST(13U, Opcode::Return).s32().Inputs(12U);
        }
    }
}

OUT_GRAPH(TestLoopWithPhiInOuterBlock, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        PARAMETER(3U, 0U).s32();
        BASIC_BLOCK(2U, 4U, 6U)
        {
            INST(6U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(0U, 3U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(6U, 4U) {}
        BASIC_BLOCK(4U, 1U)
        {
            INST(12U, Opcode::Phi).s32().Inputs({{6U, 0U}, {2U, 1U}});
            INST(13U, Opcode::Return).s32().Inputs(12U);
        }
    }
}

TEST_F(RedundantLoopEliminationTest, TestLoopWithPhiInOuterBlock)
{
    // applied for loop with exit from backedge
    // check that inputs of Phi in outer block are updated correctly
    src_graph::TestLoopWithPhiInOuterBlock::CREATE(GetGraph());
    EXPECT_TRUE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    out_graph::TestLoopWithPhiInOuterBlock::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RedundantLoopEliminationTest, InfiniteLoop)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, 2U) {}
    }
    auto cloneGraph = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<RedundantLoopElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), cloneGraph));
}

TEST_F(RedundantLoopEliminationTest, LoadObjectTest)
{
    for (bool volat : {true, false}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            CONSTANT(0U, 0U);
            CONSTANT(1U, 1U);
            CONSTANT(2U, 10U);
            PARAMETER(3U, 0U).ref();
            BASIC_BLOCK(3U, 4U, 5U)
            {
                INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
                INST(5U, Opcode::Compare).CC(CC_LT).b().Inputs(4U, 2U);
                INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
            }
            BASIC_BLOCK(4U, 3U)
            {
                INST(9U, Opcode::LoadObject).s32().Inputs(3U).Volatile(volat);
                INST(10U, Opcode::Add).s32().Inputs(4U, 1U);
            }
            BASIC_BLOCK(5U, 1U)
            {
                INST(12U, Opcode::ReturnVoid);
            }
        }
        ASSERT_TRUE(graph1->RunPass<RedundantLoopElimination>());
        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            CONSTANT(0U, 0U);
            CONSTANT(1U, 1U);
            CONSTANT(2U, 10U);
            PARAMETER(3U, 0U).ref();
            BASIC_BLOCK(2U, 5U) {}
            BASIC_BLOCK(5U, 1U)
            {
                INST(12U, Opcode::ReturnVoid);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(RedundantLoopEliminationTest, StoreObjectTest)
{
    for (bool volat : {true, false}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            CONSTANT(0U, 0U);
            CONSTANT(2U, 10U);
            PARAMETER(3U, 0U).ref();
            BASIC_BLOCK(3U, 4U, 5U)
            {
                INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
                INST(5U, Opcode::Compare).CC(CC_LT).b().Inputs(4U, 2U);
                INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
            }
            BASIC_BLOCK(4U, 3U)
            {
                INST(9U, Opcode::StoreObject).s32().Inputs(3U, 2U).Volatile(volat);
                INST(10U, Opcode::Add).s32().Inputs(4U, 2U);
            }
            BASIC_BLOCK(5U, 1U)
            {
                INST(12U, Opcode::ReturnVoid);
            }
        }
        auto graph2 = GraphCloner(graph1, graph1->GetAllocator(), graph1->GetLocalAllocator()).CloneGraph();
        ASSERT_FALSE(graph1->RunPass<RedundantLoopElimination>());
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(RedundantLoopEliminationTest, LoopWithSeveralExits)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::IfImm).b().CC(CC_NE).Inputs(0U).Imm(0U);
        }
        BASIC_BLOCK(4U, 3U, 6U)
        {
            INST(5U, Opcode::IfImm).b().CC(CC_NE).Inputs(1U).Imm(0U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(6U, Opcode::Return).s32().Inputs(0U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(7U, Opcode::Return).s32().Inputs(1U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<RedundantLoopElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(RedundantLoopEliminationTest, PotentiallyInfinitLoop)
{
    // This is potentially infinite loop. We can't remove it.
    // public static void loop1(boolean incoming) {
    //     While (incoming) {}
    // }
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).s32();
        BASIC_BLOCK(2U, 3U, 2U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_EQ).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).CC(CC_NE).Inputs(2U).Imm(0U);
        }
        BASIC_BLOCK(3U, 1U)
        {
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<RedundantLoopElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
