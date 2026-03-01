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

/**
 * @tc.name: string_util_test_001
 * @tc.desc: test whether names with wildcard characters can be successfully converted using conversion functions.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_001, TestSize.Level0)
{
    std::unordered_map<std::string, std::string> wildcardMap = {
        {"?", "[^/\\.]"},
        {"*", "[^/\\.]*"},
        {"**", ".*"},
        {"com.example.Test?", "com\\.example\\.Test[^/\\.]"},
        {"com.example.*Test*", "com\\.example\\.[^/\\.]*Test[^/\\.]*"},
        {"com.example.**Test", "com\\.example\\..*Test"},
        {"com.example.*Test<2>", "com\\.example\\.[^/\\.]*Test"},
        {"com.example.*Te?st<2>", "com\\.example\\.[^/\\.]*Te[^/\\.]st[^/\\.]"},
    };

    for (const auto &[pattern, expect] : wildcardMap) {
        auto result = ark::guard::StringUtil::ConvertWildcardToRegex(pattern);
        EXPECT_EQ(result, expect);
    }
}

/**
 * @tc.name: string_util_test_002
 * @tc.desc: test whether names with wildcard characters can be successfully converted using conversion functions.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_002, TestSize.Level0)
{
    std::unordered_map<std::string, std::string> wildcardMap = {
        {"***", ".*"},
        {"...", ".*"},
        {"**", ".*"},
        {"*", "[^/\\.]*"},
        {"?", "[^/\\.]"},
        {"%",
         "(f32|f64|i8|i16|i32|i64|i32|i64|u16|u1|std\\.core\\.String|std\\.core\\.BigInt|void|std\\.core\\.Null|std\\."
         "core\\.Object|std\\.core\\.Int|std\\.core\\.Byte|std\\.core\\.Short|std\\.core\\.Long|std\\.core\\.Double|"
         "std\\.core\\.Float|std\\.core\\.Char|std\\.core\\.Boolean)"},
        {"Test?", "Test[^/\\.]"},
        {"*Test*", "[^/\\.]*Test[^/\\.]*"},
        {"Test**Test", "Test.*Test"},
        {"Test.*Test<2>", "Test\\.[^/\\.]*Test"},
        {"int,%",
         "int,(f32|f64|i8|i16|i32|i64|i32|i64|u16|u1|std\\.core\\.String|std\\.core\\.BigInt|void|std\\.core\\.Null|"
         "std\\."
         "core\\.Object|std\\.core\\.Int|std\\.core\\.Byte|std\\.core\\.Short|std\\.core\\.Long|std\\.core\\.Double|"
         "std\\.core\\.Float|std\\.core\\.Char|std\\.core\\.Boolean)"},
        {"(...)", "(.*)"},
        {"()***", "().*"},
    };

    for (const auto &[pattern, expect] : wildcardMap) {
        auto result = ark::guard::StringUtil::ConvertWildcardToRegexEx(pattern);
        EXPECT_EQ(result, expect);
    }
}

/**
 * @tc.name: string_util_test_003
 * @tc.desc: test back reference is 0
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_003, TestSize.Level0)
{
    std::string wildcardStr = "Test.*Test<0>";
    EXPECT_DEATH(ark::guard::StringUtil::ConvertWildcardToRegex(wildcardStr), "");
    EXPECT_DEATH(ark::guard::StringUtil::ConvertWildcardToRegexEx(wildcardStr), "");
}

/**
 * @tc.name: string_util_test_004
 * @tc.desc: test prefix remove function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_004, TestSize.Level0)
{
    EXPECT_EQ(ark::guard::StringUtil::RemovePrefixIfMatches("HelloWorld", "Hello"), "World");
    EXPECT_EQ(ark::guard::StringUtil::RemovePrefixIfMatches("HelloWorld", "hi"), "HelloWorld");
    EXPECT_EQ(ark::guard::StringUtil::RemovePrefixIfMatches("", "prefix"), "");
    EXPECT_EQ(ark::guard::StringUtil::RemovePrefixIfMatches("prefix", "prefix"), "");
    EXPECT_EQ(ark::guard::StringUtil::RemovePrefixIfMatches("pre", "prefix"), "pre");
}

/**
 * @tc.name: string_util_test_005
 * @tc.desc: test ensure start/end function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_005, TestSize.Level0)
{
    // EnsureStartWithPrefix
    EXPECT_EQ(ark::guard::StringUtil::EnsureStartWithPrefix("prefix_string", "prefix"), "prefix_string");
    EXPECT_EQ(ark::guard::StringUtil::EnsureStartWithPrefix("string", "prefix_"), "prefix_string");
    EXPECT_EQ(ark::guard::StringUtil::EnsureStartWithPrefix("", "prefix"), "prefix");
    EXPECT_EQ(ark::guard::StringUtil::EnsureStartWithPrefix("string", ""), "string");
    EXPECT_EQ(ark::guard::StringUtil::EnsureStartWithPrefix("pre_string", "prefix"), "prefixpre_string");
    // EnsureEndWithSuffix
    EXPECT_EQ(ark::guard::StringUtil::EnsureEndWithSuffix("string_suffix", "suffix"), "string_suffix");
    EXPECT_EQ(ark::guard::StringUtil::EnsureEndWithSuffix("string", "_suffix"), "string_suffix");
    EXPECT_EQ(ark::guard::StringUtil::EnsureEndWithSuffix("", "suffix"), "suffix");
    EXPECT_EQ(ark::guard::StringUtil::EnsureEndWithSuffix("string", ""), "string");
    EXPECT_EQ(ark::guard::StringUtil::EnsureEndWithSuffix("string_suf", "suffix"), "string_sufsuffix");
}

/**
 * @tc.name: string_util_test_006
 * @tc.desc: test FindLongestMatchedPrefix function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_006, TestSize.Level0)
{
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefix({"hello", "hi"}, "hello world"), 5);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefix({"h", "he", "hel", "hello"}, "hello world"), 5);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefix({"foo", "bar"}, "hello world"), 0);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefix({"prefix"}, ""), 0);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefix({""}, "string"), 0);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefix({}, "string"), 0);
}

/**
 * @tc.name: string_util_test_007
 * @tc.desc: test FindLongestMatchedPrefixReg function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_007, TestSize.Level0)
{
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^h.*o", "^hi.*"}, "hello world"), 8);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^h", "^he.*", "^hel+"}, "hello world"), 11);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^h\\d+", "^h[a-z]+"}, "h123abc"), 4);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^foo", "^bar"}, "hello world"), 0);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^[A-Z][a-z]+", "^[a-z]{2,5}"}, "HelloWorld"), 5);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({"^prefix"}, ""), 0);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({""}, "string"), 0);
    EXPECT_EQ(ark::guard::StringUtil::FindLongestMatchedPrefixReg({}, "string"), 0);
}

/**
 * @tc.name: string_util_test_008
 * @tc.desc: test RemoveAllSpaces function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilTest, string_util_test_008, TestSize.Level0)
{
    std::string origin = " {Uxxx1, xxx2 } ";
    std::string expect = "{Uxxx1,xxx2}";
    std::string result = ark::guard::StringUtil::RemoveAllSpaces(origin);
    EXPECT_EQ(result, expect);
}