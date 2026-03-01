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

#include <cstdio>

#include "libarkbase/utils/json_parser.h"
#include <gtest/gtest.h>

namespace ark::json_parser::test {
TEST(JsonParser, ParsePrimitive)
{
    auto str = R"(
    {
        "key_0" : "key_0.value",
        "key_1" : "\"key_1\"\\.\u0020value\n"
    }
    )";

    JsonObject obj(str);
    ASSERT_TRUE(obj.IsValid());

    ASSERT_NE(obj.GetValue<JsonObject::StringT>("key_0"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("key_0"), "key_0.value");

    ASSERT_NE(obj.GetValue<JsonObject::StringT>("key_1"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("key_1"), "\"key_1\"\\. value\n");
}

// Issue: #14733
TEST(JsonParser, DISABLED_Arrays)
{
    auto str = R"(
    {
        "key_0" :
        [
            "elem0",
            [ "elem1.0", "elem1.1" ],
            "elem2"
        ],
        "key_1": []
    }
    )";

    JsonObject obj(str);
    ASSERT_TRUE(obj.IsValid());

    ASSERT_NE(obj.GetValue<JsonObject::ArrayT>("key_0"), nullptr);
    auto &mainArray = *obj.GetValue<JsonObject::ArrayT>("key_0");

    // Check [0]:
    ASSERT_NE(mainArray[0U].Get<JsonObject::StringT>(), nullptr);
    ASSERT_EQ(*mainArray[0U].Get<JsonObject::StringT>(), "elem0");

    // Check [1]:
    ASSERT_NE(mainArray[1U].Get<JsonObject::ArrayT>(), nullptr);
    auto &innerArray = *mainArray[1U].Get<JsonObject::ArrayT>();

    ASSERT_NE(innerArray[0U].Get<JsonObject::StringT>(), nullptr);
    ASSERT_EQ(*innerArray[0U].Get<JsonObject::StringT>(), "elem1.0");

    ASSERT_NE(innerArray[1U].Get<JsonObject::StringT>(), nullptr);
    ASSERT_EQ(*innerArray[1U].Get<JsonObject::StringT>(), "elem1.1");

    // Check [2]:
    ASSERT_NE(mainArray[2U].Get<JsonObject::StringT>(), nullptr);
    ASSERT_EQ(*mainArray[2U].Get<JsonObject::StringT>(), "elem2");

    ASSERT_NE(obj.GetValue<JsonObject::ArrayT>("key_1"), nullptr);
    auto &emptyArray = *obj.GetValue<JsonObject::ArrayT>("key_1");

    // Check [3]:
    ASSERT_EQ(emptyArray.size(), 0U);
}

// Issue: #14733
TEST(JsonParser, DISABLED_NestedObject)
{
    auto str = R"(
    {
        "key_0"          : "key_0.value",
        "repeated_key_1" : "repeated_key_1.value0",
        "key_1" :
        {
            "key_0.0"        : "key_0.0.value",
            "repeated_key_1" : "repeated_key_1.value1",
            "repeated_key_2" : "repeated_key_2.value0"
        },
        "repeated_key_2" : "repeated_key_2.value1",
        "key_2" : {}
    }
    )";

    JsonObject obj(str);
    ASSERT_TRUE(obj.IsValid());

    // Check key_0:
    ASSERT_NE(obj.GetValue<JsonObject::StringT>("key_0"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("key_0"), "key_0.value");

    // Check repeated_key_1 (in main obj):
    ASSERT_NE(obj.GetValue<JsonObject::StringT>("repeated_key_1"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("repeated_key_1"), "repeated_key_1.value0");

    // Inner object:
    ASSERT_NE(obj.GetValue<JsonObject::JsonObjPointer>("key_1"), nullptr);
    const auto *innerObj = obj.GetValue<JsonObject::JsonObjPointer>("key_1")->get();
    ASSERT_NE(innerObj, nullptr);
    ASSERT_TRUE(innerObj->IsValid());

    // Check key_0.0:
    ASSERT_NE(innerObj->GetValue<JsonObject::StringT>("key_0.0"), nullptr);
    ASSERT_EQ(*innerObj->GetValue<JsonObject::StringT>("key_0.0"), "key_0.0.value");

    // Check repeated_key_1:
    ASSERT_NE(innerObj->GetValue<JsonObject::StringT>("repeated_key_1"), nullptr);
    ASSERT_EQ(*innerObj->GetValue<JsonObject::StringT>("repeated_key_1"), "repeated_key_1.value1");

    // Check repeated_key_2:
    ASSERT_NE(innerObj->GetValue<JsonObject::StringT>("repeated_key_2"), nullptr);
    ASSERT_EQ(*innerObj->GetValue<JsonObject::StringT>("repeated_key_2"), "repeated_key_2.value0");

    // Check repeated_key_2 (in main obj):
    ASSERT_NE(obj.GetValue<JsonObject::StringT>("repeated_key_2"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("repeated_key_2"), "repeated_key_2.value1");

    // Check empty inner object (key_2):
    ASSERT_NE(obj.GetValue<JsonObject::JsonObjPointer>("key_2"), nullptr);
    const auto *emptyObj = obj.GetValue<JsonObject::JsonObjPointer>("key_2")->get();
    ASSERT_NE(emptyObj, nullptr);
    ASSERT_TRUE(emptyObj->IsValid());
    ASSERT_EQ(emptyObj->GetSize(), 0U);
}

// Issue: #14733
TEST(JsonParser, DISABLED_Null)
{
    auto str = R"(
    {
        "key": null
    }
    )";

    JsonObject obj(str);
    ASSERT_TRUE(obj.IsValid());

    ASSERT_NE(obj.GetValue<JsonObject::JsonObjPointer>("key"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::JsonObjPointer>("key"), nullptr);
}

// Issue: #14733
TEST(JsonParser, DISABLED_Numbers)
{
    auto str = R"(
    {
        "key_0" : 0,
        "key_1" : 128,
        "key_2" : -256,
        "key_3" : .512,
        "key_4" : 1.024,
        "key_5" : -204.8
    }
    )";

    JsonObject obj(str);
    ASSERT_TRUE(obj.IsValid());

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_0"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_0"), 0U);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_1"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_1"), 128U);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_2"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_2"), -256L);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_3"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_3"), .512);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_4"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_4"), 1.024);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_5"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_5"), -204.8);
}

TEST(JsonParser, Boolean)
{
    auto str = R"(
    {
        "key_0" : true,
        "key_1" : false
    }
    )";

    JsonObject obj(str);
    ASSERT_TRUE(obj.IsValid());

    ASSERT_NE(obj.GetValue<JsonObject::BoolT>("key_0"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::BoolT>("key_0"), true);

    ASSERT_NE(obj.GetValue<JsonObject::BoolT>("key_1"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::BoolT>("key_1"), false);
}

TEST(JsonParser, InvalidJson)
{
    auto repeatedKeys = R"(
    {
        "key_0" : "key_0.value0",
        "key_0" : "key_0.value1",
    }
    )";

    JsonObject obj(repeatedKeys);
    ASSERT_FALSE(obj.IsValid());
}

/**
 * @tc.name: UnescapeString
 * @tc.desc: Verify the json parser with unescape string.
 * @tc.type: FUNC
 * @tc.require:
 */
TEST(JsonParser, UnescapeString)
{
    std::stringstream ss(R"(
    {
        "key_0" : "\ttest_string1\r\n",
        "key_1" : "\ftest_string2\b"
    }
    )");

    JsonObject obj(ss.rdbuf());
    ASSERT_TRUE(obj.IsValid());

    ASSERT_NE(obj.GetValue<JsonObject::StringT>("key_0"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("key_0"), "\ttest_string1\r\n");

    ASSERT_NE(obj.GetValue<JsonObject::StringT>("key_1"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("key_1"), "\ftest_string2\b");

    auto invalidUnescapeStr(R"(
    {
        "key_0" : "\a"
    }
    )");
    JsonObject invalidObj(invalidUnescapeStr);
    ASSERT_FALSE(invalidObj.IsValid());
}
}  // namespace ark::json_parser::test
