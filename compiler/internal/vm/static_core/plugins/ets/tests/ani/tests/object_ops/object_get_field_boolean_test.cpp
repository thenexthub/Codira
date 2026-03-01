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
 * See the License for the specific langumarried governing permissions and
 * limitations under the License.
 */

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class ObjectGetFieldBooleanTest : public AniTest {
public:
    void GetTestData(ani_object *objectResult, ani_field *fieldNameResult, ani_field *fieldMarriedResult)
    {
        auto sarahRef = CallEtsFunction<ani_ref>("object_get_field_boolean_test", "newSarahObject");
        auto sarah = static_cast<ani_object>(sarahRef);

        ani_class cls;
        ASSERT_EQ(env_->FindClass("object_get_field_boolean_test.Woman", &cls), ANI_OK);

        ani_field fieldName;
        ASSERT_EQ(env_->Class_FindField(cls, "name", &fieldName), ANI_OK);

        ani_field fieldMarried;
        ASSERT_EQ(env_->Class_FindField(cls, "married", &fieldMarried), ANI_OK);

        *objectResult = sarah;
        *fieldNameResult = fieldName;
        *fieldMarriedResult = fieldMarried;
    }
};

TEST_F(ObjectGetFieldBooleanTest, get_field_boolean)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldMarried {};
    GetTestData(&sarah, &field, &fieldMarried);

    ani_boolean married = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetField_Boolean(sarah, fieldMarried, &married), ANI_OK);
    ASSERT_EQ(married, true);
}

TEST_F(ObjectGetFieldBooleanTest, get_field_invalid_env)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldMarried {};
    GetTestData(&sarah, &field, &fieldMarried);

    ani_boolean married = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Object_GetField_Boolean(nullptr, sarah, fieldMarried, &married), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldBooleanTest, get_field_boolean_invalid_field_type)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldMarried {};
    GetTestData(&sarah, &field, &fieldMarried);

    ani_boolean married = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetField_Boolean(sarah, field, &married), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetFieldBooleanTest, invalid_argument1)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldMarried {};
    GetTestData(&sarah, &field, &fieldMarried);

    ani_boolean married = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetField_Boolean(nullptr, field, &married), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldBooleanTest, invalid_argument2)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldMarried {};
    GetTestData(&sarah, &field, &fieldMarried);

    ani_boolean married = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetField_Boolean(sarah, nullptr, &married), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldBooleanTest, invalid_argument3)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldMarried {};
    GetTestData(&sarah, &field, &fieldMarried);

    ASSERT_EQ(env_->Object_GetField_Boolean(sarah, field, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
