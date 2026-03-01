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

#include "libarkbase/utils/span.h"

#include <array>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

namespace ark::test::span {

template <class T>
std::string ToString(Span<T> s)
{
    std::ostringstream oss;
    for (const auto &e : s) {
        oss << e << " ";
    }
    return oss.str();
}
template <class T>
Span<T> Double(Span<T> s)
{
    for (auto &e : s) {
        e *= 2U;
    }
    return s;
}

// NOLINTBEGIN(readability-magic-numbers,modernize-avoid-c-arrays)
TEST(Span, Conversions)
{
    int c[] {1U, 2U, 3U};
    std::vector v {4U, 5U, 6U};
    const std::vector constV {-4L, -5L, -6L};
    std::array a {7U, 8U, 9U};
    size_t sz = 3;
    auto p = std::make_unique<int[]>(sz);
    p[0U] = 10;
    p[1U] = 11;
    p[2U] = 12;
    std::string s = " !\"";

    EXPECT_EQ(ToString(Double(Span(c))), "2 4 6 ");
    EXPECT_EQ(ToString(Double(Span(v))), "8 10 12 ");
    EXPECT_EQ(ToString(Span(constV)), "-4 -5 -6 ");
    EXPECT_EQ(ToString(Double(Span(a))), "14 16 18 ");
    EXPECT_EQ(ToString(Double(Span(p.get(), sz))), "20 22 24 ");
    EXPECT_EQ(ToString(Double(Span(p.get(), p.get() + 2U))), "40 44 ");
    EXPECT_EQ(ToString(Double(Span(s))), "@ B D ");
}

TEST(Span, SubSpan)
{
    int c[] {1U, 2U, 3U, 4U, 5U};
    auto s = Span(c).SubSpan(1U, 3U);
    auto f = s.First(2U);
    auto l = s.Last(2U);

    EXPECT_EQ(ToString(s), "2 3 4 ");
    EXPECT_EQ(ToString(f), "2 3 ");
    EXPECT_EQ(ToString(l), "3 4 ");
}

TEST(Span, SubSpanT)
{
    {
        uint8_t buf[] = {1U, 1U, 1U,   1U,   1U,   0U,   0U,   0U,   2U,   0U,
                         0U, 0U, 0x78, 0x56, 0x34, 0x12, 0xfe, 0xff, 0xff, 0xff};
        struct Foo {
            uint32_t a;
            int32_t b;
        };
        auto sp = Span(buf);
#ifndef NDEBUG
        ASSERT_DEATH(sp.SubSpan<Foo>(4U, 3U), ".*");
        ASSERT_DEATH(sp.SubSpan<Foo>(3U, 2U), ".*");
#endif
        auto subSp = sp.SubSpan<Foo>(4U, 2U);
        ASSERT_EQ(subSp.size(), 2U);
        ASSERT_EQ(subSp[0U].a, 1U);
        ASSERT_EQ(subSp[0U].b, 2U);
        ASSERT_EQ(subSp[1U].a, 0x12345678U);
        ASSERT_EQ(subSp[1U].b, -2L);
    }
    {
        uint32_t buf[] = {0x01020304U, 0x05060708U, 0x090a0b0cU};
        auto sp = Span(buf);
#ifndef NDEBUG
        ASSERT_DEATH(sp.SubSpan<uint16_t>(4U, 1U), ".*");
#endif
        auto subSp = sp.SubSpan<uint16_t>(1U, 4U);
        ASSERT_EQ(subSp.size(), 4U);
        ASSERT_EQ(subSp[0U], 0x0708U);
        ASSERT_EQ(subSp[1U], 0x0506U);
        ASSERT_EQ(subSp[2U], 0x0b0cU);
        ASSERT_EQ(subSp[3U], 0x090aU);
    }
}

TEST(Span, AsBytes)
{
    const int c1[] {1U, 2U, 3U};
    int c2[] {4U, 5U, 6U};
    auto cs = Span(c1);
    auto s = Span(c2);
    EXPECT_EQ(cs.SizeBytes(), 12U);
    EXPECT_EQ(AsBytes(cs)[sizeof(int)], static_cast<std::byte>(2U));
    AsWritableBytes(s)[4U] = static_cast<std::byte>(1U);
    EXPECT_EQ(s[1U], 1U);
}

// NOLINTEND(readability-magic-numbers,modernize-avoid-c-arrays)
}  // namespace ark::test::span
