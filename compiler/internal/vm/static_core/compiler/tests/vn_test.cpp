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

#include "optimizer/ir/datatype.h"
#include "tests/graph_comparator.h"
#include "unit_test.h"
#include "optimizer/optimizations/vn.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/graph_cloner.h"

namespace ark::compiler {
class VNTest : public AsmTest {  // NOLINT(fuchsia-multiple-inheritance)
public:
    VNTest() : graph_(CreateGraphWithDefaultRuntime()) {}

    Graph *GetGraph() override
    {
        return graph_;
    }

private:
    Graph *graph_;
};

// NOLINTBEGIN(readability-magic-numbers)
SRC_GRAPH(VnTestApply1, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f64();
        PARAMETER(3U, 3U).f64();
        PARAMETER(4U, 4U).f32();
        PARAMETER(5U, 5U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(7U, Opcode::Sub).u32().Inputs(1U, 0U);
            INST(8U, Opcode::Mul).f32().Inputs(4U, 5U);
            INST(9U, Opcode::Div).f64().Inputs(3U, 2U);
            INST(10U, Opcode::Sub).u32().Inputs(1U, 0U);
            INST(11U, Opcode::Div).f64().Inputs(3U, 2U);
            INST(12U, Opcode::Mul).f32().Inputs(4U, 5U);
            INST(13U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).b().InputsAutoType(6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U, 20U);
            INST(15U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(VnTestApply1, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f64();
        PARAMETER(3U, 3U).f64();
        PARAMETER(4U, 4U).f32();
        PARAMETER(5U, 5U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(7U, Opcode::Sub).u32().Inputs(1U, 0U);
            INST(8U, Opcode::Mul).f32().Inputs(4U, 5U);
            INST(9U, Opcode::Div).f64().Inputs(3U, 2U);
            INST(10U, Opcode::Sub).u32().Inputs(1U, 0U);
            INST(11U, Opcode::Div).f64().Inputs(3U, 2U);
            INST(12U, Opcode::Mul).f32().Inputs(4U, 5U);
            INST(13U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).b().InputsAutoType(6U, 7U, 8U, 9U, 7U, 9U, 8U, 6U, 20U);
            INST(15U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(VNTest, VnTestApply1)
{
    // Remove duplicate arithmetic instructions
    src_graph::VnTestApply1::CREATE(GetGraph());
    Graph *graphEt = CreateGraphWithDefaultRuntime();
    out_graph::VnTestApply1::CREATE(graphEt);

    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(VnTestApply2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f64();
        PARAMETER(3U, 3U).f64();
        PARAMETER(4U, 4U).f32();
        PARAMETER(5U, 5U).f32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(2U);
            INST(7U, Opcode::AddI).u32().Imm(10ULL).Inputs(1U);
            INST(8U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2U, 3U);
            INST(9U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(4U, 5U);
            INST(10U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(23U, Opcode::SaveState).NoVregs();
            INST(12U, Opcode::CallStatic).b().InputsAutoType(6U, 7U, 8U, 9U, 23U);
            INST(13U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(14U, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(2U);
            INST(15U, Opcode::AddI).u32().Imm(10ULL).Inputs(1U);
            INST(16U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2U, 3U);
            INST(17U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(4U, 5U);
            INST(18U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(18U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(24U, Opcode::SaveState).NoVregs();
            INST(20U, Opcode::CallStatic).b().InputsAutoType(17U, 16U, 15U, 14U, 24U);
            INST(21U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(22U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(VnTestApply2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f64();
        PARAMETER(3U, 3U).f64();
        PARAMETER(4U, 4U).f32();
        PARAMETER(5U, 5U).f32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(2U);
            INST(7U, Opcode::AddI).u32().Imm(10ULL).Inputs(1U);
            INST(8U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2U, 3U);
            INST(9U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(4U, 5U);
            INST(10U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(23U, Opcode::SaveState).NoVregs();
            INST(12U, Opcode::CallStatic).b().InputsAutoType(6U, 7U, 8U, 9U, 23U);
            INST(13U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(14U, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(2U);
            INST(15U, Opcode::AddI).u32().Imm(10ULL).Inputs(1U);
            INST(16U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2U, 3U);
            INST(17U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(4U, 5U);
            INST(18U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(24U, Opcode::SaveState).NoVregs();
            INST(20U, Opcode::CallStatic).b().InputsAutoType(9U, 8U, 7U, 6U, 24U);
            INST(21U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(22U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(VNTest, VnTestApply2)
{
    // Remove duplicate Cast,  AddI, Fcmp and Cmp instructions
    src_graph::VnTestApply2::CREATE(GetGraph());
    Graph *graphEt = CreateGraphWithDefaultRuntime();
    out_graph::VnTestApply2::CREATE(graphEt);

    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(VnTestBitfieldExtraction, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            for (unsigned index = 0; index <= 15; ++index) {
                INST(1U + index, Opcode::ExtractBitfield).u64().Inputs(0U);
            }
            INST(17U, Opcode::SaveState).NoVregs();
            INST(18U, Opcode::CallStatic)
                .b()
                .InputsAutoType(1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U, 16U, 17U);
            INST(19U, Opcode::ReturnVoid);
        }
        for (unsigned index = 0; index <= 15; ++index) {
            auto *bfx = INS(1U + index).CastToExtractBitfield();
            bfx->SetSourceBit(10U + (index & 4U));
            bfx->SetWidth(30U + (index & 2U));
            ((index & 1U) != 0) ? bfx->AsSignBitExtension() : bfx->AsZeroExtension();
        }
    }
}

OUT_GRAPH(VnTestBitfieldExtraction, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            for (unsigned index = 0; index <= 15; ++index) {
                INST(1U + index, Opcode::ExtractBitfield).u64().Inputs(0U);
            }
            INST(17U, Opcode::SaveState).NoVregs();
            INST(18U, Opcode::CallStatic)
                .b()
                .InputsAutoType(1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 17U);
            INST(19U, Opcode::ReturnVoid);
        }
        for (unsigned index = 0; index <= 15; ++index) {
            auto *bfx = INS(1U + index).CastToExtractBitfield();
            bfx->SetSourceBit(10U + (index & 4U));
            bfx->SetWidth(30U + (index & 2U));
            ((index & 1U) != 0) ? bfx->AsSignBitExtension() : bfx->AsZeroExtension();
        }
    }
}

TEST_F(VNTest, VnTestBitfieldExtraction)
{
    // Remove duplicate Cast,  AddI, Fcmp and Cmp instructions
    src_graph::VnTestBitfieldExtraction::CREATE(GetGraph());
    Graph *graphEt = CreateGraphWithDefaultRuntime();
    out_graph::VnTestBitfieldExtraction::CREATE(graphEt);

    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(VnTestNotApply1, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f64();
        PARAMETER(3U, 3U).f64();
        PARAMETER(4U, 4U).f32();
        PARAMETER(5U, 5U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(7U, Opcode::Add).u16().Inputs(0U, 1U);
            INST(8U, Opcode::Add).u32().Inputs(0U, 7U);
            INST(9U, Opcode::Div).f64().Inputs(2U, 3U);
            INST(10U, Opcode::Div).f64().Inputs(3U, 2U);
            INST(11U, Opcode::Mul).f64().Inputs(2U, 3U);
            INST(12U, Opcode::Sub).f32().Inputs(4U, 5U);
            INST(13U, Opcode::Sub).f32().Inputs(5U, 4U);
            INST(14U, Opcode::Abs).f32().Inputs(4U);
            INST(15U, Opcode::Neg).f32().Inputs(4U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(16U, Opcode::CallStatic).b().InputsAutoType(6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U, 20U);
            INST(17U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(VnTestNotApply1, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f64();
        PARAMETER(3U, 3U).f64();
        PARAMETER(4U, 4U).f32();
        PARAMETER(5U, 5U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(7U, Opcode::Add).u16().Inputs(0U, 1U);
            INST(8U, Opcode::Add).u32().Inputs(0U, 7U);
            INST(9U, Opcode::Div).f64().Inputs(2U, 3U);
            INST(10U, Opcode::Div).f64().Inputs(3U, 2U);
            INST(11U, Opcode::Mul).f64().Inputs(2U, 3U);
            INST(12U, Opcode::Sub).f32().Inputs(4U, 5U);
            INST(13U, Opcode::Sub).f32().Inputs(5U, 4U);
            INST(14U, Opcode::Abs).f32().Inputs(4U);
            INST(15U, Opcode::Neg).f32().Inputs(4U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(16U, Opcode::CallStatic).b().InputsAutoType(6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U, 20U);
            INST(17U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(VNTest, VnTestNotApply1)
{
    // Arithmetic instructions has different type, inputs, opcodes
    src_graph::VnTestNotApply1::CREATE(GetGraph());
    Graph *graphEt = CreateGraphWithDefaultRuntime();
    // graph_et is equal the graph
    out_graph::VnTestNotApply1::CREATE(graphEt);

    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(VnTestNotApply2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f64();
        PARAMETER(3U, 3U).f64();
        PARAMETER(4U, 4U).f32();
        PARAMETER(5U, 5U).f32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::Cast).u32().SrcType(DataType::FLOAT64).Inputs(2U);
            INST(7U, Opcode::AddI).u32().Imm(9ULL).Inputs(1U);
            INST(8U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(false).Inputs(2U, 3U);
            INST(9U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(true).Inputs(4U, 5U);
            INST(10U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 1U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(23U, Opcode::SaveState).NoVregs();
            INST(12U, Opcode::CallStatic).b().InputsAutoType(6U, 7U, 8U, 9U, 23U);
            INST(13U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(14U, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(2U);
            INST(15U, Opcode::AddI).u32().Imm(10ULL).Inputs(1U);
            INST(16U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2U, 3U);
            INST(17U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(4U, 5U);
            INST(18U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(18U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(24U, Opcode::SaveState).NoVregs();
            INST(20U, Opcode::CallStatic).b().InputsAutoType(17U, 16U, 15U, 14U, 24U);
            INST(21U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(22U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(VnTestNotApply2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f64();
        PARAMETER(3U, 3U).f64();
        PARAMETER(4U, 4U).f32();
        PARAMETER(5U, 5U).f32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::Cast).u32().SrcType(DataType::FLOAT64).Inputs(2U);
            INST(7U, Opcode::AddI).u32().Imm(9ULL).Inputs(1U);
            INST(8U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(false).Inputs(2U, 3U);
            INST(9U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(true).Inputs(4U, 5U);
            INST(10U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 1U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(23U, Opcode::SaveState).NoVregs();
            INST(12U, Opcode::CallStatic).b().InputsAutoType(6U, 7U, 8U, 9U, 23U);
            INST(13U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(14U, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(2U);
            INST(15U, Opcode::AddI).u32().Imm(10ULL).Inputs(1U);
            INST(16U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT64).Fcmpg(true).Inputs(2U, 3U);
            INST(17U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(false).Inputs(4U, 5U);
            INST(18U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(18U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(24U, Opcode::SaveState).NoVregs();
            INST(20U, Opcode::CallStatic).b().InputsAutoType(17U, 16U, 15U, 14U, 24U);
            INST(21U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(22U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(VNTest, VnTestNotApply2)
{
    // Can't applies:
    //  - Cast instructions have different types
    //  - AddI instructions have different constant
    //  - Fcmp instructions have different fcmpg flag
    //  - Cmp instructions have different CC
    src_graph::VnTestNotApply2::CREATE(GetGraph());
    Graph *graphEt = CreateGraphWithDefaultRuntime();
    // graph_et is equal the graph
    out_graph::VnTestNotApply2::CREATE(graphEt);

    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(VnTestNotApply3, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f64();
        PARAMETER(3U, 3U).f64();
        PARAMETER(4U, 4U).ref();
        PARAMETER(5U, 5U).ref();
        PARAMETER(6U, 6U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(7U, Opcode::LoadArray).u64().Inputs(5U, 6U);
            INST(8U, Opcode::LoadArray).u64().Inputs(5U, 6U);
            INST(9U, Opcode::StoreArray).f64().Inputs(4U, 6U, 3U);
            INST(10U, Opcode::StoreArray).f64().Inputs(4U, 6U, 3U);
            INST(11U, Opcode::Compare).b().CC(CC_LT).Inputs(7U, 8U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(13U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(14U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(15U, Opcode::Mul).f64().Inputs(2U, 3U);
            INST(16U, Opcode::Div).f64().Inputs(2U, 3U);
            INST(30U, Opcode::SaveState).NoVregs();
            INST(17U, Opcode::CallStatic).b().InputsAutoType(13U, 14U, 15U, 16U, 30U);
            INST(18U, Opcode::CallStatic).b().InputsAutoType(13U, 14U, 15U, 16U, 30U);
            INST(19U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(21U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(22U, Opcode::Mul).f64().Inputs(2U, 3U);
            INST(23U, Opcode::Div).f64().Inputs(2U, 3U);
            INST(31U, Opcode::SaveState).NoVregs();
            INST(24U, Opcode::CallStatic).b().InputsAutoType(20U, 21U, 22U, 23U, 31U);
            INST(25U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(VnTestNotApply3, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f64();
        PARAMETER(3U, 3U).f64();
        PARAMETER(4U, 4U).ref();
        PARAMETER(5U, 5U).ref();
        PARAMETER(6U, 6U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(7U, Opcode::LoadArray).u64().Inputs(5U, 6U);
            INST(8U, Opcode::LoadArray).u64().Inputs(5U, 6U);
            INST(9U, Opcode::StoreArray).f64().Inputs(4U, 6U, 3U);
            INST(10U, Opcode::StoreArray).f64().Inputs(4U, 6U, 3U);
            INST(11U, Opcode::Compare).b().CC(CC_LT).Inputs(7U, 8U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(13U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(14U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(15U, Opcode::Mul).f64().Inputs(2U, 3U);
            INST(16U, Opcode::Div).f64().Inputs(2U, 3U);
            INST(30U, Opcode::SaveState).NoVregs();
            INST(17U, Opcode::CallStatic).b().InputsAutoType(13U, 14U, 15U, 16U, 30U);
            INST(18U, Opcode::CallStatic).b().InputsAutoType(13U, 14U, 15U, 16U, 30U);
            INST(19U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(21U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(22U, Opcode::Mul).f64().Inputs(2U, 3U);
            INST(23U, Opcode::Div).f64().Inputs(2U, 3U);
            INST(31U, Opcode::SaveState).NoVregs();
            INST(24U, Opcode::CallStatic).b().InputsAutoType(20U, 21U, 22U, 23U, 31U);
            INST(25U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(VNTest, VnTestNotApply3)
{
    // Can't applies:
    //  - Arithmetic instructions aren't dominate
    //  - CallStatic, LoadArray and StoreArray has NO_CSE
    src_graph::VnTestNotApply3::CREATE(GetGraph());

    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(13U).GetVN(), INS(20U).GetVN());
    ASSERT_EQ(INS(14U).GetVN(), INS(21U).GetVN());
    ASSERT_EQ(INS(15U).GetVN(), INS(22U).GetVN());
    ASSERT_EQ(INS(16U).GetVN(), INS(23U).GetVN());

    Graph *graphEt = CreateGraphWithDefaultRuntime();
    // graph_et is equal the graph
    out_graph::VnTestNotApply3::CREATE(graphEt);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(VnTestApply3, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(7U, Opcode::Phi).s32().Inputs(1U, 20U);
            INST(11U, Opcode::SafePoint).Inputs(0U, 5U, 7U).SrcVregs({0U, 1U, 2U});
            INST(12U, Opcode::Compare).CC(CC_EQ).b().Inputs(7U, 5U);  // i < X
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(12U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(15U, Opcode::SaveState).Inputs(0U, 1U, 2U, 5U, 7U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(16U, Opcode::NullCheck).ref().Inputs(0U, 15U);
            INST(17U, Opcode::LenArray).s32().Inputs(16U);
            INST(18U, Opcode::BoundsCheck).s32().Inputs(17U, 7U, 15U);
            INST(19U, Opcode::StoreArray).s64().Inputs(16U, 18U, 1U);
            INST(20U, Opcode::Add).s32().Inputs(7U, 2U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(23U, Opcode::Return).ref().Inputs(0U);
        }
    }
}

OUT_GRAPH(VnTestApply3, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(7U, Opcode::Phi).s32().Inputs(1U, 20U);
            INST(11U, Opcode::SafePoint).Inputs(0U, 5U, 7U).SrcVregs({0U, 1U, 2U});
            INST(12U, Opcode::Compare).CC(CC_EQ).b().Inputs(7U, 5U);  // i < X
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(12U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(15U, Opcode::SaveState).Inputs(0U, 1U, 2U, 5U, 7U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(16U, Opcode::NullCheck).ref().Inputs(0U, 15U);
            INST(17U, Opcode::LenArray).s32().Inputs(16U);
            INST(18U, Opcode::BoundsCheck).s32().Inputs(5U, 7U, 15U);
            INST(19U, Opcode::StoreArray).s64().Inputs(16U, 18U, 1U);
            INST(20U, Opcode::Add).s32().Inputs(7U, 2U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(23U, Opcode::Return).ref().Inputs(0U);
        }
    }
}

TEST_F(VNTest, VnTestApply3)
{
    src_graph::VnTestApply3::CREATE(GetGraph());
    GetGraph()->RunPass<ValNum>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(5U).GetVN(), INS(17U).GetVN());

    Graph *graphEt = CreateGraphWithDefaultRuntime();
    // graph_et is equal the graph
    out_graph::VnTestApply3::CREATE(graphEt);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

TEST_F(VNTest, CleanupTrigger)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(6U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(7U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{2U, 6U}, {3U, 5U}});
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }

    GraphChecker(GetGraph()).Check();
    GetGraph()->RunPass<ValNum>();

    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(6U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(7U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{2U, 5U}, {3U, 5U}});
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(OSR, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Compare).b().Inputs(0U, 2U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 6U}});
            INST(6U, Opcode::Sub).u64().Inputs(5U, 1U);
            INST(7U, Opcode::Compare).b().Inputs(6U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 6U}});
            INST(10U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(11U, Opcode::Add).u64().Inputs(9U, 10U);
            INST(12U, Opcode::Return).u64().Inputs(11U);
        }
    }
}

OUT_GRAPH(OSR, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Compare).b().Inputs(0U, 2U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 6U}});
            INST(6U, Opcode::Sub).u64().Inputs(5U, 1U);
            INST(7U, Opcode::Compare).b().Inputs(6U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 6U}});
            INST(11U, Opcode::Add).u64().Inputs(9U, 2U);
            INST(12U, Opcode::Return).u64().Inputs(11U);
        }
    }
}

TEST_F(VNTest, OSR)
{
    src_graph::OSR::CREATE(GetGraph());

    // Remove Inst 11 without OSR

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();

    auto graph = CreateGraphWithDefaultRuntime();
    out_graph::OSR::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));

    auto graphOsr = CreateGraphOsrWithDefaultRuntime();

    GRAPH(graphOsr)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Compare).b().Inputs(0U, 2U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 6U}});
            INST(13U, Opcode::SaveStateOsr).Inputs(0U, 1U, 5U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::Sub).u64().Inputs(5U, 1U);
            INST(7U, Opcode::Compare).b().Inputs(6U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 6U}});
            INST(10U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(11U, Opcode::Add).u64().Inputs(9U, 10U);
            INST(12U, Opcode::Return).u64().Inputs(11U);
        }
    }

    for (auto bb : graphOsr->GetBlocksRPO()) {
        if (bb->IsLoopHeader()) {
            bb->SetOsrEntry(true);
        }
    }

    auto cloneOsr = GraphCloner(graphOsr, graphOsr->GetAllocator(), graphOsr->GetLocalAllocator()).CloneGraph();

    // Don't remove Inst 11 with OSR

    graphOsr->RunPass<ValNum>();
    graphOsr->RunPass<Cleanup>();

    ASSERT_TRUE(GraphComparator().Compare(cloneOsr, graphOsr));
}

SRC_GRAPH(VnTestCommutative, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Mul).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Xor).u64().Inputs(0U, 1U);
            INST(5U, Opcode::Or).u64().Inputs(0U, 1U);
            INST(6U, Opcode::And).u64().Inputs(0U, 1U);
            INST(7U, Opcode::Min).u64().Inputs(0U, 1U);
            INST(8U, Opcode::Max).u64().Inputs(0U, 1U);
            INST(9U, Opcode::Add).u64().Inputs(1U, 0U);
            INST(10U, Opcode::Mul).u64().Inputs(1U, 0U);
            INST(11U, Opcode::Xor).u64().Inputs(1U, 0U);
            INST(12U, Opcode::Or).u64().Inputs(1U, 0U);
            INST(13U, Opcode::And).u64().Inputs(1U, 0U);
            INST(14U, Opcode::Min).u64().Inputs(1U, 0U);
            INST(15U, Opcode::Max).u64().Inputs(1U, 0U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(16U, Opcode::CallStatic)
                .b()
                .InputsAutoType(2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U, 20U);
            INST(17U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(VnTestCommutative, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Mul).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Xor).u64().Inputs(0U, 1U);
            INST(5U, Opcode::Or).u64().Inputs(0U, 1U);
            INST(6U, Opcode::And).u64().Inputs(0U, 1U);
            INST(7U, Opcode::Min).u64().Inputs(0U, 1U);
            INST(8U, Opcode::Max).u64().Inputs(0U, 1U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(16U, Opcode::CallStatic)
                .b()
                .InputsAutoType(2U, 3U, 4U, 5U, 6U, 7U, 8U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 20U);
            INST(17U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(VNTest, VnTestCommutative)
{
    // Remove commutative arithmetic instructions(See 2516)
    src_graph::VnTestCommutative::CREATE(GetGraph());
    Graph *graphEt = CreateGraphWithDefaultRuntime();
    out_graph::VnTestCommutative::CREATE(graphEt);

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(VnTestCommutativeNotAppliedNoApplied, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();
        PARAMETER(1U, 1U).f64();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Add).f64().Inputs(0U, 1U);
            INST(5U, Opcode::Mul).f64().Inputs(0U, 1U);
            INST(6U, Opcode::Min).f64().Inputs(0U, 1U);
            INST(7U, Opcode::Max).f64().Inputs(0U, 1U);
            INST(8U, Opcode::Sub).u64().Inputs(2U, 3U);
            INST(9U, Opcode::Add).f64().Inputs(1U, 0U);
            INST(10U, Opcode::Mul).f64().Inputs(1U, 0U);
            INST(11U, Opcode::Min).f64().Inputs(1U, 0U);
            INST(12U, Opcode::Max).f64().Inputs(1U, 0U);
            INST(13U, Opcode::Sub).u64().Inputs(3U, 2U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).b().InputsAutoType(2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U, 20U);
            INST(15U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(VnTestCommutativeNotAppliedNoApplied, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();
        PARAMETER(1U, 1U).f64();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Add).f64().Inputs(0U, 1U);
            INST(5U, Opcode::Mul).f64().Inputs(0U, 1U);
            INST(6U, Opcode::Min).f64().Inputs(0U, 1U);
            INST(7U, Opcode::Max).f64().Inputs(0U, 1U);
            INST(8U, Opcode::Sub).u64().Inputs(2U, 3U);
            INST(9U, Opcode::Add).f64().Inputs(1U, 0U);
            INST(10U, Opcode::Mul).f64().Inputs(1U, 0U);
            INST(11U, Opcode::Min).f64().Inputs(1U, 0U);
            INST(12U, Opcode::Max).f64().Inputs(1U, 0U);
            INST(13U, Opcode::Sub).u64().Inputs(3U, 2U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).b().InputsAutoType(2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U, 20U);
            INST(15U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(VNTest, VnTestCommutativeNotAppliedNoApplied)
{
    // We don't remove float commutative arithmetic instructions and not commutative instruction(sub)
    src_graph::VnTestCommutativeNotAppliedNoApplied::CREATE(GetGraph());
    Graph *graphEt = CreateGraphWithDefaultRuntime();
    out_graph::VnTestCommutativeNotAppliedNoApplied::CREATE(graphEt);

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

TEST_F(VNTest, VnTestIsInstance)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(12U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(2U, Opcode::LoadClass).ref().Inputs(12U).TypeId(1U);
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 2U, 12U).TypeId(1U);
            // Check that equal IsInstance with different SaveState is removed
            INST(13U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(4U, Opcode::LoadClass).ref().Inputs(13U).TypeId(1U);
            INST(5U, Opcode::IsInstance).b().Inputs(0U, 4U, 13U).TypeId(1U);
            INST(6U, Opcode::LoadClass).ref().Inputs(13U).TypeId(1U);
            INST(7U, Opcode::IsInstance).b().Inputs(1U, 6U, 13U).TypeId(1U);
            INST(8U, Opcode::LoadClass).ref().Inputs(13U).TypeId(2U);
            INST(9U, Opcode::IsInstance).b().Inputs(0U, 8U, 13U).TypeId(2U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(10U, Opcode::CallStatic).b().InputsAutoType(3U, 5U, 7U, 9U, 20U);
            INST(11U, Opcode::ReturnVoid);
        }
    }
    Graph *graphEt = CreateGraphWithDefaultRuntime();
    GRAPH(graphEt)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(12U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(2U, Opcode::LoadClass).ref().Inputs(12U).TypeId(1U);
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 2U, 12U).TypeId(1U);
            INST(13U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(7U, Opcode::IsInstance).b().Inputs(1U, 2U, 13U).TypeId(1U);
            INST(8U, Opcode::LoadClass).ref().Inputs(13U).TypeId(2U);
            INST(9U, Opcode::IsInstance).b().Inputs(0U, 8U, 13U).TypeId(2U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(10U, Opcode::CallStatic).b().InputsAutoType(3U, 3U, 7U, 9U, 20U);
            INST(11U, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

TEST_F(VNTest, VnTestInitDifferentClasses)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::InitClass).Inputs(1U).TypeId(1U);
            INST(3U, Opcode::InitClass).Inputs(1U).TypeId(2U);
            INST(4U, Opcode::ReturnVoid);
        }
    }

    auto cloneGraph = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), cloneGraph));
}

TEST_F(VNTest, VnTestInitClass)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::InitClass).Inputs(1U).TypeId(1U);
            INST(3U, Opcode::InitClass).Inputs(1U).TypeId(1U);
            INST(4U, Opcode::ReturnVoid);
        }
    }

    Graph *graphEt = CreateGraphWithDefaultRuntime();
    GRAPH(graphEt)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::InitClass).Inputs(1U).TypeId(1U);
            INST(4U, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

TEST_F(VNTest, VnTestInitAfterLoad)
{
    // If InitClass instruction dominates LoadAndInitClass, replace it with LoadAndInitClass
    // and remove the old LoadAndInitClass
    // We cannot move outputs of LoadAndInitClass to LoadClass
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 2U, 1U).TypeId(1U);
            INST(4U, Opcode::InitClass).Inputs(1U).TypeId(1U);
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(1U);
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(1U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 1U).TypeId(1U);
            INST(8U, Opcode::Return).b().Inputs(3U);
        }
    }
    Graph *graphEt = CreateGraphWithDefaultRuntime();
    GRAPH(graphEt)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 2U, 1U).TypeId(1U);
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(1U);
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(1U);
            INST(7U, Opcode::NewObject).ref().Inputs(4U, 1U).TypeId(1U);
            INST(8U, Opcode::Return).b().Inputs(3U);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

TEST_F(VNTest, VnTestLoadAfterInit)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 2U, 1U).TypeId(1U);
            INST(4U, Opcode::InitClass).Inputs(1U).TypeId(1U);
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(1U);
            INST(6U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(7U, Opcode::CheckCast).Inputs(0U, 6U, 1U).TypeId(1U);
            INST(8U, Opcode::Return).b().Inputs(3U);
        }
    }
    Graph *graphEt = CreateGraphWithDefaultRuntime();
    GRAPH(graphEt)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 2U, 1U).TypeId(1U);
            INST(4U, Opcode::InitClass).Inputs(1U).TypeId(1U);
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(1U);
            INST(7U, Opcode::CheckCast).Inputs(0U, 2U, 1U).TypeId(1U);
            INST(8U, Opcode::Return).b().Inputs(3U);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

TEST_F(VNTest, VnTestLoadAndInit)
{
    // If InitClass, LoadClass, or LoadAndInitClass is dominated by LoadAndInitClass
    // with the same TypeId, remove the dominated instruction
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U).TypeId(1U);
            INST(4U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(5U, Opcode::CheckCast).Inputs(0U, 4U, 1U).TypeId(1U);
            INST(6U, Opcode::InitClass).Inputs(1U).TypeId(1U);
            INST(7U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(1U);
            INST(8U, Opcode::NewObject).ref().Inputs(7U, 1U).TypeId(1U);
            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graphEt = CreateGraphWithDefaultRuntime();
    GRAPH(graphEt)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U).TypeId(1U);
            INST(5U, Opcode::CheckCast).Inputs(0U, 2U, 1U).TypeId(1U);
            INST(8U, Opcode::NewObject).ref().Inputs(2U, 1U).TypeId(1U);
            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(Domination, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Mul).s64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            // this inst does not dominate similar Add insts from bb 5
            INST(5U, Opcode::Add).s64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).s64().Inputs({{3U, 4U}, {4U, 5U}});
            // this inst dominates similar Add insts in this bb
            INST(7U, Opcode::Add).s64().Inputs(0U, 1U);
            INST(8U, Opcode::Mul).s64().Inputs(6U, 7U);

            INST(9U, Opcode::Add).s64().Inputs(0U, 1U);
            INST(10U, Opcode::Mul).s64().Inputs(8U, 9U);

            INST(11U, Opcode::Add).s64().Inputs(0U, 1U);
            INST(12U, Opcode::Mul).s64().Inputs(10U, 11U);

            INST(13U, Opcode::Return).s64().Inputs(12U);
        }
    }
}

OUT_GRAPH(Domination, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Mul).s64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            // this inst does not dominate similar Add insts from bb 5
            INST(5U, Opcode::Add).s64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).s64().Inputs({{3U, 4U}, {4U, 5U}});
            // this inst dominates similar Add insts in this bb
            INST(7U, Opcode::Add).s64().Inputs(0U, 1U);
            INST(8U, Opcode::Mul).s64().Inputs(6U, 7U);

            INST(10U, Opcode::Mul).s64().Inputs(8U, 7U);

            INST(12U, Opcode::Mul).s64().Inputs(10U, 7U);

            INST(13U, Opcode::Return).s64().Inputs(12U);
        }
    }
}

TEST_F(VNTest, Domination)
{
    src_graph::Domination::CREATE(GetGraph());

    GraphChecker(GetGraph()).Check();
    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();

    Graph *graph = CreateGraphWithDefaultRuntime();
    out_graph::Domination::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(VNTest, BridgeCreator)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Intrinsic)
                .ref()
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE)
                .ClearFlag(compiler::inst_flags::NO_CSE);
            INST(2U, Opcode::SaveState).Inputs(1U).SrcVregs({1U});
            INST(3U, Opcode::CallStatic).v0id().InputsAutoType(1U, 2U);
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::Intrinsic)
                .ref()
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE)
                .ClearFlag(compiler::inst_flags::NO_CSE);
            INST(6U, Opcode::SaveState).Inputs(5U).SrcVregs({2U});
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(5U, 6U);
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }

    auto graphAfter = CreateGraphWithDefaultRuntime();
    GRAPH(graphAfter)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Intrinsic)
                .ref()
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE)
                .ClearFlag(compiler::inst_flags::NO_CSE);
            INST(2U, Opcode::SaveState).Inputs(1U).SrcVregs({1U});
            INST(3U, Opcode::CallStatic).v0id().InputsAutoType(1U, 2U);
            INST(4U, Opcode::SaveState).Inputs(1U).SrcVregs({VirtualRegister::BRIDGE});
            INST(5U, Opcode::Intrinsic)
                .ref()
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE)
                .ClearFlag(compiler::inst_flags::NO_CSE)
                .ClearFlag(compiler::inst_flags::NO_DCE);
            INST(6U, Opcode::SaveState).Inputs(1U).SrcVregs({2U});
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(1U, 6U);
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ValNum>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphAfter));
}

// NOTE(schernykh): ????
TEST_F(VNTest, DISABLED_BridgeCreatorExceptionInstNotApplied)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(1U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(6U, Opcode::LoadClass).ref().Inputs(1U);
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::LoadClass).ref().Inputs(1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(10U, Opcode::InitClass).Inputs(1U).TypeId(1U);
            INST(11U, Opcode::SaveState).NoVregs();
            INST(12U, Opcode::InitClass).Inputs(1U).TypeId(1U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(13U, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(14U, Opcode::GetInstanceClass).ref().Inputs(13U);
            INST(15U, Opcode::SaveState).NoVregs();
            INST(16U, Opcode::GetInstanceClass).ref().Inputs(13U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(17U, Opcode::ReturnVoid).v0id();
        }
    }
    auto cloneGraph = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();

    ASSERT_TRUE(GetGraph()->RunPass<ValNum>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), cloneGraph));
}

SRC_GRAPH(VNMethodResolver, Graph *graph)
{
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(1U).TypeId(0U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::ResolveVirtual).ptr().Inputs(5U, 4U);
            INST(7U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 6U}, {DataType::REFERENCE, 5U}, {DataType::NO_TYPE, 4U}});
            INST(8U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::NullCheck).ref().Inputs(3U, 8U);
            INST(10U, Opcode::ResolveVirtual).ptr().Inputs(9U, 8U);
            INST(11U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 10U}, {DataType::REFERENCE, 9U}, {DataType::NO_TYPE, 8U}});
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(VNMethodResolver, Graph *graph)
{
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(1U).TypeId(0U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::ResolveVirtual).ptr().Inputs(5U, 4U);
            INST(7U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 6U}, {DataType::REFERENCE, 5U}, {DataType::NO_TYPE, 4U}});
            INST(8U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::NullCheck).ref().Inputs(3U, 8U);
            INST(11U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 6U}, {DataType::REFERENCE, 9U}, {DataType::NO_TYPE, 8U}});
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(VNTest, VNMethodResolver)
{
    src_graph::VNMethodResolver::CREATE(GetGraph());

    ASSERT_TRUE(GetGraph()->RunPass<ValNum>());
    GetGraph()->RunPass<Cleanup>();

    auto graph = CreateGraphWithDefaultRuntime();
    out_graph::VNMethodResolver::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(VNTest, DontVNMethodResolverThroughTry)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(1U).TypeId(0U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::ResolveVirtual).ptr().Inputs(5U, 4U);
            INST(7U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 6U}, {DataType::REFERENCE, 5U}, {DataType::NO_TYPE, 4U}});
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(8U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::NullCheck).ref().Inputs(3U, 8U);
            INST(10U, Opcode::ResolveVirtual).ptr().Inputs(9U, 8U);
            INST(11U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 10U}, {DataType::REFERENCE, 9U}, {DataType::NO_TYPE, 8U}});
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }

    BB(3U).SetTry(true);
    auto graph = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ValNum>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(VNFieldResolver, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(22U, 0x1U);
        CONSTANT(23U, 0x2U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(1U).TypeId(0U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::ResolveStatic).ptr().Inputs(4U);
            INST(7U, Opcode::CallResolvedStatic)
                .v0id()
                .Inputs({{DataType::POINTER, 6U}, {DataType::REFERENCE, 5U}, {DataType::NO_TYPE, 4U}});
            INST(8U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::ResolveObjectFieldStatic).ref().Inputs(8U).TypeId(123U);
            INST(10U, Opcode::StoreResolvedObjectFieldStatic).i32().Inputs(9U, 22U).TypeId(123U);
            INST(11U, Opcode::SaveState).NoVregs();
            INST(12U, Opcode::NullCheck).ref().Inputs(3U, 11U);
            INST(13U, Opcode::ResolveVirtual).ptr().Inputs(12U, 11U);
            INST(14U, Opcode::CallResolvedStatic)
                .v0id()
                .Inputs({{DataType::POINTER, 13U}, {DataType::REFERENCE, 12U}, {DataType::NO_TYPE, 11U}});
            INST(15U, Opcode::SaveState).NoVregs();
            INST(16U, Opcode::ResolveObjectFieldStatic).ref().Inputs(15U).TypeId(123U);
            INST(17U, Opcode::StoreResolvedObjectFieldStatic).i32().Inputs(16U, 23U).TypeId(123U);
            INST(18U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(VNFieldResolver, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(22U, 0x1U);
        CONSTANT(23U, 0x2U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(1U).TypeId(0U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::ResolveStatic).ptr().Inputs(4U);
            INST(7U, Opcode::CallResolvedStatic)
                .v0id()
                .Inputs({{DataType::POINTER, 6U}, {DataType::REFERENCE, 5U}, {DataType::NO_TYPE, 4U}});
            INST(8U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::ResolveObjectFieldStatic).ref().Inputs(8U).TypeId(123U);
            INST(10U, Opcode::StoreResolvedObjectFieldStatic).i32().Inputs(9U, 22U).TypeId(123U);
            INST(11U, Opcode::SaveState).NoVregs();
            INST(12U, Opcode::NullCheck).ref().Inputs(3U, 11U);
            INST(13U, Opcode::ResolveVirtual).ptr().Inputs(12U, 11U);
            INST(14U, Opcode::CallResolvedStatic)
                .v0id()
                .Inputs({{DataType::POINTER, 13U}, {DataType::REFERENCE, 12U}, {DataType::NO_TYPE, 11U}});
            INST(17U, Opcode::StoreResolvedObjectFieldStatic).i32().Inputs(9U, 23U).TypeId(123U);
            INST(18U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(VNTest, VNFieldResolver)
{
    src_graph::VNFieldResolver::CREATE(GetGraph());

    ASSERT_TRUE(GetGraph()->RunPass<ValNum>());
    GetGraph()->RunPass<Cleanup>();

    auto graph = CreateGraphWithDefaultRuntime();
    out_graph::VNFieldResolver::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(VNTest, DontVNFieldResolverIfTypeIdsDiffer)
{
    GRAPH(GetGraph())
    {
        CONSTANT(22U, 0x1U);
        CONSTANT(23U, 0x2U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(1U).TypeId(0U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::ResolveStatic).ptr().Inputs(4U);
            INST(7U, Opcode::CallResolvedStatic)
                .v0id()
                .Inputs({{DataType::POINTER, 6U}, {DataType::REFERENCE, 5U}, {DataType::NO_TYPE, 4U}});
            INST(8U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::ResolveObjectFieldStatic).ref().Inputs(8U).TypeId(123U);
            INST(10U, Opcode::StoreResolvedObjectFieldStatic).i32().Inputs(9U, 22U).TypeId(123U);
            INST(11U, Opcode::SaveState).NoVregs();
            INST(12U, Opcode::NullCheck).ref().Inputs(3U, 11U);
            INST(13U, Opcode::ResolveVirtual).ptr().Inputs(12U, 11U);
            INST(14U, Opcode::CallResolvedStatic)
                .v0id()
                .Inputs({{DataType::POINTER, 13U}, {DataType::REFERENCE, 12U}, {DataType::NO_TYPE, 11U}});
            INST(15U, Opcode::SaveState).NoVregs();
            INST(16U, Opcode::ResolveObjectFieldStatic).ref().Inputs(15U).TypeId(321U);
            INST(17U, Opcode::StoreResolvedObjectFieldStatic).i32().Inputs(16U, 23U).TypeId(321U);
            INST(18U, Opcode::ReturnVoid).v0id();
        }
    }

    auto graph = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ValNum>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(VNTest, LoadImmediateCseApplied)
{
    auto class1 = reinterpret_cast<RuntimeInterface::ClassPtr>(1U);
    auto graph1 = CreateGraphWithDefaultRuntime();
    GRAPH(graph1)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadImmediate).ref().Class(class1);
            INST(2U, Opcode::LoadImmediate).ref().Class(class1);
            INST(3U, Opcode::ReturnVoid);
        }
    }
    GraphChecker(graph1).Check();
    ASSERT_TRUE(graph1->RunPass<ValNum>());

    auto graph2 = CreateGraphWithDefaultRuntime();
    GRAPH(graph2)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadImmediate).ref().Class(class1);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).b().InputsAutoType(1U, 20U);
            INST(2U, Opcode::LoadImmediate).ref().Class(class1);
            INST(3U, Opcode::Return).ref().Inputs(2U);
        }
    }
    GraphChecker(graph2).Check();
    ASSERT_TRUE(graph2->RunPass<ValNum>());
    ASSERT_TRUE(graph2->RunPass<Cleanup>());
    auto graph3 = CreateGraphWithDefaultRuntime();
    GRAPH(graph3)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadImmediate).ref().Class(class1);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).b().InputsAutoType(1U, 20U);
            INST(3U, Opcode::Return).ref().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph2, graph3));
}

TEST_F(VNTest, LoadImmediateCseNotApplied)
{
    auto class1 = reinterpret_cast<RuntimeInterface::ClassPtr>(1U);
    auto class2 = reinterpret_cast<RuntimeInterface::ClassPtr>(2U);
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadImmediate).ref().Class(class1);
            INST(2U, Opcode::LoadImmediate).ref().Class(class2);
            INST(3U, Opcode::ReturnVoid);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    GraphChecker(GetGraph()).Check();
    ASSERT_FALSE(GetGraph()->RunPass<ValNum>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(VNTest, LoadObjFromConstCseApplied)
{
    auto obj = static_cast<uintptr_t>(1U);
    auto graph1 = CreateGraphWithDefaultRuntime();
    GRAPH(graph1)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadObjFromConst).ref().SetPtr(obj);
            INST(2U, Opcode::LoadObjFromConst).ref().SetPtr(obj);
            INST(3U, Opcode::ReturnVoid);
        }
    }
    GraphChecker(graph1).Check();
    ASSERT_TRUE(graph1->RunPass<ValNum>());

    auto graph2 = CreateGraphWithDefaultRuntime();
    GRAPH(graph2)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadObjFromConst).ref().SetPtr(obj);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).b().InputsAutoType(1U, 20U);
            INST(2U, Opcode::LoadObjFromConst).ref().SetPtr(obj);
            INST(3U, Opcode::Return).ref().Inputs(2U);
        }
    }
    GraphChecker(graph2).Check();
    ASSERT_TRUE(graph2->RunPass<ValNum>());
    ASSERT_TRUE(graph2->RunPass<Cleanup>());
    auto graph3 = CreateGraphWithDefaultRuntime();
    GRAPH(graph3)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadObjFromConst).ref().SetPtr(obj);
            INST(20U, Opcode::SaveState).Inputs(1U).SrcVregs({VirtualRegister::BRIDGE});
            INST(14U, Opcode::CallStatic).b().InputsAutoType(1U, 20U);
            INST(3U, Opcode::Return).ref().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph2, graph3));
}

SRC_GRAPH(FunctionImmediateCseApplied, Graph *graph)
{
    auto fptr = static_cast<uintptr_t>(1U);
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::FunctionImmediate).any().SetPtr(fptr);
            INST(2U, Opcode::FunctionImmediate).any().SetPtr(fptr);
            INST(3U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(FunctionImmediateCseApplied, Graph *graph)
{
    auto fptr = static_cast<uintptr_t>(1U);
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::FunctionImmediate).any().SetPtr(fptr);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).b().InputsAutoType(1U, 20U);
            INST(2U, Opcode::FunctionImmediate).any().SetPtr(fptr);
            INST(3U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(VNTest, FunctionImmediateCseApplied)
{
    auto graph1 = CreateGraphWithDefaultRuntime();
    graph1->SetDynamicMethod();
#ifndef NDEBUG
    graph1->SetDynUnitTestFlag();
#endif
    src_graph::FunctionImmediateCseApplied::CREATE(graph1);
    GraphChecker(graph1).Check();
    ASSERT_TRUE(graph1->RunPass<ValNum>());
    auto graph2 = CreateGraphWithDefaultRuntime();
    graph2->SetDynamicMethod();
#ifndef NDEBUG
    graph2->SetDynUnitTestFlag();
#endif
    out_graph::FunctionImmediateCseApplied::CREATE(graph2);
    GraphChecker(graph2).Check();
    ASSERT_TRUE(graph2->RunPass<ValNum>());
    ASSERT_TRUE(graph2->RunPass<Cleanup>());
    auto graph3 = CreateGraphWithDefaultRuntime();
    graph3->SetDynamicMethod();
#ifndef NDEBUG
    graph3->SetDynUnitTestFlag();
#endif
    auto fptr = static_cast<uintptr_t>(1U);
    GRAPH(graph3)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::FunctionImmediate).any().SetPtr(fptr);
            INST(20U, Opcode::SaveState).Inputs(1U).SrcVregs({VirtualRegister::BRIDGE});
            INST(14U, Opcode::CallStatic).b().InputsAutoType(1U, 20U);
            INST(3U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph2, graph3));
}

TEST_F(VNTest, LoadObjFromConstCseNotApplied)
{
    auto obj1 = static_cast<uintptr_t>(1U);
    auto obj2 = static_cast<uintptr_t>(2U);
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadObjFromConst).ref().SetPtr(obj1);
            INST(2U, Opcode::LoadObjFromConst).ref().SetPtr(obj2);
            INST(3U, Opcode::ReturnVoid);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    GraphChecker(GetGraph()).Check();
    ASSERT_FALSE(GetGraph()->RunPass<ValNum>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(VNTest, FunctionImmediateCseNotApplied)
{
    auto fptr1 = static_cast<uintptr_t>(1U);
    auto fptr2 = static_cast<uintptr_t>(2U);
    auto graph = GetGraph();
    graph->SetDynamicMethod();
#ifndef NDEBUG
    graph->SetDynUnitTestFlag();
#endif
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::FunctionImmediate).any().SetPtr(fptr1);
            INST(2U, Opcode::FunctionImmediate).any().SetPtr(fptr2);
            INST(3U, Opcode::ReturnVoid);
        }
    }
    auto clone = GraphCloner(graph, graph->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    GraphChecker(graph).Check();
    ASSERT_FALSE(graph->RunPass<ValNum>());
    ASSERT_TRUE(GraphComparator().Compare(graph, clone));
}

TEST_F(VNTest, LoadObjFromConstBridgeCreator)
{
    auto obj = static_cast<uintptr_t>(1U);
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadObjFromConst).ref().SetPtr(obj);
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::CallStatic).v0id().InputsAutoType(1U, 2U);
            INST(4U, Opcode::LoadObjFromConst).ref().SetPtr(obj);
            INST(5U, Opcode::SaveState);
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(4U, 5U);
            INST(7U, Opcode::ReturnVoid);
        }
    }
    GraphChecker(graph).Check();
    ASSERT_TRUE(graph->RunPass<ValNum>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());

    auto graphAfter = CreateGraphWithDefaultRuntime();
    GRAPH(graphAfter)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadObjFromConst).ref().SetPtr(obj);
            INST(2U, Opcode::SaveState).Inputs(1U).SrcVregs({VirtualRegister::BRIDGE});
            INST(3U, Opcode::CallStatic).v0id().InputsAutoType(1U, 2U);
            INST(5U, Opcode::SaveState);
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(1U, 5U);
            INST(7U, Opcode::ReturnVoid);
        }
    }
    GraphChecker(graphAfter).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graphAfter));
}

TEST_F(VNTest, FunctionImmediateBridgeCreator)
{
    auto fptr1 = static_cast<uintptr_t>(1U);
    auto graph = CreateGraphWithDefaultRuntime();
    graph->SetDynamicMethod();
#ifndef NDEBUG
    graph->SetDynUnitTestFlag();
#endif
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::FunctionImmediate).any().SetPtr(fptr1);
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::CallStatic).v0id().InputsAutoType(1U, 2U);
            INST(4U, Opcode::FunctionImmediate).any().SetPtr(fptr1);
            INST(5U, Opcode::SaveState);
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(4U, 5U);
            INST(7U, Opcode::ReturnVoid);
        }
    }
    GraphChecker(graph).Check();
    ASSERT_TRUE(graph->RunPass<ValNum>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());

    auto graphAfter = CreateGraphWithDefaultRuntime();
    graphAfter->SetDynamicMethod();
#ifndef NDEBUG
    graphAfter->SetDynUnitTestFlag();
#endif
    GRAPH(graphAfter)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::FunctionImmediate).any().SetPtr(fptr1);
            INST(2U, Opcode::SaveState).Inputs(1U).SrcVregs({VirtualRegister::BRIDGE});
            INST(3U, Opcode::CallStatic).v0id().InputsAutoType(1U, 2U);
            INST(5U, Opcode::SaveState);
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(1U, 5U);
            INST(7U, Opcode::ReturnVoid);
        }
    }
    GraphChecker(graphAfter).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graphAfter));
}

SRC_GRAPH(LoadFromConstantPool, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).any();
        PARAMETER(1U, 1U).any();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::LoadConstantPool).any().Inputs(0U);
            INST(3U, Opcode::LoadConstantPool).any().Inputs(1U);

            INST(4U, Opcode::LoadFromConstantPool).any().Inputs(2U).TypeId(1U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 4U).SrcVregs({0U, 1U, 2U, 3U});
            INST(6U, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 4U}, {DataType::NO_TYPE, 5U}});

            INST(7U, Opcode::LoadFromConstantPool).any().Inputs(2U).TypeId(2U);
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 7U).SrcVregs({0U, 1U, 2U, 4U});
            INST(9U, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 7U}, {DataType::NO_TYPE, 8U}});

            INST(10U, Opcode::LoadFromConstantPool).any().Inputs(2U).TypeId(1U);
            INST(11U, Opcode::SaveState).Inputs(0U, 1U, 2U, 10U).SrcVregs({0U, 1U, 2U, 5U});
            INST(12U, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 10U}, {DataType::NO_TYPE, 11U}});

            INST(13U, Opcode::LoadFromConstantPool).any().Inputs(3U).TypeId(1U);
            INST(14U, Opcode::SaveState).Inputs(0U, 1U, 2U, 13U).SrcVregs({0U, 1U, 2U, 6U});
            INST(15U, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 13U}, {DataType::NO_TYPE, 14U}});

            INST(16U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(LoadFromConstantPool, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).any();
        PARAMETER(1U, 1U).any();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::LoadConstantPool).any().Inputs(0U);
            INST(3U, Opcode::LoadConstantPool).any().Inputs(1U);

            INST(4U, Opcode::LoadFromConstantPool).any().Inputs(2U).TypeId(1U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 4U).SrcVregs({0U, 1U, 2U, 3U});
            INST(6U, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 4U}, {DataType::NO_TYPE, 5U}});

            INST(7U, Opcode::LoadFromConstantPool).any().Inputs(2U).TypeId(2U);
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 7U, 4U).SrcVregs({0U, 1U, 2U, 4U, VirtualRegister::BRIDGE});
            INST(9U, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 7U}, {DataType::NO_TYPE, 8U}});

            INST(11U, Opcode::SaveState).Inputs(0U, 1U, 2U, 4U).SrcVregs({0U, 1U, 2U, 5U});
            INST(12U, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 4U}, {DataType::NO_TYPE, 11U}});

            INST(13U, Opcode::LoadFromConstantPool).any().Inputs(3U).TypeId(1U);
            INST(14U, Opcode::SaveState).Inputs(0U, 1U, 2U, 13U).SrcVregs({0U, 1U, 2U, 6U});
            INST(15U, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 13U}, {DataType::NO_TYPE, 14U}});

            INST(16U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(VNTest, LoadFromConstantPool)
{
    auto graph = CreateGraphWithDefaultRuntime();
    graph->SetDynamicMethod();
#ifndef NDEBUG
    graph->SetDynUnitTestFlag();
#endif
    src_graph::LoadFromConstantPool::CREATE(graph);

    ASSERT_TRUE(graph->RunPass<ValNum>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());

    auto graph1 = CreateGraphWithDefaultRuntime();
    graph1->SetDynamicMethod();
#ifndef NDEBUG
    graph1->SetDynUnitTestFlag();
#endif
    out_graph::LoadFromConstantPool::CREATE(graph1);
    ASSERT_TRUE(GraphComparator().Compare(graph, graph1));
}

SRC_GRAPH(AddressArithmetic, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::AddI).ptr().Imm(8U).Inputs(0U);
            INST(3U, Opcode::Load).i16().Inputs(2U, 1U);
            INST(4U, Opcode::StoreArray).i16().Inputs(0U, 1U, 3U);
            INST(5U, Opcode::SafePoint).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::AddI).ptr().Imm(12U).Inputs(0U);
            INST(7U, Opcode::Load).i16().Inputs(6U, 1U);
            INST(8U, Opcode::AddI).ptr().Imm(8U).Inputs(0U);  // won't be deleted
            INST(9U, Opcode::Store).i16().Inputs(8U, 1U, 7U);
            INST(10U, Opcode::AddI).ptr().Imm(12U).Inputs(0U);  // will be deleted
            INST(11U, Opcode::Load).i16().Inputs(10U, 1U);
            INST(12U, Opcode::AddI).ptr().Imm(16U).Inputs(0U);
            INST(13U, Opcode::Store).i16().Inputs(12U, 1U, 11U);
            INST(14U, Opcode::Return).ref().Inputs(0U);
        }
    }
}

OUT_GRAPH(AddressArithmetic, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::AddI).ptr().Imm(8U).Inputs(0U);
            INST(3U, Opcode::Load).i16().Inputs(2U, 1U);
            INST(4U, Opcode::StoreArray).i16().Inputs(0U, 1U, 3U);
            INST(5U, Opcode::SafePoint).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::AddI).ptr().Imm(12U).Inputs(0U);
            INST(7U, Opcode::Load).i16().Inputs(6U, 1U);
            INST(8U, Opcode::AddI).ptr().Imm(8U).Inputs(0U);
            INST(9U, Opcode::Store).i16().Inputs(8U, 1U, 7U);
            INST(11U, Opcode::Load).i16().Inputs(6U, 1U);
            INST(12U, Opcode::AddI).ptr().Imm(16U).Inputs(0U);
            INST(13U, Opcode::Store).i16().Inputs(12U, 1U, 11U);
            INST(14U, Opcode::Return).ref().Inputs(0U);
        }
    }
}

TEST_F(VNTest, AddressArithmetic)
{
    auto graph = CreateGraphWithDefaultRuntime();
    src_graph::AddressArithmetic::CREATE(graph);
    GraphChecker(graph).Check();

    ASSERT_TRUE(graph->RunPass<ValNum>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());

    auto graph1 = CreateGraphWithDefaultRuntime();
    out_graph::AddressArithmetic::CREATE(graph1);
    GraphChecker(graph1).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graph1));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
