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

#include <cstdlib>
#include "disassembler.h"
#include <gtest/gtest.h>
#include <string>

using namespace testing::ext;
namespace panda::disasm {

static const std::string ANNOTATION_TEST_FILE_NAME_001 = GRAPH_TEST_ABC_DIR "script-string1.abc";
static const std::string ANNOTATION_TEST_FILE_NAME_002 = GRAPH_TEST_ABC_DIR "script-string2.abc";

class DisassemblerStringTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    bool ValidateString(const std::vector<std::string> &strings,
                        const std::vector<std::string> &expected_strings)
    {
        for (const auto &expected_string : expected_strings) {
            const auto string_iter = std::find(strings.begin(), strings.end(), expected_string);
            if (string_iter == strings.end()) {
                return false;
            }
        }

        return true;
    }
};

/**
* @tc.name: disassembler_string_test_001
* @tc.desc: get existed string.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerStringTest, disassembler_string_test_001, TestSize.Level1)
{
    std::vector<std::string> expected_strings = {"ClassA", "prototype", "str", "test"};
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(ANNOTATION_TEST_FILE_NAME_001, false, false);
    std::vector<std::string> strings = disasm.GetStrings();
    bool has_string = ValidateString(strings, expected_strings);
    EXPECT_TRUE(has_string);
}

/**
* @tc.name: disassembler_string_test_002
* @tc.desc: get not existed string.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerStringTest, disassembler_string_test_002, TestSize.Level1)
{
    std::vector<std::string> expected_strings = {"Student", "name", "prototype", "student_name"};
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(ANNOTATION_TEST_FILE_NAME_002, false, false);
    std::vector<std::string> strings = disasm.GetStrings();
    bool has_string = ValidateString(strings, expected_strings);
    EXPECT_TRUE(has_string);
}

};
