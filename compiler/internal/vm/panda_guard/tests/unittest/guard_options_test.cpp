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

#include "configs/guard_options.h"
#include "util/file_util.h"

#include "util/test_util.h"

using namespace testing::ext;
using namespace panda;

namespace {
const std::string OPTIONS_TEST_01_CONFIG_FILE = PANDA_GUARD_UNIT_TEST_DIR "configs/options_test_01_config.json";
const std::string OPTIONS_TEST_05_CONFIG_FILE = PANDA_GUARD_UNIT_TEST_DIR "configs/options_test_05_config.json";
const std::string TEMP_OPTIONS_TEST_JSON_FILE = PANDA_GUARD_UNIT_TEST_DIR "configs/temp_options_test.json";
}  // namespace

/**
 * @tc.name: guard_options_test_001
 * @tc.desc: test options parse is right
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardOptionsUnitTest, guard_options_test_001, TestSize.Level4)
{
    guard::GuardOptions options;
    options.Load(OPTIONS_TEST_01_CONFIG_FILE);

    EXPECT_EQ(options.GetAbcFilePath(), "xxx");
    EXPECT_EQ(options.GetObfAbcFilePath(), "xxx");
    EXPECT_EQ(options.GetObfPaFilePath(), "xxx");
    EXPECT_EQ(options.GetCompileSdkVersion(), "xxx");
    EXPECT_EQ(options.GetTargetApiVersion(), 12);
    EXPECT_EQ(options.GetTargetApiSubVersion(), "beta3");
    EXPECT_EQ(options.GetSourceName("xxx"), "xxx");
    EXPECT_EQ(options.GetEntryPackageInfo(), "xxx");
    EXPECT_EQ(options.GetDefaultNameCachePath(), "xxx");
    EXPECT_EQ(options.IsSkippedRemoteHar("xxx"), true);
    EXPECT_EQ(options.IsUseNormalizedOhmUrl(), true);
    EXPECT_EQ(options.DisableObfuscation(), false);
    EXPECT_EQ(options.IsExportObfEnabled(), true);
    EXPECT_EQ(options.IsRemoveLogObfEnabled(), true);
    EXPECT_EQ(options.IsDecoratorObfEnabled(), true);
    EXPECT_EQ(options.IsCompactObfEnabled(), true);
    EXPECT_EQ(options.GetPrintNameCache().empty(), true);
    EXPECT_EQ(options.GetApplyNameCache().empty(), true);
    EXPECT_EQ(options.IsPropertyObfEnabled(), true);
    EXPECT_EQ(options.IsToplevelObfEnabled(), true);
    EXPECT_EQ(options.IsFileNameObfEnabled(), true);
    EXPECT_EQ(options.IsReservedNames("xxx"), true);
    EXPECT_EQ(options.IsInRecordNameWhiteList("xxx"), true);
    EXPECT_EQ(options.IsReservedProperties("xxx"), true);
    EXPECT_EQ(options.IsReservedToplevelNames("xxx"), true);
    EXPECT_EQ(options.IsReservedFileNames("xxx"), true);
    EXPECT_EQ(options.IsReservedRemoteHarPkgNames("xxx"), true);
    EXPECT_EQ(options.IsKeepPath("xxx"), true);
    EXPECT_EQ(options.IsKeepPath("yyy"), false);
}

/**
 * @tc.name: guard_options_test_002
 * @tc.desc: test options abc and obf abc is empty and not has obfuscationRules field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardOptionsUnitTest, guard_options_test_002, TestSize.Level4)
{
    guard::GuardOptions options;

    std::string jsonStr = R"({
            "abcFilePath": "",
            "obfAbcFilePath": "",
            "obfPaFilePath": "",
            "targetApiVersion": 12,
            "targetApiSubVersion": "beta3",
            "compileSdkVersion": "xxx",
            "entryPackageInfo": "xxx",
            "defaultNameCachePath": "xxx",
            "obfuscationRules" : {}
        })";
    guard::FileUtil::WriteFile(TEMP_OPTIONS_TEST_JSON_FILE, jsonStr);
    EXPECT_DEATH(options.Load(TEMP_OPTIONS_TEST_JSON_FILE), "");

    guard::TestUtil::RemoveFile(TEMP_OPTIONS_TEST_JSON_FILE);

    jsonStr = R"({
            "abcFilePath": "xxx",
            "obfAbcFilePath": "xxx",
            "obfPaFilePath": "xxx",
            "targetApiVersion": 12,
            "targetApiSubVersion": "beta3",
            "compileSdkVersion": "xxx",
            "entryPackageInfo": "xxx",
            "defaultNameCachePath": "xxx"
        })";
    guard::FileUtil::WriteFile(TEMP_OPTIONS_TEST_JSON_FILE, jsonStr);
    EXPECT_DEATH(options.Load(TEMP_OPTIONS_TEST_JSON_FILE), "");

    guard::TestUtil::RemoveFile(TEMP_OPTIONS_TEST_JSON_FILE);
}

/**
 * @tc.name: guard_options_test_003
 * @tc.desc: test options obf rules default value
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardOptionsUnitTest, guard_options_test_003, TestSize.Level4)
{
    std::string jsonStr = R"({
            "abcFilePath": "xxx",
            "obfAbcFilePath": "xxx",
            "obfPaFilePath": "xxx",
            "targetApiVersion": 12,
            "targetApiSubVersion": "beta3",
            "compileSdkVersion": "xxx",
            "entryPackageInfo": "xxx",
            "defaultNameCachePath": "xxx",
            "obfuscationRules" : {}
        })";
    guard::FileUtil::WriteFile(TEMP_OPTIONS_TEST_JSON_FILE, jsonStr);
    guard::GuardOptions options;
    options.Load(TEMP_OPTIONS_TEST_JSON_FILE);

    EXPECT_EQ(options.DisableObfuscation(), false);
    EXPECT_EQ(options.IsExportObfEnabled(), false);
    EXPECT_EQ(options.IsRemoveLogObfEnabled(), false);
    EXPECT_EQ(options.IsPropertyObfEnabled(), false);
    EXPECT_EQ(options.IsToplevelObfEnabled(), false);
    EXPECT_EQ(options.IsFileNameObfEnabled(), false);
    EXPECT_EQ(options.IsKeepPath("xxx"), false);
    guard::TestUtil::RemoveFile(TEMP_OPTIONS_TEST_JSON_FILE);
}

/**
 * @tc.name: guard_options_test_004
 * @tc.desc: test options parse data not json return failed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardOptionsUnitTest, guard_options_test_004, TestSize.Level4)
{
    std::string jsonStr = R"({
            "abcFilePath": "xxx",
            "obfAbcFilePath": "xxx",
            "obfPaFilePath": "xxx",
            "targetApiVersion": 12,
            "targetApiSubVersion": "beta3",
            "compileSdkVersion": "xxx",
            "entryPackageInfo": "xxx",
            "defaultNameCachePath": "xxx",
            "obfuscationRules" : {
        })";
    guard::FileUtil::WriteFile(TEMP_OPTIONS_TEST_JSON_FILE, jsonStr);
    guard::GuardOptions options;
    EXPECT_DEATH(options.Load(TEMP_OPTIONS_TEST_JSON_FILE), "");

    guard::TestUtil::RemoveFile(TEMP_OPTIONS_TEST_JSON_FILE);
}

/**
 * @tc.name: guard_options_test_005
 * @tc.desc: test options universal reserved names match is right
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardOptionsUnitTest, guard_options_test_005, TestSize.Level4)
{
    guard::GuardOptions options;
    options.Load(OPTIONS_TEST_05_CONFIG_FILE);

    // xx.*
    EXPECT_EQ(options.IsReservedProperties("xx"), true);
    EXPECT_EQ(options.IsReservedProperties("xxabc"), true);
    EXPECT_EQ(options.IsReservedProperties("xy"), false);
    // yy.
    EXPECT_EQ(options.IsReservedToplevelNames("yya"), true);
    EXPECT_EQ(options.IsReservedToplevelNames("yy"), false);
    EXPECT_EQ(options.IsReservedToplevelNames("xy"), false);
    // .*zz.
    EXPECT_EQ(options.IsReservedFileNames("abczzA"), true);
    EXPECT_EQ(options.IsReservedFileNames("zzA"), true);
    EXPECT_EQ(options.IsReservedFileNames("abczz"), false);
    EXPECT_EQ(options.IsReservedFileNames("zz"), false);
}
