/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "utils/json_builder.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <iomanip>
#include <limits>
#include <utility>

namespace panda::test {
MATCHER_P(StringifiesAs, string, "")
{
    auto value = JsonArrayBuilder().Add(arg).Build();
    if (value.size() < 2)
        return false;

    value = value.substr(1, value.size() - 2);
    *result_listener << std::quoted(value);
    return value == string;
}

TEST(JsonBuilderTest, StringifiesNull)
{
    EXPECT_THAT(nullptr, StringifiesAs("null"));
}

TEST(JsonBuilderTest, StringifiesBooleans)
{
    EXPECT_THAT(false, StringifiesAs("false"));
    EXPECT_THAT(true, StringifiesAs("true"));
}

TEST(JsonBuilderTest, StringifiesNumbers)
{
    EXPECT_THAT(1, StringifiesAs("1"));
    EXPECT_THAT(1.5, StringifiesAs("1.5"));
    EXPECT_THAT(-100, StringifiesAs("-100"));
    EXPECT_THAT(1e24, StringifiesAs("1e+24"));
    EXPECT_THAT(1e-24, StringifiesAs("1e-24"));
    EXPECT_THAT(std::numeric_limits<double>::infinity(), StringifiesAs("null"));
    EXPECT_THAT(-std::numeric_limits<double>::infinity(), StringifiesAs("null"));
    EXPECT_THAT(std::numeric_limits<double>::quiet_NaN(), StringifiesAs("null"));
}

TEST(JsonBuilderTest, StringifiesStrings)
{
    EXPECT_THAT("", StringifiesAs(R"("")"));
    EXPECT_THAT("foo", StringifiesAs(R"("foo")"));
    EXPECT_THAT(R"("foo" bar)", StringifiesAs(R"("\"foo\" bar")"));
    EXPECT_THAT(R"("foo\" bar)", StringifiesAs(R"("\"foo\\\" bar")"));
    EXPECT_THAT(R"(\0\\1\\\2\\\\3)", StringifiesAs(R"("\\0\\\\1\\\\\\2\\\\\\\\3")"));
    EXPECT_THAT("\b\f\n\r\t", StringifiesAs(R"("\b\f\n\r\t")"));
    EXPECT_THAT("foo\tbar\n\21\1", StringifiesAs(R"("foo\tbar\n\u0011\u0001")"));
}

TEST(JsonBuilderTest, StringifiesArrays)
{
    EXPECT_THAT([](JsonArrayBuilder &) {}, StringifiesAs("[]"));
    EXPECT_THAT(
        [](JsonArrayBuilder &array) {
            array.Add(1);
            array.Add("");
            array.Add([](JsonArrayBuilder &) {});
            array.Add([](JsonObjectBuilder &object) {
                object.AddProperty("x", [](JsonArrayBuilder &x) {
                    x.Add("foo");
                    x.Add("bar");
                });
            });
        },
        StringifiesAs("[1,\"\",[],{\"x\":[\"foo\",\"bar\"]}]"));
}

TEST(JsonBuilderTest, StringifiesObjects)
{
    EXPECT_THAT([](JsonObjectBuilder &) {}, StringifiesAs("{}"));
    EXPECT_THAT(
        [](JsonObjectBuilder &object) {
            object.AddProperty("x", 1);
            object.AddProperty("y", [](JsonObjectBuilder &y) {
                y.AddProperty("a", "foo");
                y.AddProperty("b", [](JsonObjectBuilder &) {});
            });
        },
        StringifiesAs("{\"x\":1,\"y\":{\"a\":\"foo\",\"b\":{}}}"));
}

TEST(JsonArrayBuilderTest, BuildsFluently)
{
    EXPECT_EQ(
        JsonArrayBuilder().Add(1).Add("foo").Add([](JsonArrayBuilder &x) { x.Add([](JsonArrayBuilder &) {}); }).Build(),
        "[1,\"foo\",[[]]]");
}

TEST(JsonArrayBuilderTest, BuildsReferentially)
{
    JsonArrayBuilder builder;

    builder.Add(1);
    builder.Add("foo");
    builder.Add([](JsonArrayBuilder &x) { x.Add([](JsonArrayBuilder &) {}); });

    EXPECT_EQ(std::move(builder).Build(), "[1,\"foo\",[[]]]");
}

TEST(JsonObjectBuilderTest, BuildsFluently)
{
    auto z = [](JsonObjectBuilder &obj) { obj.AddProperty("_", [](JsonObjectBuilder &) {}); };

    EXPECT_EQ(JsonObjectBuilder().AddProperty("x", 1).AddProperty("y", "foo").AddProperty("z", z).Build(),
              R"({"x":1,"y":"foo","z":{"_":{}}})");
}

TEST(JsonObjectBuilderTest, BuildsReferentially)
{
    JsonObjectBuilder builder;

    builder.AddProperty("x", 1);
    builder.AddProperty("y", "foo");
    builder.AddProperty("z", [](JsonObjectBuilder &z) { z.AddProperty("_", [](JsonObjectBuilder &) {}); });

    EXPECT_EQ(std::move(builder).Build(), R"({"x":1,"y":"foo","z":{"_":{}}})");
}
}  // namespace panda::test
