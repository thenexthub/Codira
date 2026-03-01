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
#include <string>

#include <gtest/gtest.h>
#include "assembly-parser.h"
#include "disassembler.h"

static inline std::string ExtractRecordBody(const std::string &text, const std::string &header)
{
    auto beg = text.find(header);
    auto end = text.find('}', beg);

    ASSERT(beg != std::string::npos);
    ASSERT(end != std::string::npos);

    return text.substr(beg + header.length(), end - (beg + header.length()));
}

namespace ark::disasm::test {

TEST(RecordTest, EmptyRecord)
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

    std::string line;
    std::getline(ss, line);
    EXPECT_EQ(".language PandaAssembly", line);
    std::getline(ss, line);
    std::getline(ss, line);
    EXPECT_EQ(".record A {", line);
    std::getline(ss, line);
    EXPECT_EQ("}", line);
}

static std::string GetRecordWithFieldsSource()
{
    return R"(
.record A {
    u1 a
    i8 b
    u8 c
    i16 d
    u16 e
    i32 f
    u32 g
    f32 h
    f64 i
    i64 j
    u64 k
}
)";
}

TEST(RecordTest, RecordWithFields)
{
    auto program = ark::pandasm::Parser().Parse(GetRecordWithFieldsSource());
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string line;
    std::getline(ss, line);
    EXPECT_EQ(".language PandaAssembly", line);
    std::getline(ss, line);
    std::getline(ss, line);
    EXPECT_EQ(".record A {", line);
    std::getline(ss, line);
    EXPECT_EQ("\tu1 a", line);
    std::getline(ss, line);
    EXPECT_EQ("\ti8 b", line);
    std::getline(ss, line);
    EXPECT_EQ("\tu8 c", line);
    std::getline(ss, line);
    EXPECT_EQ("\ti16 d", line);
    std::getline(ss, line);
    EXPECT_EQ("\tu16 e", line);
    std::getline(ss, line);
    EXPECT_EQ("\ti32 f", line);
    std::getline(ss, line);
    EXPECT_EQ("\tu32 g", line);
    std::getline(ss, line);
    EXPECT_EQ("\tf32 h", line);
    std::getline(ss, line);
    EXPECT_EQ("\tf64 i", line);
    std::getline(ss, line);
    EXPECT_EQ("\ti64 j", line);
    std::getline(ss, line);
    EXPECT_EQ("\tu64 k", line);
}

TEST(RecordTest, RecordWithRecord)
{
    auto program = ark::pandasm::Parser().Parse(R"(
.record A {
    i64 aw
}

.record B {
    A a
    i32 c
}
    )");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string bodyA = ExtractRecordBody(ss.str(), ".record A {\n");
    std::stringstream a {bodyA};
    std::string bodyB = ExtractRecordBody(ss.str(), ".record B {\n");
    std::stringstream b {bodyB};

    std::string line;
    std::getline(b, line);
    EXPECT_EQ("\tA a", line);
    std::getline(b, line);
    EXPECT_EQ("\ti32 c", line);
    std::getline(a, line);
    EXPECT_EQ("\ti64 aw", line);
}

TEST(RecordTest, TestAnyType_1)
{
    auto parse = ark::pandasm::Parser();
    auto program = parse.Parse(
        ".record Y <external>\n"
        ".record N <external>\n"
        ".record A {Y field <access.field=public>}\n"
        ".record B {N field <access.field=public>}\n");

    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    EXPECT_TRUE(ss.str().find("Y field <access.field=public>") != std::string::npos);
}

TEST(RecordTest, TestAnyType_2)
{
    auto parse = ark::pandasm::Parser();
    auto program = parse.Parse(
        ".record Y <external>\n"
        ".record N <external>\n"
        ".record A {\n"
        "    Y field <access.field=public>\n"
        "    Y field2 <access.field=public>\n"
        "}\n"
        ".record B {\n"
        "    N field <access.field=public>\n"
        "    N field2 <access.field=public>\n"
        "}\n");

    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    EXPECT_TRUE(ss.str().find("Y field <access.field=public>") != std::string::npos);
    EXPECT_TRUE(ss.str().find("Y field2 <access.field=public>") != std::string::npos);
    EXPECT_TRUE(ss.str().find("N field <access.field=public>") != std::string::npos);
    EXPECT_TRUE(ss.str().find("N field2 <access.field=public>") != std::string::npos);
}

TEST(RecordTest, TestAnyType_3)
{
    auto parse = ark::pandasm::Parser();
    auto program = parse.Parse(R"(
        .record Y <external>
        .record N <external>
        .record A {
            Y[] field <access.field=public>
        }
        .record B {
            N[] field <access.field=public>
        }
    )");

    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    EXPECT_TRUE(ss.str().find("Y[] field <access.field=public>") != std::string::npos);
    EXPECT_TRUE(ss.str().find("N[] field <access.field=public>") != std::string::npos);
}

TEST(RecordTest, DISABLED_TestUnionCanonicalization)
{
    auto program = ark::pandasm::Parser().Parse(
        ".record std.core.Object <external>\n"
        ".record A <extends=std.core.Object, access.record=public> {}\n"
        ".record B <extends=std.core.Object, access.record=public> {}\n"
        ".record C <extends=std.core.Object, access.record=public> {}\n"
        ".record D <extends=std.core.Object, access.record=public> {}\n"

        ".record {UC,C,B,C,B} <external>\n"
        ".record {UD,C,B,A} <external>\n"
        ".record {UA,A,A,B} <external>\n"
        ".record {UA,std.core.Double} <external>\n"
        ".record {Ustd.core.Double,std.core.Double,std.core.Long} <external>\n"
        ".record {Ustd.core.Double,std.core.Int,std.core.Long} <external>\n"
        ".record {UA,B,C,B,A} <external>\n"
        ".record {UA,D,D,D} <external>");

    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    EXPECT_TRUE(ss.str().find(".record {UB,C} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {UA,B,C,D} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {UA,B} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {UA,std.core.Double} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {Ustd.core.Double,std.core.Long} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {Ustd.core.Double,std.core.Int,std.core.Long} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {UA,B,C} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {UA,D} <external>") != std::string::npos);
}

TEST(RecordTest, DISABLED_TestUnionCanonicalizationArrays)
{
    auto program = ark::pandasm::Parser().Parse(
        ".record std.core.Object <external>\n"
        ".record A <extends=std.core.Object, access.record=public> {}\n"
        ".record B <extends=std.core.Object, access.record=public> {}\n"
        ".record C <extends=std.core.Object, access.record=public> {}\n"
        ".record D <extends=std.core.Object, access.record=public> {}\n"

        ".record {UB,C,{UA,D,std.core.Double}[],A} <external>\n"
        ".record {U{UD,A,std.core.Int}[],{UA,D,std.core.Double}[],{Ustd.core.Long,D,A}[]} <external>\n"
        ".record {UB,C,{UA,B}[],A} <external>\n"
        ".record {UB,{UA,D}[],{UD,B}[],{UC,B}[],{UC,D}[]} <external>\n"
        ".record {U{UA,D}[],{UD,B}[],{UD,B}[],{UC,D}[]} <external>\n"
        ".record {U{UA,D}[],{UD,B}[][],{UD,B}[],{UC,D}[]} <external>\n"
        ".record {U{UA,D}[][],{UD,B}[]} <external>\n"
        ".record {U{UD,C}[],{UC,B}[]} <external>\n"
        ".record {U{UD,C}[],C,B} <external>\n"
        ".record {U{Ustd.core.Int,std.core.Double}[],std.core.Long,std.core.Double} <external>\n"
        ".record {U{Ustd.core.Int[],std.core.Double[]}[],std.core.Long[],std.core.Double[]} <external>");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    EXPECT_TRUE(ss.str().find(".record {UA,B,C,{UA,B}[]} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {UA,B,C,{UA,D,std.core.Double}[]} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {UB,C,{UC,D}[]} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {UB,{UA,D}[],{UB,C}[],{UB,D}[],{UC,D}[]} <external>") != std::string::npos);
    EXPECT_TRUE(
        ss.str().find(".record {Ustd.core.Double,std.core.Long,{Ustd.core.Double,std.core.Int}[]} <external>") !=
        std::string::npos);
    EXPECT_TRUE(ss.str().find(
                    ".record {Ustd.core.Double[],std.core.Long[],{Ustd.core.Double[],std.core.Int[]}[]} <external>") !=
                std::string::npos);
    EXPECT_TRUE(
        ss.str().find(".record {U{UA,D,std.core.Double}[],{UA,D,std.core.Int}[],{UA,D,std.core.Long}[]} <external>") !=
        std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {U{UA,D}[],{UB,D}[],{UC,D}[]} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {U{UA,D}[],{UB,D}[],{UB,D}[][],{UC,D}[]} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {U{UA,D}[][],{UB,D}[]} <external>") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".record {U{UB,C}[],{UC,D}[]} <external>") != std::string::npos);
}

#undef DISASM_BIN_DIR

}  // namespace ark::disasm::test
