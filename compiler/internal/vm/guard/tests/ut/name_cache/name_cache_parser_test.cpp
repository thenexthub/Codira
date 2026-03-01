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

#include "name_cache/name_cache_parser.h"

using namespace testing::ext;

namespace {
const std::string NAME_CACHE_PATH = GUARD_UNIT_TEST_DIR "ut/name_cache";
}  // namespace

/**
 * @tc.name: name_cache_parser_test_001
 * @tc.desc: verify whether the normal name cache json file is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameCacheParserTest, name_cache_parser_test_001, TestSize.Level0)
{
    string applyNameCache = NAME_CACHE_PATH + "/" + "name_cache_parser_test_001.json";
    ark::guard::NameCacheParser parser(applyNameCache);
    parser.Parse();

    auto moduleCaches = parser.GetNameCache();
    std::string moduleName = "entry.src.main.ets.file_1.ets";
    EXPECT_EQ(moduleCaches[moduleName]->obfName, "a.ets");

    auto &annotationsAndFields = moduleCaches[moduleName]->fields;
    EXPECT_EQ(annotationsAndFields["Anno"], "c");
    EXPECT_EQ(annotationsAndFields["VAR1"], "f");
    EXPECT_EQ(annotationsAndFields["VAR2"], "g");

    auto &functions = moduleCaches[moduleName]->methods;
    EXPECT_EQ(functions["foo(i32)void"], "a");
    EXPECT_EQ(functions["foo(i32)void"], "a");
    EXPECT_EQ(functions["foo1(i32)void"], "a");
    EXPECT_EQ(functions["foo1()void"], "b");

    auto &classCache = moduleCaches[moduleName]->objects["ClassA"];
    EXPECT_EQ(classCache->obfName, "d");
    EXPECT_EQ(classCache->methods["method1()void"], "d");
    EXPECT_EQ(classCache->methods["method1(std.core.String)void"], "d");
    EXPECT_EQ(classCache->methods["method2()void"], "e");
    EXPECT_EQ(classCache->methods["method2(i32)void"], "d");
    EXPECT_EQ(classCache->fields["sField"], "a");
    EXPECT_EQ(classCache->fields["field1"], "b");
    EXPECT_EQ(classCache->fields["field2 f64"], "c");

    auto &enumCache = moduleCaches[moduleName]->objects["Color"];
    EXPECT_EQ(enumCache->obfName, "e");
    EXPECT_EQ(enumCache->fields["RED entry.src.main.ets.file_1.Color"], "a");
    EXPECT_EQ(enumCache->fields["GREEN entry.src.main.ets.file_1.Color"], "b");
    EXPECT_EQ(enumCache->fields["BLUE entry.src.main.ets.file_1.Color"], "c");
}

/**
 * @tc.name: name_cache_parser_test_002
 * @tc.desc: verify whether the abnormal name cache json file is failed parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameCacheParserTest, name_cache_parser_test_002, TestSize.Level0)
{
    string applyNameCache = NAME_CACHE_PATH + "/" + "name_cache_parser_test_002.json";
    ark::guard::NameCacheParser parser(applyNameCache);
    EXPECT_DEATH(parser.Parse(), "");
}

