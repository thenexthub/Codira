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

#include "utils/string_helpers.h"

#include "configs/guard_args_parser.h"

using namespace testing::ext;
using namespace panda::guard;

/**
 * @tc.name: guard_args_parser_test_001
 * @tc.desc: test panda_guard args parse with abnormal args
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardArgsParserUnitTest, guard_args_parser_test_001, TestSize.Level4)
{
    testing::internal::CaptureStderr();

    GuardArgsParser parser;
    int argc = 2;
    char *argv[2];
    argv[0] = const_cast<char *>("xxx");
    argv[1] = const_cast<char *>("--debug=");
    bool ret = parser.Parse(argc, const_cast<const char **>(argv));
    EXPECT_EQ(ret, false);

    std::string err = testing::internal::GetCapturedStderr();
    std::string res = panda::helpers::string::Format(
        "Usage:\n"
        "panda_guard [options] config-file-path\n"
        "Supported options:\n"
        "--debug: enable debug messages (will be printed to standard output if no --debug-file was specified)\n"
        "--debug-file: (--debug-file FILENAME) set debug file name. default is std::cout\n"
        "--help: Print this message and exit\n"
        "Tail arguments:\n"
        "config-file-path: configuration file path\n\n");
    EXPECT_EQ(err, res);
}

/**
 * @tc.name: guard_args_parser_test_002
 * @tc.desc: test panda_guard args parse with no debug file and config file args
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardArgsParserUnitTest, guard_args_parser_test_002, TestSize.Level4)
{
    testing::internal::CaptureStderr();

    int argc = 3;
    char *argv[3];
    argv[0] = const_cast<char *>("xxx");
    argv[1] = const_cast<char *>("--debug");
    argv[2] = const_cast<char *>("--debug-file");

    GuardArgsParser parser;
    bool ret = parser.Parse(argc, const_cast<const char **>(argv));
    EXPECT_EQ(ret, false);

    std::string err = testing::internal::GetCapturedStderr();
    std::string res = panda::helpers::string::Format(
        "The config-file-path value is empty. Please check if the config-file-path is set correctly.\n");
    EXPECT_EQ(err, res);
}