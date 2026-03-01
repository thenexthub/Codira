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
namespace ark::compiler {
class GraphClonerTest : public CommonTest {
public:
    GraphClonerTest() : graph_(CreateGraphStartEndBlocks()) {}

    Graph *GetGraph()
    {
        return graph_;
    }

private:
    Graph *graph_;
};

// NOLINTBEGIN(readability-magic-numbers)
SRC_GRAPH(LoopCopingSimpleLoop, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);          // initial
        CONSTANT(1U, 1U);          // increment
        CONSTANT(2U, 10U);         // len_array
        PARAMETER(13U, 0U).s32();  // X
        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U);
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0U, 13U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(2U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 8U, 0U);                    // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i++
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10U, 13U);  // i < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(12U, Opcode::Return).s32().Inputs(16U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(LoopCopingSimpleLoop, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);          // initial
        CONSTANT(1U, 1U);          // increment
        CONSTANT(2U, 10U);         // len_array
        PARAMETER(13U, 0U).s32();  // X
        BASIC_BLOCK(2U, 7U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U);
        }
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0U, 13U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(2U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 8U, 0U);                    // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i++
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10U, 13U);  // i < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 8U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 10U);
        }
        BASIC_BLOCK(8U, 10U, 13U)
        {
            INST(17U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(16U, 13U);  // i < X
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
        }
        BASIC_BLOCK(10U, 10U, 13U)
        {
            INST(19U, Opcode::Phi).s32().Inputs(16U, 25U);
            INST(22U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(23U, Opcode::BoundsCheck).s32().Inputs(2U, 19U, 22U);
            INST(24U, Opcode::StoreArray).s32().Inputs(3U, 23U, 0U);                   // a[i] = 0
            INST(25U, Opcode::Add).s32().Inputs(19U, 1U);                              // i++
            INST(20U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(25U, 13U);  // i < X
            INST(21U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(20U);
        }
        BASIC_BLOCK(13U, 14U)
        {
            INST(27U, Opcode::Phi).s32().Inputs(16U, 25U);
        }
        BASIC_BLOCK(14U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(27U);
        }
    }
}

TEST_F(GraphClonerTest, LoopCopingSimpleLoop)
{
    src_graph::LoopCopingSimpleLoop::CREATE(GetGraph());

    GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneLoop(BB(3U).GetLoop());
    auto graph1 = CreateEmptyGraph();
    out_graph::LoopCopingSimpleLoop::CREATE(graph1);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

SRC_GRAPH(LoopCopingLoopSum, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);          // initial
        CONSTANT(1U, 1U);          // increment
        CONSTANT(2U, 10U);         // len_array
        PARAMETER(13U, 0U).s32();  // X
        BASIC_BLOCK(2U, 7U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U);
        }
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0U, 13U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(20U, Opcode::Phi).s32().Inputs(0U, 21U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(2U, 4U, 7U);
            INST(9U, Opcode::LoadArray).s32().Inputs(3U, 8U);  // a[i]
            INST(21U, Opcode::Add).s32().Inputs(20U, 9U);
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i++
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10U, 13U);  // i < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 5U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(22U, Opcode::Phi).s32().Inputs(0U, 21U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(22U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(LoopCopingLoopSum, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);          // initial
        CONSTANT(1U, 1U);          // increment
        CONSTANT(2U, 10U);         // len_array
        PARAMETER(13U, 0U).s32();  // X
        BASIC_BLOCK(2U, 7U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U);
        }
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0U, 13U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(20U, Opcode::Phi).s32().Inputs(0U, 21U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(2U, 4U, 7U);
            INST(9U, Opcode::LoadArray).s32().Inputs(3U, 8U);  // a[i]
            INST(21U, Opcode::Add).s32().Inputs(20U, 9U);
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i++
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10U, 13U);  // i < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 8U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(22U, Opcode::Phi).s32().Inputs(0U, 21U);
        }
        BASIC_BLOCK(8U, 10U, 13U)
        {
            INST(23U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(16U, 13U);  // i < X
            INST(24U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(23U);
        }
        BASIC_BLOCK(10U, 10U, 13U)
        {
            INST(25U, Opcode::Phi).s32().Inputs(16U, 31U);
            INST(26U, Opcode::Phi).s32().Inputs(22U, 30U);
            INST(27U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(28U, Opcode::BoundsCheck).s32().Inputs(2U, 25U, 27U);
            INST(29U, Opcode::LoadArray).s32().Inputs(3U, 28U);  // a[i]
            INST(30U, Opcode::Add).s32().Inputs(26U, 29U);
            INST(31U, Opcode::Add).s32().Inputs(25U, 1U);                              // i++
            INST(33U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(31U, 13U);  // i < X
            INST(34U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(33U);
        }
        BASIC_BLOCK(13U, 14U)
        {
            INST(35U, Opcode::Phi).s32().Inputs(16U, 31U);
            INST(36U, Opcode::Phi).s32().Inputs(22U, 30U);
        }
        BASIC_BLOCK(14U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(36U);
        }
    }
}

TEST_F(GraphClonerTest, LoopCopingLoopSum)
{
    src_graph::LoopCopingLoopSum::CREATE(GetGraph());
    GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneLoop(BB(3U).GetLoop());
    auto graph1 = CreateEmptyGraph();
    out_graph::LoopCopingLoopSum::CREATE(graph1);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(GraphClonerTest, LoopCopingDoubleLoop)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);         // initial
        CONSTANT(1U, 1U);         // increment
        CONSTANT(2U, 10U);        // len_array
        PARAMETER(3U, 0U).s32();  // X
        PARAMETER(4U, 1U).s32();  // Y
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
            INST(9U, Opcode::ReturnVoid).s32();
        }
    }
    ASSERT_FALSE(IsLoopSingleBackEdgeExitPoint((BB(2U).GetLoop())));
    ASSERT_FALSE(IsLoopSingleBackEdgeExitPoint((BB(5U).GetLoop())));
}

TEST_F(GraphClonerTest, LoopCopingTwoBackEdge)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Phi).Inputs(1U, 4U, 6U).s32();
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 2U, 5U)
        {
            INST(4U, Opcode::Add).Inputs(1U, 1U).s32();
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(5U, 2U)
        {
            INST(6U, Opcode::Add).Inputs(4U, 4U).s32();
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_FALSE(IsLoopSingleBackEdgeExitPoint((BB(2U).GetLoop())));
}

TEST_F(GraphClonerTest, LoopCopingHeadExit)
{
    // not applied
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 10U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 7U);
            INST(5U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(4U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(7U, Opcode::Add).s32().Inputs(4U, 1U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_FALSE(IsLoopSingleBackEdgeExitPoint((BB(2U).GetLoop())));
}

SRC_GRAPH(LoopCopingWithoutIndexResolver, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);          // initial
        CONSTANT(1U, 1U);          // increment
        CONSTANT(2U, 10U);         // len_array
        PARAMETER(13U, 0U).s32();  // X
        BASIC_BLOCK(2U, 7U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U);
        }
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0U, 13U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(2U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 8U, 0U);                    // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i++
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10U, 13U);  // i < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(LoopCopingWithoutIndexResolver, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);          // initial
        CONSTANT(1U, 1U);          // increment
        CONSTANT(2U, 10U);         // len_array
        PARAMETER(13U, 0U).s32();  // X
        BASIC_BLOCK(2U, 7U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U);
        }
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0U, 13U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(2U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 8U, 0U);                    // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i++
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10U, 13U);  // i < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 8U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 10U);
        }
        BASIC_BLOCK(8U, 10U, 14U)
        {
            INST(17U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(16U, 13U);  // i < X
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
        }
        BASIC_BLOCK(10U, 10U, 14U)
        {
            INST(19U, Opcode::Phi).s32().Inputs(16U, 25U);
            INST(22U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(23U, Opcode::BoundsCheck).s32().Inputs(2U, 19U, 22U);
            INST(24U, Opcode::StoreArray).s32().Inputs(3U, 23U, 0U);                   // a[i] = 0
            INST(25U, Opcode::Add).s32().Inputs(19U, 1U);                              // i++
            INST(20U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(25U, 13U);  // i < X
            INST(21U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(20U);
        }
        BASIC_BLOCK(14U, 13U) {}
        BASIC_BLOCK(13U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
}

TEST_F(GraphClonerTest, LoopCopingWithoutIndexResolver)
{
    src_graph::LoopCopingWithoutIndexResolver::CREATE(GetGraph());
    GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneLoop(BB(3U).GetLoop());
    auto graph1 = CreateEmptyGraph();
    out_graph::LoopCopingWithoutIndexResolver::CREATE(graph1);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
