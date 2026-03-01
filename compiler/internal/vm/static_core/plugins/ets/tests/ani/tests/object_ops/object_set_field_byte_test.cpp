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

class ObjectSetFieldByteTest : public AniTest {
public:
    void GetTestDataForByte(ani_object *packResult, ani_field *fieldByteResult, ani_field *fieldStringResult)
    {
        auto packRef = CallEtsFunction<ani_ref>("object_set_field_byte_test", "newPackObject");

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_set_field_byte_test.Pack", &cls), ANI_OK);

        ani_field fieldByte {};
        ASSERT_EQ(env_->Class_FindField(cls, "byte_value", &fieldByte), ANI_OK);

        ani_field fieldString {};
        ASSERT_EQ(env_->Class_FindField(cls, "string_value", &fieldString), ANI_OK);

        *packResult = static_cast<ani_object>(packRef);
        *fieldByteResult = fieldByte;
        *fieldStringResult = fieldString;
    }
};

TEST_F(ObjectSetFieldByteTest, set_field_byte)
{
    ani_object pack {};
    ani_field fieldByte {};
    ani_field fieldString {};
    GetTestDataForByte(&pack, &fieldByte, &fieldString);
    const int zoerValue = 0;
    const int maxByteValue = 127;
    const int valuE128 = -128;
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_byte_test", "checkByteValue", pack, zoerValue), ANI_TRUE);

    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetField_Byte(pack, fieldByte, maxByteValue), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_byte_test", "checkByteValue", pack, maxByteValue),
                  ANI_TRUE);

        ani_byte byt {};
        ASSERT_EQ(env_->Object_GetField_Byte(pack, fieldByte, &byt), ANI_OK);
        ASSERT_EQ(byt, maxByteValue);

        ASSERT_EQ(env_->Object_SetField_Byte(pack, fieldByte, valuE128), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_byte_test", "checkByteValue", pack, valuE128),
                  ANI_TRUE);

        ASSERT_EQ(env_->Object_GetField_Byte(pack, fieldByte, &byt), ANI_OK);
        ASSERT_EQ(byt, valuE128);

        ASSERT_EQ(env_->Object_SetField_Byte(pack, fieldByte, zoerValue), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_byte_test", "checkByteValue", pack, zoerValue),
                  ANI_TRUE);

        ASSERT_EQ(env_->Object_GetField_Byte(pack, fieldByte, &byt), ANI_OK);
        ASSERT_EQ(byt, zoerValue);
    }
}

TEST_F(ObjectSetFieldByteTest, set_field_byte_invalid_args_env)
{
    ani_object pack {};
    ani_field fieldByte {};
    ani_field fieldString {};
    GetTestDataForByte(&pack, &fieldByte, &fieldString);

    const int maxByteValue = 127;
    ASSERT_EQ(env_->c_api->Object_SetField_Byte(nullptr, pack, fieldByte, maxByteValue), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByteTest, set_field_byte_invalid_field_type)
{
    ani_object pack {};
    ani_field fieldByte {};
    ani_field fieldString {};
    GetTestDataForByte(&pack, &fieldByte, &fieldString);

    const int maxByteValue = 127;
    ASSERT_EQ(env_->Object_SetField_Byte(pack, fieldString, maxByteValue), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldByteTest, set_field_byte_invalid_args_object)
{
    ani_object pack {};
    ani_field fieldByte {};
    ani_field fieldString {};
    GetTestDataForByte(&pack, &fieldByte, &fieldString);

    const int maxByteValue = 127;
    ASSERT_EQ(env_->Object_SetField_Byte(nullptr, fieldByte, maxByteValue), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByteTest, set_field_byte_invalid_args_field)
{
    ani_object pack {};
    ani_field fieldByte {};
    ani_field fieldString {};
    GetTestDataForByte(&pack, &fieldByte, &fieldString);

    const int maxByteValue = 127;
    ASSERT_EQ(env_->Object_SetField_Byte(pack, nullptr, maxByteValue), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing