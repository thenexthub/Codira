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

#include "libarkbase/utils/utils.h"
#include "util/function_traits.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace ark::verifier::test {

struct SquareSum {
    int operator()(int x, int y) const
    {
        return (x + y) * (x + y);
    }
};

struct SquareDiversity {
    int operator()(int x, int y) const
    {
        return (x - y) * (x - y);
    }
};

struct MultByMod {
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    int mod;
    explicit MultByMod(int module) : mod {module} {}
    int operator()(int x, int y) const
    {
        return (x * y) % mod;
    }
};

TEST_F(VerifierTest, function_traits)
{
    SquareSum sqSum;
    SquareDiversity sqDiv;
    NAry<SquareSum> opSSum {sqSum};
    NAry<SquareDiversity> opSDiv {sqDiv};
    EXPECT_EQ(opSSum(2_I, 2_I), 16_I);
    EXPECT_EQ(opSDiv(2_I, 1_I), 1_I);
    EXPECT_EQ(opSSum(2_I, 1_I, 2_I), 121_I);
    EXPECT_EQ(opSDiv(2_I, 1_I, 2_I), 1_I);

    MultByMod mod5 {5_I};
    // NOLINTNEXTLINE(readability-magic-numbers)
    MultByMod mod10 {10_I};
    NAry<MultByMod> opMultMod5 {mod5};
    NAry<MultByMod> opMultMod10 {mod10};
    EXPECT_EQ(opMultMod5(2_I, 4_I), 3_I);
    EXPECT_EQ(opMultMod10(2_I, 4_I), 8_I);
    EXPECT_EQ(opMultMod5(2_I, 4_I, 2_I), 1_I);
    EXPECT_EQ(opMultMod10(2_I, 4_I, 2_I), 6_I);
}

}  // namespace ark::verifier::test
