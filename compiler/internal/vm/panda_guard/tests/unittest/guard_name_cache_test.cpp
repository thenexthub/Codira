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

#include <vector>
#include <gtest/gtest.h>
#include "configs/guard_name_cache.h"
#include "util/test_util.h"

using namespace testing::ext;
using namespace panda;

namespace {
const std::string NAME_CACHE_TEST_01_FILE = PANDA_GUARD_UNIT_TEST_DIR "configs/name_cache_test_01.json";
const std::string NAME_CACHE_TEST_02_FILE = PANDA_GUARD_UNIT_TEST_DIR "configs/name_cache_test_02.json";
const std::string NAME_CACHE_TEST_02_CONFIG_FILE = PANDA_GUARD_UNIT_TEST_DIR "configs/name_cache_test_02_config.json";
const std::string NAME_CACHE_TEST_04_FILE = PANDA_GUARD_UNIT_TEST_DIR "configs/name_cache_test_04.json";
}  // namespace

/**
 * @tc.name: guard_name_cache_test_001
 * @tc.desc: test name cache json parse is right
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardNameCacheUnitTest, guard_name_cache_test_001, TestSize.Level4)
{
    auto options = std::make_shared<guard::GuardOptions>();
    guard::NameCache nameCache(options);
    nameCache.Load(NAME_CACHE_TEST_01_FILE);

    std::set<std::string> expectList = {"/entry/src/a/b.ets", "5.0.0.1", "a", "b", "c", "d", "e",
                                        "entry|1.0.0",        "f",       "g"};
    auto historyValues = nameCache.GetHistoryUsedNames();
    bool bEqual = std::equal(historyValues.begin(), historyValues.end(), expectList.begin(), expectList.end());
    EXPECT_EQ(bEqual, true);

    auto fileMapping = nameCache.GetHistoryFileNameMapping();
    EXPECT_EQ(fileMapping["test1"], "a");
    EXPECT_EQ(fileMapping["test2"], "b");

    auto nameMapping = nameCache.GetHistoryNameMapping();
    EXPECT_EQ(nameMapping["test3"], "c");
    EXPECT_EQ(nameMapping["test4"], "d");
    EXPECT_EQ(nameMapping["test5"], "e");
    EXPECT_EQ(nameMapping["func"], "f");
    EXPECT_EQ(nameMapping["test7"], "g");
}

/**
 * @tc.name: guard_name_cache_test_002
 * @tc.desc: test name cache merge and write is right
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardNameCacheUnitTest, guard_name_cache_test_002, TestSize.Level4)
{
    auto options = std::make_shared<guard::GuardOptions>();
    options->Load(NAME_CACHE_TEST_02_CONFIG_FILE);

    guard::NameCache nameCache(options);
    nameCache.Load(NAME_CACHE_TEST_02_FILE);

    std::string filePath = "/entry/src/test1/test2.ets";
    std::string obfFilePath = "/entry/src/a/b.ets";
    nameCache.AddObfName(filePath, obfFilePath);
    nameCache.AddObfIdentifierName(filePath, "#haTest1", "a1");
    nameCache.AddObfMemberMethodName(filePath, "#haTest2:3:5", "b1");
    nameCache.AddObfPropertyName("haTest1", "a1");
    nameCache.AddObfPropertyName("haTest2", "b1");

    std::string newFilePath = "/entry/src/newTest1/newTest2.ets";
    std::string obfNewFilePath = "/entry/src/a2/b2.ets";
    nameCache.AddObfName(newFilePath, obfNewFilePath);
    nameCache.AddNewNameCacheObfFileName("newTest1", "a2");
    nameCache.AddNewNameCacheObfFileName("newTest2", "b2");
    nameCache.AddObfIdentifierName(newFilePath, "#newTest3", "c2");
    nameCache.AddObfMemberMethodName(newFilePath, "#newTest4:3:5", "d2");
    nameCache.AddObfPropertyName("newTest3", "c2");
    nameCache.AddObfPropertyName("newTest4", "d2");

    nameCache.WriteCache();

    static const std::string NEW_NAME_CACHE_TEST_JSON_FILE =
        PANDA_GUARD_UNIT_TEST_DIR "configs/new_name_cache_test.json";
    guard::NameCache newNameCache(options);
    newNameCache.Load(NEW_NAME_CACHE_TEST_JSON_FILE);

    std::set<std::string> expectList = {"/entry/src/a/b.ets",
                                        "/entry/src/a2/b2.ets",
                                        "5.0.0.2",
                                        "a",
                                        "a1",
                                        "a2",
                                        "b",
                                        "b1",
                                        "b2",
                                        "c",
                                        "c2",
                                        "d",
                                        "d2",
                                        "e",
                                        "entry|2.0.1",
                                        "f",
                                        "g"};
    auto newValues = newNameCache.GetHistoryUsedNames();
    bool bEqual = std::equal(newValues.begin(), newValues.end(), expectList.begin(), expectList.end());
    EXPECT_EQ(bEqual, true);

    guard::TestUtil::RemoveFile(NEW_NAME_CACHE_TEST_JSON_FILE);
}

/**
 * @tc.name: guard_name_cache_test_003
 * @tc.desc: test name cache json parse with abnormal args
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardNameCacheUnitTest, guard_name_cache_test_003, TestSize.Level4)
{
    auto options = std::make_shared<guard::GuardOptions>();
    guard::NameCache nameCache(options);
    nameCache.Load("");

    std::string filePath = "/entry/src/common/calc.ets";
    std::string obfName = "c";
    std::string origin = "#class1";
    nameCache.AddObfIdentifierName("", origin, obfName);
    nameCache.AddObfIdentifierName(filePath, "", obfName);
    nameCache.AddObfIdentifierName(filePath, origin, "");

    nameCache.AddObfMemberMethodName("", origin, obfName);
    nameCache.AddObfMemberMethodName(filePath, "", obfName);
    nameCache.AddObfMemberMethodName(filePath, origin, "");

    std::string obfFilePath = "/entry/src/a/b.ets";
    nameCache.AddObfName("", obfFilePath);
    nameCache.AddObfName(filePath, "");

    std::string fileName = "calc";
    std::string obfFileName = "b";
    nameCache.AddNewNameCacheObfFileName("", obfFileName);
    nameCache.AddNewNameCacheObfFileName(fileName, "");

    EXPECT_DEATH(nameCache.WriteCache(), "");
}

/**
 * @tc.name: guard_name_cache_test_004
 * @tc.desc: test name cache json parse with abnormal json data
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardNameCacheUnitTest, guard_name_cache_test_004, TestSize.Level4)
{
    auto options = std::make_shared<guard::GuardOptions>();
    guard::NameCache nameCache(options);
    EXPECT_DEATH(nameCache.Load(NAME_CACHE_TEST_04_FILE), "");
}