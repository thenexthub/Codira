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

class ObjectSetFieldLongTest : public AniTest {
public:
    void GetTestData(ani_object *packResult, ani_field *fieldLongResult, ani_field *fieldStringResult)
    {
        auto packRef = CallEtsFunction<ani_ref>("object_set_field_long_test", "newPackageObject");

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_set_field_long_test.Package", &cls), ANI_OK);

        ani_field fieldLong {};
        ASSERT_EQ(env_->Class_FindField(cls, "long_value", &fieldLong), ANI_OK);

        ani_field fieldString {};
        ASSERT_EQ(env_->Class_FindField(cls, "string_value", &fieldString), ANI_OK);

        *packResult = static_cast<ani_object>(packRef);
        *fieldLongResult = fieldLong;
        *fieldStringResult = fieldString;
    }
};

TEST_F(ObjectSetFieldLongTest, set_field_long)
{
    ani_object pack {};
    ani_field fieldLong {};
    ani_field fieldString {};
    ani_long longValue = 8L;
    ani_long longValue1 = 7L;
    GetTestData(&pack, &fieldLong, &fieldString);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_long_test", "checkLongValue", pack, ani_long(0)),
              ANI_TRUE);

    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetField_Long(pack, fieldLong, longValue), ANI_OK);
        ASSERT_EQ(
            CallEtsFunction<ani_boolean>("object_set_field_long_test", "checkLongValue", pack, ani_long(longValue)),
            ANI_TRUE);

        ani_long value {};
        ASSERT_EQ(env_->Object_GetField_Long(pack, fieldLong, &value), ANI_OK);
        ASSERT_EQ(value, longValue);

        ASSERT_EQ(env_->Object_SetField_Long(pack, fieldLong, longValue1), ANI_OK);
        ASSERT_EQ(
            CallEtsFunction<ani_boolean>("object_set_field_long_test", "checkLongValue", pack, ani_long(longValue1)),
            ANI_TRUE);

        ASSERT_EQ(env_->Object_GetField_Long(pack, fieldLong, &value), ANI_OK);
        ASSERT_EQ(value, longValue1);
    }
}

TEST_F(ObjectSetFieldLongTest, set_field_long_invalid_args_env)
{
    ani_object pack {};
    ani_field fieldLong {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldLong, &fieldString);

    ASSERT_EQ(env_->c_api->Object_SetField_Long(nullptr, pack, fieldLong, 5U), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldLongTest, set_field_long_invalid_field_type)
{
    ani_object pack {};
    ani_field fieldLong {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldLong, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Long(pack, fieldString, 5U), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldLongTest, set_field_long_invalid_args_object)
{
    ani_object pack {};
    ani_field fieldLong {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldLong, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Long(nullptr, fieldLong, 5U), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldLongTest, set_field_long_invalid_args_field)
{
    ani_object pack {};
    ani_field fieldLong {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldLong, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Long(pack, nullptr, 5U), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
