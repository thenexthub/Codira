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
#include "assembly-parser.h"
#include "disasm_backed_debug_info_extractor.h"

namespace ark::disasm::test {

TEST(ExtractorTest, LineNumberTable)
{
    auto program = ark::pandasm::Parser().Parse(R"(
        .function void func() {

            nop

        label:

            nop

            jmp label

            return
        }
    )");
    ASSERT(program);

    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::panda_file::File::EntityId methodId;
    std::string_view sourceName;
    ark::disasm::DisasmBackedDebugInfoExtractor extractor(*pf, [&methodId, &sourceName](auto id, auto sn) {
        methodId = id;
        sourceName = sn;
    });

    auto idList = extractor.GetMethodIdList();
    ASSERT_EQ(idList.size(), 1);

    auto id = idList[0];
    ASSERT_NE(extractor.GetSourceCode(id), nullptr);

    ASSERT_EQ(id, methodId);
    ASSERT_EQ(extractor.GetSourceFile(id), sourceName);

    auto lineTable = extractor.GetLineNumberTable(id);
    ASSERT_EQ(lineTable.size(), 4);

    ASSERT_EQ(lineTable[0].offset, 0);
    ASSERT_EQ(lineTable[0].line, 2);

    ASSERT_EQ(lineTable[1].offset, 1);
    ASSERT_EQ(lineTable[1].line, 4);

    ASSERT_EQ(lineTable[2].offset, 2);
    ASSERT_EQ(lineTable[2].line, 5);

    ASSERT_EQ(lineTable[3].offset, 4);
    ASSERT_EQ(lineTable[3].line, 6);
}

}  // namespace ark::disasm::test
