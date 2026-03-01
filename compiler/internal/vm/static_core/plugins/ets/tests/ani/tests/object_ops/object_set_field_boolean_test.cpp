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

class ObjectSetFieldBooleanTest : public AniTest {
public:
    void GetTestData(ani_object *packResult, ani_field *fieldBoolResult, ani_field *fieldStringResult)
    {
        auto packRef = CallEtsFunction<ani_ref>("object_set_field_boolean_test", "newPackObject");

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_set_field_boolean_test.Pack", &cls), ANI_OK);

        ani_field fieldBool {};
        ASSERT_EQ(env_->Class_FindField(cls, "bool_value", &fieldBool), ANI_OK);

        ani_field fieldString {};
        ASSERT_EQ(env_->Class_FindField(cls, "string_value", &fieldString), ANI_OK);

        *packResult = static_cast<ani_object>(packRef);
        *fieldBoolResult = fieldBool;
        *fieldStringResult = fieldString;
    }
};

TEST_F(ObjectSetFieldBooleanTest, set_field_boolean_success)
{
    ani_object pack {};
    ani_field fieldBool {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldBool, &fieldString);
    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_boolean_test", "checkBooleanValue", pack, ANI_FALSE),
                  ANI_TRUE);

        ASSERT_EQ(env_->Object_SetField_Boolean(pack, fieldBool, ANI_TRUE), ANI_OK);

        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_boolean_test", "checkBooleanValue", pack, ANI_TRUE),
                  ANI_TRUE);

        ani_boolean married = ANI_FALSE;
        ASSERT_EQ(env_->Object_GetField_Boolean(pack, fieldBool, &married), ANI_OK);
        ASSERT_EQ(married, true);

        ASSERT_EQ(env_->Object_SetField_Boolean(pack, fieldBool, ANI_FALSE), ANI_OK);

        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_boolean_test", "checkBooleanValue", pack, ANI_FALSE),
                  ANI_TRUE);

        ASSERT_EQ(env_->Object_GetField_Boolean(pack, fieldBool, &married), ANI_OK);
        ASSERT_EQ(married, false);
    }
}

TEST_F(ObjectSetFieldBooleanTest, set_field_boolean_invalid_env)
{
    ani_object pack {};
    ani_field fieldBool {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldBool, &fieldString);

    ASSERT_EQ(env_->c_api->Object_SetField_Boolean(nullptr, pack, fieldBool, ANI_TRUE), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldBooleanTest, set_field_boolean_invalid_field_type)
{
    ani_object pack {};
    ani_field fieldBool {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldBool, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Boolean(pack, fieldString, ANI_TRUE), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldBooleanTest, set_field_boolean_invalid_object)
{
    ani_object pack {};
    ani_field fieldBool {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldBool, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Boolean(nullptr, fieldBool, ANI_TRUE), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldBooleanTest, set_field_boolean_invalid_field)
{
    ani_object pack {};
    ani_field fieldBool {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldBool, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Boolean(pack, nullptr, ANI_TRUE), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldBooleanTest, set_field_boolean_false)
{
    ani_object pack {};
    ani_field fieldBool {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldBool, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Boolean(pack, fieldBool, ANI_TRUE), ANI_OK);

    ASSERT_EQ(env_->Object_SetField_Boolean(pack, fieldBool, ANI_FALSE), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_boolean_test", "checkBooleanValue", pack, ANI_FALSE),
              ANI_TRUE);
}

}  // namespace ark::ets::ani::testing
