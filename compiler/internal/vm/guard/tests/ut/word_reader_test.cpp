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

#include "configuration/word_reader.h"

#include "test_util/assert.h"

using namespace testing::ext;

/**
 * @tc.name: word_reader_test_001
 * @tc.desc: verify whether the regular class_specification is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(WordReaderTest, word_reader_test_001, TestSize.Level0)
{
    std::string keepStr = R"(-keep @AnnotationA !final class classA extends @AnnotationB classB {
        @AnnotationC
        private static alias: string = 'test';

        @AnnotationD
        public async doAction(param: string): void;
    })";
    const std::vector<std::string> expectWordList = {
        "-keep",  "@",    "AnnotationA", "!final",      "class",       "classA", "extends", "@",        "AnnotationB",
        "classB", "{",    "@",           "AnnotationC", "private",     "static", "alias",   ":",        "string",
        "=",      "test", ";",           "@",           "AnnotationD", "public", "async",   "doAction", "(",
        "param",  ":",    "string",      ")",           ":",           "void",   ";",       "}"};

    ark::guard::WordReader reader(keepStr);
    std::vector<std::string> wordList;
    std::string word;
    while (!(word = reader.NextWord()).empty()) {
        wordList.emplace_back(word);
    }
    ASSERT_ARRAY_EQUAL(expectWordList, wordList);
}

/**
 * @tc.name: word_reader_test_002
 * @tc.desc: verify whether the class_specification with wildcard is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(WordReaderTest, word_reader_test_002, TestSize.Level0)
{
    std::string keepStr = R"(-keep @AnnotationA !final class * extends @AnnotationB class? {
        @AnnotationC
        <fields>;

        @AnnotationD
        <methods>;
    })";
    const std::vector<std::string> expectWordList = {
        "-keep", "@", "AnnotationA", "!final",   "class", "*", "extends",     "@",         "AnnotationB", "class?",
        "{",     "@", "AnnotationC", "<fields>", ";",     "@", "AnnotationD", "<methods>", ";",           "}"};

    ark::guard::WordReader reader(keepStr);
    std::vector<std::string> wordList;
    std::string word;
    while (!(word = reader.NextWord()).empty()) {
        wordList.emplace_back(word);
    }
    ASSERT_ARRAY_EQUAL(expectWordList, wordList);
}

/**
 * @tc.name: word_reader_test_003
 * @tc.desc: verify whether the class_specification with keep package is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(WordReaderTest, word_reader_test_003, TestSize.Level0)
{
    std::string keepStr = R"(-keep package a.b.c)";
    const std::vector<std::string> expectWordList = {"-keep", "package", "a.b.c"};

    ark::guard::WordReader reader(keepStr);
    std::vector<std::string> wordList;
    std::string word;
    while (!(word = reader.NextWord()).empty()) {
        wordList.emplace_back(word);
    }
    ASSERT_ARRAY_EQUAL(expectWordList, wordList);
}

/**
 * @tc.name: word_reader_test_004
 * @tc.desc: verify whether the class_specification with keep directory is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(WordReaderTest, word_reader_test_004, TestSize.Level0)
{
    std::string keepStr = R"(-keep a/b/c)";
    const std::vector<std::string> expectWordList = {"-keep", "a/b/c"};

    ark::guard::WordReader reader(keepStr);
    std::vector<std::string> wordList;
    std::string word;
    while (!(word = reader.NextWord()).empty()) {
        wordList.emplace_back(word);
    }
    ASSERT_ARRAY_EQUAL(expectWordList, wordList);
}

/**
 * @tc.name: word_reader_test_005
 * @tc.desc: verify the class_specification is empty, whether return th empty string.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(WordReaderTest, word_reader_test_005, TestSize.Level0)
{
    ark::guard::WordReader reader("");
    EXPECT_EQ(reader.NextWord(), "");
}

/**
 * @tc.name: word_reader_test_006
 * @tc.desc: verify whether the class_specification is abnormal is parsed failed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(WordReaderTest, word_reader_test_006, TestSize.Level0)
{
    // field only have one '\''
    std::string keepStr = R"(-keep class classA {
        private alias: string = 'test;
    })";

    ark::guard::WordReader reader(keepStr);
    EXPECT_EQ(reader.NextWord(), "-keep");
    EXPECT_EQ(reader.NextWord(), "class");
    EXPECT_EQ(reader.NextWord(), "classA");
    EXPECT_EQ(reader.NextWord(), "{");
    EXPECT_EQ(reader.NextWord(), "private");
    EXPECT_EQ(reader.NextWord(), "alias");
    EXPECT_EQ(reader.NextWord(), ":");
    EXPECT_EQ(reader.NextWord(), "string");
    EXPECT_EQ(reader.NextWord(), "=");
    EXPECT_DEATH(reader.NextWord(), "");
}

/**
 * @tc.name: word_reader_test_007
 * @tc.desc: verify whether the regular class_specification with comments is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(WordReaderTest, word_reader_test_007, TestSize.Level0)
{
    std::string keepStr = R"(
        # AAAAAA
        -keep @AnnotationA !final class classA extends @AnnotationB classB {
        @AnnotationC #BBBBB
        private static alias: string = 'test#ccc';
    })";
    const std::vector<std::string> expectWordList = {
        "-keep",       "@",      "AnnotationA", "!final",   "class",       "classA",  "extends", "@",
        "AnnotationB", "classB", "{",           "@",        "AnnotationC", "private", "static",  "alias",
        ":",           "string", "=",           "test#ccc", ";",           "}"};

    ark::guard::WordReader reader(keepStr);
    std::vector<std::string> wordList;
    std::string word;
    while (!(word = reader.NextWord()).empty()) {
        wordList.emplace_back(word);
    }
    ASSERT_ARRAY_EQUAL(expectWordList, wordList);
}
