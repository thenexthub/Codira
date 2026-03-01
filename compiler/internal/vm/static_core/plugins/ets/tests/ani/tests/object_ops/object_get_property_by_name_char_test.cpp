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

class ObjectGetPropertyByNameCharTest : public AniTest {
public:
    ani_object NewCar()
    {
        auto carRef = CallEtsFunction<ani_ref>("object_get_property_by_name_char_test", "newCarObject");
        return static_cast<ani_object>(carRef);
    }
};

TEST_F(ObjectGetPropertyByNameCharTest, get_field_property)
{
    ani_object car = NewCar();

    ani_char type = '\0';
    ani_char typee = 'a';
    ASSERT_EQ(env_->Object_GetPropertyByName_Char(car, "type", &type), ANI_OK);
    ASSERT_EQ(type, typee);
}

TEST_F(ObjectGetPropertyByNameCharTest, get_getter_property)
{
    ani_object car = NewCar();

    ani_char band = '\0';
    ani_char bandd = 'z';
    ASSERT_EQ(env_->Object_GetPropertyByName_Char(car, "band", &band), ANI_OK);
    ASSERT_EQ(band, bandd);
}

TEST_F(ObjectGetPropertyByNameCharTest, invalid_env)
{
    ani_object car = NewCar();

    ani_char type = '\0';
    ASSERT_EQ(env_->c_api->Object_GetPropertyByName_Char(nullptr, car, "band", &type), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameCharTest, invalid_parameter)
{
    ani_object car = NewCar();

    ani_char model = '\0';
    ASSERT_EQ(env_->Object_GetPropertyByName_Char(car, "bandA", &model), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_GetPropertyByName_Char(car, "", &model), ANI_NOT_FOUND);
}

TEST_F(ObjectGetPropertyByNameCharTest, invalid_argument1)
{
    ani_char type = '\0';
    ASSERT_EQ(env_->Object_GetPropertyByName_Char(nullptr, "type", &type), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameCharTest, invalid_argument2)
{
    ani_object car = NewCar();

    ani_char type = '\0';
    ASSERT_EQ(env_->Object_GetPropertyByName_Char(car, nullptr, &type), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameCharTest, invalid_argument3)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->Object_GetPropertyByName_Char(car, "type", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameCharTest, get_field_property_invalid_type)
{
    ani_object car = NewCar();

    ani_char manufacturer = '\0';
    ASSERT_EQ(env_->Object_GetPropertyByName_Char(car, "manufacturer", &manufacturer), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetPropertyByNameCharTest, get_getter_property_invalid_type)
{
    ani_object car = NewCar();

    ani_char model = '\0';
    ASSERT_EQ(env_->Object_GetPropertyByName_Char(car, "model", &model), ANI_INVALID_TYPE);
}

}  // namespace ark::ets::ani::testing
