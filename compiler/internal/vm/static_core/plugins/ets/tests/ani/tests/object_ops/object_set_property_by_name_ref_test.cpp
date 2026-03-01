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

class ObjectSetPropertyByNameRefTest : public AniTest {
public:
    ani_object NewCar()
    {
        auto carRef = CallEtsFunction<ani_ref>("object_set_property_by_name_ref_test", "newCarObject");
        return static_cast<ani_object>(carRef);
    }

    ani_object NewC1()
    {
        auto c1Ref = CallEtsFunction<ani_ref>("object_set_property_by_name_ref_test", "newC1");
        return static_cast<ani_object>(c1Ref);
    }

    ani_object NewC2()
    {
        auto c2Ref = CallEtsFunction<ani_ref>("object_set_property_by_name_ref_test", "newC2");
        return static_cast<ani_object>(c2Ref);
    }

    void RetrieveStringFromAni(ani_env *env, ani_string string, std::string &resString)
    {
        ani_size result = 0U;
        ASSERT_EQ(env->String_GetUTF8Size(string, &result), ANI_OK);
        ani_size substrOffset = 0U;
        ani_size substrSize = result;
        const ani_size bufferExtension = 10U;
        resString.resize(substrSize + bufferExtension);
        ani_size resSize = resString.size();
        result = 0U;
        auto status =
            env->String_GetUTF8SubString(string, substrOffset, substrSize, resString.data(), resSize, &result);
        ASSERT_EQ(status, ANI_OK);
    }
};

TEST_F(ObjectSetPropertyByNameRefTest, set_field_property)
{
    ani_object car = NewCar();

    ani_ref highPerformance {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(car, "highPerformance", &highPerformance), ANI_OK);
    std::string defaultVal {};
    RetrieveStringFromAni(env_, reinterpret_cast<ani_string>(highPerformance), defaultVal);
    ASSERT_STREQ(defaultVal.data(), "Porsche 911");

    ani_string string {};
    std::string toSet = "Abracadabra";
    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->String_NewUTF8(toSet.data(), toSet.size(), &string), ANI_OK);
        ASSERT_EQ(env_->Object_SetPropertyByName_Ref(car, "highPerformance", string), ANI_OK);
        ASSERT_EQ(env_->Object_GetPropertyByName_Ref(car, "highPerformance", &highPerformance), ANI_OK);
        RetrieveStringFromAni(env_, reinterpret_cast<ani_string>(highPerformance), defaultVal);
        ASSERT_STREQ(defaultVal.data(), "Abracadabra");
    }
}

TEST_F(ObjectSetPropertyByNameRefTest, set_setter_property)
{
    ani_object car = NewCar();

    ani_ref ecoFriendly {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(car, "ecoFriendly", &ecoFriendly), ANI_OK);
    std::string defaultVal {};
    RetrieveStringFromAni(env_, reinterpret_cast<ani_string>(ecoFriendly), defaultVal);
    ASSERT_STREQ(defaultVal.data(), "Porsche");

    ani_string string {};
    std::string toSet = "AbracadabraSetter";
    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->String_NewUTF8(toSet.data(), toSet.size(), &string), ANI_OK);
        ASSERT_EQ(env_->Object_SetPropertyByName_Ref(car, "ecoFriendly", string), ANI_OK);
        ASSERT_EQ(env_->Object_GetPropertyByName_Ref(car, "ecoFriendly", &ecoFriendly), ANI_OK);
        RetrieveStringFromAni(env_, reinterpret_cast<ani_string>(ecoFriendly), defaultVal);
        ASSERT_STREQ(defaultVal.data(), "AbracadabraSetter");
    }
}

TEST_F(ObjectSetPropertyByNameRefTest, invalid_env)
{
    ani_object car = NewCar();

    ani_ref ecoFriendly {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(car, "ecoFriendly", &ecoFriendly), ANI_OK);
    std::string defaultVal {};
    RetrieveStringFromAni(env_, reinterpret_cast<ani_string>(ecoFriendly), defaultVal);
    ASSERT_STREQ(defaultVal.data(), "Porsche");

    ani_string string {};
    std::string toSet = "AbracadabraSetter";
    ASSERT_EQ(env_->String_NewUTF8(toSet.data(), toSet.size(), &string), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_SetPropertyByName_Ref(nullptr, car, "ecoFriendly", string), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Object_GetPropertyByName_Ref(nullptr, car, "ecoFriendly", &ecoFriendly), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetPropertyByNameRefTest, invalid_parameter)
{
    ani_object car = NewCar();

    ani_ref ecoFriendly {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(car, "ecoFriendly", &ecoFriendly), ANI_OK);
    std::string defaultVal {};
    RetrieveStringFromAni(env_, reinterpret_cast<ani_string>(ecoFriendly), defaultVal);
    ASSERT_STREQ(defaultVal.data(), "Porsche");

    ani_string string {};
    std::string toSet = "AbracadabraSetter";
    ASSERT_EQ(env_->String_NewUTF8(toSet.data(), toSet.size(), &string), ANI_OK);
    ASSERT_EQ(env_->Object_SetPropertyByName_Ref(car, "ecoFriendlyA", string), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(car, "", &ecoFriendly), ANI_NOT_FOUND);
}

TEST_F(ObjectSetPropertyByNameRefTest, invalid_argument)
{
    ani_object car = NewCar();
    ani_string string {};

    ASSERT_EQ(env_->Object_SetPropertyByName_Ref(nullptr, "highPerformance", string), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_SetPropertyByName_Ref(car, nullptr, string), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetPropertyByNameRefTest, set_field_property_invalid_type)
{
    ani_object car = NewCar();
    ani_string string {};

    std::string_view utf8string = "12345";
    ASSERT_EQ(env_->String_NewUTF8(utf8string.data(), utf8string.size(), &string), ANI_OK);

    ASSERT_EQ(env_->Object_SetPropertyByName_Ref(car, "manufacturer", string), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetPropertyByNameRefTest, set_setter_property_invalid_type)
{
    ani_object car = NewCar();
    ani_string string {};

    std::string_view utf8string = "12345";
    ASSERT_EQ(env_->String_NewUTF8(utf8string.data(), utf8string.size(), &string), ANI_OK);

    ASSERT_EQ(env_->Object_SetPropertyByName_Ref(car, "model", string), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetPropertyByNameRefTest, set_interface_field)
{
    ani_object c1 = NewC1();

    ani_ref prop {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(c1, "prop", &prop), ANI_OK);
    std::string defaultVal {};
    RetrieveStringFromAni(env_, reinterpret_cast<ani_string>(prop), defaultVal);
    ASSERT_STREQ(defaultVal.data(), "Default");

    ani_string string {};
    std::string toSet = "InterfaceField";
    ASSERT_EQ(env_->String_NewUTF8(toSet.data(), toSet.size(), &string), ANI_OK);
    ASSERT_EQ(env_->Object_SetPropertyByName_Ref(c1, "prop", string), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(c1, "prop", &prop), ANI_OK);
    RetrieveStringFromAni(env_, reinterpret_cast<ani_string>(prop), defaultVal);
    ASSERT_STREQ(defaultVal.data(), "InterfaceField");
}

TEST_F(ObjectSetPropertyByNameRefTest, set_interface_property)
{
    ani_object c2 = NewC2();

    ani_ref prop {};
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(c2, "prop", &prop), ANI_OK);
    std::string defaultVal {};
    RetrieveStringFromAni(env_, reinterpret_cast<ani_string>(prop), defaultVal);
    ASSERT_STREQ(defaultVal.data(), "Default");

    ani_string string {};
    std::string toSet = "InterfaceProp";
    ASSERT_EQ(env_->String_NewUTF8(toSet.data(), toSet.size(), &string), ANI_OK);
    ASSERT_EQ(env_->Object_SetPropertyByName_Ref(c2, "prop", string), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Ref(c2, "prop", &prop), ANI_OK);
    RetrieveStringFromAni(env_, reinterpret_cast<ani_string>(prop), defaultVal);
    ASSERT_STREQ(defaultVal.data(), "InterfaceProp");
}

}  // namespace ark::ets::ani::testing
