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

#include "common.h"
#include "codegen.h"
#include "optimize_bytecode.h"
#include "mangling.h"

namespace ark::bytecodeopt::test {

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(AsmTest, BitopsBitwiseAnd)
{
    // naive translation of bitops-bitwise-and benchmark
    auto source = R"(
    .function u1 main() {
    movi.64 v0, 0x100000000
    mov.64 v4, v0
    movi v0, 0x1
    mov v6, v0
    label_1: mov v0, v6
    movi v1, 0x5f5e100
    lda v0
    jge v1, label_0
    mov.64 v0, v4
    mov v2, v6
    lda v2
    i32toi64
    sta.64 v2
    lda.64 v0
    and2.64 v2
    sta.64 v0
    mov.64 v4, v0
    inci v6, 0x1
    jmp label_1
    label_0: mov.64 v0, v4
    mov.64 v6, v0
    movi.64 v0, 0x0
    mov.64 v3, v0
    mov.64 v0, v6
    lda.64 v0
    cmp.64 v3
    sta v0
    lda v0
    jeqz label_2
    movi v0, 0x1
    lda v0
    return
    label_2: movi v0, 0x2
    lda v0
    return
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto &program = res.Value();

    ASSERT_TRUE(ParseToGraph(&program, "main"));

    EXPECT_TRUE(RunOptimizations(GetGraph()));

    // NOLINTBEGIN(readability-magic-numbers)
    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;

        BASIC_BLOCK(2U, 3U)
        {
            CONSTANT(23U, 0x5f5e100U).s32();
            CONSTANT(2U, 1U).s32();
            CONSTANT(1U, 0x100000000U).s64();
            INST(25U, Opcode::SaveState);
            INST(0U, Opcode::SaveStateDeoptimize).NoVregs();
            INST(26U, Opcode::SpillFill);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(4U, Opcode::Phi).s64().Inputs(1U, 10U);
            INST(5U, Opcode::Phi).s32().Inputs(2U, 20U);
            INST(19U, Opcode::If).CC(compiler::CC_GE).SrcType(INT32).Inputs(5U, 23U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(9U, Opcode::Cast).s64().SrcType(INT32).Inputs(5U);
            INST(10U, Opcode::And).s64().Inputs(9U, 4U);
            INST(20U, Opcode::AddI).s32().Inputs(5U).Imm(1U);
        }
        BASIC_BLOCK(5U, 7U, 6U)
        {
            INST(24U, Opcode::SaveState);
            CONSTANT(22U, 0U).s64();
            INST(13U, Opcode::Cmp).s32().Inputs(4U, 22U);
            INST(15U, Opcode::IfImm).SrcType(INT32).CC(compiler::CC_EQ).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(16U, Opcode::Return).b().Inputs(2U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            CONSTANT(21U, 2U).s32();
            INST(18U, Opcode::Return).b().Inputs(21U);
        }
    }
    // NOLINTEND(readability-magic-numbers)
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expected));

    const auto sigMain = pandasm::GetFunctionSignatureFromName("main", {});

    auto &function = program.functionStaticTable.at(sigMain);
    EXPECT_TRUE(GetGraph()->RunPass<BytecodeGen>(&function, GetIrInterface()));
}

}  // namespace ark::bytecodeopt::test
