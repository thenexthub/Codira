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

class GetNullTest : public AniTest {};

TEST_F(GetNullTest, get_null)
{
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->GetNull(&ref), ANI_OK);

    auto isNull = CallEtsFunction<ani_boolean>("get_null_test", "isNull", ref);
    ASSERT_EQ(isNull, ANI_TRUE);
}

TEST_F(GetNullTest, invalid_argument)
{
    ASSERT_EQ(env_->GetNull(nullptr), ANI_INVALID_ARGS);
}

TEST_F(GetNullTest, invalid_argument2)
{
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->GetNull(nullptr, &ref), ANI_INVALID_ARGS);
}

TEST_F(GetNullTest, testGetNull)
{
    ani_ref ref = nullptr;
    ani_env *env = nullptr;
    ASSERT_EQ(vm_->GetEnv(ANI_VERSION_1, &env), ANI_OK);
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env->GetNull(&ref), ANI_OK);
        auto isNull = CallEtsFunction<ani_boolean>("get_null_test", "isNull", ref);
        ASSERT_EQ(isNull, ANI_TRUE);
    }
}
}  // namespace ark::ets::ani::testing
