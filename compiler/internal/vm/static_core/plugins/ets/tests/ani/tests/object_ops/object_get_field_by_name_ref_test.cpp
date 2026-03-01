/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class ObjectGetFieldByNameRefTest : public AniTest {
public:
    ani_object NewAnimal()
    {
        auto animalRef = CallEtsFunction<ani_ref>("object_get_field_by_name_ref_test", "newAnimalObject");
        return static_cast<ani_object>(animalRef);
    }
};

TEST_F(ObjectGetFieldByNameRefTest, get_field)
{
    ani_object animal = NewAnimal();

    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetFieldByName_Ref(animal, "name", &nameRef), ANI_OK);

    auto name = static_cast<ani_string>(nameRef);
    std::array<char, 6U> buffer {};
    ani_size nameSize = 0;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 3U, buffer.data(), buffer.size(), &nameSize), ANI_OK);
    ASSERT_EQ(nameSize, 3U);
    ASSERT_STREQ(buffer.data(), "Cat");
}

TEST_F(ObjectGetFieldByNameRefTest, invalid_env)
{
    ani_object animal = NewAnimal();

    ani_ref nameRef {};
    ASSERT_EQ(env_->c_api->Object_GetFieldByName_Ref(nullptr, animal, "name", &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldByNameRefTest, not_found)
{
    ani_object animal = NewAnimal();

    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetFieldByName_Ref(animal, "x", &nameRef), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_GetFieldByName_Ref(animal, "", &nameRef), ANI_NOT_FOUND);
}

TEST_F(ObjectGetFieldByNameRefTest, invalid_type)
{
    ani_object animal = NewAnimal();

    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetFieldByName_Ref(animal, "age", &nameRef), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetFieldByNameRefTest, invalid_object)
{
    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetFieldByName_Ref(nullptr, "x", &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldByNameRefTest, invalid_name)
{
    ani_object animal = NewAnimal();

    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetFieldByName_Ref(animal, nullptr, &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldByNameRefTest, invalid_result)
{
    ani_object animal = NewAnimal();

    ASSERT_EQ(env_->Object_GetFieldByName_Ref(animal, "x", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
