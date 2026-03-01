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

#include <iostream>
#include <string>

#include <gtest/gtest.h>
#include "disassembler.h"
#include "assembly-parser.h"

static inline std::string ExtractFuncBody(const std::string &text, const std::string &header)
{
    auto beg = text.find(header);
    auto end = text.find('}', beg);

    ASSERT(beg != std::string::npos);
    ASSERT(end != std::string::npos);

    return text.substr(beg + header.length(), end - (beg + header.length()));
}

static std::string GetStrWithExceptions()
{
    return R"(
.record A {}
.record A_exception <external>

.function A A_exception.getMessage(A_exception a0) <external>

.function u1 main() {
    movi v0, 1
try_begin:
    ldai 1
    return
try_end:
    ldai 3
catch_block1_begin:
    call.virt A_exception.getMessage, v0

    return
catch_block1_end:
    ldai 6
catch_block2_begin:
    ldai 7
    return

.catch A_exception, try_begin, try_end, catch_block1_begin, catch_block1_end
.catchall try_begin, try_end, catch_block2_begin
}
    )";
}

static std::string GetTest2Source()
{
    return R"(
.function u1 g() {
    label_0:
    movi v0, 0
    label_1:
    movi v0, 1
    label_2:
    movi v0, 2
    label_3:
    movi v0, 3
    label_4:
    movi v0, 4
    label_5:
    movi v0, 5
    label_6:
    movi v0, 6
    label_7:
    movi v0, 7

    jmp label_0
    jmp label_7
    jmp label_1
    jmp label_6
    jmp label_2
    jmp label_5
    jmp label_3
    jmp label_4

    return
}
)";
}

namespace ark::disasm::test {

TEST(LabelTest, test1)
{
    auto program = ark::pandasm::Parser().Parse(R"(
.function u1 g() {
    start:
    jmp start

    return
}

.function u1 gg() {
    jmp start
    start:

    return
}
    )");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string bodyG = ExtractFuncBody(ss.str(), "u1 g() <static> {\n");
    std::string bodyGg = ExtractFuncBody(ss.str(), "u1 gg() <static> {\n");
    EXPECT_EQ(bodyG, "jump_label_0:\n\tjmp jump_label_0\n\treturn\n");
    EXPECT_EQ(bodyGg, "\tjmp jump_label_0\njump_label_0:\n\treturn\n");
}

static void CheckTest2(std::stringstream &g, std::string &line)
{
    std::getline(g, line);
    EXPECT_EQ("jump_label_0:", line);
    std::getline(g, line);
    EXPECT_EQ("\tmovi v0, 0x0", line);
    std::getline(g, line);
    EXPECT_EQ("jump_label_2:", line);
    std::getline(g, line);
    EXPECT_EQ("\tmovi v0, 0x1", line);
    std::getline(g, line);
    EXPECT_EQ("jump_label_4:", line);
    std::getline(g, line);
    EXPECT_EQ("\tmovi v0, 0x2", line);
    std::getline(g, line);
    EXPECT_EQ("jump_label_6:", line);
    std::getline(g, line);
    EXPECT_EQ("\tmovi v0, 0x3", line);
    std::getline(g, line);
    EXPECT_EQ("jump_label_7:", line);
    std::getline(g, line);
    EXPECT_EQ("\tmovi v0, 0x4", line);
    std::getline(g, line);
    EXPECT_EQ("jump_label_5:", line);
    std::getline(g, line);
    EXPECT_EQ("\tmovi v0, 0x5", line);
    std::getline(g, line);
    EXPECT_EQ("jump_label_3:", line);
    std::getline(g, line);
    EXPECT_EQ("\tmovi v0, 0x6", line);
    std::getline(g, line);
    EXPECT_EQ("jump_label_1:", line);
    std::getline(g, line);
    EXPECT_EQ("\tmovi v0, 0x7", line);
    std::getline(g, line);
    EXPECT_EQ("\tjmp jump_label_0", line);
    std::getline(g, line);
    EXPECT_EQ("\tjmp jump_label_1", line);
    std::getline(g, line);
    EXPECT_EQ("\tjmp jump_label_2", line);
    std::getline(g, line);
    EXPECT_EQ("\tjmp jump_label_3", line);
    std::getline(g, line);
    EXPECT_EQ("\tjmp jump_label_4", line);
    std::getline(g, line);
    EXPECT_EQ("\tjmp jump_label_5", line);
    std::getline(g, line);
    EXPECT_EQ("\tjmp jump_label_6", line);
    std::getline(g, line);
    EXPECT_EQ("\tjmp jump_label_7", line);
}

TEST(LabelTest, test2)
{
    auto program = ark::pandasm::Parser().Parse(GetTest2Source());
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string line;
    std::string bodyG = ExtractFuncBody(ss.str(), "g() <static> {\n");
    std::stringstream g {bodyG};
    CheckTest2(g, line);
}

TEST(LabelTest, TestExceptions)
{
    auto program = ark::pandasm::Parser().Parse(GetStrWithExceptions());
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string bodyMain = ExtractFuncBody(ss.str(), "main() <static> {\n");

    std::string line;
    std::stringstream main {bodyMain};
    std::getline(main, line);
    EXPECT_EQ("\tmovi v0, 0x1", line);
    std::getline(main, line);
    EXPECT_EQ("try_begin_label_0:", line);
    std::getline(main, line);
    EXPECT_EQ("\tldai 0x1", line);
    std::getline(main, line);
    EXPECT_EQ("\treturn", line);
    std::getline(main, line);
    EXPECT_EQ("try_end_label_0:", line);
    std::getline(main, line);
    EXPECT_EQ("\tldai 0x3", line);
    std::getline(main, line);
    EXPECT_EQ("handler_begin_label_0_0:", line);
    std::getline(main, line);
    EXPECT_EQ("\tcall.virt.short A_exception.getMessage:(A_exception), v0", line);
    std::getline(main, line);
    EXPECT_EQ("\treturn", line);
    std::getline(main, line);
    EXPECT_EQ("handler_end_label_0_0:", line);
    std::getline(main, line);
    EXPECT_EQ("\tldai 0x6", line);
    std::getline(main, line);
    EXPECT_EQ("handler_begin_label_0_1:", line);
    std::getline(main, line);
    EXPECT_EQ("\tldai 0x7", line);
    std::getline(main, line);
    EXPECT_EQ("\treturn", line);
    std::getline(main, line);
    std::getline(main, line);
    EXPECT_EQ(".catch A_exception, try_begin_label_0, try_end_label_0, handler_begin_label_0_0, handler_end_label_0_0",
              line);
    std::getline(main, line);
    EXPECT_EQ(".catchall try_begin_label_0, try_end_label_0, handler_begin_label_0_1", line);
}

#undef DISASM_BIN_DIR

}  // namespace ark::disasm::test
