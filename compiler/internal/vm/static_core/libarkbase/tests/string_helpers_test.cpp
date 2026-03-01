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

#include "libarkbase/utils/string_helpers.h"

#include <cerrno>
#include <cstdarg>

#include <sstream>

#include <gtest/gtest.h>

namespace ark::helpers::string::test {

TEST(StringHelpers, Format)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    EXPECT_EQ(Format("abc"), "abc");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    EXPECT_EQ(Format("%s %d %c", "a", 10U, 0x31U), "a 10 1");

    std::stringstream ss;
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (size_t i = 0; i < 10000U; i++) {
        ss << " ";
    }

    ss << "abc";

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    EXPECT_EQ(Format("%10003s", "abc"), ss.str());
}

TEST(StringHelpers, ParseIntCheckFormat)
{
    errno = 0;
    int i = 0;
    // check format
    ASSERT_FALSE(ParseInt("x", &i));
    ASSERT_EQ(EINVAL, errno);
    errno = 0;
    ASSERT_FALSE(ParseInt("123x", &i));
    ASSERT_EQ(EINVAL, errno);

    ASSERT_TRUE(ParseInt("123", &i));
    ASSERT_EQ(123U, i);
    ASSERT_EQ(0U, errno);
    i = 0;
    EXPECT_TRUE(ParseInt("  123", &i));
    EXPECT_EQ(123U, i);
    ASSERT_TRUE(ParseInt("-123", &i));
    ASSERT_EQ(-123L, i);
    i = 0;
    EXPECT_TRUE(ParseInt("  -123", &i));
    EXPECT_EQ(-123L, i);

    // NOLINTNEXTLINE(google-runtime-int)
    short s = 0;
    ASSERT_TRUE(ParseInt("1234", &s));
    ASSERT_EQ(1234U, s);
}

TEST(StringHelpers, ParseIntCheckRange)
{
    // check range
    int i = 0;
    ASSERT_TRUE(ParseInt("12", &i, 0, 15));
    ASSERT_EQ(12U, i);
    errno = 0;
    ASSERT_FALSE(ParseInt("-12", &i, 0, 15));
    ASSERT_EQ(ERANGE, errno);
    errno = 0;
    ASSERT_FALSE(ParseInt("16", &i, 0, 15));
    ASSERT_EQ(ERANGE, errno);

    errno = 0;
    ASSERT_FALSE(ParseInt<int>("x", nullptr));
    ASSERT_EQ(EINVAL, errno);
    errno = 0;
    ASSERT_FALSE(ParseInt<int>("123x", nullptr));
    ASSERT_EQ(EINVAL, errno);
    ASSERT_TRUE(ParseInt<int>("1234", nullptr));

    i = 0;
    ASSERT_TRUE(ParseInt("0123", &i));
    ASSERT_EQ(123U, i);

    i = 0;
    ASSERT_TRUE(ParseInt("0x123", &i));
    ASSERT_EQ(0x123U, i);
    i = 0;
    EXPECT_TRUE(ParseInt("  0x123", &i));
    EXPECT_EQ(0x123U, i);

    i = 0;
    ASSERT_TRUE(ParseInt(std::string("123"), &i));
    ASSERT_EQ(123U, i);

    // NOLINTNEXTLINE(readability-magic-numbers)
    i = 123;
    ASSERT_FALSE(ParseInt("456x", &i));
    ASSERT_EQ(123U, i);
}

}  // namespace ark::helpers::string::test
