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

class ReferenceIsNullTest : public AniTest {
public:
    static constexpr int32_t LOOP_COUNT = 3;
};

TEST_F(ReferenceIsNullTest, check_null)
{
    auto ref = CallEtsFunction<ani_ref>("reference_is_null_test", "GetNull");
    ani_boolean isNull = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsNull(ref, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_TRUE);
}

TEST_F(ReferenceIsNullTest, check_undefined)
{
    auto ref = CallEtsFunction<ani_ref>("reference_is_null_test", "GetUndefined");
    ani_boolean isNull = ANI_TRUE;
    ASSERT_EQ(env_->Reference_IsNull(ref, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_FALSE);
}

TEST_F(ReferenceIsNullTest, check_object)
{
    auto ref = CallEtsFunction<ani_ref>("reference_is_null_test", "getObject");
    ani_boolean isNull = ANI_TRUE;
    ASSERT_EQ(env_->Reference_IsNull(ref, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_FALSE);
}

TEST_F(ReferenceIsNullTest, invalid_argument)
{
    auto ref = CallEtsFunction<ani_ref>("reference_is_null_test", "GetNull");
    ASSERT_EQ(env_->Reference_IsNull(ref, nullptr), ANI_INVALID_ARGS);
    ani_boolean isNull = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Reference_IsNull(nullptr, ref, &isNull), ANI_INVALID_ARGS);
}

TEST_F(ReferenceIsNullTest, check_null_loop)
{
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        auto ref = CallEtsFunction<ani_ref>("reference_is_null_test", "GetNull");
        ani_boolean isNull = ANI_FALSE;
        ASSERT_EQ(env_->Reference_IsNull(ref, &isNull), ANI_OK);
        ASSERT_EQ(isNull, ANI_TRUE);
    }
}

TEST_F(ReferenceIsNullTest, mix_test)
{
    auto ref = CallEtsFunction<ani_ref>("reference_is_null_test", "GetNull");
    ani_boolean isNull = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsNull(ref, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_TRUE);

    auto objectRef = CallEtsFunction<ani_ref>("reference_is_null_test", "getObject");
    ASSERT_EQ(env_->Reference_IsNull(objectRef, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_FALSE);

    ani_boolean isEquals = ANI_TRUE;
    ASSERT_EQ(env_->Reference_Equals(ref, objectRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceIsNullTest, get_undef_test)
{
    ani_ref ref {};
    ani_boolean isNull = ANI_FALSE;
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->Reference_IsNull(ref, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_FALSE);
}

TEST_F(ReferenceIsNullTest, get_null_test)
{
    ani_ref ref {};
    ani_boolean isNull = ANI_FALSE;
    ASSERT_EQ(env_->GetNull(&ref), ANI_OK);
    ASSERT_EQ(env_->Reference_IsNull(ref, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_TRUE);
}

}  // namespace ark::ets::ani::testing
