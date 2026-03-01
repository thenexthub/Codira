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

class ObjectSetFieldIntTest : public AniTest {
public:
    void GetTestData(ani_object *packResult, ani_field *fieldIntResult, ani_field *fieldStringResult)
    {
        auto packRef = CallEtsFunction<ani_ref>("object_set_field_int_test", "newPackObject");

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_set_field_int_test.Pack", &cls), ANI_OK);

        ani_field fieldInt {};
        ASSERT_EQ(env_->Class_FindField(cls, "int_value", &fieldInt), ANI_OK);

        ani_field fieldString {};
        ASSERT_EQ(env_->Class_FindField(cls, "string_value", &fieldString), ANI_OK);

        *packResult = static_cast<ani_object>(packRef);
        *fieldIntResult = fieldInt;
        *fieldStringResult = fieldString;
    }
};

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.02-CPP, G.PRE.06) solid logic
#define CHECK_FIELD_VALUE(obj, field, target, setTar)                      \
    do {                                                                   \
        ani_int result = 0U;                                               \
        ASSERT_EQ(env_->Object_GetField_Int(obj, field, &result), ANI_OK); \
        ASSERT_EQ(result, target);                                         \
        ASSERT_EQ(env_->Object_SetField_Int(obj, field, setTar), ANI_OK);  \
        ASSERT_EQ(env_->Object_GetField_Int(obj, field, &result), ANI_OK); \
        ASSERT_EQ(result, setTar);                                         \
        ASSERT_EQ(env_->Object_SetField_Int(obj, field, target), ANI_OK);  \
    } while (0)
// NOLINTEND(cppcoreguidelines-macro-usage)

TEST_F(ObjectSetFieldIntTest, set_field_int)
{
    ani_object pack {};
    ani_field fieldInt {};
    ani_field fieldString {};
    const ani_int value1 = 2;
    const ani_int value2 = 3;
    GetTestData(&pack, &fieldInt, &fieldString);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_int_test", "checkIntValue", pack, 0), ANI_TRUE);

    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetField_Int(pack, fieldInt, value1), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_int_test", "checkIntValue", pack, value1), ANI_TRUE);

        ani_int result = 0;
        ASSERT_EQ(env_->Object_GetField_Int(pack, fieldInt, &result), ANI_OK);
        ASSERT_EQ(result, value1);

        ASSERT_EQ(env_->Object_SetField_Int(pack, fieldInt, value2), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_int_test", "checkIntValue", pack, value2), ANI_TRUE);

        ASSERT_EQ(env_->Object_GetField_Int(pack, fieldInt, &result), ANI_OK);
        ASSERT_EQ(result, value2);
    }
}

TEST_F(ObjectSetFieldIntTest, set_field_int_invalid_args_env)
{
    ani_object pack {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldInt, &fieldString);

    ASSERT_EQ(env_->c_api->Object_SetField_Int(nullptr, pack, fieldInt, 2U), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldIntTest, set_field_int_invalid_field_type)
{
    ani_object pack {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldInt, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Int(pack, fieldString, 2U), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldIntTest, set_field_int_invalid_args_object)
{
    ani_object pack {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldInt, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Int(nullptr, fieldInt, 2U), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldIntTest, set_field_int_invalid_args_field)
{
    ani_object pack {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldInt, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Int(pack, nullptr, 2U), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldIntTest, check_hierarchy)
{
    ani_class clsParent {};
    ASSERT_EQ(env_->FindClass("object_set_field_int_test.Parent", &clsParent), ANI_OK);
    ani_method ctorParent {};
    ASSERT_EQ(env_->Class_FindMethod(clsParent, "<ctor>", ":", &ctorParent), ANI_OK);
    ani_object objParent {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_New(clsParent, ctorParent, &objParent), ANI_OK);

    ani_class clsChild {};
    ASSERT_EQ(env_->FindClass("object_set_field_int_test.Child", &clsChild), ANI_OK);
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

    CHECK_FIELD_VALUE(objParent, parentFieldInParent, 0U, 1U);
    CHECK_FIELD_VALUE(objParent, parentFieldInChild, 0U, 2U);
    CHECK_FIELD_VALUE(objParent, overridedFieldInChild, 0U, 3U);
    CHECK_FIELD_VALUE(objParent, overridedFieldInParent, 0U, 4U);

    CHECK_FIELD_VALUE(objChild, parentFieldInParent, 0U, 5U);
    CHECK_FIELD_VALUE(objChild, parentFieldInParent, 0U, 6U);
    CHECK_FIELD_VALUE(objChild, childFieldInChild, 0U, 7U);
    CHECK_FIELD_VALUE(objChild, overridedFieldInChild, 0U, 8U);
    CHECK_FIELD_VALUE(objChild, overridedFieldInParent, 0U, 9U);
}

}  // namespace ark::ets::ani::testing
