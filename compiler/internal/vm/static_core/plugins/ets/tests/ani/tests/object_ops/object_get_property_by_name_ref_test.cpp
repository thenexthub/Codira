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

class ObjectGetPropertyByNameRefTest : public AniTest {
public:
    ani_object NewPerson()
    {
        auto personRef = CallEtsFunction<ani_ref>("object_get_property_by_name_ref_test", "newPersonObject");
        return static_cast<ani_object>(personRef);
    }
};

TEST_F(ObjectGetPropertyByNameRefTest, get_field_property)
{
    ani_object person = NewPerson();

    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(person, "name", &nameRef), ANI_OK);

    auto name = static_cast<ani_string>(nameRef);
    std::array<char, 6U> buffer {};
    ani_size nameSize = 0;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 3U, buffer.data(), buffer.size(), &nameSize), ANI_OK);
    ASSERT_EQ(nameSize, 3U);
    ASSERT_STREQ(buffer.data(), "Max");
}

TEST_F(ObjectGetPropertyByNameRefTest, get_getter_property)
{
    ani_object person = NewPerson();

    ani_ref surnameRef {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(person, "surname", &surnameRef), ANI_OK);

    auto surname = static_cast<ani_string>(surnameRef);
    std::array<char, 6U> buffer {};
    ani_size surnameSize = 0;
    ASSERT_EQ(env_->String_GetUTF8SubString(surname, 0U, 4U, buffer.data(), buffer.size(), &surnameSize), ANI_OK);
    ASSERT_EQ(surnameSize, 4U);
    ASSERT_STREQ(buffer.data(), "Pain");
}

TEST_F(ObjectGetPropertyByNameRefTest, invalid_env)
{
    ani_object person = NewPerson();

    ani_ref surnameRef {};
    ASSERT_EQ(env_->c_api->Object_GetPropertyByName_Ref(nullptr, person, "surname", &surnameRef), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameRefTest, invalid_parameter)
{
    ani_object person = NewPerson();

    ani_ref surnameRef {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(person, "surnameA", &surnameRef), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(person, "", &surnameRef), ANI_NOT_FOUND);
}

TEST_F(ObjectGetPropertyByNameRefTest, invalid_argument1)
{
    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(nullptr, "name", &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameRefTest, invalid_argument2)
{
    ani_object person = NewPerson();

    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(person, nullptr, &nameRef), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameRefTest, invalid_argument3)
{
    ani_object person = NewPerson();

    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(person, "name", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameRefTest, get_field_property_invalid_type)
{
    ani_object person = NewPerson();

    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(person, "age", &nameRef), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetPropertyByNameRefTest, get_getter_property_invalid_type)
{
    ani_object person = NewPerson();

    ani_ref nameRef {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(person, "realAge", &nameRef), ANI_INVALID_TYPE);
}

}  // namespace ark::ets::ani::testing
