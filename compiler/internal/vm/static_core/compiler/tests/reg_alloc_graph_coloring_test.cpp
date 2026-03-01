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
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/live_registers.h"
#include "optimizer/code_generator/registers_description.h"
#include "optimizer/optimizations/regalloc/reg_alloc_graph_coloring.h"
#include "optimizer/optimizations/regalloc/spill_fills_resolver.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/graph_cloner.h"

namespace ark::compiler {
class RegAllocGraphColoringTest : public GraphTest {
public:
    SpillFillData GetParameterSpillFilll(Inst *param)
    {
        ASSERT(param->IsParameter());
        ASSERT(param->GetNext()->IsSpillFill());
        auto spillFill = param->GetNext()->CastToSpillFill()->GetSpillFill(0U);
        auto paramLiveness = GetGraph()->GetAnalysis<LivenessAnalyzer>().GetInstLifeIntervals(param);
        EXPECT_EQ(paramLiveness->GetReg(), spillFill.SrcValue());
        EXPECT_EQ(paramLiveness->GetSibling()->GetReg(), spillFill.DstValue());
        return spillFill;
    }
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(RegAllocGraphColoringTest, Simple)
{
    // Test is for bigger number of registers (spilling is not supported yet)
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        return;
    }

    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 10U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2U, 0U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(5U, Opcode::Add).u64().Inputs(0U, 2U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Phi).u64().Inputs(2U, 5U);
            INST(7U, Opcode::Add).u64().Inputs(6U, 1U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocGraphColoring>();
    ASSERT_TRUE(result);
    GraphChecker(GetGraph()).Check();
    EXPECT_NE(INS(0U).GetDstReg(), INS(6U).GetDstReg());
    EXPECT_NE(INS(0U).GetDstReg(), INS(1U).GetDstReg());
    EXPECT_NE(INS(0U).GetDstReg(), INS(2U).GetDstReg());

    auto arch = GetGraph()->GetArch();
    size_t firstCallee = arch != Arch::NONE ? GetFirstCalleeReg(arch, false) : 0U;
    EXPECT_LT(INS(0U).CastToParameter()->GetLocationData().DstValue(), firstCallee);
    EXPECT_LT(INS(1U).GetDstReg(), firstCallee);
    EXPECT_LT(INS(2U).GetDstReg(), firstCallee);
    EXPECT_LT(INS(5U).GetDstReg(), firstCallee);
    EXPECT_LT(INS(6U).GetDstReg(), firstCallee);
    EXPECT_LT(INS(7U).GetDstReg(), firstCallee);
}

SRC_GRAPH(AffineComponent, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 10U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2U, 0U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(3U, 8U)
        {
            INST(5U, Opcode::Add).u64().Inputs(0U, 2U);
        }

        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(7U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(1U, 0U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(5U, 7U)
        {
            CONSTANT(9U, 188U);
        }

        BASIC_BLOCK(6U, 7U)
        {
            INST(6U, Opcode::Add).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(7U, 8U)
        {
            INST(10U, Opcode::Phi).u64().Inputs(9U, 6U);
        }

        BASIC_BLOCK(8U, -1L)
        {
            INST(11U, Opcode::Phi).u64().Inputs(5U, 10U);
            INST(12U, Opcode::Add).u64().Inputs(11U, 1U);
            INST(13U, Opcode::Add).u64().Inputs(12U, 0U);
            INST(14U, Opcode::Return).u64().Inputs(13U);
        }
    }
}

TEST_F(RegAllocGraphColoringTest, AffineComponent)
{
    // Test is for bigger number of registers (spilling is not supported yet)
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        return;
    }

    src_graph::AffineComponent::CREATE(GetGraph());
    auto result = GetGraph()->RunPass<RegAllocGraphColoring>();
    ASSERT_TRUE(result);
    GraphChecker(GetGraph()).Check();
    EXPECT_EQ(INS(0U).GetDstReg(), INS(3U).GetSrcReg(1U));
    EXPECT_EQ(INS(0U).GetDstReg(), INS(7U).GetSrcReg(1U));

    // Check affinity group
    EXPECT_EQ(INS(5U).GetDstReg(), INS(6U).GetDstReg());
    EXPECT_EQ(INS(5U).GetDstReg(), INS(9U).GetDstReg());
    EXPECT_EQ(INS(5U).GetDstReg(), INS(10U).GetDstReg());
    EXPECT_EQ(INS(5U).GetDstReg(), INS(11U).GetDstReg());
}

SRC_GRAPH(SimpleCall, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 1U);
        CONSTANT(3U, 10U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(3U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(6U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(7U, Opcode::CallStatic).b().InputsAutoType(0U, 1U, 2U, 6U);
            INST(8U, Opcode::Add).u64().Inputs(1U, 3U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs(3U, 8U);
            INST(10U, Opcode::Add).u64().Inputs(9U, 1U);
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }
    }
}

TEST_F(RegAllocGraphColoringTest, SimpleCall)
{
    // Test is for bigger number of registers (spilling is not supported yet)
    if (GetGraph()->GetArch() == Arch::AARCH32 || GetGraph()->GetCallingConvention() == nullptr) {
        return;
    }

    src_graph::SimpleCall::CREATE(GetGraph());
    auto regalloc = RegAllocGraphColoring(GetGraph());
    auto arch = GetGraph()->GetArch();
    size_t firstCallee = arch != Arch::NONE ? GetFirstCalleeReg(arch, false) : 0U;

    auto result = regalloc.Run();
    ASSERT_TRUE(result);
    GraphChecker(GetGraph()).Check();

    auto param1Sf = GetParameterSpillFilll(&INS(1U));
    EXPECT_NE(param1Sf.DstValue(), INS(8U).GetDstReg());
    EXPECT_NE(param1Sf.DstValue(), INS(2U).GetDstReg());
    EXPECT_NE(param1Sf.DstValue(), INS(3U).GetDstReg());

    // Check intervals in calle registers and splits
    EXPECT_LT(INS(0U).GetDstReg(), firstCallee);
    EXPECT_GE(param1Sf.DstValue(), firstCallee);
}

// Check fallback to Linearscan (spilling is not implemented yet)
TEST_F(RegAllocGraphColoringTest, HighPressure)
{
    // Test is for bigger number of registers (spilling is not supported yet)
    if (GetGraph()->GetArch() == Arch::AARCH32 || GetGraph()->GetCallingConvention() == nullptr) {
        return;
    }

    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 10U);
        CONSTANT(3U, 10U);
        CONSTANT(4U, 10U);
        CONSTANT(5U, 10U);
        CONSTANT(6U, 10U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(8U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(9U, Opcode::Add).u64().Inputs(0U, 3U);
            INST(10U, Opcode::Add).u64().Inputs(0U, 4U);
            INST(11U, Opcode::Add).u64().Inputs(0U, 5U);
            INST(12U, Opcode::Add).u64().Inputs(0U, 6U);
            INST(13U, Opcode::Add).u64().Inputs(0U, 7U);
            INST(14U, Opcode::Add).u64().Inputs(0U, 8U);
            INST(15U, Opcode::Add).u64().Inputs(8U, 7U);
            INST(16U, Opcode::Add).u64().Inputs(9U, 8U);

            INST(17U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(18U, Opcode::CallStatic).b().InputsAutoType(0U, 1U, 2U, 17U);

            INST(20U, Opcode::Add).u64().Inputs(1U, 2U);
            INST(21U, Opcode::Add).u64().Inputs(3U, 4U);
            INST(22U, Opcode::Add).u64().Inputs(5U, 6U);
            INST(23U, Opcode::Add).u64().Inputs(7U, 8U);
            INST(24U, Opcode::Add).u64().Inputs(9U, 10U);
            INST(25U, Opcode::Add).u64().Inputs(11U, 12U);
            INST(26U, Opcode::Add).u64().Inputs(13U, 14U);
            INST(27U, Opcode::Add).u64().Inputs(15U, 16U);

            INST(60U, Opcode::Return).u64().Inputs(27U);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocGraphColoring>();
    ASSERT_TRUE(result);
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
