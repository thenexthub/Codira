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

class GlobalReferenceDeleteTest : public AniTest {};

TEST_F(GlobalReferenceDeleteTest, delete_null_gref)
{
    ani_ref nullRef;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ani_ref nullGRef;
    ASSERT_EQ(env_->GlobalReference_Create(nullRef, &nullGRef), ANI_OK);

    ASSERT_EQ(env_->GlobalReference_Delete(nullGRef), ANI_OK);
}

TEST_F(GlobalReferenceDeleteTest, delete_undefined_gref)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_ref undefinedGRef;
    ASSERT_EQ(env_->GlobalReference_Create(undefinedRef, &undefinedGRef), ANI_OK);

    ASSERT_EQ(env_->GlobalReference_Delete(undefinedGRef), ANI_OK);
}

TEST_F(GlobalReferenceDeleteTest, delete_object_gref)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);
    ani_ref objectGRef;
    ASSERT_EQ(env_->GlobalReference_Create(objectRef, &objectGRef), ANI_OK);

    ASSERT_EQ(env_->GlobalReference_Delete(objectGRef), ANI_OK);
}

TEST_F(GlobalReferenceDeleteTest, delete_object_local_ref)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);

    ASSERT_EQ(env_->GlobalReference_Delete(objectRef), ANI_INCORRECT_REF);
}

TEST_F(GlobalReferenceDeleteTest, invalid_env)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_ref undefinedGRef;
    ASSERT_EQ(env_->GlobalReference_Create(undefinedRef, &undefinedGRef), ANI_OK);

    ASSERT_EQ(env_->c_api->GlobalReference_Delete(nullptr, undefinedGRef), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
