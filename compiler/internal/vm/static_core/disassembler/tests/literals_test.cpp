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

static inline std::string ExtractArrayBody(const std::string &text, const std::string &header)
{
    auto beg = text.find(header);
    auto end = text.find('}', beg);

    ASSERT(beg != std::string::npos);
    ASSERT(end != std::string::npos);

    return text.substr(beg + header.length(), end - (beg + header.length()));
}

namespace ark::disasm::test {

TEST(LiteralsTest, LiteralsTestNames)
{
    auto program = ark::pandasm::Parser().Parse(R"(
.array array0 i32 3 { 2 3 4}
.array array1 i32 3 { 2 3 4}
.array array2 i32 3 { 2 3 4}
.array array3 i32 3 { 2 3 4}
    )");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    EXPECT_TRUE(ss.str().find(".language PandaAssembly") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".array array_0 i32 3 { 2 3 4 }") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".array array_1 i32 3 { 2 3 4 }") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".array array_2 i32 3 { 2 3 4 }") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".array array_3 i32 3 { 2 3 4 }") != std::string::npos);
}

TEST(LiteralsTest, LiteralsTestValues)
{
    auto program = ark::pandasm::Parser().Parse(R"(
.record panda.String <external>

.array array0 panda.String 3 { "a" "ab" "abc"}
.array array1 u1 3 { 0 1 0}
.array array2 u8 3 { 1 2 8}
.array array3 i8 3 { 1 -30 8}
.array array4 i32 3 { 2 -300 4}
.array array5 f32 3 { 5.1 6.2 -122.345 }
    )");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    EXPECT_TRUE(ss.str().find(".language PandaAssembly") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".array array_0 panda.String 3 { \"a\" \"ab\" \"abc\" }") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".array array_1 u1 3 { 0 1 0 }") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".array array_2 u8 3 { 1 2 8 }") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".array array_3 i8 3 { 1 -30 8 }") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".array array_4 i32 3 { 2 -300 4 }") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".array array_5 f32 3 { 5.1 6.2 -122.345 }") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record panda.String <external>") != std::string::npos);
}

static std::string GetDynamicVeluesSource()
{
    return R"(
.record panda.String <external>

.array array {
    u1 1
    u8 2
    i8 -30
    u16 400
    i16 -5000
    u32 60000
    i32 -70000
    u64 800000
    i64 -900000
    f32 -122.345
    f64 -1.12345
    panda.String "panda.String"
}
)";
}

TEST(LiteralsTest, LiteralsTestDynamicValues)
{
    auto program = ark::pandasm::Parser().Parse(GetDynamicVeluesSource());
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    auto arrayBody = ExtractArrayBody(ss.str(), ".array array_0 {\n");

    std::string line;
    std::stringstream arr {arrayBody};
    std::getline(arr, line);
    EXPECT_EQ("\tu1 1", line);
    std::getline(arr, line);
    EXPECT_EQ("\ti32 2", line);
    std::getline(arr, line);
    EXPECT_EQ("\ti32 -30", line);
    std::getline(arr, line);
    EXPECT_EQ("\ti32 400", line);
    std::getline(arr, line);
    EXPECT_EQ("\ti32 -5000", line);
    std::getline(arr, line);
    EXPECT_EQ("\ti32 60000", line);
    std::getline(arr, line);
    EXPECT_EQ("\ti32 -70000", line);
    std::getline(arr, line);
    EXPECT_EQ("\ti64 800000", line);
    std::getline(arr, line);
    EXPECT_EQ("\ti64 -900000", line);
    std::getline(arr, line);
    EXPECT_EQ("\tf32 -122.345", line);
    std::getline(arr, line);
    EXPECT_EQ("\tf64 -1.12345", line);
    std::getline(arr, line);
    EXPECT_EQ("\tpanda.String \"panda.String\"", line);
}

TEST(LiteralsTest, LiteralsTestEscapeValue)
{
    auto program = ark::pandasm::Parser().Parse(R"(
.record panda.String <external>

.array array0 panda.String 3 { "\t\t" "\n\f\n" "\\\n\v" }
.array array1 panda.String 1 { "\n" }
.array array2 panda.String 4 { "\a\t" "\n" "\t" "\n\t\r\b\t\\\t\n" }
    )");

    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    EXPECT_TRUE(ss.str().find(".language PandaAssembly") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".array array_0 panda.String 3 { \"\\t\\t\" \"\\n\\f\\n\" \"\\\\\\n\\v\" }") !=
                std::string::npos);
    EXPECT_TRUE(ss.str().find(".array array_1 panda.String 1 { \"\\n\" }") != std::string::npos);
    EXPECT_TRUE(
        ss.str().find(".array array_2 panda.String 4 { \"\\a\\t\" \"\\n\" \"\\t\" \"\\n\\t\\r\\b\\t\\\\\\t\\n\" }") !=
        std::string::npos);
}

#undef DISASM_BIN_DIR

}  // namespace ark::disasm::test
