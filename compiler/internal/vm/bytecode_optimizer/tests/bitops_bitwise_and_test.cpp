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

namespace panda::bytecodeopt::test {

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

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        using namespace compiler::DataType;

        BASIC_BLOCK(2, 3)
        {
            CONSTANT(23, 0x5f5e100).s32();
            CONSTANT(2, 1).s32();
            CONSTANT(1, 0x100000000).s64();
            INST(0, Opcode::SaveStateDeoptimize).NoVregs();
            INST(26, Opcode::SpillFill);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(4, Opcode::Phi).s64().Inputs(1, 10);
            INST(5, Opcode::Phi).s32().Inputs(2, 20);
            INST(19, Opcode::If).CC(compiler::CC_GE).SrcType(INT32).Inputs(5, 23);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(9, Opcode::Cast).s64().SrcType(INT32).Inputs(5);
            INST(10, Opcode::And).s64().Inputs(9, 4);
            INST(20, Opcode::AddI).s32().Inputs(5).Imm(1);
        }
        BASIC_BLOCK(5, 7, 6)
        {
            CONSTANT(22, 0).s64();
            INST(13, Opcode::Cmp).s32().Inputs(4, 22);
            INST(15, Opcode::IfImm).SrcType(INT32).CC(compiler::CC_EQ).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(16, Opcode::Return).b().Inputs(2);
        }
        BASIC_BLOCK(7, -1)
        {
            CONSTANT(21, 2).s32();
            INST(18, Opcode::Return).b().Inputs(21);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expected));

    const auto sig_main = pandasm::GetFunctionSignatureFromName("main", {});

    auto &function = program.function_table.at(sig_main);
    EXPECT_TRUE(GetGraph()->RunPass<BytecodeGen>(&function, GetIrInterface()));
}

}  // namespace panda::bytecodeopt::test
