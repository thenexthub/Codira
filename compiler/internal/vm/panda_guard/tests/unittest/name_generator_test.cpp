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

#include "generator/name_mapping.h"
#include "generator/order_name_generator.h"

using namespace testing::ext;
using namespace panda::guard;

/**
 * @tc.name: name_mapping_get_name_test_001
 * @tc.desc: test name mapping get name is order
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameGeneratotUnitTest, name_mapping_get_name_test_001, TestSize.Level4)
{
    auto options = std::make_shared<GuardOptions>();
    auto nameCache = std::make_shared<NameCache>(options);
    auto nameMapping = std::make_shared<NameMapping>(nameCache);

    vector<std::string> expectList = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",  "n",
                                      "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "a1", "b1"};
    for (size_t idx = 0; idx < expectList.size(); idx++) {
        std::string origin = "test" + std::to_string(idx);
        std::string obfName = nameMapping->GetName(origin);
        EXPECT_EQ(obfName, expectList[idx]);

        obfName = nameMapping->GetFileName(origin);
        EXPECT_EQ(obfName, expectList[idx]);
    }

    std::string oriNum = "123";
    std::string obfNum = nameMapping->GetName(oriNum);
    EXPECT_EQ(oriNum, obfNum);
}

/**
 * @tc.name: name_mapping_get_name_test_002
 * @tc.desc: test name mapping get name is order
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameGeneratotUnitTest, name_mapping_get_name_test_002, TestSize.Level4)
{
    auto options = std::make_shared<GuardOptions>();
    auto nameCache = std::make_shared<NameCache>(options);
    auto nameMapping = std::make_shared<NameMapping>(nameCache);

    std::string obfName = nameMapping->GetName("test0");
    EXPECT_EQ(obfName, "a");

    obfName = nameMapping->GetFileName("fileName_test0");
    EXPECT_EQ(obfName, "a");

    std::set<std::string> newReservedNames = {"b", "c"};
    nameMapping->AddReservedNames(newReservedNames);

    obfName = nameMapping->GetName("test1");
    EXPECT_EQ(obfName, "d");

    obfName = nameMapping->GetFileName("fileName_test1");
    EXPECT_EQ(obfName, "d");

    obfName = nameMapping->GetName("test0");
    EXPECT_EQ(obfName, "a");

    obfName = nameMapping->GetFileName("fileName_test0");
    EXPECT_EQ(obfName, "a");
}

/**
 * @tc.name: name_mapping_get_name_test_003
 * @tc.desc: test name mapping get multiple method name is same
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameGeneratotUnitTest, name_mapping_get_name_test_003, TestSize.Level4)
{
    auto options = std::make_shared<GuardOptions>();
    auto nameCache = std::make_shared<NameCache>(options);
    auto nameMapping = std::make_shared<NameMapping>(nameCache);

    std::string obfName = nameMapping->GetName("foo");
    EXPECT_EQ(obfName, "a");

    obfName = nameMapping->GetName("foo^1");
    EXPECT_EQ(obfName, "a^1");

    obfName = nameMapping->GetName("foo^a");
    EXPECT_EQ(obfName, "a^a");

    obfName = nameMapping->GetName("foo^1a");
    EXPECT_EQ(obfName, "a^1a");
}
