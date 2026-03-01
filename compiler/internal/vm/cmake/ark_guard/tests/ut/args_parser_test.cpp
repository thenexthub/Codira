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

#include "libarkbase/utils/string_helpers.h"

#include "args_parser.h"

using namespace testing::ext;

namespace {
std::string GetArkGuardHelpInfo()
{
    std::string helpInfo = ark::helpers::string::Format(
        "Usage:\n"
        "guard [options] config-file-path\n"
        "Supported options:\n"
        "--debug: enable debug messages (will be printed to standard output if no --debug-file was specified)\n"
        "--debug-file: (--debug-file FILENAME) set debug file name. default is std::cout\n"
        "--help: Print this message and exit\n\n");

    return helpInfo;
}
}  // namespace

/**
 * @tc.name: args_parser_test_001
 * @tc.desc: verify guard args parse with abnormal args
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ArgParserTest, args_parser_test_001, TestSize.Level0)
{
    testing::internal::CaptureStderr();

    ark::guard::ArgsParser parser;
    int argc = 2;
    char *argv[2];
    argv[0] = const_cast<char *>("xxx");
    argv[1] = const_cast<char *>("--debug=");
    bool ret = parser.Parse(argc, const_cast<const char **>(argv));
    EXPECT_EQ(ret, false);

    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(err, GetArkGuardHelpInfo());
}

/**
 * @tc.name: args_parser_test_002
 * @tc.desc: test guard args parse with no debug file and config file args
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ArgParserTest, args_parser_test_002, TestSize.Level0)
{
    testing::internal::CaptureStderr();

    int argc = 3;
    char *argv[3];
    argv[0] = const_cast<char *>("xxx");
    argv[1] = const_cast<char *>("--debug");
    argv[2] = const_cast<char *>("--debug-file");

    ark::guard::ArgsParser parser;
    bool ret = parser.Parse(argc, const_cast<const char **>(argv));
    EXPECT_EQ(ret, false);

    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(err, GetArkGuardHelpInfo());
}

/**
 * @tc.name: args_parser_test_003
 * @tc.desc: test guard args parse with param help
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ArgParserTest, args_parser_test_003, TestSize.Level0)
{
    testing::internal::CaptureStderr();

    int argc = 2;
    char *argv[2];
    argv[0] = const_cast<char *>("xxx");
    argv[1] = const_cast<char *>("--help");

    ark::guard::ArgsParser parser;
    bool ret = parser.Parse(argc, const_cast<const char **>(argv));
    EXPECT_EQ(ret, false);
    EXPECT_EQ(parser.IsHelp(), true);

    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(err, GetArkGuardHelpInfo());
}

/**
 * @tc.name: args_parser_test_004
 * @tc.desc: test guard args parse with normal params
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ArgParserTest, args_parser_test_004, TestSize.Level0)
{
    std::string debugFilePath = "./xxx.txt";
    std::string configFilePath = "./config.json";

    int argc = 5;
    char *argv[5];
    argv[0] = const_cast<char *>("xxx");
    argv[1] = const_cast<char *>("--debug");
    argv[2] = const_cast<char *>("--debug-file");
    argv[3] = const_cast<char *>(debugFilePath.data());
    argv[4] = const_cast<char *>(configFilePath.data());

    ark::guard::ArgsParser parser;
    bool ret = parser.Parse(argc, const_cast<const char **>(argv));
    EXPECT_EQ(ret, true);
    EXPECT_EQ(parser.IsDebug(), true);
    EXPECT_EQ(parser.GetDebugFile(), debugFilePath);
    EXPECT_EQ(parser.GetConfigFilePath(), configFilePath);
}