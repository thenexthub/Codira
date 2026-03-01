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

class ObjectGetPropertyByNameFloatTest : public AniTest {
public:
    ani_object NewCar()
    {
        auto carRef = CallEtsFunction<ani_ref>("object_get_property_by_name_float_test", "newCarObject");
        return static_cast<ani_object>(carRef);
    }
};

TEST_F(ObjectGetPropertyByNameFloatTest, get_field_property)
{
    ani_object car = NewCar();

    ani_float length = 0.0F;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "length", &length), ANI_OK);
    ASSERT_EQ(length, 4275.0F);
}

TEST_F(ObjectGetPropertyByNameFloatTest, get_getter_property)
{
    ani_object car = NewCar();

    ani_float year = 0.0F;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "year", &year), ANI_OK);
    ASSERT_EQ(year, 1989.0F);
}

TEST_F(ObjectGetPropertyByNameFloatTest, invalid_env)
{
    ani_object car = NewCar();

    ani_float year = 0.0F;
    ASSERT_EQ(env_->c_api->Object_GetPropertyByName_Float(nullptr, car, "year", &year), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameFloatTest, invalid_parameter)
{
    ani_object car = NewCar();

    ani_float year = 0.0F;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "yearA", &year), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "", &year), ANI_NOT_FOUND);
}

TEST_F(ObjectGetPropertyByNameFloatTest, invalid_argument)
{
    ani_object car = NewCar();

    ani_float length = 0.0F;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(nullptr, "length", &length), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, nullptr, &length), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "length", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameFloatTest, get_field_property_invalid_type)
{
    ani_object car = NewCar();

    ani_float manufacturer = 0.0F;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "manufacturer", &manufacturer), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetPropertyByNameFloatTest, get_getter_property_invalid_type)
{
    ani_object car = NewCar();

    ani_float model = 0.0F;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "model", &model), ANI_INVALID_TYPE);
}

}  // namespace ark::ets::ani::testing
