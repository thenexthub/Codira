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

#include "utils/math_helpers.h"

#include <cmath>
#include <gtest/gtest.h>

namespace panda::helpers::math::test {

HWTEST(MathHelpers, GetIntLog2, testing::ext::TestSize.Level0)
{
    uint64_t val = 0;
    for (uint32_t i = 1; i < 32; i++) {
        val = 1U << i;
        EXPECT_EQ(GetIntLog2(val), i);
        EXPECT_EQ(GetIntLog2(val), log2(static_cast<double>(val)));
    }

    for (int i = 1; i < 32; i++) {
        val = (1U << i) + (i == 31 ? -1 : 1);
#if !defined(NDEBUG) && defined(HOST_UT)
        EXPECT_DEATH_IF_SUPPORTED(GetIntLog2(val), "");
#endif
    }
}

HWTEST(MathHelpers, IsPowerOfTwo, testing::ext::TestSize.Level0)
{
    EXPECT_TRUE(IsPowerOfTwo(1));
    EXPECT_TRUE(IsPowerOfTwo(2));
    EXPECT_TRUE(IsPowerOfTwo(4));
    EXPECT_TRUE(IsPowerOfTwo(64));
    EXPECT_TRUE(IsPowerOfTwo(1024));
    EXPECT_TRUE(IsPowerOfTwo(2048));
    EXPECT_FALSE(IsPowerOfTwo(3));
    EXPECT_FALSE(IsPowerOfTwo(63));
    EXPECT_FALSE(IsPowerOfTwo(65));
    EXPECT_FALSE(IsPowerOfTwo(100));
}

HWTEST(MathHelpers, GetPowerOfTwoValue32, testing::ext::TestSize.Level0)
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

HWTEST(MathHelpers, MinAndMaxTest, testing::ext::TestSize.Level0)
{
    ASSERT_EQ(0.0, panda::helpers::math::min<float>(0.0, 0.0));
    ASSERT_EQ(1.0, panda::helpers::math::min<float>(1.0, 2.0));

    ASSERT_EQ(0.0, panda::helpers::math::max<float>(0.0, 0.0));
    ASSERT_EQ(2.0, panda::helpers::math::max<float>(2.0, 1.0));
}

}  // namespace panda::helpers::math::test
