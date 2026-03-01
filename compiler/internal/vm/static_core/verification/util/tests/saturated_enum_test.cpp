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

#include "util/saturated_enum.h"

#include "util/tests/verifier_test.h"
#include "libarkbase/utils/utils.h"

#include <gtest/gtest.h>

namespace ark::verifier::test {

enum class Numbers { ONE, TWO, THREE, FOUR, FIVE };

TEST_F(VerifierTest, saturated_enum)
{
    SaturatedEnum<Numbers, Numbers::FOUR, Numbers::THREE, Numbers::TWO, Numbers::ONE> sEnum;
    EXPECT_EQ(sEnum, Numbers::ONE);

#ifndef NDEBUG
    EXPECT_DEATH(sEnum[Numbers::FIVE], "");
#endif

    EXPECT_FALSE(sEnum[Numbers::TWO]);
    EXPECT_TRUE(sEnum[Numbers::ONE]);

    sEnum |= Numbers::THREE;
    EXPECT_EQ(sEnum, Numbers::THREE);
    EXPECT_FALSE(sEnum[Numbers::FOUR]);
    EXPECT_TRUE(sEnum[Numbers::TWO]);

    int i = 0;
    sEnum.EnumerateValues([&i](Numbers) {
        i++;
        return true;
    });
    EXPECT_EQ(i, 3_I);

    i = 0;
    sEnum.EnumerateValues([&i](Numbers en) {
        i++;
        return en != Numbers::TWO;
    });
    EXPECT_EQ(i, 2_I);
}

}  // namespace ark::verifier::test