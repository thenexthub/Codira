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

class ObjectGetFieldRefTest : public AniTest {
public:
    void GetTestData(ani_object *objectResult, ani_field *fieldNameResult, ani_field *fieldAgeResult)
    {
        auto bobRef = CallEtsFunction<ani_ref>("object_get_field_ref_test", "newBobObject");
        auto bob = static_cast<ani_object>(bobRef);

        ani_class cls;
        ASSERT_EQ(env_->FindClass("object_get_field_ref_test.Man", &cls), ANI_OK);

        ani_field fieldName;
        ASSERT_EQ(env_->Class_FindField(cls, "name", &fieldName), ANI_OK);

        ani_field fieldAge;
        ASSERT_EQ(env_->Class_FindField(cls, "age", &fieldAge), ANI_OK);

        *objectResult = bob;
        *fieldNameResult = fieldName;
        *fieldAgeResult = fieldAge;
    }
};

TEST_F(ObjectGetFieldRefTest, get_field_ref)
{
    ani_object bob {};
    ani_field field {};
    ani_field fieldAge {};
    GetTestData(&bob, &field, &fieldAge);

    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetField_Ref(bob, field, &nameRef), ANI_OK);

    auto name = static_cast<ani_string>(nameRef);
    std::array<char, 6U> buffer {};
    ani_size nameSize = 0;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 3U, buffer.data(), buffer.size(), &nameSize), ANI_OK);
    ASSERT_EQ(nameSize, 3U);
    ASSERT_STREQ(buffer.data(), "Bob");
}

TEST_F(ObjectGetFieldRefTest, get_field_ref_invalid_env)
{
    ani_object bob {};
    ani_field field {};
    ani_field fieldAge {};
    GetTestData(&bob, &field, &fieldAge);

    ani_ref nameRef {};
    ASSERT_EQ(env_->c_api->Object_GetField_Ref(nullptr, bob, field, &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldRefTest, get_field_ref_invalid_field_type)
{
    ani_object bob {};
    ani_field field {};
    ani_field fieldAge {};
    GetTestData(&bob, &field, &fieldAge);

    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetField_Ref(bob, fieldAge, &nameRef), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetFieldRefTest, invalid_argument1)
{
    ani_object bob {};
    ani_field field {};
    ani_field fieldAge {};
    GetTestData(&bob, &field, &fieldAge);

    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetField_Ref(nullptr, field, &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldRefTest, invalid_argument2)
{
    ani_object bob {};
    ani_field field {};
    ani_field fieldAge {};
    GetTestData(&bob, &field, &fieldAge);

    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetField_Ref(bob, nullptr, &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldRefTest, invalid_argument3)
{
    ani_object bob {};
    ani_field field {};
    ani_field fieldAge {};
    GetTestData(&bob, &field, &fieldAge);

    ASSERT_EQ(env_->Object_GetField_Ref(bob, field, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
