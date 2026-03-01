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

class ObjectSetFieldFloatTest : public AniTest {
public:
    void GetTestData(ani_object *packResult, ani_field *fieldFloatResult, ani_field *fieldStringResult)
    {
        auto packRef = CallEtsFunction<ani_ref>("object_set_field_float_test", "newPackObject");

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_set_field_float_test.Pack", &cls), ANI_OK);

        ani_field fieldFloat {};
        ASSERT_EQ(env_->Class_FindField(cls, "float_value", &fieldFloat), ANI_OK);

        ani_field fieldString {};
        ASSERT_EQ(env_->Class_FindField(cls, "string_value", &fieldString), ANI_OK);

        *packResult = static_cast<ani_object>(packRef);
        *fieldFloatResult = fieldFloat;
        *fieldStringResult = fieldString;
    }
};

TEST_F(ObjectSetFieldFloatTest, set_field_float)
{
    ani_object pack {};
    ani_field fieldFloat {};
    ani_field fieldString {};
    const ani_float value = 3.14F;
    const ani_float value1 = 2.71F;
    GetTestData(&pack, &fieldFloat, &fieldString);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_float_test", "checkFloatValue", pack, 0.0F), ANI_TRUE);

    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetField_Float(pack, fieldFloat, value), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_float_test", "checkFloatValue", pack, value),
                  ANI_TRUE);

        ani_float result = 0.0F;
        ASSERT_EQ(env_->Object_GetField_Float(pack, fieldFloat, &result), ANI_OK);
        ASSERT_EQ(result, value);

        ASSERT_EQ(env_->Object_SetField_Float(pack, fieldFloat, value1), ANI_OK);

        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_float_test", "checkFloatValue", pack, value1),
                  ANI_TRUE);

        ASSERT_EQ(env_->Object_GetField_Float(pack, fieldFloat, &result), ANI_OK);
        ASSERT_EQ(result, value1);
    }
}

TEST_F(ObjectSetFieldFloatTest, set_field_float_negative_value)
{
    ani_object pack {};
    ani_field fieldFloat {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldFloat, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Float(pack, fieldFloat, -2.71F), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_float_test", "checkFloatValue", pack, -2.71F), ANI_TRUE);
}

TEST_F(ObjectSetFieldFloatTest, set_field_float_invalid_args_env)
{
    ani_object pack {};
    ani_field fieldFloat {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldFloat, &fieldString);

    ASSERT_EQ(env_->c_api->Object_SetField_Float(nullptr, pack, fieldFloat, 3.14F), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldFloatTest, set_field_float_invalid_field_type)
{
    ani_object pack {};
    ani_field fieldFloat {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldFloat, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Float(pack, fieldString, 3.14F), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldFloatTest, set_field_float_invalid_args_object)
{
    ani_object pack {};
    ani_field fieldFloat {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldFloat, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Float(nullptr, fieldFloat, 3.14F), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldFloatTest, set_field_float_invalid_args_field)
{
    ani_object pack {};
    ani_field fieldFloat {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldFloat, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Float(pack, nullptr, 3.14F), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldFloatTest, set_field_float_boundary_values)
{
    ani_object pack {};
    ani_field fieldFloat {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldFloat, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Float(pack, fieldFloat, 3.4028235e+38F), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_float_test", "checkFloatValue", pack, 3.4028235e+38F),
              ANI_TRUE);

    ASSERT_EQ(env_->Object_SetField_Float(pack, fieldFloat, -3.4028235e+38F), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_float_test", "checkFloatValue", pack, -3.4028235e+38F),
              ANI_TRUE);
}

}  // namespace ark::ets::ani::testing