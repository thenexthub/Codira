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
#include <string>
#include "disassembler.h"

using namespace testing::ext;

namespace panda::disasm {
class DisasmTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
* @tc.name: disassembler_line_number_test_001
* @tc.desc: Check abc file line number function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisasmTest, disassembler_line_number_test_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "line-number1.abc";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    disasm.CollectInfo();
    // The known line number in the abc file
    std::vector<size_t> expectedLineNumber = {-1, 15, -1, 15, -1};
    std::vector<size_t> lineNumber = disasm.GetLineNumber();
    ASSERT_EQ(expectedLineNumber.size(), lineNumber.size());
    for (size_t i = 0; i < lineNumber.size(); ++i) {
        EXPECT_EQ(expectedLineNumber[i], lineNumber[i]);
    }
}

/**
* @tc.name: disassembler_line_number_test_002
* @tc.desc: Check abc file line number function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisasmTest, disassembler_line_number_test_002, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "line-number2.abc";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    disasm.CollectInfo();
    // The known line number in the abc file
    std::vector<size_t> expectedLineNumber = {-1, 25, 26, 27, -1, 29, -1, 17,
                                              18, 19, -1, 21, -1, 15, -1};
    std::vector<size_t> lineNumber = disasm.GetLineNumber();
    ASSERT_EQ(expectedLineNumber.size(), lineNumber.size());
    for (size_t i = 0; i < lineNumber.size(); ++i) {
        EXPECT_EQ(expectedLineNumber[i], lineNumber[i]);
    }
}

/**
* @tc.name: disassembler_line_number_test_003
* @tc.desc: Check abc file line number function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisasmTest, disassembler_line_number_test_003, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "line-number3.abc";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    disasm.CollectInfo();
    // The known line number in the abc file
    std::vector<size_t> expectedLineNumber = {-1, 16, -1, 17, 15, 17, -1};
    std::vector<size_t> lineNumber = disasm.GetLineNumber();
    ASSERT_EQ(expectedLineNumber.size(), lineNumber.size());
    for (size_t i = 0; i < lineNumber.size(); ++i) {
        EXPECT_EQ(expectedLineNumber[i], lineNumber[i]);
    }
}

/**
* @tc.name: disassembler_line_number_test_004
* @tc.desc: Check abc file line number function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisasmTest, disassembler_line_number_test_004, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "line-number4.abc";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    disasm.CollectInfo();
    // The known line number in the abc file
    std::vector<size_t> expectedLineNumber = {-1, 16, 17, 18, 20, 18, -1, 20, 15, -1};
    std::vector<size_t> lineNumber = disasm.GetLineNumber();
    ASSERT_EQ(expectedLineNumber.size(), lineNumber.size());
    for (size_t i = 0; i < lineNumber.size(); ++i) {
        EXPECT_EQ(expectedLineNumber[i], lineNumber[i]);
    }
}

/**
* @tc.name: disassembler_line_number_test_005
* @tc.desc: Check abc file line number function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisasmTest, disassembler_line_number_test_005, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "line-number5.abc";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    disasm.CollectInfo();
    // The known line number in the abc file
    std::vector<size_t> expectedLineNumber = {-1, 16, 17, 18, 19, 21, 19, -1, 21, 15, 21, -1};
    std::vector<size_t> lineNumber = disasm.GetLineNumber();
    ASSERT_EQ(expectedLineNumber.size(), lineNumber.size());
    for (size_t i = 0; i < lineNumber.size(); ++i) {
        EXPECT_EQ(expectedLineNumber[i], lineNumber[i]);
    }
}
}