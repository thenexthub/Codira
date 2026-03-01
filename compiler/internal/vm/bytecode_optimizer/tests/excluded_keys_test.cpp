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

#include <gtest/gtest.h>

#include "assembler/assembly-parser.h"

#include "bytecode_optimizer/optimize_bytecode.h"

namespace panda::bytecodeopt {
using LiteralArray = pandasm::LiteralArray;
using Literal = LiteralArray::Literal;
using LiteralTag = panda_file::LiteralTag;

class ExcludedKeysTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    void EmitAndOptimize(const std::string &abc_file_name, panda::pandasm::Program &program) const
    {
        panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps panda_file_to_asm_maps {};
        EXPECT_TRUE(panda::pandasm::AsmEmitter::Emit(abc_file_name, program, nullptr, &panda_file_to_asm_maps, false));
        EXPECT_TRUE(panda::bytecodeopt::OptimizeBytecode(&program, &panda_file_to_asm_maps, abc_file_name, true));
    }
};

HWTEST_F(ExcludedKeysTest, excluded_keys_001, testing::ext::TestSize.Level1)
{
    /* js source code
    function foo()
    {
        let {a, b, c, ...d} = {a:'a', b:'b', c:'c', f:'f', e:'e'}
    }
    */
    const auto source = R"(
        .language ECMAScript
        .function any foo(any a0, any a1, any a2) {
            mov v0, a0
            mov v1, a1
            mov v2, a2
            ldundefined
            sta v3
            lda v3
            sta v3
            sta v4
            lda v4
            ldobjbyname 0x1, "a"
            sttoglobalrecord 0x3, "a"
            lda v4
            ldobjbyname 0x4, "b"
            sttoglobalrecord 0x6, "b"
            lda v4
            ldobjbyname 0x7, "c"
            sttoglobalrecord 0x9, "c"
            lda.str "a"
            sta v9
            lda.str "b"
            sta v10
            lda.str "c"
            sta v11
            createobjectwithexcludedkeys 0x2, v4, v9
            sttoglobalrecord 0xa, "d"
            lda v3
            returnundefined
        }
    )";
    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &program = res.Value();
    const std::string fun_name = "foo:(any,any,any)";
    auto it = program.function_table.find(fun_name);
    EXPECT_NE(it, program.function_table.end());
    auto &func = it->second;
    for (auto &ins : func.ins) {
        std::cerr << ins->ToString() << std::endl;
    }

    EmitAndOptimize("ExcludedKeysTest1.abc", program);

    // Collect valid instructions
    std::vector<const panda::pandasm::Ins *> insns;
    for (const auto &in : func.ins) {
        if (in->opcode != panda::pandasm::Opcode::INVALID) {
            insns.emplace_back(in.get());
        }
    }

    // Expected instructions after optimization in binary file
    const auto expected = R"(
        .language ECMAScript
        .function any foo(any a0, any a1, any a2) {
            ldundefined
            sta v0
            ldobjbyname 0x1, "a"
            sttoglobalrecord 0x3, "a"
            lda v0
            ldobjbyname 0x4, "b"
            sttoglobalrecord 0x6, "b"
            lda v0
            ldobjbyname 0x7, "c"
            sttoglobalrecord 0x9, "c"
            lda.str "a"
            sta v1
            lda.str "b"
            sta v2
            lda.str "c"
            sta v3
            mov v4, v0
            mov v5, v1
            mov v6, v2
            mov v7, v3
            createobjectwithexcludedkeys 0x2, v4, v5
            sttoglobalrecord 0xa, "d"
            returnundefined
        }
    )";
    panda::pandasm::Parser parser1;
    auto res1 = parser1.Parse(expected);
    auto &program1 = res1.Value();
    auto it1 = program1.function_table.find(fun_name);
    EXPECT_NE(it1, program1.function_table.end());
    auto &expected_func = it1->second;

    // Compare
    for (size_t i = 0; i < insns.size(); i++) {
        const auto &opt_ins = insns[i];
        const auto &expected_ins = expected_func.ins[i];
        EXPECT_EQ(opt_ins->opcode, expected_ins->opcode);

        EXPECT_EQ(opt_ins->Imms().size(), expected_ins->Imms().size());
        for (size_t k = 0; k < opt_ins->Imms().size(); k++) {
            EXPECT_EQ(opt_ins->Imms()[k], expected_ins->Imms()[k]);
        }

        EXPECT_EQ(opt_ins->Ids().size(), expected_ins->Ids().size());
        for (size_t k = 0; k < opt_ins->Ids().size(); k++) {
            EXPECT_EQ(opt_ins->Ids()[k], expected_ins->Ids()[k]);
        }
    }
}
}  // namespace panda::bytecodeopt
