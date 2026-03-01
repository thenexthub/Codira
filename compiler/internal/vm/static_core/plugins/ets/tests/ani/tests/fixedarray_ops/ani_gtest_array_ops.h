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

#ifndef ANI_GTEST_ARRAY_OPS_H
#define ANI_GTEST_ARRAY_OPS_H

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class AniGTestArrayOps : public AniTest {
public:
    template <typename T, ani_size LENGTH = 5U>
    void CompareArray(const std::array<T, LENGTH> &nativeBuffer1, const std::array<T, LENGTH> &nativeBuffer2)
    {
        for (ani_size i = 0; i < LENGTH; i++) {
            ASSERT_EQ(nativeBuffer1[i], nativeBuffer2[i]);
        }
    }

    static constexpr ani_size OFFSET_0 = 0;
    static constexpr ani_size OFFSET_1 = 1;
    static constexpr ani_size OFFSET_2 = 2;
    static constexpr ani_size OFFSET_3 = 3;
    static constexpr ani_size OFFSET_4 = 4;
    static constexpr ani_size OFFSET_5 = 5;
    static constexpr ani_size LENGTH_1 = 1;
    static constexpr ani_size LENGTH_2 = 2;
    static constexpr ani_size LENGTH_3 = 3;
    static constexpr ani_size LENGTH_5 = 5;
    static constexpr ani_size LENGTH_6 = 6;
    static constexpr ani_size LENGTH_10 = 10;
    static constexpr ani_int LOOP_COUNT = 3;
};
}  // namespace ark::ets::ani::testing

#endif  // ANI_GTEST_ARRAY_OPS_H