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

class ObjectSetFieldDoubleTest : public AniTest {
public:
    void GetTestData(ani_object *packResult, ani_field *fieldDoubleResult, ani_field *fieldStringResult)
    {
        auto packRef = CallEtsFunction<ani_ref>("object_set_field_double_test", "newPackObject");

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_set_field_double_test.Pack", &cls), ANI_OK);

        ani_field fieldDouble {};
        ASSERT_EQ(env_->Class_FindField(cls, "double_value", &fieldDouble), ANI_OK);

        ani_field fieldString {};
        ASSERT_EQ(env_->Class_FindField(cls, "string_value", &fieldString), ANI_OK);

        *packResult = static_cast<ani_object>(packRef);
        *fieldDoubleResult = fieldDouble;
        *fieldStringResult = fieldString;
    }
};

TEST_F(ObjectSetFieldDoubleTest, set_field_double)
{
    ani_object pack {};
    ani_field fieldDouble {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldDouble, &fieldString);
    const double zero = 0.0;
    const double pi = 3.14159;
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_double_test", "checkDoubleValue", pack, zero), ANI_TRUE);

    ASSERT_EQ(env_->Object_SetField_Double(pack, fieldDouble, pi), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_double_test", "checkDoubleValue", pack, pi), ANI_TRUE);
}

TEST_F(ObjectSetFieldDoubleTest, set_field_double_negative_value)
{
    ani_object pack {};
    ani_field fieldDouble {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldDouble, &fieldString);
    const double eulersNumberNegative = -2.71828;
    ASSERT_EQ(env_->Object_SetField_Double(pack, fieldDouble, eulersNumberNegative), ANI_OK);

    ASSERT_EQ(
        CallEtsFunction<ani_boolean>("object_set_field_double_test", "checkDoubleValue", pack, eulersNumberNegative),
        ANI_TRUE);
}

TEST_F(ObjectSetFieldDoubleTest, set_field_double_boundary_values)
{
    ani_object pack {};
    ani_field fieldDouble {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldDouble, &fieldString);

    const double numbeR1 = 1.7976931348623157e+308;
    const double minusMaxNumber = -1.7976931348623157e+308;
    const double numbeR2 = 2.2250738585072014e-308;

    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetField_Double(pack, fieldDouble, numbeR1), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_double_test", "checkDoubleValue", pack, numbeR1),
                  ANI_TRUE);

        ani_double num {};
        ASSERT_EQ(env_->Object_GetField_Double(pack, fieldDouble, &num), ANI_OK);
        ASSERT_EQ(num, numbeR1);

        ASSERT_EQ(env_->Object_SetField_Double(pack, fieldDouble, numbeR2), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_double_test", "checkDoubleValue", pack, numbeR2),
                  ANI_TRUE);

        ASSERT_EQ(env_->Object_GetField_Double(pack, fieldDouble, &num), ANI_OK);
        ASSERT_EQ(num, numbeR2);

        ASSERT_EQ(env_->Object_SetField_Double(pack, fieldDouble, minusMaxNumber), ANI_OK);
        ASSERT_EQ(
            CallEtsFunction<ani_boolean>("object_set_field_double_test", "checkDoubleValue", pack, minusMaxNumber),
            ANI_TRUE);

        ASSERT_EQ(env_->Object_GetField_Double(pack, fieldDouble, &num), ANI_OK);
        ASSERT_EQ(num, minusMaxNumber);
    }
}

TEST_F(ObjectSetFieldDoubleTest, set_field_double_invalid_args_env)
{
    ani_object pack {};
    ani_field fieldDouble {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldDouble, &fieldString);

    const double pi = 3.14159;
    ASSERT_EQ(env_->c_api->Object_SetField_Double(nullptr, pack, fieldDouble, pi), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldDoubleTest, set_field_double_invalid_field_type)
{
    ani_object pack {};
    ani_field fieldDouble {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldDouble, &fieldString);

    const double pi = 3.14159;
    ASSERT_EQ(env_->Object_SetField_Double(pack, fieldString, pi), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldDoubleTest, set_field_double_invalid_args_object)
{
    ani_object pack {};
    ani_field fieldDouble {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldDouble, &fieldString);

    const double pi = 3.14159;
    ASSERT_EQ(env_->Object_SetField_Double(nullptr, fieldDouble, pi), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldDoubleTest, set_field_double_invalid_args_field)
{
    ani_object pack {};
    ani_field fieldDouble {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldDouble, &fieldString);

    const double pi = 3.14159;
    ASSERT_EQ(env_->Object_SetField_Double(pack, nullptr, pi), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing