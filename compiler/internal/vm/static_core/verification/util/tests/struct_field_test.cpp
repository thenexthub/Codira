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

#include <vector>
#include <array>
#include <map>

#include "libarkbase/utils/utils.h"
#include "util/struct_field.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace ark::verifier::test {

struct MyArr final {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes,readability-magic-numbers)
    int32_t num0 = 0;
    int32_t num1 = 1;
    int32_t num2 = 2;
    int32_t num3 = 3;
    int32_t num4 = 4;
    int32_t num5 = 5;

    using Offset = StructField<int32_t, int32_t>;

    std::map<char, Offset> elemMap = {{'0', Offset {0}},  {'1', Offset {4}},  {'2', Offset {8}},
                                      {'3', Offset {12}}, {'4', Offset {16}}, {'5', Offset {20}}};
    // NOLINTEND(misc-non-private-member-variables-in-classes,readability-magic-numbers)

    int32_t &Access(char id)
    {
        auto it = elemMap.find(id);
        ASSERT(it != elemMap.end());
        return it->second.Of(num0);
    }
};

TEST_F(VerifierTest, struct_field)
{
    std::vector<int32_t> vec(5U);
    vec[3U] = 5_I;
    int32_t &pos1 = vec[1U];
    StructField<int32_t, int32_t> sF1 {8U};
    int32_t &pos2 = sF1.Of(pos1);
    EXPECT_EQ(pos2, 5_I);

    std::array<int64_t, 3U> arr {};
    arr[2U] = 5L;
    int64_t &pos3 = arr[0];
    // NOLINTNEXTLINE(readability-magic-numbers)
    StructField<int64_t, int64_t> sF2 {16U};
    int64_t &pos4 = sF2.Of(pos3);
    EXPECT_EQ(pos4, 5U);

    MyArr myArr;
    EXPECT_EQ(myArr.Access('3'), 3_I);
    // NOLINTNEXTLINE(readability-magic-numbers)
    myArr.Access('4') = 44_I;
    EXPECT_EQ(myArr.num4, 44_I);
}

}  // namespace ark::verifier::test
