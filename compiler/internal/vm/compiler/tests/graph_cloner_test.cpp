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
#include "optimizer/ir/graph_cloner.h"
namespace panda::compiler {
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

TEST_F(GraphClonerTest, LoopCoping_SimpleLoop)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);          // initial
        CONSTANT(1, 1);          // increment
        CONSTANT(2, 10);         // len_array
        PARAMETER(13, 0).s32();  // X
        BASIC_BLOCK(2, 3, 6)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2);
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0, 13);  // i < X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::BoundsCheck).s32().Inputs(2, 4, 7);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 8, 0);                     // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);                              // i++
            INST(5, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10, 13);  // i < X
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 10);
            INST(12, Opcode::Return).s32().Inputs(16);
        }
    }

    GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneLoop(BB(3).GetLoop());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 0);          // initial
        CONSTANT(1, 1);          // increment
        CONSTANT(2, 10);         // len_array
        PARAMETER(13, 0).s32();  // X
        BASIC_BLOCK(2, 7)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2);
        }
        BASIC_BLOCK(7, 3, 6)
        {
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0, 13);  // i < X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::BoundsCheck).s32().Inputs(2, 4, 7);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 8, 0);                     // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);                              // i++
            INST(5, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10, 13);  // i < X
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(6, 8)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 10);
        }
        BASIC_BLOCK(8, 10, 13)
        {
            INST(17, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(16, 13);  // i < X
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(17);
        }
        BASIC_BLOCK(10, 10, 13)
        {
            INST(19, Opcode::Phi).s32().Inputs(16, 25);
            INST(22, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(23, Opcode::BoundsCheck).s32().Inputs(2, 19, 22);
            INST(24, Opcode::StoreArray).s32().Inputs(3, 23, 0);                    // a[i] = 0
            INST(25, Opcode::Add).s32().Inputs(19, 1);                              // i++
            INST(20, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(25, 13);  // i < X
            INST(21, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(20);
        }
        BASIC_BLOCK(13, 14)
        {
            INST(27, Opcode::Phi).s32().Inputs(16, 25);
        }
        BASIC_BLOCK(14, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(27);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(GraphClonerTest, LoopCoping_LoopSum)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);          // initial
        CONSTANT(1, 1);          // increment
        CONSTANT(2, 10);         // len_array
        PARAMETER(13, 0).s32();  // X
        BASIC_BLOCK(2, 7)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2);
        }
        BASIC_BLOCK(7, 3, 6)
        {
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0, 13);  // i < X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(20, Opcode::Phi).s32().Inputs(0, 21);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::BoundsCheck).s32().Inputs(2, 4, 7);
            INST(9, Opcode::LoadArray).s32().Inputs(3, 8);  // a[i]
            INST(21, Opcode::Add).s32().Inputs(20, 9);
            INST(10, Opcode::Add).s32().Inputs(4, 1);                              // i++
            INST(5, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10, 13);  // i < X
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(6, 5)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 10);
            INST(22, Opcode::Phi).s32().Inputs(0, 21);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(22);
        }
    }
    GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneLoop(BB(3).GetLoop());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 0);          // initial
        CONSTANT(1, 1);          // increment
        CONSTANT(2, 10);         // len_array
        PARAMETER(13, 0).s32();  // X
        BASIC_BLOCK(2, 7)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2);
        }
        BASIC_BLOCK(7, 3, 6)
        {
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0, 13);  // i < X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(20, Opcode::Phi).s32().Inputs(0, 21);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::BoundsCheck).s32().Inputs(2, 4, 7);
            INST(9, Opcode::LoadArray).s32().Inputs(3, 8);  // a[i]
            INST(21, Opcode::Add).s32().Inputs(20, 9);
            INST(10, Opcode::Add).s32().Inputs(4, 1);                              // i++
            INST(5, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10, 13);  // i < X
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(6, 8)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 10);
            INST(22, Opcode::Phi).s32().Inputs(0, 21);
        }
        BASIC_BLOCK(8, 10, 13)
        {
            INST(23, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(16, 13);  // i < X
            INST(24, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(23);
        }
        BASIC_BLOCK(10, 10, 13)
        {
            INST(25, Opcode::Phi).s32().Inputs(16, 31);
            INST(26, Opcode::Phi).s32().Inputs(22, 30);
            INST(27, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(28, Opcode::BoundsCheck).s32().Inputs(2, 25, 27);
            INST(29, Opcode::LoadArray).s32().Inputs(3, 28);  // a[i]
            INST(30, Opcode::Add).s32().Inputs(26, 29);
            INST(31, Opcode::Add).s32().Inputs(25, 1);                              // i++
            INST(33, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(31, 13);  // i < X
            INST(34, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(33);
        }
        BASIC_BLOCK(13, 14)
        {
            INST(35, Opcode::Phi).s32().Inputs(16, 31);
            INST(36, Opcode::Phi).s32().Inputs(22, 30);
        }
        BASIC_BLOCK(14, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(36);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(GraphClonerTest, LoopCoping_DoubleLoop)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);         // initial
        CONSTANT(1, 1);         // increment
        CONSTANT(2, 10);        // len_array
        PARAMETER(3, 0).s32();  // X
        PARAMETER(4, 1).s32();  // Y
        BASIC_BLOCK(2, 5, 4)
        {
            INST(5, Opcode::Phi).s32().Inputs(0, 8);
            INST(6, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5, 4);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(5, 6, 3)
        {
            INST(10, Opcode::Phi).s32().Inputs(0, 13);
            INST(11, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(10, 3);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(6, 5)
        {
            INST(13, Opcode::Add).s32().Inputs(10, 1);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(8, Opcode::Add).s32().Inputs(5, 1);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(9, Opcode::ReturnVoid).s32();
        }
    }
    ASSERT_FALSE(IsLoopSingleBackEdgeExitPoint((BB(2).GetLoop())));
    ASSERT_FALSE(IsLoopSingleBackEdgeExitPoint((BB(5).GetLoop())));
}

TEST_F(GraphClonerTest, LoopCoping_TwoBackEdge)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Phi).Inputs(1, 4, 6).s32();
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 2, 5)
        {
            INST(4, Opcode::Add).Inputs(1, 1).s32();
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(5, 2)
        {
            INST(6, Opcode::Add).Inputs(4, 4).s32();
        }
        BASIC_BLOCK(4, -1)
        {
            INST(7, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_FALSE(IsLoopSingleBackEdgeExitPoint((BB(2).GetLoop())));
}

TEST_F(GraphClonerTest, LoopCoping_HeadExit)
{
    // not applied
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(2, 10);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 7);
            INST(5, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(4, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(7, Opcode::Add).s32().Inputs(4, 1);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_FALSE(IsLoopSingleBackEdgeExitPoint((BB(2).GetLoop())));
}

TEST_F(GraphClonerTest, LoopCoping_WithoutIndexResolver)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);          // initial
        CONSTANT(1, 1);          // increment
        CONSTANT(2, 10);         // len_array
        PARAMETER(13, 0).s32();  // X
        BASIC_BLOCK(2, 7)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2);
        }
        BASIC_BLOCK(7, 3, 6)
        {
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0, 13);  // i < X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::BoundsCheck).s32().Inputs(2, 4, 7);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 8, 0);                     // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);                              // i++
            INST(5, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10, 13);  // i < X
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneLoop(BB(3).GetLoop());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 0);          // initial
        CONSTANT(1, 1);          // increment
        CONSTANT(2, 10);         // len_array
        PARAMETER(13, 0).s32();  // X
        BASIC_BLOCK(2, 7)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2);
        }
        BASIC_BLOCK(7, 3, 6)
        {
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0, 13);  // i < X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::BoundsCheck).s32().Inputs(2, 4, 7);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 8, 0);                     // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);                              // i++
            INST(5, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10, 13);  // i < X
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(6, 8)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 10);
        }
        BASIC_BLOCK(8, 10, 14)
        {
            INST(17, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(16, 13);  // i < X
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(17);
        }
        BASIC_BLOCK(10, 10, 14)
        {
            INST(19, Opcode::Phi).s32().Inputs(16, 25);
            INST(22, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(23, Opcode::BoundsCheck).s32().Inputs(2, 19, 22);
            INST(24, Opcode::StoreArray).s32().Inputs(3, 23, 0);                    // a[i] = 0
            INST(25, Opcode::Add).s32().Inputs(19, 1);                              // i++
            INST(20, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(25, 13);  // i < X
            INST(21, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(20);
        }
        BASIC_BLOCK(14, 13) {}
        BASIC_BLOCK(13, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}
}  // namespace panda::compiler
