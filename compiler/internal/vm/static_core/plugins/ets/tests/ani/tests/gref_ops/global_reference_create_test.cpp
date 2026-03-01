/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class GlobalReferenceCreateTest : public AniTest {};

TEST_F(GlobalReferenceCreateTest, from_null_ref)
{
    auto ref = CallEtsFunction<ani_ref>("global_reference_create_test", "GetNull");
    ani_ref gref;
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref), ANI_OK);

    ani_boolean isNull {};
    ASSERT_EQ(env_->Reference_IsNull(gref, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_TRUE);
}

TEST_F(GlobalReferenceCreateTest, from_undefined_ref)
{
    auto ref = CallEtsFunction<ani_ref>("global_reference_create_test", "GetUndefined");
    ani_ref gref;
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref), ANI_OK);

    ani_boolean isUndefined {};
    ASSERT_EQ(env_->Reference_IsUndefined(gref, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);
}

TEST_F(GlobalReferenceCreateTest, from_object_ref)
{
    auto ref = CallEtsFunction<ani_ref>("global_reference_create_test", "getObject");
    ani_ref gref;
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("global_reference_create_test", "CheckObject", ref, gref), ANI_TRUE);
}

TEST_F(GlobalReferenceCreateTest, from_null_gref)
{
    auto ref = CallEtsFunction<ani_ref>("global_reference_create_test", "GetNull");
    ani_ref gref;
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref), ANI_OK);

    ani_ref gref2;
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref2), ANI_OK);

    ani_boolean isNull {};
    ASSERT_EQ(env_->Reference_IsNull(gref2, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_TRUE);
}

TEST_F(GlobalReferenceCreateTest, from_undefined_gref)
{
    auto ref = CallEtsFunction<ani_ref>("global_reference_create_test", "GetUndefined");
    ani_ref gref;
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref), ANI_OK);

    ani_ref gref2;
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref2), ANI_OK);

    ani_boolean isUndefined {};
    ASSERT_EQ(env_->Reference_IsUndefined(gref2, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);
}

TEST_F(GlobalReferenceCreateTest, from_object_gref)
{
    auto ref = CallEtsFunction<ani_ref>("global_reference_create_test", "getObject");
    ani_ref gref;
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref), ANI_OK);

    ani_ref gref2;
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref2), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("global_reference_create_test", "CheckObject", ref, gref2), ANI_TRUE);
}

TEST_F(GlobalReferenceCreateTest, delete_as_local_ref)
{
    auto ref = CallEtsFunction<ani_ref>("global_reference_create_test", "getObject");
    ani_ref gref;
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref), ANI_OK);

    ASSERT_EQ(env_->Reference_Delete(gref), ANI_INCORRECT_REF);
}

TEST_F(GlobalReferenceCreateTest, invalid_result)
{
    auto ref = CallEtsFunction<ani_ref>("global_reference_create_test", "getObject");
    ASSERT_EQ(env_->GlobalReference_Create(ref, nullptr), ANI_INVALID_ARGS);
}

TEST_F(GlobalReferenceCreateTest, global_reference_create_test)
{
    auto ref = CallEtsFunction<ani_ref>("global_reference_create_test", "getObject");
    ani_ref gref;
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("global_reference_create_test", "CheckObject", ref, gref), ANI_TRUE);

    ASSERT_EQ(env_->GlobalReference_Delete(gref), ANI_OK);
}

TEST_F(GlobalReferenceCreateTest, invalid_env)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_ref gref {};
    ASSERT_EQ(env_->c_api->GlobalReference_Create(nullptr, undefinedRef, &gref), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
