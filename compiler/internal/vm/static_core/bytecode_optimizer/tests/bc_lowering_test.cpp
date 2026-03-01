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

#include "libarkbase/utils/utils.h"
#include "common.h"
#include "check_resolver.h"
#include "compiler/optimizer/optimizations/cleanup.h"
#include "compiler/optimizer/optimizations/lowering.h"

namespace ark::bytecodeopt::test {

class LoweringTest : public CommonTest {};

// NOLINTBEGIN(readability-magic-numbers)

// Checks the results of lowering pass if negative operands are used
TEST_F(IrBuilderTest, Lowering)
{
    // ISA opcodes with expected lowering IR opcodes
    std::map<std::string, compiler::Opcode> opcodes = {
        {"add", compiler::Opcode::SubI}, {"sub", compiler::Opcode::AddI}, {"mul", compiler::Opcode::MulI},
        {"and", compiler::Opcode::AndI}, {"xor", compiler::Opcode::XorI}, {"or", compiler::Opcode::OrI},
        {"div", compiler::Opcode::DivI}, {"mod", compiler::Opcode::ModI},
    };

    const std::string templateSource = R"(
    .function i32 main() {
        movi v0, 0x3
        movi v1, 0xffffffffffffffe2
        OPCODE v0, v1
        return
    }
    )";

    for (auto const &opcode : opcodes) {
        // Specialize template source to the current opcode
        std::string source(templateSource);
        size_t startPos = source.find("OPCODE");
        source.replace(startPos, 6U, opcode.first);

        ASSERT_TRUE(ParseToGraph(source, "main"));
#ifndef NDEBUG
        GetGraph()->SetLowLevelInstructionsEnabled();
#endif
        GetGraph()->RunPass<CheckResolver>();
        GetGraph()->RunPass<compiler::Lowering>();
        GetGraph()->RunPass<compiler::Cleanup>();

        int32_t imm = -30_I;
        // Note: `AddI -30` is handled as `SubI 30`. `SubI -30` is handled as `AddI 30`.
        if (opcode.second == compiler::Opcode::AddI || opcode.second == compiler::Opcode::SubI) {
            imm = 30_I;
        }

        auto expected = CreateEmptyGraph();
        GRAPH(expected)
        {
            CONSTANT(1U, 3U).s32();

            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, opcode.second).s32().Inputs(1U).Imm(imm);
                INST(3U, Opcode::Return).s32().Inputs(2U);
            }
        }

        EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expected));
    }
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(LoweringTest, AddSub)
{
    auto init = CreateEmptyGraph();
    GRAPH(init)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f32();
        CONSTANT(3U, 12U).s32();
        CONSTANT(4U, 150U).s32();
        CONSTANT(5U, 0U).s64();
        CONSTANT(6U, 1.2F).f32();
        CONSTANT(7U, -1L).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::Add).u32().Inputs(0U, 3U);
            INST(9U, Opcode::Sub).u32().Inputs(0U, 3U);
            INST(10U, Opcode::Add).u32().Inputs(0U, 4U);
            INST(11U, Opcode::Sub).u32().Inputs(0U, 4U);
            INST(12U, Opcode::Add).u64().Inputs(1U, 5U);
            INST(13U, Opcode::Sub).f32().Inputs(2U, 6U);
            INST(14U, Opcode::Sub).u32().Inputs(0U, 7U);
            INST(15U, Opcode::SaveState).NoVregs();
            INST(20U, Opcode::CallStatic).b().InputsAutoType(8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U);
            INST(21U, Opcode::Return).b().Inputs(20U);
        }
    }
#ifndef NDEBUG
    init->SetLowLevelInstructionsEnabled();
#endif
    init->RunPass<compiler::Lowering>();
    init->RunPass<compiler::Cleanup>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f32();
        CONSTANT(4U, 150U).s32();
        CONSTANT(5U, 0U).s64();
        CONSTANT(6U, 1.2F).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(22U, Opcode::AddI).u32().Inputs(0U).Imm(0xcU);
            INST(23U, Opcode::SubI).u32().Inputs(0U).Imm(0xcU);
            INST(10U, Opcode::Add).u32().Inputs(0U, 4U);
            INST(11U, Opcode::Sub).u32().Inputs(0U, 4U);
            INST(12U, Opcode::Add).u64().Inputs(1U, 5U);
            INST(13U, Opcode::Sub).f32().Inputs(2U, 6U);
            INST(24U, Opcode::AddI).u32().Inputs(0U).Imm(1U);
            INST(19U, Opcode::SaveState).NoVregs();
            INST(20U, Opcode::CallStatic).b().InputsAutoType(22U, 23U, 10U, 11U, 12U, 13U, 24U, 19U);
            INST(21U, Opcode::Return).b().Inputs(20U);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(init, expected));
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(LoweringTest, MulDivMod)
{
    auto init = CreateEmptyGraph();
    GRAPH(init)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f32();
        CONSTANT(3U, 12U).s32();
        CONSTANT(4U, 150U).s32();
        CONSTANT(5U, 0U).s64();
        CONSTANT(6U, 1.2F).f32();
        CONSTANT(7U, -1L).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::Div).s32().Inputs(0U, 3U);
            INST(9U, Opcode::Div).u32().Inputs(0U, 4U);
            INST(10U, Opcode::Div).u64().Inputs(1U, 5U);
            INST(11U, Opcode::Div).f32().Inputs(2U, 6U);
            INST(12U, Opcode::Div).s32().Inputs(0U, 7U);

            INST(13U, Opcode::Mod).s32().Inputs(0U, 3U);
            INST(14U, Opcode::Mod).u32().Inputs(0U, 4U);
            INST(15U, Opcode::Mod).u64().Inputs(1U, 5U);
            INST(16U, Opcode::Mod).f32().Inputs(2U, 6U);
            INST(17U, Opcode::Mod).s32().Inputs(0U, 7U);

            INST(18U, Opcode::Mul).s32().Inputs(0U, 3U);
            INST(19U, Opcode::Mul).u32().Inputs(0U, 4U);
            INST(20U, Opcode::Mul).u64().Inputs(1U, 5U);
            INST(21U, Opcode::Mul).f32().Inputs(2U, 6U);
            INST(22U, Opcode::Mul).s32().Inputs(0U, 7U);
            INST(31U, Opcode::SaveState).NoVregs();
            INST(23U, Opcode::CallStatic)
                .b()
                .InputsAutoType(8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U, 16U, 17U, 18U, 19U, 20U, 21U, 22U, 31U);
            INST(24U, Opcode::Return).b().Inputs(23U);
        }
    }

#ifndef NDEBUG
    init->SetLowLevelInstructionsEnabled();
#endif
    init->RunPass<compiler::Lowering>();
    init->RunPass<compiler::Cleanup>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).f32();
        CONSTANT(4U, 150U).s32();
        CONSTANT(5U, 0U).s64();
        CONSTANT(6U, 1.2F).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(25U, Opcode::DivI).s32().Inputs(0U).Imm(0xcU);
            INST(9U, Opcode::Div).u32().Inputs(0U, 4U);
            INST(10U, Opcode::Div).u64().Inputs(1U, 5U);
            INST(11U, Opcode::Div).f32().Inputs(2U, 6U);
            INST(26U, Opcode::DivI).s32().Inputs(0U).Imm(static_cast<uint64_t>(-1L));
            INST(27U, Opcode::ModI).s32().Inputs(0U).Imm(0xcU);
            INST(14U, Opcode::Mod).u32().Inputs(0U, 4U);
            INST(15U, Opcode::Mod).u64().Inputs(1U, 5U);
            INST(16U, Opcode::Mod).f32().Inputs(2U, 6U);
            INST(28U, Opcode::ModI).s32().Inputs(0U).Imm(static_cast<uint64_t>(-1L));
            INST(29U, Opcode::MulI).s32().Inputs(0U).Imm(0xcU);
            INST(19U, Opcode::Mul).u32().Inputs(0U, 4U);
            INST(20U, Opcode::Mul).u64().Inputs(1U, 5U);
            INST(21U, Opcode::Mul).f32().Inputs(2U, 6U);
            INST(30U, Opcode::MulI).s32().Inputs(0U).Imm(static_cast<uint64_t>(-1L));
            INST(31U, Opcode::SaveState).NoVregs();
            INST(23U, Opcode::CallStatic)
                .b()
                .InputsAutoType(25U, 9U, 10U, 11U, 26U, 27U, 14U, 15U, 16U, 28U, 29U, 19U, 20U, 21U, 30U, 31U);
            INST(24U, Opcode::Return).b().Inputs(23U);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(init, expected));
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(LoweringTest, Logic)
{
    auto init = CreateEmptyGraph();
    GRAPH(init)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(24U, 1U).s64();
        CONSTANT(1U, 12U).s32();
        CONSTANT(2U, 50U).s32();
        CONSTANT(25U, 0U).s64();
        CONSTANT(27U, 300U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Or).u32().Inputs(0U, 1U);
            INST(5U, Opcode::Or).u32().Inputs(0U, 2U);
            INST(6U, Opcode::And).u32().Inputs(0U, 1U);
            INST(8U, Opcode::And).u32().Inputs(0U, 2U);
            INST(9U, Opcode::Xor).u32().Inputs(0U, 1U);
            INST(11U, Opcode::Xor).u32().Inputs(0U, 2U);
            INST(12U, Opcode::Or).u8().Inputs(0U, 1U);
            INST(13U, Opcode::And).u32().Inputs(0U, 0U);
            INST(26U, Opcode::And).s64().Inputs(24U, 25U);
            INST(28U, Opcode::Xor).u32().Inputs(0U, 27U);
            INST(29U, Opcode::Or).s64().Inputs(24U, 25U);
            INST(30U, Opcode::Xor).s64().Inputs(24U, 25U);
            INST(31U, Opcode::And).u32().Inputs(0U, 27U);
            INST(32U, Opcode::Or).u32().Inputs(0U, 27U);
            INST(15U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic)
                .b()
                .InputsAutoType(3U, 5U, 6U, 8U, 9U, 11U, 12U, 13U, 26U, 28U, 29U, 30U, 31U, 32U, 15U);
            INST(23U, Opcode::Return).b().Inputs(14U);
        }
    }

#ifndef NDEBUG
    init->SetLowLevelInstructionsEnabled();
#endif
    init->RunPass<compiler::Lowering>();
    init->RunPass<compiler::Cleanup>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(24U, 1U).s64();
        CONSTANT(1U, 12U).s32();
        CONSTANT(25U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(33U, Opcode::OrI).u32().Inputs(0U).Imm(0xcU);
            INST(34U, Opcode::OrI).u32().Inputs(0U).Imm(0x32U);
            INST(35U, Opcode::AndI).u32().Inputs(0U).Imm(0xcU);
            INST(36U, Opcode::AndI).u32().Inputs(0U).Imm(0x32U);
            INST(37U, Opcode::XorI).u32().Inputs(0U).Imm(0xcU);
            INST(38U, Opcode::XorI).u32().Inputs(0U).Imm(0x32U);
            INST(12U, Opcode::Or).u8().Inputs(0U, 1U);
            INST(13U, Opcode::And).u32().Inputs(0U, 0U);
            INST(26U, Opcode::And).s64().Inputs(24U, 25U);
            INST(39U, Opcode::XorI).u32().Inputs(0U).Imm(0x12cU);
            INST(29U, Opcode::Or).s64().Inputs(24U, 25U);
            INST(30U, Opcode::Xor).s64().Inputs(24U, 25U);
            INST(40U, Opcode::AndI).u32().Inputs(0U).Imm(0x12cU);
            INST(41U, Opcode::OrI).u32().Inputs(0U).Imm(0x12cU);
            INST(15U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic)
                .b()
                .InputsAutoType(33U, 34U, 35U, 36U, 37U, 38U, 12U, 13U, 26U, 39U, 29U, 30U, 40U, 41U, 15U);
            INST(23U, Opcode::Return).b().Inputs(14U);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(init, expected));
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(LoweringTest, Shift)
{
    auto init = CreateEmptyGraph();
    GRAPH(init)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(24U, 1U).s64();
        CONSTANT(1U, 12U).s32();
        CONSTANT(2U, 64U).s32();
        CONSTANT(25U, 0U).s64();
        CONSTANT(27U, 200U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Shr).u32().Inputs(0U, 1U);
            INST(5U, Opcode::Shr).u32().Inputs(0U, 2U);
            INST(6U, Opcode::AShr).u32().Inputs(0U, 1U);
            INST(8U, Opcode::AShr).u32().Inputs(0U, 2U);
            INST(9U, Opcode::Shl).u32().Inputs(0U, 1U);
            INST(11U, Opcode::Shl).u32().Inputs(0U, 2U);
            INST(12U, Opcode::Shl).u8().Inputs(0U, 1U);
            INST(13U, Opcode::Shr).u32().Inputs(0U, 0U);
            INST(26U, Opcode::Shr).s64().Inputs(24U, 25U);
            INST(28U, Opcode::AShr).s32().Inputs(0U, 27U);
            INST(29U, Opcode::AShr).s64().Inputs(24U, 25U);
            INST(30U, Opcode::Shl).s64().Inputs(24U, 25U);
            INST(31U, Opcode::Shr).s32().Inputs(0U, 27U);
            INST(32U, Opcode::Shl).s32().Inputs(0U, 27U);
            INST(15U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic)
                .b()
                .InputsAutoType(3U, 5U, 6U, 8U, 9U, 11U, 12U, 13U, 26U, 28U, 29U, 30U, 31U, 32U, 15U);
            INST(23U, Opcode::Return).b().Inputs(14U);
        }
    }
#ifndef NDEBUG
    init->SetLowLevelInstructionsEnabled();
#endif
    init->RunPass<compiler::Lowering>();
    init->RunPass<compiler::Cleanup>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(24U, 1U).s64();
        CONSTANT(1U, 12U).s32();
        CONSTANT(2U, 64U).s32();
        CONSTANT(25U, 0U).s64();
        CONSTANT(27U, 200U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(33U, Opcode::ShrI).u32().Inputs(0U).Imm(0xcU);
            INST(5U, Opcode::Shr).u32().Inputs(0U, 2U);
            INST(34U, Opcode::AShrI).u32().Inputs(0U).Imm(0xcU);
            INST(8U, Opcode::AShr).u32().Inputs(0U, 2U);
            INST(35U, Opcode::ShlI).u32().Inputs(0U).Imm(0xcU);
            INST(11U, Opcode::Shl).u32().Inputs(0U, 2U);
            INST(12U, Opcode::Shl).u8().Inputs(0U, 1U);
            INST(13U, Opcode::Shr).u32().Inputs(0U, 0U);
            INST(26U, Opcode::Shr).s64().Inputs(24U, 25U);
            INST(28U, Opcode::AShr).s32().Inputs(0U, 27U);
            INST(29U, Opcode::AShr).s64().Inputs(24U, 25U);
            INST(30U, Opcode::Shl).s64().Inputs(24U, 25U);
            INST(31U, Opcode::Shr).s32().Inputs(0U, 27U);
            INST(32U, Opcode::Shl).s32().Inputs(0U, 27U);
            INST(15U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic)
                .b()
                .InputsAutoType(33U, 5U, 34U, 8U, 35U, 11U, 12U, 13U, 26U, 28U, 29U, 30U, 31U, 32U, 15U);
            INST(23U, Opcode::Return).b().Inputs(14U);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(init, expected));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::bytecodeopt::test
