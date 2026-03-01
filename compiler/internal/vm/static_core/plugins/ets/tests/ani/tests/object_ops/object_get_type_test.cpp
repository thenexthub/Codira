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
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
namespace ark::ets::ani::testing {

/**
 * @brief Unit test class for retrieves the type of a given object.
 *
 * Inherits from the AniTest base class and provides test cases to verify
 * correct functionality of calling  retrieves the type of the specified object methods
 * with various parameter scenarios.
 */
class ObjectGetTypeTest : public AniTest {
public:
    void GetMethodData(ani_object *objectResult, ani_class *classResult, const char *className,
                       const char *newClassName, const char *signature)
    {
        ani_class cls {};
        // Locate the class in the environment.
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        // Emulate allocation an instance of class.
        ani_static_method newMethod {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, newClassName, signature, &newMethod), ANI_OK);
        ani_ref ref {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        *objectResult = static_cast<ani_object>(ref);
        *classResult = cls;
    }

    static constexpr int32_t LOOP_COUNT = 3;
};

TEST_F(ObjectGetTypeTest, class_obect_type)
{
    ani_class classA {};
    ani_object objectA {};
    GetMethodData(&objectA, &classA, "object_get_type_test.A", "new_A", ":C{object_get_type_test.A}");

    ani_type type {};
    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetType(objectA, &type), ANI_OK);
    ASSERT_EQ(env_->Object_InstanceOf(objectA, type, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(ObjectGetTypeTest, string_obect_type)
{
    ani_string result = nullptr;
    auto status = env_->String_NewUTF8("a", 1U, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_type type {};
    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_GetType(result, &type), ANI_OK);
    ASSERT_EQ(env_->Object_InstanceOf(result, type, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(ObjectGetTypeTest, invalid_parameters)
{
    ani_string result = nullptr;
    auto status = env_->String_NewUTF8("a", 1U, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Object_GetType(result, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_GetType(nullptr, nullptr), ANI_INVALID_ARGS);
    ani_type type {};
    ASSERT_EQ(env_->c_api->Object_GetType(nullptr, result, &type), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetTypeTest, invalid_object)
{
    ani_object object = nullptr;
    ani_type type {};
    ASSERT_EQ(env_->Object_GetType(object, &type), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetTypeTest, class_object_type_loop)
{
    ani_class classA {};
    ani_object objectA {};
    GetMethodData(&objectA, &classA, "object_get_type_test.A", "new_A", ":C{object_get_type_test.A}");

    ani_type type {};
    ani_boolean res = ANI_FALSE;
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Object_GetType(objectA, &type), ANI_OK);
        ASSERT_EQ(env_->Object_InstanceOf(objectA, type, &res), ANI_OK);
        ASSERT_EQ(res, ANI_TRUE);
    }
}

TEST_F(ObjectGetTypeTest, eq_obj_type_enum)
{
    ani_enum enmColor {};
    ASSERT_EQ(env_->FindEnum("object_get_type_test.Color", &enmColor), ANI_OK);
    ani_enum_item itemRed {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(enmColor, "RED", &itemRed), ANI_OK);
    ani_type typeColor {};
    ASSERT_EQ(env_->Object_GetType(itemRed, &typeColor), ANI_OK);
    ani_boolean res {};
    ASSERT_EQ(env_->Reference_Equals(enmColor, typeColor, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
