//===-- JSONTest.cpp ------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "JSON.h"

template <typename T>
void TestJSON(JSONValue *json_val, const std::function<void(T &)> &test_func) {
  ASSERT_THAT(json_val, testing::NotNull());
  ASSERT_TRUE(T::classof(json_val));
  test_func(static_cast<T &>(*json_val));
}

JSONValue::SP ParseJSON(const char *json_string) {
  return JSONParser(json_string).ParseJSONValue();
}

template <typename T>
void ParseAndTestJSON(
    const char *json_string,
    const std::function<void(T &)> &test_func = [](T &) {}) {
  auto json_val = ParseJSON(json_string);
  TestJSON<T>(json_val.get(), test_func);
}

TEST(JSON, Parse) {
  ParseAndTestJSON<JSONString>("\"foo\"", [](JSONString &string_val) {
    EXPECT_EQ(string_val.GetData(), "foo");
  });
  EXPECT_THAT(ParseJSON("\"foo"), testing::IsNull());
  ParseAndTestJSON<JSONNumber>("3", [](JSONNumber &number_val) {
    EXPECT_EQ(number_val.GetAsSigned(), 3);
    EXPECT_EQ(number_val.GetAsUnsigned(), 3u);
    EXPECT_EQ(number_val.GetAsDouble(), 3.0);
  });
  ParseAndTestJSON<JSONNumber>("-5", [](JSONNumber &number_val) {
    EXPECT_EQ(number_val.GetAsSigned(), -5);
    EXPECT_EQ(number_val.GetAsDouble(), -5.0);
  });
  ParseAndTestJSON<JSONNumber>("-6.4", [](JSONNumber &number_val) {
    EXPECT_EQ(number_val.GetAsSigned(), -6);
    EXPECT_EQ(number_val.GetAsDouble(), -6.4);
  });
  EXPECT_THAT(ParseJSON("-1.2.3"), testing::IsNull());
  ParseAndTestJSON<JSONTrue>("true");
  ParseAndTestJSON<JSONFalse>("false");
  ParseAndTestJSON<JSONNull>("null");
  ParseAndTestJSON<JSONObject>(
      "{ \"key1\": 4, \"key2\": \"foobar\" }", [](JSONObject &obj_val) {
        TestJSON<JSONNumber>(obj_val.GetObject("key1").get(),
                             [](JSONNumber &number_val) {
                               EXPECT_EQ(number_val.GetAsSigned(), 4);
                               EXPECT_EQ(number_val.GetAsUnsigned(), 4u);
                               EXPECT_EQ(number_val.GetAsDouble(), 4.0);
                             });
        TestJSON<JSONString>(obj_val.GetObject("key2").get(),
                             [](JSONString &string_val) {
                               EXPECT_EQ(string_val.GetData(), "foobar");
                             });
      });
  ParseAndTestJSON<JSONArray>("[1, \"bar\", 3.14]", [](JSONArray &array_val) {
    EXPECT_EQ(array_val.GetNumElements(), 3u);
    TestJSON<JSONNumber>(array_val.GetObject(0).get(),
                         [](JSONNumber &number_val) {
                           EXPECT_EQ(number_val.GetAsSigned(), 1);
                           EXPECT_EQ(number_val.GetAsUnsigned(), 1u);
                           EXPECT_EQ(number_val.GetAsDouble(), 1.0);
                         });
    TestJSON<JSONString>(
        array_val.GetObject(1).get(),
        [](JSONString &string_val) { EXPECT_EQ(string_val.GetData(), "bar"); });
    TestJSON<JSONNumber>(array_val.GetObject(2).get(),
                         [](JSONNumber &number_val) {
                           EXPECT_EQ(number_val.GetAsSigned(), 3);
                           EXPECT_EQ(number_val.GetAsUnsigned(), 3u);
                           EXPECT_EQ(number_val.GetAsDouble(), 3.14);
                         });
  });
  ParseAndTestJSON<JSONArray>("[]", [](JSONArray &array_val) {
    EXPECT_EQ(array_val.GetNumElements(), 0u);
  });
}
