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

#include "util/string_util.h"

using namespace testing::ext;
using namespace panda::guard;

/**
 * @tc.name: string_util_test_001
 * @tc.desc: test string util split function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilUnitTest, string_util_test_001, TestSize.Level4)
{
    std::string delimiter = "#";

    std::string str = "##a#";
    auto result = StringUtil::Split(str, delimiter);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "a");

    str = "error";
    result = StringUtil::Split(str, delimiter);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "error");
}

/**
 * @tc.name: string_util_test_002
 * @tc.desc: test string util strict split function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilUnitTest, string_util_test_002, TestSize.Level4)
{
    std::string str = "##a#";
    std::string delimiter = "#";
    auto result = StringUtil::StrictSplit(str, delimiter);
    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "a");
    EXPECT_EQ(result[3], "");
}

/**
 * @tc.name: string_util_test_003
 * @tc.desc: test string util is anonymous function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilUnitTest, string_util_test_003, TestSize.Level4)
{
    auto result = StringUtil::IsAnonymousFunctionName("");
    EXPECT_EQ(result, true);

    result = StringUtil::IsAnonymousFunctionName("^1");
    EXPECT_EQ(result, true);

    result = StringUtil::IsAnonymousFunctionName("^11");
    EXPECT_EQ(result, true);

    result = StringUtil::IsAnonymousFunctionName("^a");
    EXPECT_EQ(result, true);

    result = StringUtil::IsAnonymousFunctionName("^a1");
    EXPECT_EQ(result, true);

    result = StringUtil::IsAnonymousFunctionName("^1a");
    EXPECT_EQ(result, true);

    result = StringUtil::IsAnonymousFunctionName("^aa");
    EXPECT_EQ(result, true);

    result = StringUtil::IsAnonymousFunctionName("^g");
    EXPECT_EQ(result, false);

    result = StringUtil::IsAnonymousFunctionName("^1g");
    EXPECT_EQ(result, false);

    result = StringUtil::IsAnonymousFunctionName("^g1");
    EXPECT_EQ(result, false);

    result = StringUtil::IsAnonymousFunctionName("^gg");
    EXPECT_EQ(result, false);
}