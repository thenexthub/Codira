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
 * See the License for the specific languindex governing permissions and
 * limitations under the License.
 */

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class ObjectGetFieldByNameCharTest : public AniTest {
public:
    ani_object NewAnimal()
    {
        auto animalRef = CallEtsFunction<ani_ref>("object_get_field_by_name_char_test", "newAnimalObject");
        return static_cast<ani_object>(animalRef);
    }
};

TEST_F(ObjectGetFieldByNameCharTest, get_field)
{
    ani_object animal = NewAnimal();

    ani_char index = '\0';
    ani_char xx = 'a';
    ASSERT_EQ(env_->Object_GetFieldByName_Char(animal, "index", &index), ANI_OK);
    ASSERT_EQ(index, xx);
}

TEST_F(ObjectGetFieldByNameCharTest, invalid_env)
{
    ani_object animal = NewAnimal();

    ani_char index = '\0';
    ASSERT_EQ(env_->c_api->Object_GetFieldByName_Char(nullptr, animal, "index", &index), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldByNameCharTest, not_found)
{
    ani_object animal = NewAnimal();

    ani_char index = '\0';
    ASSERT_EQ(env_->Object_GetFieldByName_Char(animal, "x", &index), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_GetFieldByName_Char(animal, "", &index), ANI_NOT_FOUND);
}

TEST_F(ObjectGetFieldByNameCharTest, invalid_type)
{
    ani_object animal = NewAnimal();

    ani_char index = '\0';
    ASSERT_EQ(env_->Object_GetFieldByName_Char(animal, "name", &index), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetFieldByNameCharTest, invalid_object)
{
    ani_char index = '\0';
    ASSERT_EQ(env_->Object_GetFieldByName_Char(nullptr, "index", &index), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldByNameCharTest, invalid_name)
{
    ani_object animal = NewAnimal();

    ani_char index = '\0';
    ASSERT_EQ(env_->Object_GetFieldByName_Char(animal, nullptr, &index), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldByNameCharTest, invalid_result)
{
    ani_object animal = NewAnimal();

    ASSERT_EQ(env_->Object_GetFieldByName_Char(animal, "index", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
