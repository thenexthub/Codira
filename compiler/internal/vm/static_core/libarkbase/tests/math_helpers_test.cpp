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

#include "libarkbase/utils/math_helpers.h"

#include <cmath>
#include <gtest/gtest.h>

namespace ark::helpers::math::test {

// NOLINTBEGIN(readability-magic-numbers,hicpp-signed-bitwise)

TEST(MathHelpers, GetIntLog2)
{
    for (size_t i = 1; i < 32U; i++) {
        uint64_t val = 1U << i;
        EXPECT_EQ(GetIntLog2(val), i);
        EXPECT_EQ(GetIntLog2(val), log2(static_cast<double>(val)));
    }

    for (size_t i = 1; i < 32U; i++) {
        uint64_t val = (1U << i) + (i == 31U ? -1L : 1U);
        (void)val;
#ifndef NDEBUG
        EXPECT_DEATH_IF_SUPPORTED(GetIntLog2(val), "");
#endif
    }
}

TEST(MathHelpers, IsPowerOfTwo)
{
    auto testIsPowerOfTwo = [](uint64_t num, bool expect) {
        EXPECT_EQ(expect, IsPowerOfTwo(num));
        EXPECT_EQ(expect, IsPowerOfTwo(bit_cast<int64_t>(num)));
        // NB! (-num) for unsigned num is well-defined for all num values (unlike for signed -(INT64_MIN))
        EXPECT_EQ(expect, IsPowerOfTwo(bit_cast<int64_t>(-num)));
    };

    testIsPowerOfTwo(0U, false);
    testIsPowerOfTwo(1U, true);
    testIsPowerOfTwo(2U, true);
    testIsPowerOfTwo(4U, true);
    testIsPowerOfTwo(64U, true);
    testIsPowerOfTwo(1024U, true);
    testIsPowerOfTwo(2048U, true);
    testIsPowerOfTwo(3U, false);
    testIsPowerOfTwo(63U, false);
    testIsPowerOfTwo(65U, false);
    testIsPowerOfTwo(100U, false);
    testIsPowerOfTwo(bit_cast<uint64_t>(std::numeric_limits<int64_t>::min()), true);
}

TEST(MathHelpers, AbsOrMin)
{
    auto testAbsOrMin = [](int64_t num, int64_t expect) { EXPECT_EQ(expect, AbsOrMin(num)); };

    testAbsOrMin(0L, 0L);
    testAbsOrMin(1L, 1L);
    testAbsOrMin(10L, 10L);
    testAbsOrMin(1000L, 1000L);

    testAbsOrMin(-1L, 1L);
    testAbsOrMin(-10L, 10L);
    testAbsOrMin(-1000L, 1000L);

    // NB! AbsOrMin(INT64_MIN) gives INT64_MIN (negative value)
    testAbsOrMin(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::min());
    testAbsOrMin(std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::max());
}

TEST(MathHelpers, GetPowerOfTwoValue32)
{
    for (int i = 0; i <= 1; i++) {
        EXPECT_EQ(GetPowerOfTwoValue32(i), 1U);
    }
    for (int i = 2; i <= 2; i++) {
        EXPECT_EQ(GetPowerOfTwoValue32(i), 2U);
    }
    for (int i = 9; i <= 16; i++) {
        EXPECT_EQ(GetPowerOfTwoValue32(i), 16U);
    }
    for (int i = 33; i <= 64; i++) {
        EXPECT_EQ(GetPowerOfTwoValue32(i), 64U);
    }
    for (int i = 1025; i <= 2048; i++) {
        EXPECT_EQ(GetPowerOfTwoValue32(i), 2048U);
    }
}

// NOLINTEND(readability-magic-numbers,hicpp-signed-bitwise)

}  // namespace ark::helpers::math::test
