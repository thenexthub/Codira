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

const ani_short G_USHORTVAL100 = 100;
const uint16_t VAL200 = 200;
const uint16_t USHORT_VAL123 = 123;
const ani_short G_MINUSSHORTVAL300 = -300;

class ObjectSetFieldShortTest : public AniTest {
public:
    void GetTestData(ani_object *packResult, ani_field *fieldShortResult, ani_field *fieldStringResult)
    {
        auto packRef = CallEtsFunction<ani_ref>("object_set_field_short_test", "newPackObject");

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_set_field_short_test.Pack", &cls), ANI_OK);

        ani_field fieldShort {};
        ASSERT_EQ(env_->Class_FindField(cls, "short_value", &fieldShort), ANI_OK);

        ani_field fieldString {};
        ASSERT_EQ(env_->Class_FindField(cls, "string_value", &fieldString), ANI_OK);

        *packResult = static_cast<ani_object>(packRef);
        *fieldShortResult = fieldShort;
        *fieldStringResult = fieldString;
    }
};

TEST_F(ObjectSetFieldShortTest, set_field_short)
{
    ani_object pack {};
    ani_field fieldShort {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldShort, &fieldString);

    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetField_Short(pack, fieldShort, G_USHORTVAL100), ANI_OK);

        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_short_test", "checkShortValue", pack, G_USHORTVAL100),
                  ANI_TRUE);

        ani_short sh {};
        ASSERT_EQ(env_->Object_GetField_Short(pack, fieldShort, &sh), ANI_OK);
        ASSERT_EQ(sh, G_USHORTVAL100);

        ASSERT_EQ(env_->Object_SetField_Short(pack, fieldShort, VAL200), ANI_OK);

        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_short_test", "checkShortValue", pack, VAL200),
                  ANI_TRUE);

        ASSERT_EQ(env_->Object_GetField_Short(pack, fieldShort, &sh), ANI_OK);
        ASSERT_EQ(sh, VAL200);
    }
}

TEST_F(ObjectSetFieldShortTest, set_field_short_negative_value)
{
    ani_object pack {};
    ani_field fieldShort {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldShort, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Short(pack, fieldShort, G_MINUSSHORTVAL300), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_short_test", "checkShortValue", pack, G_MINUSSHORTVAL300),
              ANI_TRUE);

    ani_short sh {};
    ASSERT_EQ(env_->Object_GetField_Short(pack, fieldShort, &sh), ANI_OK);
    ASSERT_EQ(sh, G_MINUSSHORTVAL300);
}

TEST_F(ObjectSetFieldShortTest, set_field_short_invalid_args_env)
{
    ani_object pack {};
    ani_field fieldShort {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldShort, &fieldString);

    ASSERT_EQ(env_->c_api->Object_SetField_Short(nullptr, pack, fieldShort, USHORT_VAL123), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldShortTest, set_field_short_invalid_field_type)
{
    ani_object pack {};
    ani_field fieldShort {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldShort, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Short(pack, fieldString, USHORT_VAL123), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldShortTest, set_field_short_invalid_args_object)
{
    ani_object pack {};
    ani_field fieldShort {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldShort, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Short(nullptr, fieldShort, USHORT_VAL123), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldShortTest, set_field_short_invalid_args_field)
{
    ani_object pack {};
    ani_field fieldShort {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldShort, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Short(pack, nullptr, USHORT_VAL123), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
