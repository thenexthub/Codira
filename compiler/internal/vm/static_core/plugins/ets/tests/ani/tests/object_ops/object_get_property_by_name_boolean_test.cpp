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

class ObjectGetPropertyByNameBooleanTest : public AniTest {
public:
    ani_object NewCar()
    {
        auto carRef = CallEtsFunction<ani_ref>("object_get_property_by_name_boolean_test", "newCarObject");
        return static_cast<ani_object>(carRef);
    }
};

TEST_F(ObjectGetPropertyByNameBooleanTest, get_field_property)
{
    ani_object car = NewCar();

    ani_boolean highPerformance = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(car, "highPerformance", &highPerformance), ANI_OK);
    ASSERT_EQ(highPerformance, true);
}

TEST_F(ObjectGetPropertyByNameBooleanTest, get_getter_property)
{
    ani_object car = NewCar();

    ani_boolean ecoFriendly = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(car, "ecoFriendly", &ecoFriendly), ANI_OK);
    ASSERT_EQ(ecoFriendly, false);
}

TEST_F(ObjectGetPropertyByNameBooleanTest, invalid_env)
{
    ani_object car = NewCar();

    ani_boolean highPerformance = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Object_GetPropertyByName_Boolean(nullptr, car, "ecoFriendly", &highPerformance),
              ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameBooleanTest, invalid_parameter)
{
    ani_object car = NewCar();

    ani_boolean highPerformance = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(car, "ecoFriendlyA", &highPerformance), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(car, "", &highPerformance), ANI_NOT_FOUND);
}

TEST_F(ObjectGetPropertyByNameBooleanTest, invalid_argument)
{
    ani_object car = NewCar();

    ani_boolean highPerformance = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(nullptr, "highPerformance", &highPerformance), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(car, nullptr, &highPerformance), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(car, "highPerformance", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameBooleanTest, get_field_property_invalid_type)
{
    ani_object car = NewCar();

    ani_boolean manufacturer = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(car, "manufacturer", &manufacturer), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetPropertyByNameBooleanTest, get_getter_property_invalid_type)
{
    ani_object car = NewCar();

    ani_boolean model = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(car, "model", &model), ANI_INVALID_TYPE);
}

}  // namespace ark::ets::ani::testing
