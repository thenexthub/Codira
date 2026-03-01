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

class ObjectGetFieldByNameShortTest : public AniTest {
public:
    ani_object NewAnimal()
    {
        auto animalRef = CallEtsFunction<ani_ref>("object_get_field_by_name_short_test", "newAnimalObject");
        return static_cast<ani_object>(animalRef);
    }
};

TEST_F(ObjectGetFieldByNameShortTest, get_field)
{
    ani_object animal = NewAnimal();

    ani_short age = 0U;
    ASSERT_EQ(env_->Object_GetFieldByName_Short(animal, "age", &age), ANI_OK);
    ASSERT_EQ(age, 2U);
}

TEST_F(ObjectGetFieldByNameShortTest, invalid_env)
{
    ani_object animal = NewAnimal();

    ani_short age = 0U;
    ASSERT_EQ(env_->c_api->Object_GetFieldByName_Short(nullptr, animal, "age", &age), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldByNameShortTest, not_found)
{
    ani_object animal = NewAnimal();

    ani_short age = 0U;
    ASSERT_EQ(env_->Object_GetFieldByName_Short(animal, "x", &age), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_GetFieldByName_Short(animal, "", &age), ANI_NOT_FOUND);
}

TEST_F(ObjectGetFieldByNameShortTest, invalid_type)
{
    ani_object animal = NewAnimal();

    ani_short age = 0U;
    ASSERT_EQ(env_->Object_GetFieldByName_Short(animal, "name", &age), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetFieldByNameShortTest, invalid_object)
{
    ani_short age = 0U;
    ASSERT_EQ(env_->Object_GetFieldByName_Short(nullptr, "age", &age), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldByNameShortTest, invalid_name)
{
    ani_object animal = NewAnimal();

    ani_short age = 0U;
    ASSERT_EQ(env_->Object_GetFieldByName_Short(animal, nullptr, &age), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldByNameShortTest, invalid_result)
{
    ani_object animal = NewAnimal();

    ASSERT_EQ(env_->Object_GetFieldByName_Short(animal, "age", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
