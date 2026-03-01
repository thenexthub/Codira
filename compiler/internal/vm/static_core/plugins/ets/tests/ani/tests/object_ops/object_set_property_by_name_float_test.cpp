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

class ObjectSetPropertyByNameFloatTest : public AniTest {
public:
    ani_object NewCar()
    {
        auto carRef = CallEtsFunction<ani_ref>("object_set_property_by_name_float_test", "newCarObject");
        return static_cast<ani_object>(carRef);
    }

    ani_object NewC1()
    {
        auto c1Ref = CallEtsFunction<ani_ref>("object_set_property_by_name_float_test", "newC1");
        return static_cast<ani_object>(c1Ref);
    }

    ani_object NewC2()
    {
        auto c2Ref = CallEtsFunction<ani_ref>("object_set_property_by_name_float_test", "newC2");
        return static_cast<ani_object>(c2Ref);
    }
};

TEST_F(ObjectSetPropertyByNameFloatTest, set_field)
{
    ani_object car = NewCar();

    ani_float highPerformance = 0U;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "highPerformance", &highPerformance), ANI_OK);
    ASSERT_EQ(highPerformance, 0U);

    const ani_float value = 0.1;
    const ani_float value1 = 0.2;
    ASSERT_EQ(env_->Object_SetPropertyByName_Float(car, "highPerformance", value), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "highPerformance", &highPerformance), ANI_OK);
    ASSERT_EQ(highPerformance, value);

    ASSERT_EQ(env_->Object_SetPropertyByName_Float(car, "ecoFriendly", value1), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "ecoFriendly", &highPerformance), ANI_OK);
    ASSERT_EQ(highPerformance, value1);
}

TEST_F(ObjectSetPropertyByNameFloatTest, set_field_property)
{
    ani_object car = NewCar();

    ani_float highPerformance = 0U;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "highPerformance", &highPerformance), ANI_OK);
    ASSERT_EQ(static_cast<int32_t>(highPerformance), 0U);

    const int32_t loopCount = 3;
    const ani_float value = -3.4028235E38;
    const ani_float value1 = 3.4028235E38;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetPropertyByName_Float(car, "highPerformance", value), ANI_OK);
        ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "highPerformance", &highPerformance), ANI_OK);
        ASSERT_EQ(highPerformance, value);

        ASSERT_EQ(env_->Object_SetPropertyByName_Float(car, "highPerformance", value1), ANI_OK);
        ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "highPerformance", &highPerformance), ANI_OK);
        ASSERT_EQ(highPerformance, value1);
    }
}

TEST_F(ObjectSetPropertyByNameFloatTest, set_setter_property)
{
    ani_object car = NewCar();

    ani_float ecoFriendly = 0U;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "ecoFriendly", &ecoFriendly), ANI_OK);
    ASSERT_EQ(static_cast<int32_t>(ecoFriendly), 0U);

    const int32_t loopCount = 3;
    const ani_float value = -3.4028235E38;
    const ani_float value1 = 3.4028235E38;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetPropertyByName_Float(car, "ecoFriendly", value), ANI_OK);
        ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "ecoFriendly", &ecoFriendly), ANI_OK);
        ASSERT_EQ(ecoFriendly, value);

        ASSERT_EQ(env_->Object_SetPropertyByName_Float(car, "ecoFriendly", value1), ANI_OK);
        ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "ecoFriendly", &ecoFriendly), ANI_OK);
        ASSERT_EQ(ecoFriendly, value1);
    }
}

TEST_F(ObjectSetPropertyByNameFloatTest, invalid_env)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->c_api->Object_SetPropertyByName_Float(nullptr, car, "ecoFriendly", 1U), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetPropertyByNameFloatTest, invalid_parameter)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->Object_SetPropertyByName_Float(car, "ecoFriendlyA", 1U), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_SetPropertyByName_Float(car, "", 1U), ANI_NOT_FOUND);
}

TEST_F(ObjectSetPropertyByNameFloatTest, invalid_argument)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->Object_SetPropertyByName_Float(nullptr, "highPerformance", 1U), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_SetPropertyByName_Float(car, nullptr, 1U), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetPropertyByNameFloatTest, set_field_property_invalid_type)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->Object_SetPropertyByName_Float(car, "manufacturer", 1U), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetPropertyByNameFloatTest, set_setter_property_invalid_type)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->Object_SetPropertyByName_Float(car, "model", 1U), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetPropertyByNameFloatTest, set_interface_field)
{
    ani_object c1 = NewC1();

    ani_float prop = 0U;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(c1, "prop", &prop), ANI_OK);
    ASSERT_EQ(static_cast<int32_t>(prop), 0U);

    ASSERT_EQ(env_->Object_SetPropertyByName_Float(c1, "prop", 1U), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(c1, "prop", &prop), ANI_OK);
    ASSERT_EQ(static_cast<int32_t>(prop), 1U);
}

TEST_F(ObjectSetPropertyByNameFloatTest, set_interface_property)
{
    ani_object c2 = NewC2();

    ani_float prop = 0U;
    ASSERT_EQ(env_->Object_SetPropertyByName_Float(c2, "prop", 0U), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(c2, "prop", &prop), ANI_OK);
    ASSERT_EQ(static_cast<int32_t>(prop), 0U);

    ASSERT_EQ(env_->Object_SetPropertyByName_Float(c2, "prop", 1U), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(c2, "prop", &prop), ANI_OK);
    ASSERT_EQ(static_cast<int32_t>(prop), 1U);
}

}  // namespace ark::ets::ani::testing
