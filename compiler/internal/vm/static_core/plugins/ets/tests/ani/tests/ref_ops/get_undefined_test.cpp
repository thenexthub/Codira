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

class GetUndefinedTest : public AniTest {};

TEST_F(GetUndefinedTest, get_undefined)
{
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);

    auto isUndefined = CallEtsFunction<ani_boolean>("get_undefined_test", "isUndefined", ref);
    ASSERT_EQ(isUndefined, ANI_TRUE);
}

TEST_F(GetUndefinedTest, invalid_argument)
{
    ASSERT_EQ(env_->GetUndefined(nullptr), ANI_INVALID_ARGS);
}

TEST_F(GetUndefinedTest, testGetUndefined)
{
    ani_ref ref = nullptr;
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
        auto isUndefined = CallEtsFunction<ani_boolean>("get_undefined_test", "isUndefined", ref);
        ASSERT_EQ(isUndefined, ANI_TRUE);
    }
}
}  // namespace ark::ets::ani::testing
