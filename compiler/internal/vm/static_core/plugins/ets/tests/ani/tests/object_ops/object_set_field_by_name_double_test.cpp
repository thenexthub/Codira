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

class ObjectSetFieldByNameDoubleTest : public AniTest {
public:
    ani_object NewAnimal()
    {
        auto animalRef = CallEtsFunction<ani_ref>("object_set_field_by_name_double_test", "newAnimalObject");
        return static_cast<ani_object>(animalRef);
    }
};

constexpr double CMP_VALUE = 1.7976931348623157E308;
constexpr double SET_VALUE = -1.7976931348623157E308;

TEST_F(ObjectSetFieldByNameDoubleTest, set_field)
{
    const ani_double tmpValue = 1.12;
    ani_object animal = NewAnimal();
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_by_name_double_test", "checkObjectField", animal,
                                           static_cast<ani_double>(SET_VALUE)),
              ANI_TRUE);

    ASSERT_EQ(env_->Object_SetFieldByName_Double(animal, "age", static_cast<ani_double>(tmpValue)), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_by_name_double_test", "checkObjectField", animal,
                                           static_cast<ani_double>(tmpValue)),
              ANI_TRUE);

    ani_double value {};
    ASSERT_EQ(env_->Object_GetFieldByName_Double(animal, "age", &value), ANI_OK);
    ASSERT_EQ(value, tmpValue);
}

TEST_F(ObjectSetFieldByNameDoubleTest, set_field01)
{
    ani_object animal = NewAnimal();

    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetFieldByName_Double(animal, "age", static_cast<ani_double>(CMP_VALUE)), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_by_name_double_test", "checkObjectField", animal,
                                               static_cast<ani_double>(CMP_VALUE)),
                  ANI_TRUE);

        ani_double value {};
        ASSERT_EQ(env_->Object_GetFieldByName_Double(animal, "age", &value), ANI_OK);
        ASSERT_EQ(value, CMP_VALUE);

        ASSERT_EQ(env_->Object_SetFieldByName_Double(animal, "age", static_cast<ani_double>(SET_VALUE)), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_by_name_double_test", "checkObjectField", animal,
                                               static_cast<ani_double>(SET_VALUE)),
                  ANI_TRUE);

        ASSERT_EQ(env_->Object_GetFieldByName_Double(animal, "age", &value), ANI_OK);
        ASSERT_EQ(value, SET_VALUE);
    }
}

TEST_F(ObjectSetFieldByNameDoubleTest, invalid_env)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->c_api->Object_SetFieldByName_Double(nullptr, animal, "age", CMP_VALUE), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByNameDoubleTest, not_found_name)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Double(animal, "x", SET_VALUE), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_SetFieldByName_Double(animal, "", SET_VALUE), ANI_NOT_FOUND);
}

TEST_F(ObjectSetFieldByNameDoubleTest, invalid_type)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Double(animal, "name", SET_VALUE), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldByNameDoubleTest, invalid_object)
{
    ASSERT_EQ(env_->Object_SetFieldByName_Double(nullptr, "x", SET_VALUE), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByNameDoubleTest, invalid_name)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Double(animal, nullptr, SET_VALUE), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
