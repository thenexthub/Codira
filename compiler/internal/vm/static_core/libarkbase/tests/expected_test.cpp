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

#include "libarkbase/utils/expected.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace ark::test::expected {

// NOLINTBEGIN(readability-magic-numbers)

enum class ErrorCode { FIRST, SECOND };

static Expected<int, ErrorCode> Helper(int v)
{
    switch (v) {
        case 0U:
            return Unexpected {ErrorCode::FIRST};
        case 1U:
            return {42U};
        default:
            return Unexpected {ErrorCode::SECOND};
    }
}

struct Default {
    int v;
};

struct NonDefaultConstructible {
    NonDefaultConstructible() = delete;
};

TEST(Expected, Unexpected)
{
    int e = 1;
    auto u = Unexpected(e);
    EXPECT_EQ(Unexpected<int>(1U).Value(), 1U);
    EXPECT_EQ(u.Value(), 1U);
    EXPECT_EQ(static_cast<const Unexpected<int> &>(u).Value(), 1U);
}

TEST(Expected, Ctor)
{
    int v = 1;
    auto e = Expected<int, ErrorCode>(v);
    EXPECT_TRUE(e);
    EXPECT_EQ(e.Value(), 1U);
    EXPECT_EQ(*e, 1U);

    auto e0 = Expected<int, ErrorCode>();
    EXPECT_EQ(*e0, 0U);

    auto e1 = Expected<int, ErrorCode>(2U);
    EXPECT_EQ(e1.Value(), 2U);

    auto e2 = Expected<int, ErrorCode>(Unexpected(ErrorCode::FIRST));
    auto u = Unexpected(ErrorCode::SECOND);
    auto e3 = Expected<int, ErrorCode>(u);
    EXPECT_FALSE(e2);
    EXPECT_EQ(e2.Error(), ErrorCode::FIRST);
    EXPECT_EQ(e3.Error(), ErrorCode::SECOND);

    // Default constructor is only enabled if T is default constructible.
    EXPECT_FALSE((std::is_default_constructible_v<Expected<NonDefaultConstructible, ErrorCode>>));
}

TEST(Expected, Access)
{
    const auto e1 = Expected<int, ErrorCode>(Unexpected(ErrorCode::FIRST));
    EXPECT_EQ(e1.Error(), ErrorCode::FIRST);
    EXPECT_EQ((Expected<int, ErrorCode>(Unexpected(ErrorCode::SECOND)).Error()), ErrorCode::SECOND);
    const auto e2 = Expected<int, ErrorCode>(1U);
    EXPECT_EQ(e2.Value(), 1U);
    EXPECT_EQ(*e2, 1U);
    EXPECT_EQ((*Expected<int, ErrorCode>(2U)), 2U);
    EXPECT_EQ((Expected<int, ErrorCode>(3U).Value()), 3U);
}

TEST(Expected, Assignment)
{
    auto d = Default {1U};
    Expected<Default, ErrorCode> t = d;
    t.Value() = Default {2U};
    EXPECT_TRUE(t);
    EXPECT_EQ((*t).v, 2U);
    t = Unexpected(ErrorCode::FIRST);
    EXPECT_FALSE(t);
    EXPECT_EQ(t.Error(), ErrorCode::FIRST);
}

TEST(Expected, Basic)
{
    auto res1 = Helper(0U);
    auto res2 = Helper(1U);
    auto res3 = Helper(2U);
    EXPECT_FALSE(res1);
    EXPECT_TRUE(res2);
    EXPECT_FALSE(res3);
    EXPECT_EQ(res1.Error(), ErrorCode::FIRST);
    EXPECT_EQ(*res2, 42U);
    EXPECT_EQ(res3.Error(), ErrorCode::SECOND);
}

TEST(Expected, ValueOr)
{
    auto res1 = Helper(0U).ValueOr(1U);
    auto res2 = Helper(res1).ValueOr(res1);
    auto e = Expected<int, ErrorCode>(1U);
    EXPECT_EQ(res1, 1U);
    EXPECT_EQ(res2, 42U);
    EXPECT_EQ(e.ValueOr(0U), 1U);
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::test::expected
