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
#include <sstream>

#include "utils/json_parser.h"
#include <gtest/gtest.h>

namespace panda::json_parser::test {
HWTEST(JsonParser, ParsePrimitive, testing::ext::TestSize.Level0)
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

    ASSERT_EQ(obj.GetValue<JsonObject::StringT>("key_2"), nullptr);
}

HWTEST(JsonParser, Arrays, testing::ext::TestSize.Level0)
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
    auto &main_array = *obj.GetValue<JsonObject::ArrayT>("key_0");

    // Check [0]:
    ASSERT_NE(main_array[0].Get<JsonObject::StringT>(), nullptr);
    ASSERT_EQ(*main_array[0].Get<JsonObject::StringT>(), "elem0");

    // Check [1]:
    ASSERT_NE(main_array[1].Get<JsonObject::ArrayT>(), nullptr);
    auto &inner_array = *main_array[1].Get<JsonObject::ArrayT>();

    ASSERT_NE(inner_array[0].Get<JsonObject::StringT>(), nullptr);
    ASSERT_EQ(*inner_array[0].Get<JsonObject::StringT>(), "elem1.0");

    ASSERT_NE(inner_array[1].Get<JsonObject::StringT>(), nullptr);
    ASSERT_EQ(*inner_array[1].Get<JsonObject::StringT>(), "elem1.1");

    // Check [2]:
    ASSERT_NE(main_array[2].Get<JsonObject::StringT>(), nullptr);
    ASSERT_EQ(*main_array[2].Get<JsonObject::StringT>(), "elem2");

    ASSERT_NE(obj.GetValue<JsonObject::ArrayT>("key_1"), nullptr);
    auto &empty_array = *obj.GetValue<JsonObject::ArrayT>("key_1");

    // Check [3]:
    ASSERT_EQ(empty_array.size(), 0U);
}

HWTEST(JsonParser, NestedObject, testing::ext::TestSize.Level0)
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
    const auto *inner_obj = obj.GetValue<JsonObject::JsonObjPointer>("key_1")->get();
    ASSERT_NE(inner_obj, nullptr);
    ASSERT_TRUE(inner_obj->IsValid());

    // Check key_0.0:
    ASSERT_NE(inner_obj->GetValue<JsonObject::StringT>("key_0.0"), nullptr);
    ASSERT_EQ(*inner_obj->GetValue<JsonObject::StringT>("key_0.0"), "key_0.0.value");

    // Check repeated_key_1:
    ASSERT_NE(inner_obj->GetValue<JsonObject::StringT>("repeated_key_1"), nullptr);
    ASSERT_EQ(*inner_obj->GetValue<JsonObject::StringT>("repeated_key_1"), "repeated_key_1.value1");

    // Check repeated_key_2:
    ASSERT_NE(inner_obj->GetValue<JsonObject::StringT>("repeated_key_2"), nullptr);
    ASSERT_EQ(*inner_obj->GetValue<JsonObject::StringT>("repeated_key_2"), "repeated_key_2.value0");

    // Check repeated_key_2 (in main obj):
    ASSERT_NE(obj.GetValue<JsonObject::StringT>("repeated_key_2"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("repeated_key_2"), "repeated_key_2.value1");

    // Check empty inner object (key_2):
    ASSERT_NE(obj.GetValue<JsonObject::JsonObjPointer>("key_2"), nullptr);
    const auto *empty_obj = obj.GetValue<JsonObject::JsonObjPointer>("key_2")->get();
    ASSERT_NE(empty_obj, nullptr);
    ASSERT_TRUE(empty_obj->IsValid());
    ASSERT_EQ(empty_obj->GetSize(), 0U);
}

HWTEST(JsonParser, Numbers, testing::ext::TestSize.Level0)
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
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_0"), 0);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_1"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_1"), 128);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_2"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_2"), -256);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_3"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_3"), .512);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_4"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_4"), 1.024);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_5"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_5"), -204.8);
}

HWTEST(JsonParser, Boolean, testing::ext::TestSize.Level0)
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

HWTEST(JsonParser, InvalidJson, testing::ext::TestSize.Level0)
{
    auto repeated_keys = R"(
    {
        "key_0" : "key_0.value0",
        "key_0" : "key_0.value1",
    }
    )";

    JsonObject obj(repeated_keys);
    ASSERT_FALSE(obj.IsValid());
}

/**
 * @tc.name: UnescapeString
 * @tc.desc: Verify the json parser with unescape string.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(JsonParser, UnescapeString, testing::ext::TestSize.Level0)
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

    auto invalid_unescape_str(R"(
    {
        "key_0" : "\a"
    }
    )");
    JsonObject invalid_obj(invalid_unescape_str);
    ASSERT_FALSE(invalid_obj.IsValid());
}
}  // namespace panda::json_parser::test
