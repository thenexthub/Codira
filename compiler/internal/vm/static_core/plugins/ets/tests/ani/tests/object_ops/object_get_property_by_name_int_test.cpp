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

class ObjectGetPropertyByNameIntTest : public AniTest {
public:
    ani_object NewCar()
    {
        auto carRef = CallEtsFunction<ani_ref>("object_get_property_by_name_int_test", "newCarObject");
        return static_cast<ani_object>(carRef);
    }
};

TEST_F(ObjectGetPropertyByNameIntTest, get_field_property)
{
    ani_object car = NewCar();

    ani_int length = 0U;
    ASSERT_EQ(env_->Object_GetPropertyByName_Int(car, "length", &length), ANI_OK);
    ASSERT_EQ(length, 4275U);
}

TEST_F(ObjectGetPropertyByNameIntTest, get_getter_property)
{
    ani_object car = NewCar();

    ani_int year = 0U;
    ASSERT_EQ(env_->Object_GetPropertyByName_Int(car, "year", &year), ANI_OK);
    ASSERT_EQ(year, 1989U);
}

TEST_F(ObjectGetPropertyByNameIntTest, invalid_env)
{
    ani_object car = NewCar();

    ani_int length = 0U;
    ASSERT_EQ(env_->c_api->Object_GetPropertyByName_Int(nullptr, car, "year", &length), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameIntTest, invalid_parameter)
{
    ani_object car = NewCar();

    ani_int length = 0U;
    ASSERT_EQ(env_->Object_GetPropertyByName_Int(car, "yearA", &length), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_GetPropertyByName_Int(car, "", &length), ANI_NOT_FOUND);
}

TEST_F(ObjectGetPropertyByNameIntTest, invalid_argument1)
{
    ani_int length = 0U;
    ASSERT_EQ(env_->Object_GetPropertyByName_Int(nullptr, "length", &length), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameIntTest, invalid_argument2)
{
    ani_object car = NewCar();

    ani_int length = 0U;
    ASSERT_EQ(env_->Object_GetPropertyByName_Int(car, nullptr, &length), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameIntTest, invalid_argument3)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->Object_GetPropertyByName_Int(car, "length", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameIntTest, get_field_property_invalid_type)
{
    ani_object car = NewCar();

    ani_int manufacturer = 0U;
    ASSERT_EQ(env_->Object_GetPropertyByName_Int(car, "manufacturer", &manufacturer), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetPropertyByNameIntTest, get_getter_property_invalid_type)
{
    ani_object car = NewCar();

    ani_int model = 0U;
    ASSERT_EQ(env_->Object_GetPropertyByName_Int(car, "model", &model), ANI_INVALID_TYPE);
}

}  // namespace ark::ets::ani::testing
