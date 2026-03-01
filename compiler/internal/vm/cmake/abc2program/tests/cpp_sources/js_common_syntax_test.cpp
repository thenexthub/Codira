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

const bool REMOVE_DUMP_RESULT_FILES = false;
const std::string JS_COMMON_SYNTAX_ABC_TEST_FILE_NAME =
    GRAPH_TEST_ABC_DIR "JsCommonSyntax.abc";
const std::string JS_COMMON_SYNTAX_DUMP_RESULT_FILE_NAME =
    GRAPH_JS_TEST_ABC_DUMP_DIR "JsCommonSyntaxDumpResult.txt";
const std::string JS_COMMON_SYNTAX_DUMP_EXPECTED_FILE_NAME =
    GRAPH_JS_TEST_ABC_DUMP_DIR "JsCommonSyntaxDumpExpected.txt";

const std::string JS_COMMON_SYNTAX_DEBUG_ABC_TEST_FILE_NAME =
    GRAPH_TEST_ABC_DIR "JsCommonSyntaxDebug.abc";
const std::string JS_COMMON_SYNTAX_DEBUG_DUMP_RESULT_FILE_NAME =
    GRAPH_JS_TEST_ABC_DUMP_DIR "JsCommonSyntaxDebugDumpResult.txt";
const std::string JS_COMMON_SYNTAX_DEBUG_DUMP_EXPECTED_FILE_NAME =
    GRAPH_JS_TEST_ABC_DUMP_DIR "JsCommonSyntaxDebugDumpExpected.txt";

class Abc2ProgramJsCommonSyntaxTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        (void)driver_.Compile(JS_COMMON_SYNTAX_ABC_TEST_FILE_NAME);
        prog_ = &(driver_.GetProgram());
    }

    void TearDown() {}

    Abc2ProgramDriver driver_;
    const pandasm::Program *prog_ = nullptr;
};

class Abc2ProgramJsCommonSyntaxDebugTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        (void)driver_.Compile(JS_COMMON_SYNTAX_DEBUG_ABC_TEST_FILE_NAME);
        prog_ = &(driver_.GetProgram());
    }

    void TearDown() {}

    Abc2ProgramDriver driver_;
    const pandasm::Program *prog_ = nullptr;
};

HWTEST_F(Abc2ProgramJsCommonSyntaxTest, abc2program_js_common_syntax_test_dump, TestSize.Level1)
{
    driver_.Dump(JS_COMMON_SYNTAX_DUMP_RESULT_FILE_NAME);
    EXPECT_TRUE(Abc2ProgramTestUtils::ValidateDumpResult(JS_COMMON_SYNTAX_DUMP_RESULT_FILE_NAME,
                                                         JS_COMMON_SYNTAX_DUMP_EXPECTED_FILE_NAME));
    if (REMOVE_DUMP_RESULT_FILES) {
        Abc2ProgramTestUtils::RemoveDumpResultFile(JS_COMMON_SYNTAX_DUMP_RESULT_FILE_NAME);
    }
}

HWTEST_F(Abc2ProgramJsCommonSyntaxDebugTest, abc2program_js_common_syntax_test_dump, TestSize.Level1)
{
    driver_.Dump(JS_COMMON_SYNTAX_DEBUG_DUMP_RESULT_FILE_NAME);
    EXPECT_TRUE(Abc2ProgramTestUtils::ValidateDumpResult(JS_COMMON_SYNTAX_DEBUG_DUMP_RESULT_FILE_NAME,
                                                         JS_COMMON_SYNTAX_DEBUG_DUMP_EXPECTED_FILE_NAME));
    if (REMOVE_DUMP_RESULT_FILES) {
        Abc2ProgramTestUtils::RemoveDumpResultFile(JS_COMMON_SYNTAX_DEBUG_DUMP_RESULT_FILE_NAME);
    }
}

};  // panda::abc2program