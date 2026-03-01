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
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>
#include "abc2program_driver.h"
#include "abc2program_test_utils.h"
#include "common/abc_file_utils.h"
#include "dump_utils.h"
#include "modifiers.h"

using namespace testing::ext;

namespace panda::abc2program {
const std::string TS_COMMON_SYNTAX_ABC_TEST_FILE_NAME = GRAPH_TEST_ABC_DIR "TsCommonSyntax.abc";
const std::string TS_COMMON_SYNTAX_DUMP_RESULT_FILE_NAME = GRAPH_TEST_ABC_DIR "TsCommonSyntax.txt";
const std::string TS_COMMON_SYNTAX_DUMP_EXPECTED_FILE_NAME =
    GRAPH_TEST_ABC_DUMP_DIR "debug-line-number/TsCommonSyntaxExpected.txt";

class Abc2ProgramLineNumberTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}

    void TearDown() {}

    Abc2ProgramDriver driver_;
    const pandasm::Program *prog_ = nullptr;
};

/**
 * @tc.name: abc2program_ts_release_number_test_dump
 * @tc.desc: check dump result in release mode.
 * @tc.type: FUNC
 * @tc.require: IADG92
 */
HWTEST_F(Abc2ProgramLineNumberTest, abc2program_ts_line_number_common_syntax_test, TestSize.Level1)
{
    driver_.Compile(TS_COMMON_SYNTAX_ABC_TEST_FILE_NAME);
    driver_.Dump(TS_COMMON_SYNTAX_DUMP_RESULT_FILE_NAME);
    EXPECT_TRUE(Abc2ProgramTestUtils::ValidateDumpResult(TS_COMMON_SYNTAX_DUMP_RESULT_FILE_NAME,
                                                         TS_COMMON_SYNTAX_DUMP_EXPECTED_FILE_NAME));
    // Abc2ProgramTestUtils::RemoveDumpResultFile(COMMON_SYNTAX_DUMP_RESULT_FILE_NAME);
}
}; // panda::abc2program