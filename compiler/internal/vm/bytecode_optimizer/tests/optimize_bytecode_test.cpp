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
#include "compiler/optimizer/ir/basicblock.h"
#include "graph_test.h"
#include "mem/pool_manager.h"
#include "optimize_bytecode.h"

using namespace testing::ext;

namespace panda::bytecodeopt {
using namespace compiler;
using namespace panda::pandasm;

class OptimizeBytecodeTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    void EmitAndOptimize(const std::string &abc_file_name, panda::pandasm::Program &program) const
    {
        panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps panda_file_to_asm_maps {};
        EXPECT_TRUE(panda::pandasm::AsmEmitter::Emit(abc_file_name, program, nullptr, &panda_file_to_asm_maps, false));
        EXPECT_TRUE(panda::bytecodeopt::OptimizeBytecode(&program, &panda_file_to_asm_maps, abc_file_name, true));
    }
};

/**
 * @tc.name: optimize_bytecode_test_001
 * @tc.desc: Verify the OptimizeBytecode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizeBytecodeTest, optimize_bytecode_test_001, TestSize.Level1) {
    const auto source = R"(
        .language ECMAScript
        .array array_unsigned_byte u8 3 { 1 2 3 }
        .record Asm4 { i32 asm1 }
        .function any func_main_0(any a0, any a1, any a2) <static> {
            ldundefined
            stglobalvar 0x0, "j"
            ldundefined
            stglobalvar 0x1, "n"
            lda.str "a b c"
            stglobalvar 0x2, "n"
            ldglobalvar 0x3, "n"
            getpropiterator
            sta v0
        jump_label_1:
            getnextpropname v0
            sta v1
            ldundefined
            eq 0x4, v1
            jnez jump_label_0
            lda v1
            stglobalvar 0x5, "j"
            tryldglobalbyname 0x6, "console"
            sta v1
            ldobjbyname 0x7, "log"
            sta v2
            ldglobalvar 0x9, "n"
            sta v3
            ldglobalvar 0xa, "j"
            ldobjbyvalue 0xb, v3
            sta v3
            lda v2
            callthis1 0xd, v1, v3
            jmp jump_label_1
        jump_label_0:
            ldundefined
            returnundefined
        }
    )";

    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &program = res.Value();
    const std::string fun_name = "func_main_0:(any,any,any)";
    auto it = program.function_table.find(fun_name);
    EXPECT_NE(it, program.function_table.end());
    EmitAndOptimize("codegenTryCatch.abc", program);
}

/**
 * @tc.name: optimize_bytecode_test_002
 * @tc.desc: Verify the OptimizeBytecode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizeBytecodeTest, optimize_bytecode_test_002, TestSize.Level1) {
    const auto source = R"(
        .language ECMAScript
        .array array_unsigned_byte u8 3 { 1 2 3 }
        .record Asm4 { i32 asm1 }
        .function any func_main_0(any a0, any a1, any a2) <static> {
            ldundefined
            stglobalvar 0x0, "j"
            ldundefined
            stglobalvar 0x1, "n"
            lda.str "a b c"
            stglobalvar 0x2, "n"
            ldglobalvar 0x3, "n"
            getpropiterator
            sta v0
        jump_label_1:
            getnextpropname v0
            sta v1
            ldundefined
            eq 0x4, v1
            jnez jump_label_0
            lda v1
            stglobalvar 0x5, "j"
            tryldglobalbyname 0x6, "console"
            sta v1
            ldobjbyname 0x7, "log"
            sta v2
            ldglobalvar 0x9, "n"
            sta v3
            ldglobalvar 0xa, "j"
            ldobjbyvalue 0xb, v3
            sta v3
            lda v2
            callthis1 0xd, v1, v3
            jmp jump_label_1
        jump_label_0:
            ldundefined
            returnundefined
        }
    )";
    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &program = res.Value();
    const std::string fun_name = "func_main_0:(any,any,any)";
    auto it = program.function_table.find(fun_name);

    EXPECT_NE(it, program.function_table.end());
    const std::string abc_file_name = "codegenTryCatch.abc";

    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps panda_file_to_asm_maps {};
    EXPECT_TRUE(panda::pandasm::AsmEmitter::Emit(abc_file_name, program, nullptr, &panda_file_to_asm_maps, false));
}

/**
 * @tc.name: optimize_bytecode_test_003
 * @tc.desc: Verify the OptimizeBytecode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizeBytecodeTest, optimize_bytecode_test_003, TestSize.Level1) {
    const auto source = R"(
        .language ECMAScript
        .array array_unsigned_byte u8 3 { 1 2 3 }
        .record Asm4 { i32 asm1 }
        .function any func_main_0(any a0, any a1, any a2) <static> {
            ldundefined
            stglobalvar 0x0, "j"
            ldundefined
            stglobalvar 0x1, "n"
            lda.str "a b c"
            stglobalvar 0x2, "n"
            ldglobalvar 0x3, "n"
            getpropiterator
            sta v0
        jump_label_1:
            getnextpropname v0
            sta v1
            ldundefined
            eq 0x4, v1
            jnez jump_label_0
            lda v1
            stglobalvar 0x5, "j"
            tryldglobalbyname 0x6, "console"
            sta v1
            ldobjbyname 0x7, "log"
            sta v2
            ldglobalvar 0x9, "n"
            sta v3
            ldglobalvar 0xa, "j"
            ldobjbyvalue 0xb, v3
            sta v3
            lda v2
            callthis1 0xd, v1, v3
            jmp jump_label_1
        jump_label_0:
            ldundefined
            returnundefined
        }
    )";

    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &program = res.Value();
    const std::string fun_name = "func_main_0:(any,any,any)";
    auto it = program.function_table.find(fun_name);

    Function::CatchBlock cat;
    cat.try_end_label = "random";  // random: It's a random string
    it->second.catch_blocks.push_back(cat);
    EXPECT_NE(it, program.function_table.end());

    const std::string abc_file_name = "codegenTryCatch.abc";

    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps panda_file_to_asm_maps {};
    EXPECT_TRUE(panda::pandasm::AsmEmitter::Emit(abc_file_name, program, nullptr, &panda_file_to_asm_maps, false));
    EXPECT_TRUE(panda::bytecodeopt::OptimizeBytecode(&program, &panda_file_to_asm_maps, abc_file_name, true));
}
}  // namespace panda::bytecodeopt