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
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "disassembler.h"

using namespace testing::ext;
namespace panda::disasm {

static const std::string SOURCE_FILE_PATH = GRAPH_TEST_ABC_DIR "line-number1.abc";
static const std::string FILE_NAME = "line-number1.abc";
static const std::string CHECK_MESSAGE = "# source binary: " + FILE_NAME;

class DisassemblerGetFileNameTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        std::filesystem::path file_name{ FILE_NAME };
        std::filesystem::path src_file_name{ SOURCE_FILE_PATH };
    };

    static void TearDownTestCase(void) {};
    
    void SetUp() {};
    void TearDown() {};

    bool Find(std::stringstream &ss, const std::string &dst)
    {
        return ss.str().find(dst) != std::string::npos;
    }
};

/**
* @tc.name: disassembler_getfilename_test_001
* @tc.desc: test disasm with relative path.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerGetFileNameTest, disassembler_getfilename_test_001, TestSize.Level1)
{
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(SOURCE_FILE_PATH, false, false);
    std::stringstream ss {};
    disasm.Serialize(ss);
    EXPECT_TRUE(Find(ss, CHECK_MESSAGE));
}

/**
* @tc.name: disassembler_getfilename_test_003
* @tc.desc: test disasm with absolute path.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerGetFileNameTest, disassembler_getfilename_test_003, TestSize.Level1)
{
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(std::filesystem::absolute(SOURCE_FILE_PATH).u8string(), false, false);
    std::stringstream ss {};
    disasm.Serialize(ss);
    EXPECT_TRUE(Find(ss, CHECK_MESSAGE));
}
};
