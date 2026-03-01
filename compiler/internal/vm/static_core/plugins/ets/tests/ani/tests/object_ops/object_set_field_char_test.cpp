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

class ObjectSetFieldCharTest : public AniTest {
public:
    void GetTestDataForChar(ani_object *packResult, ani_field *fieldCharResult, ani_field *fieldStringResult)
    {
        auto packRef = CallEtsFunction<ani_ref>("object_set_field_char_test", "newPackObject");

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_set_field_char_test.Pack", &cls), ANI_OK);

        ani_field fieldChar {};
        ASSERT_EQ(env_->Class_FindField(cls, "char_value", &fieldChar), ANI_OK);

        ani_field fieldString {};
        ASSERT_EQ(env_->Class_FindField(cls, "string_value", &fieldString), ANI_OK);

        *packResult = static_cast<ani_object>(packRef);
        *fieldCharResult = fieldChar;
        *fieldStringResult = fieldString;
    }
};

TEST_F(ObjectSetFieldCharTest, set_field_char)
{
    ani_object pack {};
    ani_field fieldChar {};
    ani_field fieldString {};
    GetTestDataForChar(&pack, &fieldChar, &fieldString);
    const char zoerValue = 'a';
    const char maxCharValue = 'b';
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_char_test", "checkCharValue", pack, zoerValue), ANI_TRUE);

    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetField_Char(pack, fieldChar, maxCharValue), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_char_test", "checkCharValue", pack, maxCharValue),
                  ANI_TRUE);

        ani_char value {};
        ASSERT_EQ(env_->Object_GetField_Char(pack, fieldChar, &value), ANI_OK);
        ASSERT_EQ(value, maxCharValue);

        ASSERT_EQ(env_->Object_SetField_Char(pack, fieldChar, zoerValue), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_char_test", "checkCharValue", pack, zoerValue),
                  ANI_TRUE);

        ASSERT_EQ(env_->Object_GetField_Char(pack, fieldChar, &value), ANI_OK);
        ASSERT_EQ(value, zoerValue);
    }
}

TEST_F(ObjectSetFieldCharTest, set_field_char_invalid_args_env)
{
    ani_object pack {};
    ani_field fieldChar {};
    ani_field fieldString {};
    GetTestDataForChar(&pack, &fieldChar, &fieldString);

    const char maxCharValue = 'a';
    ASSERT_EQ(env_->c_api->Object_SetField_Char(nullptr, pack, fieldChar, maxCharValue), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldCharTest, set_field_char_invalid_field_type)
{
    ani_object pack {};
    ani_field fieldChar {};
    ani_field fieldString {};
    GetTestDataForChar(&pack, &fieldChar, &fieldString);

    const char maxCharValue = 'a';
    ASSERT_EQ(env_->Object_SetField_Char(pack, fieldString, maxCharValue), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldCharTest, set_field_char_invalid_args_object)
{
    ani_object pack {};
    ani_field fieldChar {};
    ani_field fieldString {};
    GetTestDataForChar(&pack, &fieldChar, &fieldString);

    const char maxCharValue = 'a';
    ASSERT_EQ(env_->Object_SetField_Char(nullptr, fieldChar, maxCharValue), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldCharTest, set_field_char_invalid_args_field)
{
    ani_object pack {};
    ani_field fieldChar {};
    ani_field fieldString {};
    GetTestDataForChar(&pack, &fieldChar, &fieldString);

    const char maxCharValue = 'a';
    ASSERT_EQ(env_->Object_SetField_Char(pack, nullptr, maxCharValue), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing