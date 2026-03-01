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

class ObjectGetFieldIntTest : public AniTest {
public:
    void GetTestData(ani_object *objectResult, ani_field *fieldNameResult, ani_field *fieldAgeResult)
    {
        auto sarahRef = CallEtsFunction<ani_ref>("object_get_field_int_test", "newSarahObject");
        auto sarah = static_cast<ani_object>(sarahRef);

        ani_class cls;
        ASSERT_EQ(env_->FindClass("object_get_field_int_test.Woman", &cls), ANI_OK);

        ani_field fieldName;
        ASSERT_EQ(env_->Class_FindField(cls, "name", &fieldName), ANI_OK);

        ani_field fieldAge;
        ASSERT_EQ(env_->Class_FindField(cls, "age", &fieldAge), ANI_OK);

        *objectResult = sarah;
        *fieldNameResult = fieldName;
        *fieldAgeResult = fieldAge;
    }
};

TEST_F(ObjectGetFieldIntTest, get_field_int)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldAge {};
    GetTestData(&sarah, &field, &fieldAge);

    ani_int age = 0U;
    ASSERT_EQ(env_->Object_GetField_Int(sarah, fieldAge, &age), ANI_OK);
    ASSERT_EQ(age, 24U);
}

TEST_F(ObjectGetFieldIntTest, get_field_int_invalid_env)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldAge {};
    GetTestData(&sarah, &field, &fieldAge);

    ani_int age = 0U;
    ASSERT_EQ(env_->c_api->Object_GetField_Int(nullptr, sarah, fieldAge, &age), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldIntTest, get_field_int_invalid_field_type)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldAge {};
    GetTestData(&sarah, &field, &fieldAge);

    ani_int age = 0U;
    ASSERT_EQ(env_->Object_GetField_Int(sarah, field, &age), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetFieldIntTest, invalid_argument1)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldAge {};
    GetTestData(&sarah, &field, &fieldAge);

    ani_int age = 0U;
    ASSERT_EQ(env_->Object_GetField_Int(nullptr, field, &age), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldIntTest, invalid_argument2)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldAge {};
    GetTestData(&sarah, &field, &fieldAge);

    ani_int age = 0U;
    ASSERT_EQ(env_->Object_GetField_Int(sarah, nullptr, &age), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldIntTest, invalid_argument3)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldAge {};
    GetTestData(&sarah, &field, &fieldAge);

    ASSERT_EQ(env_->Object_GetField_Int(sarah, field, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldIntTest, check_hierarchy)
{
    ani_class clsParent {};
    ASSERT_EQ(env_->FindClass("object_get_field_int_test.Parent", &clsParent), ANI_OK);
    ani_method ctorParent {};
    ASSERT_EQ(env_->Class_FindMethod(clsParent, "<ctor>", ":", &ctorParent), ANI_OK);
    ani_object objParent {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_New(clsParent, ctorParent, &objParent), ANI_OK);

    ani_class clsChild {};
    ASSERT_EQ(env_->FindClass("object_get_field_int_test.Child", &clsChild), ANI_OK);
    ani_method ctorChild {};
    ASSERT_EQ(env_->Class_FindMethod(clsChild, "<ctor>", ":", &ctorChild), ANI_OK);
    ani_object objChild {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_New(clsChild, ctorChild, &objChild), ANI_OK);

    ani_field parentFieldInParent {};
    ASSERT_EQ(env_->Class_FindField(clsParent, "parentField", &parentFieldInParent), ANI_OK);
    ani_field parentFieldInChild {};
    ASSERT_EQ(env_->Class_FindField(clsChild, "parentField", &parentFieldInChild), ANI_OK);
    ani_field childFieldInParent {};
    ASSERT_EQ(env_->Class_FindField(clsParent, "childField", &childFieldInParent), ANI_NOT_FOUND);
    ani_field childFieldInChild {};
    ASSERT_EQ(env_->Class_FindField(clsChild, "childField", &childFieldInChild), ANI_OK);
    ani_field overridedFieldInParent {};
    ASSERT_EQ(env_->Class_FindField(clsParent, "overridedField", &overridedFieldInParent), ANI_OK);
    ani_field overridedFieldInChild {};
    ASSERT_EQ(env_->Class_FindField(clsChild, "overridedField", &overridedFieldInChild), ANI_OK);

    const ani_int parentFieldValueInParent = 1;
    const ani_int overridedFieldValueInParent = 2;
    const ani_int childFieldValueInChild = 3;
    const ani_int overridedFieldValueInChild = 4;

    // |-------------------------------------------------------------------------------------------------------|
    // | ani_object  |             ani_field              |  ani_status  |               action                |
    // |-------------|------------------------------------|--------------|-------------------------------------|
    // |   parent    |   parentField from Parent class    |    ANI_OK    |    access to parent.parentField     |
    // |   parent    |   parentField from Child class     |    ANI_OK    |    access to parent.parentField     |
    // |   parent    |    childField from Child class     |      UB      |                 --                  |
    // |   parent    |  overridedField from Child class   |    ANI_OK    |   access to parent.overridedField   |
    // |   parent    |  overridedField from Parent class  |    ANI_OK    |   access to parent.overridedField   |
    // |   child     |    parentField from Parent class   |    ANI_OK    |    access to parent.parentField     |
    // |   child     |    parentField from Child class    |    ANI_OK    |    access to parent.parentField     |
    // |   child     |    childField from Child class     |    ANI_OK    |     access to child.childField      |
    // |   child     |  overridedField from Child class   |    ANI_OK    |   access to parent.overridedField   |
    // |   child     |  overridedField from Parent class  |    ANI_OK    |   access to parent.overridedField   |
    // |-------------|------------------------------------|--------------|-------------------------------------|

    ani_int value {};
    ASSERT_EQ(env_->Object_GetField_Int(objParent, parentFieldInParent, &value), ANI_OK);
    ASSERT_EQ(value, parentFieldValueInParent);
    ASSERT_EQ(env_->Object_GetField_Int(objParent, parentFieldInChild, &value), ANI_OK);
    ASSERT_EQ(value, parentFieldValueInParent);
    ASSERT_EQ(env_->Object_GetField_Int(objParent, overridedFieldInChild, &value), ANI_OK);
    ASSERT_EQ(value, overridedFieldValueInParent);
    ASSERT_EQ(env_->Object_GetField_Int(objParent, overridedFieldInParent, &value), ANI_OK);
    ASSERT_EQ(value, overridedFieldValueInParent);

    ASSERT_EQ(env_->Object_GetField_Int(objChild, parentFieldInParent, &value), ANI_OK);
    ASSERT_EQ(value, parentFieldValueInParent);
    ASSERT_EQ(env_->Object_GetField_Int(objChild, parentFieldInChild, &value), ANI_OK);
    ASSERT_EQ(value, parentFieldValueInParent);
    ASSERT_EQ(env_->Object_GetField_Int(objChild, childFieldInChild, &value), ANI_OK);
    ASSERT_EQ(value, childFieldValueInChild);
    ASSERT_EQ(env_->Object_GetField_Int(objChild, overridedFieldInChild, &value), ANI_OK);
    ASSERT_EQ(value, overridedFieldValueInChild);
    ASSERT_EQ(env_->Object_GetField_Int(objChild, overridedFieldInParent, &value), ANI_OK);
    ASSERT_EQ(value, overridedFieldValueInChild);
}

}  // namespace ark::ets::ani::testing
