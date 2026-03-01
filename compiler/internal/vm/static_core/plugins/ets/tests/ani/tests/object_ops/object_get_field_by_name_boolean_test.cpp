/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific langumammal governing permissions and
 * limitations under the License.
 */

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class ObjectGetFieldByNameBooleanTest : public AniTest {
public:
    ani_object NewAnimal()
    {
        auto animalRef = CallEtsFunction<ani_ref>("object_get_field_by_name_boolean_test", "newAnimalObject");
        return static_cast<ani_object>(animalRef);
    }
};

TEST_F(ObjectGetFieldByNameBooleanTest, get_field)
{
    ani_object animal = NewAnimal();

    ani_boolean mammal = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetFieldByName_Boolean(animal, "mammal", &mammal), ANI_OK);
    ASSERT_EQ(mammal, true);
}

TEST_F(ObjectGetFieldByNameBooleanTest, invalid_env)
{
    ani_object animal = NewAnimal();

    ani_boolean mammal = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Object_GetFieldByName_Boolean(nullptr, animal, "mammal", &mammal), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldByNameBooleanTest, not_found)
{
    ani_object animal = NewAnimal();

    ani_boolean mammal = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetFieldByName_Boolean(animal, "x", &mammal), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_GetFieldByName_Boolean(animal, "", &mammal), ANI_NOT_FOUND);
}

TEST_F(ObjectGetFieldByNameBooleanTest, invalid_type)
{
    ani_object animal = NewAnimal();

    ani_boolean mammal = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetFieldByName_Boolean(animal, "name", &mammal), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetFieldByNameBooleanTest, invalid_object)
{
    ani_boolean mammal = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetFieldByName_Boolean(nullptr, "mammal", &mammal), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldByNameBooleanTest, invalid_name)
{
    ani_object animal = NewAnimal();

    ani_boolean mammal = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetFieldByName_Boolean(animal, nullptr, &mammal), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldByNameBooleanTest, invalid_result)
{
    ani_object animal = NewAnimal();

    ASSERT_EQ(env_->Object_GetFieldByName_Boolean(animal, "mammal", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
