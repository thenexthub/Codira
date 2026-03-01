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

namespace ark::disasm::test {

TEST(TestDebugInfo, TestDebugInfo)
{
    auto program = ark::pandasm::Parser().Parse(R"(
.record A {
    u1 fld
}

.function A A.init() {
    newobj v0, A
    lda.obj v0

    return.obj
}

.function u1 g() {
    mov v0, v1
    mov.64 v2, v3
    mov.obj v4, v5

    movi v0, -1
    movi.64 v0, 2
    fmovi.64 v0, 3.01

    lda v1
    lda.64 v0
    lda.obj v1
}
    )");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.CollectInfo();
    d.Serialize(ss, true, true);

    std::string bodyG = ExtractFuncBody(ss.str(), "g() <static> {");

    ASSERT_NE(bodyG.find("#   LINE_NUMBER_TABLE:"), std::string::npos);
    ASSERT_NE(bodyG.find("#\tline 14: 0\n"), std::string::npos);

    size_t codeStart = bodyG.find("#   CODE:\n");
    ASSERT_NE(codeStart, std::string::npos) << "Code section in function g not found";
    size_t codeEnd = bodyG.find("\n\n");  // First gap in function body is code section end
    ASSERT_NE(codeEnd, std::string::npos) << "Gap after code section in function g not found";
    ASSERT_LT(codeStart, codeEnd);
    std::string instructions =
        bodyG.substr(codeStart + strlen("#   CODE:\n"), codeEnd + 1 - (codeStart + strlen("#   CODE:\n")));

    std::regex const instRegex("# offset: ");
    std::ptrdiff_t const instructionCount(std::distance(
        std::sregex_iterator(instructions.begin(), instructions.end(), instRegex), std::sregex_iterator()));

    const ark::disasm::ProgInfo &progInfo = d.GetProgInfo();
    auto gIt = progInfo.methodsStaticInfo.find("g:()");
    ASSERT_NE(gIt, progInfo.methodsStaticInfo.end());
    // In case of pandasm the table should contain entry on each instruction
    ASSERT_EQ(gIt->second.lineNumberTable.size(), instructionCount);

    // There should be no local variables for panda assembler
    ASSERT_EQ(bodyG.find("#   LOCAL_VARIABLE_TABLE:"), std::string::npos);
}

}  // namespace ark::disasm::test
