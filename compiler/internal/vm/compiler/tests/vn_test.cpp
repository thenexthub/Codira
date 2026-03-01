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
#include "optimizer/optimizations/vn.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/graph_cloner.h"

namespace panda::compiler {
class VNTest : public AsmTest {
public:
    VNTest() : graph_(CreateGraphWithDefaultRuntime()) {}

    Graph *GetGraph() override
    {
        return graph_;
    }

private:
    Graph *graph_;
};

TEST_F(VNTest, VnTestApply1)
{
    // Remove duplicate arithmetic instructions
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 1);
            INST(7, Opcode::Sub).u32().Inputs(1, 0);
            INST(8, Opcode::Mul).f32().Inputs(4, 5);
            INST(9, Opcode::Div).f64().Inputs(3, 2);
            INST(10, Opcode::Sub).u32().Inputs(1, 0);
            INST(11, Opcode::Div).f64().Inputs(3, 2);
            INST(12, Opcode::Mul).f32().Inputs(4, 5);
            INST(13, Opcode::Add).u64().Inputs(0, 1);
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 10, 11, 12, 13, 20);
            INST(15, Opcode::ReturnVoid);
        }
    }
    Graph *graph_et = CreateGraphWithDefaultRuntime();
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 1);
            INST(7, Opcode::Sub).u32().Inputs(1, 0);
            INST(8, Opcode::Mul).f32().Inputs(4, 5);
            INST(9, Opcode::Div).f64().Inputs(3, 2);
            INST(10, Opcode::Sub).u32().Inputs(1, 0);
            INST(11, Opcode::Div).f64().Inputs(3, 2);
            INST(12, Opcode::Mul).f32().Inputs(4, 5);
            INST(13, Opcode::Add).u64().Inputs(0, 1);
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 7, 9, 8, 6, 20);
            INST(15, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, VnTestApply2)
{
    // Remove duplicate Cast,  AddI, Fcmp and Cmp instructions
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(2);
            INST(7, Opcode::AddI).u32().Imm(10ULL).Inputs(1);
            INST(8, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2, 3);
            INST(9, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(4, 5);
            INST(10, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(23, Opcode::SaveState).NoVregs();
            INST(12, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 23);
            INST(13, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(14, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(2);
            INST(15, Opcode::AddI).u32().Imm(10ULL).Inputs(1);
            INST(16, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2, 3);
            INST(17, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(4, 5);
            INST(18, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(18);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(24, Opcode::SaveState).NoVregs();
            INST(20, Opcode::CallStatic).b().InputsAutoType(17, 16, 15, 14, 24);
            INST(21, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(22, Opcode::ReturnVoid);
        }
    }
    Graph *graph_et = CreateGraphWithDefaultRuntime();
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(2);
            INST(7, Opcode::AddI).u32().Imm(10ULL).Inputs(1);
            INST(8, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2, 3);
            INST(9, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(4, 5);
            INST(10, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(23, Opcode::SaveState).NoVregs();
            INST(12, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 23);
            INST(13, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(14, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(2);
            INST(15, Opcode::AddI).u32().Imm(10ULL).Inputs(1);
            INST(16, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2, 3);
            INST(17, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(4, 5);
            INST(18, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(24, Opcode::SaveState).NoVregs();
            INST(20, Opcode::CallStatic).b().InputsAutoType(9, 8, 7, 6, 24);
            INST(21, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(22, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, VnTestNotApply1)
{
    // Arithmetic instructions has different type, inputs, opcodes
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 1);
            INST(7, Opcode::Add).u16().Inputs(0, 1);
            INST(8, Opcode::Add).u32().Inputs(0, 7);
            INST(9, Opcode::Div).f64().Inputs(2, 3);
            INST(10, Opcode::Div).f64().Inputs(3, 2);
            INST(11, Opcode::Mul).f64().Inputs(2, 3);
            INST(12, Opcode::Sub).f32().Inputs(4, 5);
            INST(13, Opcode::Sub).f32().Inputs(5, 4);
            INST(14, Opcode::Abs).f32().Inputs(4);
            INST(15, Opcode::Neg).f32().Inputs(4);
            INST(20, Opcode::SaveState).NoVregs();
            INST(16, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 20);
            INST(17, Opcode::ReturnVoid);
        }
    }
    Graph *graph_et = CreateGraphWithDefaultRuntime();
    // graph_et is equal the graph
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 1);
            INST(7, Opcode::Add).u16().Inputs(0, 1);
            INST(8, Opcode::Add).u32().Inputs(0, 7);
            INST(9, Opcode::Div).f64().Inputs(2, 3);
            INST(10, Opcode::Div).f64().Inputs(3, 2);
            INST(11, Opcode::Mul).f64().Inputs(2, 3);
            INST(12, Opcode::Sub).f32().Inputs(4, 5);
            INST(13, Opcode::Sub).f32().Inputs(5, 4);
            INST(14, Opcode::Abs).f32().Inputs(4);
            INST(15, Opcode::Neg).f32().Inputs(4);
            INST(20, Opcode::SaveState).NoVregs();
            INST(16, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 20);
            INST(17, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, VnTestNotApply2)
{
    // Can't applies:
    //  - Cast instructions have different types
    //  - AddI instructions have different constant
    //  - Fcmp instructions have different fcmpg flag
    //  - Cmp instructions have different CC
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Cast).u32().SrcType(DataType::FLOAT64).Inputs(2);
            INST(7, Opcode::AddI).u32().Imm(9ULL).Inputs(1);
            INST(8, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(false).Inputs(2, 3);
            INST(9, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(true).Inputs(4, 5);
            INST(10, Opcode::Compare).b().CC(CC_LT).Inputs(0, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(23, Opcode::SaveState).NoVregs();
            INST(12, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 23);
            INST(13, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(14, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(2);
            INST(15, Opcode::AddI).u32().Imm(10ULL).Inputs(1);
            INST(16, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2, 3);
            INST(17, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(4, 5);
            INST(18, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(18);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(24, Opcode::SaveState).NoVregs();
            INST(20, Opcode::CallStatic).b().InputsAutoType(17, 16, 15, 14, 24);
            INST(21, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(22, Opcode::ReturnVoid);
        }
    }
    Graph *graph_et = CreateGraphWithDefaultRuntime();
    // graph_et is equal the graph
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Cast).u32().SrcType(DataType::FLOAT64).Inputs(2);
            INST(7, Opcode::AddI).u32().Imm(9ULL).Inputs(1);
            INST(8, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(false).Inputs(2, 3);
            INST(9, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(true).Inputs(4, 5);
            INST(10, Opcode::Compare).b().CC(CC_LT).Inputs(0, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(23, Opcode::SaveState).NoVregs();
            INST(12, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 23);
            INST(13, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(14, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(2);
            INST(15, Opcode::AddI).u32().Imm(10ULL).Inputs(1);
            INST(16, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2, 3);
            INST(17, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(4, 5);
            INST(18, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(18);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(24, Opcode::SaveState).NoVregs();
            INST(20, Opcode::CallStatic).b().InputsAutoType(17, 16, 15, 14, 24);
            INST(21, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(22, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, VnTestNotApply3)
{
    // Can't applies:
    //  - Arithmetic instructions aren't dominate
    //  - CallStatic, LoadArray and StoreArray has NO_CSE
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).ref();
        PARAMETER(5, 5).ref();
        PARAMETER(6, 6).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(7, Opcode::LoadArray).u64().Inputs(5, 6);
            INST(8, Opcode::LoadArray).u64().Inputs(5, 6);
            INST(9, Opcode::StoreArray).f64().Inputs(4, 6, 3);
            INST(10, Opcode::StoreArray).f64().Inputs(4, 6, 3);
            INST(11, Opcode::Compare).b().CC(CC_LT).Inputs(7, 8);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(13, Opcode::Add).u64().Inputs(0, 1);
            INST(14, Opcode::Sub).u64().Inputs(0, 1);
            INST(15, Opcode::Mul).f64().Inputs(2, 3);
            INST(16, Opcode::Div).f64().Inputs(2, 3);
            INST(30, Opcode::SaveState).NoVregs();
            INST(17, Opcode::CallStatic).b().InputsAutoType(13, 14, 15, 16, 30);
            INST(18, Opcode::CallStatic).b().InputsAutoType(13, 14, 15, 16, 30);
            INST(19, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(20, Opcode::Add).u64().Inputs(0, 1);
            INST(21, Opcode::Sub).u64().Inputs(0, 1);
            INST(22, Opcode::Mul).f64().Inputs(2, 3);
            INST(23, Opcode::Div).f64().Inputs(2, 3);
            INST(31, Opcode::SaveState).NoVregs();
            INST(24, Opcode::CallStatic).b().InputsAutoType(20, 21, 22, 23, 31);
            INST(25, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(13).GetVN(), INS(20).GetVN());
    ASSERT_EQ(INS(14).GetVN(), INS(21).GetVN());
    ASSERT_EQ(INS(15).GetVN(), INS(22).GetVN());
    ASSERT_EQ(INS(16).GetVN(), INS(23).GetVN());

    Graph *graph_et = CreateGraphWithDefaultRuntime();
    // graph_et is equal the graph
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).ref();
        PARAMETER(5, 5).ref();
        PARAMETER(6, 6).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(7, Opcode::LoadArray).u64().Inputs(5, 6);
            INST(8, Opcode::LoadArray).u64().Inputs(5, 6);
            INST(9, Opcode::StoreArray).f64().Inputs(4, 6, 3);
            INST(10, Opcode::StoreArray).f64().Inputs(4, 6, 3);
            INST(11, Opcode::Compare).b().CC(CC_LT).Inputs(7, 8);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(13, Opcode::Add).u64().Inputs(0, 1);
            INST(14, Opcode::Sub).u64().Inputs(0, 1);
            INST(15, Opcode::Mul).f64().Inputs(2, 3);
            INST(16, Opcode::Div).f64().Inputs(2, 3);
            INST(30, Opcode::SaveState).NoVregs();
            INST(17, Opcode::CallStatic).b().InputsAutoType(13, 14, 15, 16, 30);
            INST(18, Opcode::CallStatic).b().InputsAutoType(13, 14, 15, 16, 30);
            INST(19, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(20, Opcode::Add).u64().Inputs(0, 1);
            INST(21, Opcode::Sub).u64().Inputs(0, 1);
            INST(22, Opcode::Mul).f64().Inputs(2, 3);
            INST(23, Opcode::Div).f64().Inputs(2, 3);
            INST(31, Opcode::SaveState).NoVregs();
            INST(24, Opcode::CallStatic).b().InputsAutoType(20, 21, 22, 23, 31);
            INST(25, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, VnTestApply3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(7, Opcode::Phi).s32().Inputs(1, 20);
            INST(11, Opcode::SafePoint).Inputs(0, 5, 7).SrcVregs({0, 1, 2});
            INST(12, Opcode::Compare).CC(CC_EQ).b().Inputs(7, 5);  // i < X
            INST(13, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(12);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(15, Opcode::SaveState).Inputs(0, 1, 2, 5, 7).SrcVregs({0, 1, 2, 3, 4});
            INST(16, Opcode::NullCheck).ref().Inputs(0, 15);
            INST(17, Opcode::LenArray).s32().Inputs(16);
            INST(18, Opcode::BoundsCheck).s32().Inputs(17, 7, 15);
            INST(19, Opcode::StoreArray).s64().Inputs(16, 18, 1);
            INST(20, Opcode::Add).s32().Inputs(7, 2);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(23, Opcode::Return).ref().Inputs(0);
        }
    }
    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(5).GetVN(), INS(17).GetVN());

    Graph *graph_et = CreateGraphWithDefaultRuntime();
    // graph_et is equal the graph
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(7, Opcode::Phi).s32().Inputs(1, 20);
            INST(11, Opcode::SafePoint).Inputs(0, 5, 7).SrcVregs({0, 1, 2});
            INST(12, Opcode::Compare).CC(CC_EQ).b().Inputs(7, 5);  // i < X
            INST(13, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(12);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(15, Opcode::SaveState).Inputs(0, 1, 2, 5, 7).SrcVregs({0, 1, 2, 3, 4});
            INST(16, Opcode::NullCheck).ref().Inputs(0, 15);
            INST(17, Opcode::LenArray).s32().Inputs(16);
            INST(18, Opcode::BoundsCheck).s32().Inputs(5, 7, 15);
            INST(19, Opcode::StoreArray).s64().Inputs(16, 18, 1);
            INST(20, Opcode::Add).s32().Inputs(7, 2);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(23, Opcode::Return).ref().Inputs(0);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, CleanupTrigger)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 1);
            INST(6, Opcode::Add).u64().Inputs(0, 1);
            INST(7, Opcode::Compare).b().Inputs(0, 1);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs({{2, 6}, {3, 5}});
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }

    GraphChecker(GetGraph()).Check();
    GetGraph()->RunPass<ValNum>();

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 1);
            INST(6, Opcode::Add).u64().Inputs(0, 1);
            INST(7, Opcode::Compare).b().Inputs(0, 1);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs({{2, 5}, {3, 5}});
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(VNTest, OSR)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            INST(3, Opcode::Compare).b().Inputs(0, 2);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(5, Opcode::Phi).u64().Inputs({{2, 0}, {3, 6}});
            INST(6, Opcode::Sub).u64().Inputs(5, 1);
            INST(7, Opcode::Compare).b().Inputs(6, 1);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs({{2, 0}, {3, 6}});
            INST(10, Opcode::Add).u64().Inputs(0, 1);
            INST(11, Opcode::Add).u64().Inputs(9, 10);
            INST(12, Opcode::Return).u64().Inputs(11);
        }
    }

    // Remove Inst 11 without OSR

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();

    auto graph = CreateGraphWithDefaultRuntime();

    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            INST(3, Opcode::Compare).b().Inputs(0, 2);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(5, Opcode::Phi).u64().Inputs({{2, 0}, {3, 6}});
            INST(6, Opcode::Sub).u64().Inputs(5, 1);
            INST(7, Opcode::Compare).b().Inputs(6, 1);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs({{2, 0}, {3, 6}});
            INST(11, Opcode::Add).u64().Inputs(9, 2);
            INST(12, Opcode::Return).u64().Inputs(11);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));

    auto graph_osr = CreateGraphOsrWithDefaultRuntime();

    GRAPH(graph_osr)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            INST(3, Opcode::Compare).b().Inputs(0, 2);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(5, Opcode::Phi).u64().Inputs({{2, 0}, {3, 6}});
            INST(13, Opcode::SaveStateOsr).Inputs(0, 1, 5).SrcVregs({0, 1, 2});
            INST(6, Opcode::Sub).u64().Inputs(5, 1);
            INST(7, Opcode::Compare).b().Inputs(6, 1);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs({{2, 0}, {3, 6}});
            INST(10, Opcode::Add).u64().Inputs(0, 1);
            INST(11, Opcode::Add).u64().Inputs(9, 10);
            INST(12, Opcode::Return).u64().Inputs(11);
        }
    }

    for (auto bb : graph_osr->GetBlocksRPO()) {
        if (bb->IsLoopHeader()) {
            bb->SetOsrEntry(true);
        }
    }

    auto clone_osr = GraphCloner(graph_osr, graph_osr->GetAllocator(), graph_osr->GetLocalAllocator()).CloneGraph();

    // Don't remove Inst 11 with OSR

    graph_osr->RunPass<ValNum>();
    graph_osr->RunPass<Cleanup>();

    ASSERT_TRUE(GraphComparator().Compare(clone_osr, graph_osr));
}

TEST_F(VNTest, VnTestCommutative)
{
    // Remove commutative arithmetic instructions(See 2516)
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            INST(3, Opcode::Mul).u64().Inputs(0, 1);
            INST(4, Opcode::Xor).u64().Inputs(0, 1);
            INST(5, Opcode::Or).u64().Inputs(0, 1);
            INST(6, Opcode::And).u64().Inputs(0, 1);
            INST(7, Opcode::Min).u64().Inputs(0, 1);
            INST(8, Opcode::Max).u64().Inputs(0, 1);
            INST(9, Opcode::Add).u64().Inputs(1, 0);
            INST(10, Opcode::Mul).u64().Inputs(1, 0);
            INST(11, Opcode::Xor).u64().Inputs(1, 0);
            INST(12, Opcode::Or).u64().Inputs(1, 0);
            INST(13, Opcode::And).u64().Inputs(1, 0);
            INST(14, Opcode::Min).u64().Inputs(1, 0);
            INST(15, Opcode::Max).u64().Inputs(1, 0);
            INST(20, Opcode::SaveState).NoVregs();
            INST(16, Opcode::CallStatic).b().InputsAutoType(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 20);
            INST(17, Opcode::ReturnVoid);
        }
    }
    Graph *graph_et = CreateGraphWithDefaultRuntime();
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            INST(3, Opcode::Mul).u64().Inputs(0, 1);
            INST(4, Opcode::Xor).u64().Inputs(0, 1);
            INST(5, Opcode::Or).u64().Inputs(0, 1);
            INST(6, Opcode::And).u64().Inputs(0, 1);
            INST(7, Opcode::Min).u64().Inputs(0, 1);
            INST(8, Opcode::Max).u64().Inputs(0, 1);
            INST(20, Opcode::SaveState).NoVregs();
            INST(16, Opcode::CallStatic).b().InputsAutoType(2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 20);
            INST(17, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, VnTestCommutativeNotAppliedNoApplied)
{
    // We don't remove float commutative arithmetic instructions and not commutative instruction(sub)
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).f64();
        PARAMETER(1, 1).f64();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Add).f64().Inputs(0, 1);
            INST(5, Opcode::Mul).f64().Inputs(0, 1);
            INST(6, Opcode::Min).f64().Inputs(0, 1);
            INST(7, Opcode::Max).f64().Inputs(0, 1);
            INST(8, Opcode::Sub).u64().Inputs(2, 3);
            INST(9, Opcode::Add).f64().Inputs(1, 0);
            INST(10, Opcode::Mul).f64().Inputs(1, 0);
            INST(11, Opcode::Min).f64().Inputs(1, 0);
            INST(12, Opcode::Max).f64().Inputs(1, 0);
            INST(13, Opcode::Sub).u64().Inputs(3, 2);
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).b().InputsAutoType(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 20);
            INST(15, Opcode::ReturnVoid);
        }
    }
    Graph *graph_et = CreateGraphWithDefaultRuntime();
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).f64();
        PARAMETER(1, 1).f64();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Add).f64().Inputs(0, 1);
            INST(5, Opcode::Mul).f64().Inputs(0, 1);
            INST(6, Opcode::Min).f64().Inputs(0, 1);
            INST(7, Opcode::Max).f64().Inputs(0, 1);
            INST(8, Opcode::Sub).u64().Inputs(2, 3);
            INST(9, Opcode::Add).f64().Inputs(1, 0);
            INST(10, Opcode::Mul).f64().Inputs(1, 0);
            INST(11, Opcode::Min).f64().Inputs(1, 0);
            INST(12, Opcode::Max).f64().Inputs(1, 0);
            INST(13, Opcode::Sub).u64().Inputs(3, 2);
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).b().InputsAutoType(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 20);
            INST(15, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, VnTestIsInstance)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(12, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(2, Opcode::LoadClass).ref().Inputs(12).TypeId(1);
            INST(3, Opcode::IsInstance).b().Inputs(0, 2, 12).TypeId(1);
            INST(4, Opcode::LoadClass).ref().Inputs(12).TypeId(1);
            INST(5, Opcode::IsInstance).b().Inputs(0, 4, 12).TypeId(1);
            INST(6, Opcode::LoadClass).ref().Inputs(12).TypeId(1);
            INST(7, Opcode::IsInstance).b().Inputs(1, 6, 12).TypeId(1);
            INST(8, Opcode::LoadClass).ref().Inputs(12).TypeId(2);
            INST(9, Opcode::IsInstance).b().Inputs(0, 8, 12).TypeId(2);
            INST(20, Opcode::SaveState).NoVregs();
            INST(10, Opcode::CallStatic).b().InputsAutoType(3, 5, 7, 9, 20);
            INST(11, Opcode::ReturnVoid);
        }
    }
    Graph *graph_et = CreateGraphWithDefaultRuntime();
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(12, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(2, Opcode::LoadClass).ref().Inputs(12).TypeId(1);
            INST(3, Opcode::IsInstance).b().Inputs(0, 2, 12).TypeId(1);
            INST(7, Opcode::IsInstance).b().Inputs(1, 2, 12).TypeId(1);
            INST(8, Opcode::LoadClass).ref().Inputs(12).TypeId(2);
            INST(9, Opcode::IsInstance).b().Inputs(0, 8, 12).TypeId(2);
            INST(20, Opcode::SaveState).NoVregs();
            INST(10, Opcode::CallStatic).b().InputsAutoType(3, 3, 7, 9, 20);
            INST(11, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, VnTestInitDifferentClasses)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).NoVregs();
            INST(2, Opcode::InitClass).Inputs(1).TypeId(1);
            INST(3, Opcode::InitClass).Inputs(1).TypeId(2);
            INST(4, Opcode::ReturnVoid);
        }
    }

    auto clone_graph =
        GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone_graph));
}

TEST_F(VNTest, VnTestInitClass)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).NoVregs();
            INST(2, Opcode::InitClass).Inputs(1).TypeId(1);
            INST(3, Opcode::InitClass).Inputs(1).TypeId(1);
            INST(4, Opcode::ReturnVoid);
        }
    }

    Graph *graph_et = CreateGraphWithDefaultRuntime();
    GRAPH(graph_et)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).NoVregs();
            INST(2, Opcode::InitClass).Inputs(1).TypeId(1);
            INST(4, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, VnTestInitAfterLoad)
{
    // If InitClass instruction dominates LoadAndInitClass, replace it with LoadAndInitClass
    // and remove the old LoadAndInitClass
    // We cannot move outputs of LoadAndInitClass to LoadClass
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadClass).ref().Inputs(1).TypeId(1);
            INST(3, Opcode::IsInstance).b().Inputs(0, 2, 1).TypeId(1);
            INST(4, Opcode::InitClass).Inputs(1).TypeId(1);
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(1);
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(1);
            INST(7, Opcode::NewObject).ref().Inputs(6, 1).TypeId(1);
            INST(8, Opcode::Return).b().Inputs(3);
        }
    }
    Graph *graph_et = CreateGraphWithDefaultRuntime();
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadClass).ref().Inputs(1).TypeId(1);
            INST(3, Opcode::IsInstance).b().Inputs(0, 2, 1).TypeId(1);
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(1);
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(1);
            INST(7, Opcode::NewObject).ref().Inputs(4, 1).TypeId(1);
            INST(8, Opcode::Return).b().Inputs(3);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, VnTestLoadAfterInit)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadClass).ref().Inputs(1).TypeId(1);
            INST(3, Opcode::IsInstance).b().Inputs(0, 2, 1).TypeId(1);
            INST(4, Opcode::InitClass).Inputs(1).TypeId(1);
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(1);
            INST(6, Opcode::LoadClass).ref().Inputs(1).TypeId(1);
            INST(7, Opcode::CheckCast).b().Inputs(0, 6, 1).TypeId(1);
            INST(8, Opcode::Return).b().Inputs(3);
        }
    }
    Graph *graph_et = CreateGraphWithDefaultRuntime();
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadClass).ref().Inputs(1).TypeId(1);
            INST(3, Opcode::IsInstance).b().Inputs(0, 2, 1).TypeId(1);
            INST(4, Opcode::InitClass).Inputs(1).TypeId(1);
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(1);
            INST(7, Opcode::CheckCast).b().Inputs(0, 2, 1).TypeId(1);
            INST(8, Opcode::Return).b().Inputs(3);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, VnTestLoadAndInit)
{
    // If InitClass, LoadClass, or LoadAndInitClass is dominated by LoadAndInitClass
    // with the same TypeId, remove the dominated instruction
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1).TypeId(1);
            INST(4, Opcode::LoadClass).ref().Inputs(1).TypeId(1);
            INST(5, Opcode::CheckCast).b().Inputs(0, 4, 1).TypeId(1);
            INST(6, Opcode::InitClass).Inputs(1).TypeId(1);
            INST(7, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(1);
            INST(8, Opcode::NewObject).ref().Inputs(7, 1).TypeId(1);
            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graph_et = CreateGraphWithDefaultRuntime();
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1).TypeId(1);
            INST(5, Opcode::CheckCast).b().Inputs(0, 2, 1).TypeId(1);
            INST(8, Opcode::NewObject).ref().Inputs(2, 1).TypeId(1);
            INST(9, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(VNTest, Domination)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Mul).s64().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            // this inst does not dominate similar Add insts from bb 5
            INST(5, Opcode::Add).s64().Inputs(0, 1);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).s64().Inputs({{3, 4}, {4, 5}});
            // this inst dominates similar Add insts in this bb
            INST(7, Opcode::Add).s64().Inputs(0, 1);
            INST(8, Opcode::Mul).s64().Inputs(6, 7);

            INST(9, Opcode::Add).s64().Inputs(0, 1);
            INST(10, Opcode::Mul).s64().Inputs(8, 9);

            INST(11, Opcode::Add).s64().Inputs(0, 1);
            INST(12, Opcode::Mul).s64().Inputs(10, 11);

            INST(13, Opcode::Return).s64().Inputs(12);
        }
    }

    GraphChecker(GetGraph()).Check();
    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();

    Graph *graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Mul).s64().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            // this inst does not dominate similar Add insts from bb 5
            INST(5, Opcode::Add).s64().Inputs(0, 1);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).s64().Inputs({{3, 4}, {4, 5}});
            // this inst dominates similar Add insts in this bb
            INST(7, Opcode::Add).s64().Inputs(0, 1);
            INST(8, Opcode::Mul).s64().Inputs(6, 7);

            INST(10, Opcode::Mul).s64().Inputs(8, 7);

            INST(12, Opcode::Mul).s64().Inputs(10, 7);

            INST(13, Opcode::Return).s64().Inputs(12);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

}  // namespace panda::compiler
