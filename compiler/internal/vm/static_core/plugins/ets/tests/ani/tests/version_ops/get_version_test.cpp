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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class GetVersionTest : public AniTest {
public:
    static constexpr int32_t LOOP_COUNT = 3;
};

TEST_F(GetVersionTest, valid_argument)
{
    uint32_t aniVersion = 0;
    ASSERT_EQ(env_->GetVersion(&aniVersion), ANI_OK);
    ASSERT_EQ(aniVersion, ANI_VERSION_1);
}

TEST_F(GetVersionTest, invalid_argument)
{
    ASSERT_EQ(env_->GetVersion(nullptr), ANI_INVALID_ARGS);
}

TEST_F(GetVersionTest, invalid_argument_2)
{
    uint32_t aniVersion = 0;
    ASSERT_EQ(env_->c_api->GetVersion(nullptr, &aniVersion), ANI_INVALID_ARGS);
}

TEST_F(GetVersionTest, multiple_call)
{
    uint32_t aniVersion = 0;
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        aniVersion = 0;
        ASSERT_EQ(env_->GetVersion(&aniVersion), ANI_OK);
        ASSERT_EQ(aniVersion, ANI_VERSION_1);
    }
}
}  // namespace ark::ets::ani::testing
