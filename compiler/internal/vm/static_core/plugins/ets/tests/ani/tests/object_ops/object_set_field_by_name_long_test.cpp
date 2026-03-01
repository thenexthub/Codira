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

class ObjectSetFieldByNameLongTest : public AniTest {
public:
    ani_object NewAnimal()
    {
        auto animalRef = CallEtsFunction<ani_ref>("object_set_field_by_name_long_test", "newAnimalObject");
        return static_cast<ani_object>(animalRef);
    }
};

constexpr int64_t CMP_VALUE = 9007199254740991;
constexpr int64_t SET_VALUE = -9007199254740991;

TEST_F(ObjectSetFieldByNameLongTest, set_field01)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_by_name_long_test", "checkObjectField", animal,
                                           static_cast<ani_long>(CMP_VALUE)),
              ANI_TRUE);

    ASSERT_EQ(env_->Object_SetFieldByName_Long(animal, "value", static_cast<ani_long>(0)), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_by_name_long_test", "checkObjectField", animal,
                                           static_cast<ani_long>(0)),
              ANI_TRUE);

    ani_long value = 0L;
    ASSERT_EQ(env_->Object_GetFieldByName_Long(animal, "value", &value), ANI_OK);
    ASSERT_EQ(value, 0L);
}

TEST_F(ObjectSetFieldByNameLongTest, set_field02)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_by_name_long_test", "checkObjectField", animal,
                                           static_cast<ani_long>(CMP_VALUE)),
              ANI_TRUE);

    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetFieldByName_Long(animal, "value", static_cast<ani_long>(SET_VALUE)), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_by_name_long_test", "checkObjectField", animal,
                                               static_cast<ani_long>(SET_VALUE)),
                  ANI_TRUE);

        ani_long value = 0L;
        ASSERT_EQ(env_->Object_GetFieldByName_Long(animal, "value", &value), ANI_OK);
        ASSERT_EQ(value, SET_VALUE);

        ASSERT_EQ(env_->Object_SetFieldByName_Long(animal, "value", static_cast<ani_long>(CMP_VALUE)), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_by_name_long_test", "checkObjectField", animal,
                                               static_cast<ani_long>(CMP_VALUE)),
                  ANI_TRUE);

        ASSERT_EQ(env_->Object_GetFieldByName_Long(animal, "value", &value), ANI_OK);
        ASSERT_EQ(value, CMP_VALUE);
    }
}

TEST_F(ObjectSetFieldByNameLongTest, invalid_env)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->c_api->Object_SetFieldByName_Long(nullptr, animal, "value", static_cast<ani_long>(SET_VALUE)),
              ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByNameLongTest, not_found_name)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Long(animal, "x", static_cast<ani_long>(SET_VALUE)), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_SetFieldByName_Long(animal, "", static_cast<ani_long>(SET_VALUE)), ANI_NOT_FOUND);
}

TEST_F(ObjectSetFieldByNameLongTest, invalid_type)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Long(animal, "name", static_cast<ani_long>(SET_VALUE)), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldByNameLongTest, invalid_object)
{
    ASSERT_EQ(env_->Object_SetFieldByName_Long(nullptr, "x", static_cast<ani_long>(SET_VALUE)), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByNameLongTest, invalid_name)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Long(animal, nullptr, static_cast<ani_long>(SET_VALUE)), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
