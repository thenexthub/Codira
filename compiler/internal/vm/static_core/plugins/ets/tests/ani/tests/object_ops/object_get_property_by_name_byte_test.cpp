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

class ObjectGetPropertyByNameByteTest : public AniTest {
public:
    ani_object NewCar()
    {
        auto carRef = CallEtsFunction<ani_ref>("object_get_property_by_name_byte_test", "newCarObject");
        return static_cast<ani_object>(carRef);
    }
};

TEST_F(ObjectGetPropertyByNameByteTest, get_field_property)
{
    ani_object car = NewCar();

    ani_byte carAge {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(car, "carAge", &carAge), ANI_OK);
    ASSERT_EQ(carAge, 61U);
}

TEST_F(ObjectGetPropertyByNameByteTest, get_getter_property)
{
    ani_object car = NewCar();

    ani_byte price {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(car, "price", &price), ANI_OK);
    ASSERT_EQ(price, 100U);
}

TEST_F(ObjectGetPropertyByNameByteTest, invalid_env)
{
    ani_object car = NewCar();

    ani_byte type {};
    ASSERT_EQ(env_->c_api->Object_GetPropertyByName_Byte(nullptr, car, "price", &type), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameByteTest, invalid_parameter)
{
    ani_object car = NewCar();

    ani_byte model {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(car, "priceA", &model), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(car, "", &model), ANI_NOT_FOUND);
}

TEST_F(ObjectGetPropertyByNameByteTest, invalid_argument)
{
    ani_object car = NewCar();

    ani_byte carAge {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(nullptr, "carAge", &carAge), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(car, nullptr, &carAge), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(car, "carAge", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameByteTest, get_field_property_invalid_type)
{
    ani_object car = NewCar();

    ani_byte manufacturer {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(car, "manufacturer", &manufacturer), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetPropertyByNameByteTest, get_getter_property_invalid_type)
{
    ani_object car = NewCar();

    ani_byte model {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(car, "model", &model), ANI_INVALID_TYPE);
}

}  // namespace ark::ets::ani::testing
