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

#include <iostream>
#include <regex>
#include <string>

#include <gtest/gtest.h>
#include "assembly-parser.h"
#include "disassembler.h"

static auto g_testCallsSource = R"(
.record B {}

.function u8 B.Bhandler_unspec(B a0) {
    return
}
.function u8 B.Bhandler_short(B a0, u1 a1) {
    return
}
.function u8 B.Bhandler_short2(B a0, u1[] a1, i64 a2) {
    return
}
.function u16 B.Bhandler_long(B a0, i8 a1, i16 a2, i32 a3) {
    return
}
.function u16 B.Bhandler_long2(B a0, i8 a1, i16 a2, i32 a3, i64 a4) {
    return
}
.function u16 B.Bhandler_range(B a0, i8 a1, i16 a2, i32 a3, i8 a4, i16 a5, i32 a6) {
    return
}
.function u16 long_function(i8 a0, i16 a1, i32 a2, i8 a3, i16 a4, i32 a5, i64 a6, f32 a7) {
    return
}

.function u8 handler_unspec() {
    return
}
.function u8 handler_short(u1 a0) {
    return
}
.function u8 handler_short2(u1 a0, i64 a1) {
    return
}
.function u16 handler_long(i8 a0, i16 a1, i32 a2) {
    return
}
.function u16 handler_long2(i8 a0, i16 a1, i32 a2, f64 a3) {
    return
}
.function u16 handler_range(i8 a0, i16 a1, i32 a2, i8 a3, i16 a4, i32 a5) {
    return
}

.function u8 handler_unspec_ext() <external>
.function u8 handler_short_ext(u1 a0) <external>
.function u8 handler_short2_ext(u1 a0, i64 a1) <external>
.function u16 handler_long_ext(i8 a0, i16 a1, i32 a2) <external>
.function u16 handler_long2_ext(i8 a0, i16 a1, i32 a2, f64 a3) <external>
.function u16 handler_range_ext(i8 a0, i16 a1, i32 a2, i8 a3, i16 a4, i32 a5) <external>

.function u1 g(u1 a0) {
    call.virt B.Bhandler_unspec, v4
    call.virt B.Bhandler_short, v4, v1
    call.virt B.Bhandler_short2, v4, v1, v2
    call.virt B.Bhandler_long, v4, v0, v1, v2
    call.virt.range B.Bhandler_range, v4
    call handler_unspec
    call.short handler_short, v1
    call.short handler_short2, v1, v2
    call handler_long, v0, v1, v2
    call handler_long2, v0, v1, v2, v3
    call.range handler_range, v0
    initobj B.Bhandler_unspec
    initobj.short B.Bhandler_short, v1
    initobj.short B.Bhandler_short2, v1, v2
    initobj B.Bhandler_long, v0, v1, v2
    initobj B.Bhandler_long2, v0, v1, v2, v3
    initobj.range B.Bhandler_range, v0
    call handler_unspec_ext
    call.short handler_short_ext, v1
    call.short handler_short2_ext, v1, v2
    call handler_long_ext, v0, v1, v2
    call handler_long2_ext, v0, v1, v2, v3
    call.range handler_range_ext, v0
    call.acc.short handler_short, v0, 0
    call.acc.short handler_short2, a0, 1
    return
}
)";

static std::string GetNewarrSource()
{
    return R"(
.function u1 g(u1 a0) {
    newarr v0, a0, u1[]
    newarr v0, a0, i8[]
    newarr v0, a0, u8[]
    newarr v0, a0, i16[]
    newarr v0, a0, u16[]
    newarr v0, a0, i32[]
    newarr v0, a0, u32[]
    newarr v0, a0, f32[]
    newarr v0, a0, f64[]
    newarr v0, a0, i64[]
    newarr v0, a0, u64[]

    ldai 0
    return
}
)";
}

static inline std::string ExtractFuncBody(const std::string &text, const std::string &header)
{
    auto beg = text.find(header);
    auto end = text.find('}', beg);

    ASSERT(beg != std::string::npos);
    ASSERT(end != std::string::npos);

    return text.substr(beg + header.length(), end - (beg + header.length()));
}

namespace ark::disasm::test {

TEST(InstructionsTest, TestLanguagePandaAssembly)
{
    auto program = ark::pandasm::Parser().Parse(R"(
        .record A {}
    )");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    EXPECT_TRUE(ss.str().find(".language PandaAssembly") != std::string::npos);
}

static void CheckTestCalls(std::stringstream &g, std::string &line)
{
    std::getline(g, line);
    EXPECT_EQ("\tcall.virt.range B.Bhandler_range:(B,i8,i16,i32,i8,i16,i32), v4", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.short <static> handler_unspec:()", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.short <static> handler_short:(u1), v1", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.short <static> handler_short2:(u1,i64), v1, v2", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall <static> handler_long:(i8,i16,i32), v0, v1, v2", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall <static> handler_long2:(i8,i16,i32,f64), v0, v1, v2, v3", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.range <static> handler_range:(i8,i16,i32,i8,i16,i32), v0", line);
    std::getline(g, line);
    EXPECT_EQ("\tinitobj.short B.Bhandler_unspec:(B)", line);
    std::getline(g, line);
    EXPECT_EQ("\tinitobj.short B.Bhandler_short:(B,u1), v1", line);
    std::getline(g, line);
    EXPECT_EQ("\tinitobj.short B.Bhandler_short2:(B,u1[],i64), v1, v2", line);
    std::getline(g, line);
    EXPECT_EQ("\tinitobj B.Bhandler_long:(B,i8,i16,i32), v0, v1, v2", line);
    std::getline(g, line);
    EXPECT_EQ("\tinitobj B.Bhandler_long2:(B,i8,i16,i32,i64), v0, v1, v2, v3", line);
    std::getline(g, line);
    EXPECT_EQ("\tinitobj.range B.Bhandler_range:(B,i8,i16,i32,i8,i16,i32), v0", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.short <static> handler_unspec_ext:()", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.short <static> handler_short_ext:(u1), v1", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.short <static> handler_short2_ext:(u1,i64), v1, v2", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall <static> handler_long_ext:(i8,i16,i32), v0, v1, v2", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall <static> handler_long2_ext:(i8,i16,i32,f64), v0, v1, v2, v3", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.range <static> handler_range_ext:(i8,i16,i32,i8,i16,i32), v0", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.acc.short <static> handler_short:(u1), v0, 0x0", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.acc.short <static> handler_short2:(u1,i64), a0, 0x1", line);
}

TEST(InstructionsTest, TestCalls)
{
    auto program = ark::pandasm::Parser().Parse(g_testCallsSource);
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    EXPECT_TRUE(
        ss.str().find(".function u16 long_function(i8 a0, i16 a1, i32 a2, i8 a3, i16 a4, i32 a5, i64 a6, f32 a7)") !=
        std::string::npos);

    std::string bodyG = ExtractFuncBody(ss.str(), "g(u1 a0) <static> {\n");

    std::string line;
    std::stringstream g {bodyG};

    std::getline(g, line);
    EXPECT_EQ("\tcall.virt.short B.Bhandler_unspec:(B), v4", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.virt.short B.Bhandler_short:(B,u1), v4, v1", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.virt B.Bhandler_short2:(B,u1[],i64), v4, v1, v2", line);
    std::getline(g, line);
    EXPECT_EQ("\tcall.virt B.Bhandler_long:(B,i8,i16,i32), v4, v0, v1, v2", line);

    CheckTestCalls(g, line);
}

TEST(InstructionsTest, TestNewarr)
{
    auto program = ark::pandasm::Parser().Parse(GetNewarrSource());
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string bodyG = ExtractFuncBody(ss.str(), "g(u1 a0) <static> {\n");

    std::string line;
    std::stringstream g {bodyG};
    std::getline(g, line);
    EXPECT_EQ("\tnewarr v0, a0, u1[]", line);
    std::getline(g, line);
    EXPECT_EQ("\tnewarr v0, a0, i8[]", line);
    std::getline(g, line);
    EXPECT_EQ("\tnewarr v0, a0, u8[]", line);
    std::getline(g, line);
    EXPECT_EQ("\tnewarr v0, a0, i16[]", line);
    std::getline(g, line);
    EXPECT_EQ("\tnewarr v0, a0, u16[]", line);
    std::getline(g, line);
    EXPECT_EQ("\tnewarr v0, a0, i32[]", line);
    std::getline(g, line);
    EXPECT_EQ("\tnewarr v0, a0, u32[]", line);
    std::getline(g, line);
    EXPECT_EQ("\tnewarr v0, a0, f32[]", line);
    std::getline(g, line);
    EXPECT_EQ("\tnewarr v0, a0, f64[]", line);
    std::getline(g, line);
    EXPECT_EQ("\tnewarr v0, a0, i64[]", line);
    std::getline(g, line);
    EXPECT_EQ("\tnewarr v0, a0, u64[]", line);
}

TEST(InstructionsTest, TestCorrectReg)
{
    auto program = ark::pandasm::Parser().Parse(R"(
.record panda.String <external>

.function panda.String handler(panda.String a0) <external>

.function panda.String main() {
    lda.str "string"
    call.acc.short handler, v0, 0x0
    return.obj
}

    )");

    ASSERT(program);
    ark::pandasm::Program *prog = &(program.Value());
    auto mainFunc = prog->functionStaticTable.find("main:()");
    mainFunc->second.regsNum = 0U;

    auto pf = ark::pandasm::AsmEmitter::Emit(*prog);
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string bodyMain = ExtractFuncBody(ss.str(), "main() <static> {\n");

    std::string line;
    std::stringstream main {bodyMain};
    std::getline(main, line);
    EXPECT_EQ("\tlda.str \"string\"", line);
    std::getline(main, line);
    EXPECT_EQ("\tcall.acc.short <static> handler:(panda.String), v0, 0x0", line);
    std::getline(main, line);
    EXPECT_EQ("\treturn.obj", line);
}

#undef DISASM_BIN_DIR

}  // namespace ark::disasm::test
