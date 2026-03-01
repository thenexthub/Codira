/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "assembler/assembly-emitter.h"
#include "canonicalization.h"
#include "codegen.h"
#include "common.h"
#include "compiler/optimizer/optimizations/lowering.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "reg_encoder.h"

namespace ark::bytecodeopt::test {

// NOLINTBEGIN(readability-magic-numbers)

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(CommonTest, RegEncoderF32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0.0).f32();
        CONSTANT(1U, 0.0).f32();
        CONSTANT(2U, 0.0).f32();
        CONSTANT(3U, 0.0).f32();
        CONSTANT(4U, 0.0).f32();
        CONSTANT(5U, 0.0).f32();
        CONSTANT(6U, 0.0).f32();
        CONSTANT(7U, 0.0).f32();
        CONSTANT(8U, 0.0).f32();
        CONSTANT(9U, 0.0).f32();
        CONSTANT(10U, 0.0).f32();
        CONSTANT(11U, 0.0).f32();
        CONSTANT(12U, 0.0).f32();
        CONSTANT(13U, 0.0).f32();
        CONSTANT(14U, 0.0).f32();
        CONSTANT(15U, 0.0).f32();
        CONSTANT(16U, 0.0).f32();
        CONSTANT(17U, 0.0).f32();
        CONSTANT(18U, 0.0).f32();
        CONSTANT(19U, 0.0).f32();
        CONSTANT(20U, 0.0).f32();
        CONSTANT(21U, 0.0).f32();
        CONSTANT(22U, 0.0).f32();
        CONSTANT(23U, 0.0).f32();
        CONSTANT(24U, 0.0).f32();
        CONSTANT(25U, 0.0).f32();
        CONSTANT(26U, 0.0).f32();
        CONSTANT(27U, 0.0).f32();
        CONSTANT(28U, 0.0).f32();
        CONSTANT(29U, 0.0).f32();
        CONSTANT(30U, 0.0).f32();
        CONSTANT(31U, 0.0).f32();

        CONSTANT(32U, 1.0).f64();
        CONSTANT(33U, 2.0_D).f64();

        BASIC_BLOCK(2U, -1L)
        {
            // NOLINTNEXTLINE(google-build-using-namespace)
            using namespace compiler::DataType;
            INST(40U, Opcode::Sub).f64().Inputs(32U, 33U);
            INST(41U, Opcode::Return).f64().Inputs(40U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    EXPECT_TRUE(graph->RunPass<RegEncoder>());
    EXPECT_TRUE(graph->RunPass<compiler::Cleanup>());

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        CONSTANT(32U, 1.0).f64();
        CONSTANT(33U, 2.0_D).f64();

        BASIC_BLOCK(2U, -1L)
        {
            // NOLINTNEXTLINE(google-build-using-namespace)
            using namespace compiler::DataType;
            INST(40U, Opcode::Sub).f64().Inputs(32U, 33U);
            INST(41U, Opcode::Return).f64().Inputs(40U);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(CommonTest, RegEncoderHoldingSpillFillInst)
{
    RuntimeInterfaceMock interface(2U);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&interface);
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;

        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).b();
        CONSTANT(26U, 0xfffffffffffffffaU).s64();
        CONSTANT(27U, 0x6U).s64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::LoadObject).s64().Inputs(0U);
            INST(10U, Opcode::LoadObject).s64().Inputs(0U);
            INST(11U, Opcode::Add).s64().Inputs(10U, 4U);
            CONSTANT(52U, 0x5265c00U).s64();
            INST(15U, Opcode::Div).s64().Inputs(11U, 52U);
            INST(16U, Opcode::Mul).s64().Inputs(15U, 52U);
            INST(20U, Opcode::Sub).s64().Inputs(16U, 10U);
            CONSTANT(53U, 0x2932e00U).s64();
            INST(22U, Opcode::Add).s64().Inputs(20U, 53U);
            INST(25U, Opcode::IfImm).SrcType(BOOL).CC(compiler::CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(3U, 4U) {}

        BASIC_BLOCK(4U, -1L)
        {
            INST(28U, Opcode::Phi).s64().Inputs(26U, 27U);
            CONSTANT(54U, 0x36ee80U).s64();
            INST(31U, Opcode::Mul).s64().Inputs(28U, 54U);
            INST(32U, Opcode::Add).s64().Inputs(31U, 22U);
            INST(33U, Opcode::SaveState).NoVregs();
            INST(35U, Opcode::CallVirtual).v0id().Inputs({{REFERENCE, 0U}, {INT64, 32U}, {NO_TYPE, 33U}});
            INST(36U, Opcode::SaveState).NoVregs();
            INST(37U, Opcode::LoadAndInitClass).ref().Inputs(36U);
            INST(39U, Opcode::SaveState).NoVregs();
            INST(58U, Opcode::InitObject).ref().Inputs({{REFERENCE, 37U}, {REFERENCE, 0U}, {NO_TYPE, 39U}});
            CONSTANT(55U, 0.0093026_D).f64();
            CONSTANT(56U, 0.0098902_D).f64();
            CONSTANT(57U, 0x1388U).s64();
            INST(45U, Opcode::SaveState).NoVregs();
            INST(47U, Opcode::CallStatic)
                .s64()
                .Inputs({{REFERENCE, 0},
                         {REFERENCE, 58U},
                         {BOOL, 1U},
                         {FLOAT64, 55U},
                         {FLOAT64, 56U},
                         {INT64, 57U},
                         {NO_TYPE, 45U}});
            INST(48U, Opcode::SaveState).NoVregs();
            INST(50U, Opcode::CallVirtual).v0id().Inputs({{REFERENCE, 0U}, {INT64, 4U}, {NO_TYPE, 48U}});
            INST(51U, Opcode::Return).s64().Inputs(47U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    EXPECT_TRUE(graph->RunPass<RegEncoder>());
    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;

        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).b();
        CONSTANT(26U, 0xfffffffffffffffaU).s64();
        CONSTANT(27U, 0x6U).s64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::LoadObject).s64().Inputs(0U);
            INST(10U, Opcode::LoadObject).s64().Inputs(0U);
            INST(11U, Opcode::Add).s64().Inputs(10U, 4U);
            CONSTANT(52U, 0x5265c00U).s64();
            INST(15U, Opcode::Div).s64().Inputs(11U, 52U);
            INST(16U, Opcode::Mul).s64().Inputs(15U, 52U);
            INST(20U, Opcode::Sub).s64().Inputs(16U, 10U);
            CONSTANT(53U, 0x2932e00U).s64();
            INST(22U, Opcode::Add).s64().Inputs(20U, 53U);
            INST(25U, Opcode::IfImm).SrcType(BOOL).CC(compiler::CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            // SpillFill added
            INST(60U, Opcode::SpillFill);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(28U, Opcode::Phi).s64().Inputs(26U, 27U);
            CONSTANT(54U, 0x36ee80U).s64();
            INST(31U, Opcode::Mul).s64().Inputs(28U, 54U);
            INST(32U, Opcode::Add).s64().Inputs(31U, 22U);
            INST(33U, Opcode::SaveState).NoVregs();
            INST(35U, Opcode::CallVirtual).v0id().Inputs({{REFERENCE, 0U}, {INT64, 32U}, {NO_TYPE, 33U}});
            INST(36U, Opcode::SaveState).NoVregs();
            INST(37U, Opcode::LoadAndInitClass).ref().Inputs(36U);
            INST(39U, Opcode::SaveState).NoVregs();
            INST(58U, Opcode::InitObject).ref().Inputs({{REFERENCE, 37U}, {REFERENCE, 0U}, {NO_TYPE, 39U}});
            CONSTANT(55U, 0.0093026_D).f64();
            CONSTANT(56U, 0.0098902_D).f64();
            CONSTANT(57U, 0x1388U).s64();
            INST(45U, Opcode::SaveState).NoVregs();

            // SpillFill added
            INST(70U, Opcode::SpillFill);
            INST(47U, Opcode::CallStatic)
                .s64()
                .Inputs({{REFERENCE, 0},
                         {REFERENCE, 58U},
                         {BOOL, 1U},
                         {FLOAT64, 55U},
                         {FLOAT64, 56U},
                         {INT64, 57U},
                         {NO_TYPE, 45U}});
            INST(48U, Opcode::SaveState).NoVregs();
            INST(50U, Opcode::CallVirtual).v0id().Inputs({{REFERENCE, 0U}, {INT64, 4U}, {NO_TYPE, 48U}});
            INST(51U, Opcode::Return).s64().Inputs(47U);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(CommonTest, RegEncoderStoreObject)
{
    // This test covers function CheckWidthVisitor::VisitStoreObject
    RuntimeInterfaceMock interface(4U);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&interface);
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).ref();
        PARAMETER(3U, 3U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            // NOLINTNEXTLINE(google-build-using-namespace)
            using namespace compiler::DataType;

            INST(4U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 0U}, {NO_TYPE, 4U}});
            INST(9U, Opcode::StoreObject).ref().Inputs(0U, 2U);
            INST(12U, Opcode::StoreObject).s32().Inputs(0U, 3U);
            INST(15U, Opcode::LoadObject).ref().Inputs(1U);
            INST(16U, Opcode::SaveState).NoVregs();
            INST(18U, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 15U}, {NO_TYPE, 16U}});
            INST(19U, Opcode::SaveState).NoVregs();
            INST(21U, Opcode::LoadClass).ref().Inputs(19U);
            INST(20U, Opcode::CheckCast).Inputs(18U, 21U, 19U);
            INST(23U, Opcode::StoreObject).ref().Inputs(0U, 18U);
            INST(26U, Opcode::LoadObject).ref().Inputs(1U);
            INST(27U, Opcode::SaveState).NoVregs();
            INST(29U, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 26U}, {NO_TYPE, 27U}});
            INST(30U, Opcode::SaveState).NoVregs();
            INST(32U, Opcode::LoadClass).ref().Inputs(30U);
            INST(31U, Opcode::CheckCast).Inputs(29U, 32U, 30U);
            INST(34U, Opcode::StoreObject).ref().Inputs(0U, 29U);
            INST(37U, Opcode::LoadObject).ref().Inputs(1U);
            INST(38U, Opcode::SaveState).NoVregs();
            INST(40U, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 37U}, {NO_TYPE, 38U}});
            INST(41U, Opcode::SaveState).NoVregs();
            INST(43U, Opcode::LoadClass).ref().Inputs(41U);
            INST(42U, Opcode::CheckCast).Inputs(40U, 43U, 41U);
            INST(45U, Opcode::StoreObject).ref().Inputs(0U, 40U);
            INST(48U, Opcode::LoadObject).ref().Inputs(1U);
            INST(49U, Opcode::SaveState).NoVregs();
            INST(51U, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 48U}, {NO_TYPE, 49U}});
            INST(52U, Opcode::SaveState).NoVregs();
            INST(54U, Opcode::LoadClass).ref().Inputs(52U);
            INST(53U, Opcode::CheckCast).Inputs(51U, 54U, 52U);
            INST(56U, Opcode::StoreObject).ref().Inputs(0U, 51U);
            INST(57U, Opcode::ReturnVoid).v0id();
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    EXPECT_TRUE(graph->RunPass<RegEncoder>());
    EXPECT_FALSE(graph->RunPass<compiler::Cleanup>());

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).ref();
        PARAMETER(3U, 3U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            // NOLINTNEXTLINE(google-build-using-namespace)
            using namespace compiler::DataType;

            INST(4U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 0U}, {NO_TYPE, 4U}});
            INST(9U, Opcode::StoreObject).ref().Inputs(0U, 2U);
            INST(12U, Opcode::StoreObject).s32().Inputs(0U, 3U);
            INST(15U, Opcode::LoadObject).ref().Inputs(1U);
            INST(16U, Opcode::SaveState).NoVregs();
            INST(18U, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 15U}, {NO_TYPE, 16U}});
            INST(60U, Opcode::SaveState).NoVregs();
            INST(21U, Opcode::LoadClass).ref().Inputs(60U);
            INST(20U, Opcode::CheckCast).Inputs(18U, 21U, 60U);
            INST(23U, Opcode::StoreObject).ref().Inputs(0U, 18U);
            INST(26U, Opcode::LoadObject).ref().Inputs(1U);
            INST(27U, Opcode::SaveState).NoVregs();
            INST(29U, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 26U}, {NO_TYPE, 27U}});
            INST(63U, Opcode::SaveState).NoVregs();
            INST(32U, Opcode::LoadClass).ref().Inputs(63U);
            INST(31U, Opcode::CheckCast).Inputs(29U, 32U, 63U);
            INST(34U, Opcode::StoreObject).ref().Inputs(0U, 29U);
            INST(37U, Opcode::LoadObject).ref().Inputs(1U);
            INST(38U, Opcode::SaveState).NoVregs();
            INST(40U, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 37U}, {NO_TYPE, 38U}});
            INST(41U, Opcode::SaveState).NoVregs();
            INST(43U, Opcode::LoadClass).ref().Inputs(41U);
            INST(42U, Opcode::CheckCast).Inputs(40U, 43U, 41U);
            INST(45U, Opcode::StoreObject).ref().Inputs(0U, 40U);
            INST(48U, Opcode::LoadObject).ref().Inputs(1U);
            INST(49U, Opcode::SaveState).NoVregs();
            INST(51U, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 48U}, {NO_TYPE, 49U}});
            INST(52U, Opcode::SaveState).NoVregs();
            INST(54U, Opcode::LoadClass).ref().Inputs(52U);
            INST(53U, Opcode::CheckCast).Inputs(51U, 54U, 52U);
            INST(56U, Opcode::StoreObject).ref().Inputs(0U, 51U);
            INST(57U, Opcode::ReturnVoid).v0id();
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

// Check processing instructions with the same args by RegEncoder.
TEST_F(CommonTest, RegEncoderSameArgsInst)
{
    auto srcGraph = CreateEmptyGraph();
    ArenaVector<bool> regMask(254U, false, srcGraph->GetLocalAllocator()->Adapter());
    srcGraph->InitUsedRegs<compiler::DataType::INT64>(&regMask);
    GRAPH(srcGraph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::StoreArray).s32().Inputs(0U, 1U, 2U).SrcReg(0U, 17U).SrcReg(1U, 17U).SrcReg(2U, 5U);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    srcGraph->RunPass<RegEncoder>();

    auto optGraph = CreateEmptyGraph();
    optGraph->InitUsedRegs<compiler::DataType::INT64>(&regMask);
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SpillFill);
            INST(4U, Opcode::StoreArray).s32().Inputs(0U, 1U, 2U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(srcGraph, optGraph));

    for (auto bb : srcGraph->GetBlocksRPO()) {
        for (auto inst : bb->AllInstsSafe()) {
            if (inst->GetOpcode() == Opcode::StoreArray) {
                ASSERT_TRUE(inst->GetSrcReg(0U) == inst->GetSrcReg(1U));
            }
        }
    }
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::bytecodeopt::test
