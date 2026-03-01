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

#include "ani.h"
#include "ani_gtest.h"
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
namespace ark::ets::ani::testing {

/**
 * @brief Unit test class for testing boolean method calls on ani objects.
 *
 * Inherits from the AniTest base class and provides test cases to verify
 * correct functionality of calling boolean-returning methods with various
 * parameter scenarios.
 */
class ObjectInstanceOfTest : public AniTest {
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
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, newClassName, signature, &newMethod), ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        *objectResult = static_cast<ani_object>(ref);
        *classResult = cls;
    }

    static constexpr int32_t LOOP_COUNT = 3;
};

/**
 * @brief Test case for calling a boolean-returning method with an argument array.
 *
 * This test verifies the correct behavior of calling a method using an array
 * of integer arguments and checks the return value.
 */
TEST_F(ObjectInstanceOfTest, object_instance_of)
{
    ani_object objectA {};
    ani_class classA {};
    GetMethodData(&objectA, &classA, "object_instance_of_test.A", "new_A", ":C{object_instance_of_test.A}");

    ani_object objectB {};
    ani_class classB {};
    GetMethodData(&objectB, &classB, "object_instance_of_test.B", "new_B", ":C{object_instance_of_test.B}");

    ani_object objectC {};
    ani_class classC {};
    GetMethodData(&objectC, &classC, "object_instance_of_test.C", "new_C", ":C{object_instance_of_test.C}");

    ani_object objectD {};
    ani_class classD {};
    GetMethodData(&objectD, &classD, "object_instance_of_test.D", "new_D", ":C{object_instance_of_test.D}");

    ani_type typeRefC = classC;
    ani_type typeRefA = classA;
    ani_boolean res = ANI_FALSE;

    ASSERT_EQ(env_->Object_InstanceOf(objectC, typeRefC, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ASSERT_EQ(env_->Object_InstanceOf(objectB, typeRefC, &res), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    ASSERT_EQ(env_->Object_InstanceOf(objectC, typeRefA, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ASSERT_EQ(env_->Object_InstanceOf(objectD, typeRefA, &res), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    ASSERT_EQ(env_->Object_InstanceOf(nullptr, typeRefA, &res), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_InstanceOf(objectC, nullptr, &res), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_InstanceOf(objectC, typeRefC, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ObjectInstanceOfTest, invalid_parameter)
{
    ani_object objectA {};
    ani_class classA {};
    ani_boolean res = ANI_FALSE;
    GetMethodData(&objectA, &classA, "object_instance_of_test.A", "new_A", ":C{object_instance_of_test.A}");

    ani_type type = nullptr;
    ASSERT_EQ(env_->Object_InstanceOf(objectA, type, &res), ANI_INVALID_ARGS);
    ASSERT_EQ(res, false);

    ani_type typeRefA = classA;
    ASSERT_EQ(env_->c_api->Object_InstanceOf(nullptr, objectA, typeRefA, &res), ANI_INVALID_ARGS);
    ASSERT_EQ(res, false);
}

TEST_F(ObjectInstanceOfTest, object_instance_of_loop)
{
    ani_object objectA {};
    ani_class classA {};
    GetMethodData(&objectA, &classA, "object_instance_of_test.A", "new_A", ":C{object_instance_of_test.A}");

    ani_object objectB {};
    ani_class classB {};
    GetMethodData(&objectB, &classB, "object_instance_of_test.B", "new_B", ":C{object_instance_of_test.B}");

    ani_object objectC {};
    ani_class classC {};
    GetMethodData(&objectC, &classC, "object_instance_of_test.C", "new_C", ":C{object_instance_of_test.C}");

    ani_type typeRefC = classC;
    ani_type typeRefA = classA;
    ani_boolean res = ANI_FALSE;

    for (uint16_t i = 0; i < LOOP_COUNT; ++i) {
        ASSERT_EQ(env_->Object_InstanceOf(objectC, typeRefC, &res), ANI_OK);
        ASSERT_EQ(res, ANI_TRUE);

        ASSERT_EQ(env_->Object_InstanceOf(objectB, typeRefC, &res), ANI_OK);
        ASSERT_EQ(res, false);

        ASSERT_EQ(env_->Object_InstanceOf(objectC, typeRefA, &res), ANI_OK);
        ASSERT_EQ(res, ANI_TRUE);
    }
}

TEST_F(ObjectInstanceOfTest, object_instance_of_array)
{
    ani_object objectArr;
    ani_class classA;
    GetMethodData(&objectArr, &classA, "object_instance_of_test.A", "new_A_array", ":C{std.core.Array}");

    ani_boolean res;
    ani_class arrayCls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &arrayCls), ANI_OK);
    ASSERT_EQ(env_->Object_InstanceOf(objectArr, arrayCls, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(ObjectInstanceOfTest, object_interface_instance_of)
{
    ani_object objectI;
    ani_class classF;
    GetMethodData(&objectI, &classF, "object_instance_of_test.F", "new_I", ":C{object_instance_of_test.I}");

    ani_boolean res;
    ani_class interfaceI;
    ASSERT_EQ(env_->FindClass("object_instance_of_test.I", &interfaceI), ANI_OK);
    ASSERT_EQ(env_->Object_InstanceOf(objectI, classF, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->Object_InstanceOf(objectI, interfaceI, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ani_object objectF;
    GetMethodData(&objectF, &classF, "object_instance_of_test.F", "new_F", ":C{object_instance_of_test.F}");
    ASSERT_EQ(env_->Object_InstanceOf(objectF, classF, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->Object_InstanceOf(objectF, interfaceI, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(ObjectInstanceOfTest, object_union_instance_of)
{
    ani_object objectU;
    ani_class classF;
    GetMethodData(&objectU, &classF, "object_instance_of_test.F", "new_Union", nullptr);

    ani_object objectU1;
    ani_class classD;
    GetMethodData(&objectU1, &classD, "object_instance_of_test.D", "new_Union", nullptr);

    ani_boolean res;
    ASSERT_EQ(env_->Object_InstanceOf(objectU, classF, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->Object_InstanceOf(objectU, classD, &res), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
    ASSERT_EQ(env_->Object_InstanceOf(objectU1, classF, &res), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
    ASSERT_EQ(env_->Object_InstanceOf(objectU1, classD, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(ObjectInstanceOfTest, pure_object_instance_of)
{
    ani_object object;
    ani_object objectD;
    ani_class classF;
    ani_class classD;
    GetMethodData(&object, &classF, "object_instance_of_test.F", "new_Object", ":C{std.core.Object}");
    GetMethodData(&objectD, &classD, "object_instance_of_test.D", "new_D", ":C{object_instance_of_test.D}");

    ani_class classObject;
    ani_class interfaceI;
    ani_class classA;
    ASSERT_EQ(env_->FindClass("std.core.Object", &classObject), ANI_OK);
    ASSERT_EQ(env_->FindClass("object_instance_of_test.I", &interfaceI), ANI_OK);
    ASSERT_EQ(env_->FindClass("object_instance_of_test.A", &classA), ANI_OK);

    ani_boolean res;
    ASSERT_EQ(env_->Object_InstanceOf(objectD, classObject, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->Object_InstanceOf(object, classObject, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->Object_InstanceOf(object, classF, &res), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
    ASSERT_EQ(env_->Object_InstanceOf(object, classD, &res), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
    ASSERT_EQ(env_->Object_InstanceOf(object, interfaceI, &res), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
    ASSERT_EQ(env_->Object_InstanceOf(object, classA, &res), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(ObjectInstanceOfTest, object_boxed_primitive_instance_of)
{
    ani_object objectInt;
    ani_class classF;
    GetMethodData(&objectInt, &classF, "object_instance_of_test.F", "new_Boxed_Primitive",
                  ":X{C{std.core.Int}C{std.core.String}}");

    ani_boolean res;
    ani_class classInt;
    ani_class classObject;
    ASSERT_EQ(env_->FindClass("std.core.Int", &classInt), ANI_OK);
    ASSERT_EQ(env_->FindClass("std.core.Object", &classObject), ANI_OK);
    ASSERT_EQ(env_->Object_InstanceOf(objectInt, classInt, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->Object_InstanceOf(objectInt, classObject, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(ObjectInstanceOfTest, ani_enum_instance_of)
{
    ani_object objectColors;
    ani_class classF;
    GetMethodData(&objectColors, &classF, "object_instance_of_test.F", "new_Color", nullptr);

    ani_object objectItems;
    GetMethodData(&objectItems, &classF, "object_instance_of_test.F", "new_Items", nullptr);

    ani_type colorsType {};
    ASSERT_EQ(env_->Object_GetType(objectColors, &colorsType), ANI_OK);
    ASSERT_NE(colorsType, nullptr);

    ani_type itemsType {};
    ASSERT_EQ(env_->Object_GetType(objectItems, &itemsType), ANI_OK);
    ASSERT_NE(itemsType, nullptr);

    ani_boolean res;
    ASSERT_EQ(env_->Object_InstanceOf(objectColors, colorsType, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ASSERT_EQ(env_->Object_InstanceOf(objectColors, itemsType, &res), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(ObjectInstanceOfTest, ani_enum_item_instance_of)
{
    ani_enum enmColor {};
    ASSERT_EQ(env_->FindEnum("object_instance_of_test.Color", &enmColor), ANI_OK);
    ASSERT_NE(enmColor, nullptr);

    ani_enum enmItems {};
    ASSERT_EQ(env_->FindEnum("object_instance_of_test.Items", &enmItems), ANI_OK);
    ASSERT_NE(enmItems, nullptr);

    ani_enum_item itemRed {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(enmColor, "RED", &itemRed), ANI_OK);
    ASSERT_NE(itemRed, nullptr);

    ani_boolean res {};
    ASSERT_EQ(env_->Object_InstanceOf(itemRed, enmColor, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ASSERT_EQ(env_->Object_InstanceOf(itemRed, enmItems, &res), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
