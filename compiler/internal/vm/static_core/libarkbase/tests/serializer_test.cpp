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

#include "libarkbase/serializer/serializer.h"
#include "libarkbase/utils/utils.h"
#include <gtest/gtest.h>

namespace ark {

class SerializatorTest : public testing::Test {
protected:
    void SetUp() override
    {
        buffer_.resize(0U);
    }
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::vector<uint8_t> buffer_;
};

template <typename T>
void SerializerTypeToBuffer(const T &type, std::vector<uint8_t> &buffer, size_t retVal)
{
    auto ret = serializer::TypeToBuffer(type, buffer);
    ASSERT_TRUE(ret);
    ASSERT_EQ(ret.Value(), retVal);
}

template <typename T>
void SerializerBufferToType(const std::vector<uint8_t> &buffer, T &type, size_t retVal)
{
    auto ret = serializer::BufferToType(buffer.data(), buffer.size(), type);
    ASSERT_TRUE(ret);
    ASSERT_EQ(ret.Value(), retVal);
}

template <typename T>
void DoTest(T value, int retVal)
{
    constexpr const int64_t IMM_FOUR = 4;
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    T a = value;
    T b {};
    std::vector<uint8_t> buffer;
    SerializerTypeToBuffer(a, buffer, retVal);
    buffer.resize(IMM_FOUR * buffer.size());
    SerializerBufferToType(buffer, b, retVal);
    ASSERT_EQ(a, value);
    ASSERT_EQ(b, value);
    ASSERT_EQ(a, b);
}

template <typename T>
void TestPod(T value)
{
    static_assert(std::is_pod<T>::value, "Type is not supported");

    DoTest(value, sizeof(value));
}

struct PodStruct {
    uint8_t a;
    int16_t b;
    uint32_t c;
    int64_t d;
    float e;
    long double f;
};
bool operator==(const PodStruct &lhs, const PodStruct &rhs)
{
    return lhs.a == rhs.a && lhs.b == rhs.b && lhs.c == rhs.c && lhs.d == rhs.d && lhs.e == rhs.e && lhs.f == rhs.f;
}

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(SerializatorTest, TestPodTypes)
{
    TestPod<uint8_t>(0xacU);
    TestPod<uint16_t>(0xc0deU);
    TestPod<uint32_t>(0x123f567fU);
    TestPod<uint64_t>(0xff12345789103c4bU);

    TestPod<int8_t>(0x1cU);
    TestPod<int16_t>(0x1ebdU);
    TestPod<int32_t>(0xfe52567fU);
    TestPod<int64_t>(0xff1234fdec57891bU);

    TestPod<float>(0.234664_D);
    TestPod<double>(22345.3453453_D);
    TestPod<long double>(99453.64345_D);

    TestPod<PodStruct>({0xffU, -23458L, 10345893U, -98343451L, -3.54634_D, 1.44e6});
}

TEST_F(SerializatorTest, TestString)
{
    DoTest<std::string>({}, 4U);
    DoTest<std::string>("", 4U);
    DoTest<std::string>("Hello World!", 4U + 12U);
    DoTest<std::string>("1", 4U + 1U);
    DoTest<std::string>({}, 4U);
}

TEST_F(SerializatorTest, TestVectorPod)
{
    DoTest<std::vector<uint8_t>>({1U, 2U, 3U, 4U}, 4U + 1U * 4U);
    DoTest<std::vector<uint16_t>>({143U, 452U, 334U}, 4U + 2U * 3U);
    DoTest<std::vector<uint32_t>>({15434U, 4564562U, 33453U, 43456U, 346346U}, 4U + 5U * 4U);
    DoTest<std::vector<uint64_t>>({14345665644345U, 34645345465U}, 4U + 8U * 2U);
    DoTest<std::vector<char>>({}, 4U + 1U * 0U);
}

TEST_F(SerializatorTest, TestUnorderedMap1)
{
    using Map = std::unordered_map<uint32_t, uint16_t>;
    DoTest<Map>(
        {
            {12343526U, 23424U},
            {3U, 234356U},
            {45764746U, 4U},
        },
        4U + 3U * (4U + 2U));
}

TEST_F(SerializatorTest, TestUnorderedMap2)
{
    using Map = std::unordered_map<std::string, std::string>;
    DoTest<Map>(
        {
            {"one", {}},
            {"two", "123"},
            {"three", ""},
            {"", {}},
        },
        4U + 4U + 3U + 4U + 0U + 4U + 3U + 4U + 3U + 4U + 5U + 4U + 0U + 4U + 0U + 4U + 0U);
}

TEST_F(SerializatorTest, TestUnorderedMap3)
{
    using Map = std::unordered_map<std::string, std::vector<uint32_t>>;
    DoTest<Map>(
        {
            {"one", {}},
            {"two", {1U, 2U, 3U, 4U}},
            {"three", {9U, 34U, 45335U}},
            {"", {}},
        },
        4U + 4U + 3U + 4U + 4U * 0U + 4U + 3U + 4U + 4U * 4U + 4U + 5U + 4U + 4U * 3U + 4U + 0U + 4U + 4U * 0U);
}

struct TestStruct {
    uint8_t a {};
    uint16_t b {};
    uint32_t c {};
    uint64_t d {};
    std::string e;
    std::vector<int> f;
};
bool operator==(const TestStruct &lhs, const TestStruct &rhs)
{
    return lhs.a == rhs.a && lhs.b == rhs.b && lhs.c == rhs.c && lhs.d == rhs.d && lhs.e == rhs.e && lhs.f == rhs.f;
}

TEST_F(SerializatorTest, TestStruct)
{
    TestStruct testStruct {1U, 2U, 3U, 4U, "Liza", {8U, 9U, 5U}};
    unsigned testRet = 1U + 2U + 4U + 8U + 4U + 4U + 4U + sizeof(int) * 3U;

    TestStruct a = testStruct;
    TestStruct b;
    ASSERT_EQ(serializer::StructToBuffer<6U>(a, buffer_), true);
    buffer_.resize(4U * buffer_.size());
    auto ret = serializer::RawBufferToStruct<6U>(buffer_.data(), buffer_.size(), b);
    ASSERT_TRUE(ret.HasValue());
    ASSERT_EQ(ret.Value(), testRet);
    ASSERT_EQ(a, testStruct);
    ASSERT_EQ(b, testStruct);
    ASSERT_EQ(a, b);
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark
