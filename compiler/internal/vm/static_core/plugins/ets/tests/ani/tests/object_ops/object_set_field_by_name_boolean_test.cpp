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

class ObjectSetFieldByNameBooleanTest : public AniTest {
public:
    ani_object NewAnimal()
    {
        auto animalRef = CallEtsFunction<ani_ref>("object_set_field_by_name_boolean_test", "newAnimalObject");
        return static_cast<ani_object>(animalRef);
    }
};

TEST_F(ObjectSetFieldByNameBooleanTest, set_field)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(
        CallEtsFunction<ani_boolean>("object_set_field_by_name_boolean_test", "checkObjectField", animal, ANI_TRUE),
        ANI_TRUE);

    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetFieldByName_Boolean(animal, "value", ANI_FALSE), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_by_name_boolean_test", "checkObjectField", animal,
                                               ANI_FALSE),
                  ANI_TRUE);

        ani_boolean mammal {};
        ASSERT_EQ(env_->Object_GetFieldByName_Boolean(animal, "value", &mammal), ANI_OK);
        ASSERT_EQ(mammal, false);

        ASSERT_EQ(env_->Object_SetFieldByName_Boolean(animal, "value", ANI_TRUE), ANI_OK);
        ASSERT_EQ(
            CallEtsFunction<ani_boolean>("object_set_field_by_name_boolean_test", "checkObjectField", animal, ANI_TRUE),
            ANI_TRUE);

        ASSERT_EQ(env_->Object_GetFieldByName_Boolean(animal, "value", &mammal), ANI_OK);
        ASSERT_EQ(mammal, true);
    }
}

TEST_F(ObjectSetFieldByNameBooleanTest, invalid_env)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->c_api->Object_SetFieldByName_Boolean(nullptr, animal, "value", ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByNameBooleanTest, not_found_name)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Boolean(animal, "x", ANI_TRUE), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_SetFieldByName_Boolean(animal, "", ANI_TRUE), ANI_NOT_FOUND);
}

TEST_F(ObjectSetFieldByNameBooleanTest, invalid_type)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Boolean(animal, "name", ANI_TRUE), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldByNameBooleanTest, invalid_object)
{
    ASSERT_EQ(env_->Object_SetFieldByName_Boolean(nullptr, "x", ANI_TRUE), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByNameBooleanTest, invalid_name)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Boolean(animal, nullptr, ANI_TRUE), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
