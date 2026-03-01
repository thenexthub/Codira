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

class ObjectSetFieldRefTest : public AniTest {
public:
    void GetTestData(ani_object *boxResult, ani_field *fieldIntResult, ani_field *fieldStringResult)
    {
        auto boxRef = CallEtsFunction<ani_ref>("object_set_field_ref_test", "newBoxObject");

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_set_field_ref_test.Boxx", &cls), ANI_OK);

        ani_field fieldInt {};
        ASSERT_EQ(env_->Class_FindField(cls, "int_value", &fieldInt), ANI_OK);

        ani_field fieldString {};
        ASSERT_EQ(env_->Class_FindField(cls, "string_value", &fieldString), ANI_OK);

        *boxResult = static_cast<ani_object>(boxRef);
        *fieldIntResult = fieldInt;
        *fieldStringResult = fieldString;
    }
};

TEST_F(ObjectSetFieldRefTest, set_field_ref)
{
    ani_object box {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&box, &fieldInt, &fieldString);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    const int32_t loopCount = 3;
    for (int32_t i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetField_Ref(box, fieldString, string), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_ref_test", "checkStringValue", box, string), ANI_TRUE);

        ani_ref nameRef {};
        ASSERT_EQ(env_->Object_GetField_Ref(box, fieldString, &nameRef), ANI_OK);

        auto name = static_cast<ani_string>(nameRef);
        std::array<char, 7U> buffer {};
        ani_size nameSize {};
        ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 6U, buffer.data(), buffer.size(), &nameSize), ANI_OK);
        ASSERT_EQ(nameSize, 6U);
        ASSERT_STREQ(buffer.data(), "abcdef");

        ani_string string1 {};
        ASSERT_EQ(env_->String_NewUTF8("abcdefg", 7U, &string1), ANI_OK);

        ASSERT_EQ(env_->Object_SetField_Ref(box, fieldString, string1), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_ref_test", "checkStringValue", box, string1),
                  ANI_TRUE);

        ani_ref nameRef1 {};
        ASSERT_EQ(env_->Object_GetField_Ref(box, fieldString, &nameRef1), ANI_OK);

        auto name1 = static_cast<ani_string>(nameRef1);
        std::array<char, 8U> buffer1 {};
        ani_size nameSize1 {};
        ASSERT_EQ(env_->String_GetUTF8SubString(name1, 0U, 7U, buffer1.data(), buffer1.size(), &nameSize1), ANI_OK);
        ASSERT_EQ(nameSize1, 7U);
        ASSERT_STREQ(buffer1.data(), "abcdefg");
    }
}

TEST_F(ObjectSetFieldRefTest, set_field_ref2)
{
    auto boxc = static_cast<ani_object>(CallEtsFunction<ani_ref>("object_set_field_ref_test", "newBoxcObject"));

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_set_field_ref_test.Boxc", &cls), ANI_OK);

    ani_field fieldInt {};
    ASSERT_EQ(env_->Class_FindField(cls, "int_value", &fieldInt), ANI_OK);

    ani_field fieldString {};
    ASSERT_EQ(env_->Class_FindField(cls, "string_value", &fieldString), ANI_OK);

    ani_field fieldStr {};
    ASSERT_EQ(env_->Class_FindField(cls, "str_value", &fieldStr), ANI_OK);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Object_SetField_Ref(boxc, fieldString, string), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_ref_test", "checkStringValue", boxc, string), ANI_TRUE);

    ani_string str {};
    ASSERT_EQ(env_->String_NewUTF8("fedcba", 6U, &str), ANI_OK);

    ASSERT_EQ(env_->Object_SetField_Ref(boxc, fieldStr, str), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_ref_test", "checkStrValue", boxc, str), ANI_TRUE);
}

TEST_F(ObjectSetFieldRefTest, set_field_ref_invalid_args_env)
{
    ani_object box {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&box, &fieldInt, &fieldString);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->c_api->Object_SetField_Ref(nullptr, box, fieldString, string), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldRefTest, set_field_ref_invalid_field_type)
{
    ani_object box {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&box, &fieldInt, &fieldString);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Object_SetField_Ref(box, fieldInt, string), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldRefTest, set_field_ref_invalid_args_object)
{
    ani_object box {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&box, &fieldInt, &fieldString);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Object_SetField_Ref(nullptr, fieldString, string), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldRefTest, set_field_ref_invalid_args_field)
{
    ani_object box {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&box, &fieldInt, &fieldString);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Object_SetField_Ref(box, nullptr, string), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldRefTest, set_field_ref_invalid_args_value)
{
    ani_object box {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&box, &fieldInt, &fieldString);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8("abcdef", 6U, &string), ANI_OK);

    ASSERT_EQ(env_->Object_SetField_Ref(box, fieldString, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
