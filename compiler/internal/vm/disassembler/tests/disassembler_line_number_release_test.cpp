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
class DisasmReleaseTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
* @tc.name: disassembler_line_release_test_001
* @tc.desc: Check abc file line number function in release.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisasmReleaseTest, disassembler_line_release_test_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "line_number_release.abc";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    disasm.CollectInfo();
    // The known line number in the abc file
    std::vector<size_t> expectedLineNumber = {17, 18, 20, 21, 22, 24, 25, 26, 28, 29, 30, 32, 33, 34, 36, 37, 38, 40,
                                              41, 42, 44, 45, 46, 48, 49, 50, 52, 53, 54, 56, 57, 58, 60, 61, 62, 64,
                                              66, -1};
    std::vector<size_t> lineNumber = disasm.GetLineNumber();
    ASSERT_EQ(expectedLineNumber.size(), lineNumber.size());
    for (size_t i = 0; i < lineNumber.size(); ++i) {
        std::cout << lineNumber[i] << std::endl;
        EXPECT_EQ(expectedLineNumber[i], lineNumber[i]);
    }
}

}