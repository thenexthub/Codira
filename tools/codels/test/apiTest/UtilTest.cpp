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


#include <algorithm>
#include <cangjie/Utils/FileUtil.h>
#include <fstream>
#include <gtest/gtest.h>
#include <vector>
#include "../../../src/languageserver/common/Utils.h"

using namespace Codira::FileUtil;

namespace apitest {

    class GetAllFilesTest : public ::testing::Test {
    protected:
        void SetUp() override
        {
            CreateDirs("test_dir/");
            std::ofstream("test_dir/file.code").close();
            std::ofstream("test_dir/file_test.code").close();
        }

        void TearDown() override
        {
            Remove("test_dir/file.code");
            Remove("test_dir/file_test.code");
            Remove("test_dir");
        }
    };

    TEST_F(GetAllFilesTest, FindsCodeFiles)
    {
        auto result = GetAllFilesUnderCurrentPath("test_dir", "code");
        EXPECT_EQ(result.size(), 1);
        EXPECT_EQ(result[0], "file.code");
    }

    TEST_F(GetAllFilesTest, IncludesTestFiles)
    {
        auto result = GetAllFilesUnderCurrentPath("test_dir", "code", false);
        EXPECT_EQ(result.size(), 2);
        EXPECT_TRUE(std::find(result.begin(), result.end(), "file_test.code") != result.end());
    }

    class StringMatcherTest : public ::testing::Test {
    protected:
        void SetUp() override {}

        void TearDown() override {}
    };

    TEST_F(StringMatcherTest, CaseSensitiveMatching)
    {
        EXPECT_TRUE(ark::IsMatchingCompletion("Hld", "HelloWorld", true));

        EXPECT_FALSE(ark::IsMatchingCompletion("hw", "HelloWorld", true));

        EXPECT_TRUE(ark::IsMatchingCompletion("HW", "HelloWorld", true));
        EXPECT_FALSE(ark::IsMatchingCompletion("WH", "HelloWorld", true));

        EXPECT_FALSE(ark::IsMatchingCompletion("xyz", "HelloWorld", true));
    }

    TEST_F(StringMatcherTest, CaseInsensitiveMatching)
    {
        EXPECT_TRUE(ark::IsMatchingCompletion("hw", "HelloWorld", false));
        EXPECT_TRUE(ark::IsMatchingCompletion("HELLO", "HelloWorld", false));
        EXPECT_TRUE(ark::IsMatchingCompletion("hElLo", "HelloWorld", false));

        EXPECT_TRUE(ark::IsMatchingCompletion("hw", "HelloWorld", false));
        EXPECT_FALSE(ark::IsMatchingCompletion("wh", "HelloWorld", false));

        EXPECT_FALSE(ark::IsMatchingCompletion("xyz", "HelloWorld", false));

        EXPECT_TRUE(ark::IsMatchingCompletion("hW", "HelloWorld", false));
        EXPECT_TRUE(ark::IsMatchingCompletion("Hw", "HelloWorld", false));
    }

}
