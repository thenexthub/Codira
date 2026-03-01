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

class ReferenceDeleteTest : public AniTest {};

TEST_F(ReferenceDeleteTest, delete_null_local_ref)
{
    ani_ref nullRef;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);

    ASSERT_EQ(env_->Reference_Delete(nullRef), ANI_OK);
}

TEST_F(ReferenceDeleteTest, delete_undefined_local_ref)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ASSERT_EQ(env_->Reference_Delete(undefinedRef), ANI_OK);
}

TEST_F(ReferenceDeleteTest, delete_object_local_ref)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);

    ASSERT_EQ(env_->Reference_Delete(objectRef), ANI_OK);
}

TEST_F(ReferenceDeleteTest, delete_object_global_ref)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);
    ani_ref objectGRef;
    ASSERT_EQ(env_->GlobalReference_Create(objectRef, &objectGRef), ANI_OK);

    ASSERT_EQ(env_->Reference_Delete(objectGRef), ANI_INCORRECT_REF);
}

TEST_F(ReferenceDeleteTest, invalid_env)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetNull(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->c_api->Reference_Delete(nullptr, undefinedRef), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
