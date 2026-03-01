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
#include "util/shifted_vector.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace ark::verifier::test {

TEST_F(VerifierTest, shifted_vector)
{
    ShiftedVector<2_I, int> shiftVec {5U};
    ASSERT_EQ(shiftVec.BeginIndex(), -2_I);
    ASSERT_EQ(shiftVec.EndIndex(), 3_I);
    EXPECT_TRUE(shiftVec.InValidRange(-1_I));
    EXPECT_FALSE(shiftVec.InValidRange(5_I));

    shiftVec[0] = 7_I;
    shiftVec.At(1) = 8_I;
    EXPECT_EQ(shiftVec.At(0), 7_I);
    EXPECT_EQ(shiftVec[1], 8_I);

    shiftVec.ExtendToInclude(5_I);
    ASSERT_EQ(shiftVec.BeginIndex(), -2_I);
    ASSERT_EQ(shiftVec.EndIndex(), 6_I);
    EXPECT_TRUE(shiftVec.InValidRange(-1_I));
    EXPECT_TRUE(shiftVec.InValidRange(5_I));
    EXPECT_EQ(shiftVec.At(0), 7_I);
    EXPECT_EQ(shiftVec[1], 8_I);
    shiftVec[4U] = 4_I;
    EXPECT_EQ(shiftVec.At(4U), 4_I);
}

}  // namespace ark::verifier::test
