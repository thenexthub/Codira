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
#include <cstring>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers,
// readability-identifier-naming)
namespace ark::ets::ani::testing {

class VariableSetValueRefTest : public AniTest {
public:
    void SetUp() override
    {
        AniTest::SetUp();
        ASSERT_EQ(env_->FindNamespace("variable_set_value_ref_test.anyns", &ns_), ANI_OK);
        ASSERT_NE(ns_, nullptr);
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ani_namespace ns_ {};
};

TEST_F(VariableSetValueRefTest, set_int_value_normal)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "stringValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable, &ref), ANI_OK);
    auto str = static_cast<ani_string>(ref);
    std::array<char, 10U> buffer {};
    ani_size strSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(str, 0U, 4U, buffer.data(), buffer.size(), &strSize), ANI_OK);
    ASSERT_EQ(strSize, 4U);
    ASSERT_STREQ(buffer.data(), "test");

    ani_string string = {};
    auto status = env_->String_NewUTF8("abcdefg", 7U, &string);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable, string), ANI_OK);

    ASSERT_EQ(env_->Variable_GetValue_Ref(variable, &ref), ANI_OK);
    str = static_cast<ani_string>(ref);
    ASSERT_EQ(env_->String_GetUTF8SubString(str, 0U, 7U, buffer.data(), buffer.size(), &strSize), ANI_OK);
    ASSERT_EQ(strSize, 7U);
    ASSERT_STREQ(buffer.data(), "abcdefg");
}

TEST_F(VariableSetValueRefTest, invalid_env)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "stringValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_string string = {};
    auto status = env_->String_NewUTF8("abcdefg", 7U, &string);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(env_->c_api->Variable_SetValue_Ref(nullptr, variable, string), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueRefTest, invalid_variable_type)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "shortValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_string value = {};
    auto status = env_->String_NewUTF8("abcdefg", 7U, &value);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable, value), ANI_INVALID_TYPE);
}

TEST_F(VariableSetValueRefTest, invalid_variable)
{
    ani_string value = {};
    auto status = env_->String_NewUTF8("abcdefg", 7U, &value);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(nullptr, value), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueRefTest, invalid_value)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "stringValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ASSERT_EQ(env_->Variable_SetValue_Ref(variable, nullptr), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueRefTest, composite_case_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("variable_set_value_ref_test.anyns.A", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "add", ":C{std.core.String}", &method), ANI_OK);

    ani_ref sum = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method, &sum), ANI_OK);
    auto str = static_cast<ani_string>(sum);
    std::array<char, 20U> buffer {};
    ani_size strSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(str, 0U, 10U, buffer.data(), buffer.size(), &strSize), ANI_OK);
    ASSERT_EQ(strSize, 10U);
    ASSERT_STREQ(buffer.data(), "helloworld");

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "sum", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_ref getValue = nullptr;
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable, &getValue), ANI_OK);
    str = static_cast<ani_string>(getValue);
    ASSERT_EQ(env_->String_GetUTF8SubString(str, 0U, 10U, buffer.data(), buffer.size(), &strSize), ANI_OK);
    ASSERT_EQ(strSize, 10U);
    ASSERT_STREQ(buffer.data(), "helloworld");

    ani_string string = {};
    auto status = env_->String_NewUTF8("abcdefg", 7U, &string);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable, string), ANI_OK);
    ani_ref result = nullptr;
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable, &result), ANI_OK);
    str = static_cast<ani_string>(result);
    ASSERT_EQ(env_->String_GetUTF8SubString(str, 0U, 7U, buffer.data(), buffer.size(), &strSize), ANI_OK);
    ASSERT_EQ(strSize, 7U);
    ASSERT_STREQ(buffer.data(), "abcdefg");
}

TEST_F(VariableSetValueRefTest, composite_case_2)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "stringValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_string string = {};
    ASSERT_EQ(env_->String_NewUTF8("abcdefg", 7U, &string), ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable, string), ANI_OK);

    ASSERT_EQ(env_->String_NewUTF8("xxxx", 4U, &string), ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable, string), ANI_OK);

    ASSERT_EQ(env_->String_NewUTF8("hello world", 11U, &string), ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable, string), ANI_OK);

    ani_ref result = nullptr;
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable, &result), ANI_OK);
    auto str = static_cast<ani_string>(result);
    std::array<char, 20U> buffer {};
    ani_size strSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(str, 0U, 11U, buffer.data(), buffer.size(), &strSize), ANI_OK);
    ASSERT_EQ(strSize, 11U);
    ASSERT_STREQ(buffer.data(), "hello world");

    ASSERT_EQ(env_->Variable_GetValue_Ref(variable, &result), ANI_OK);
    str = static_cast<ani_string>(result);
    ASSERT_EQ(env_->String_GetUTF8SubString(str, 0U, 11U, buffer.data(), buffer.size(), &strSize), ANI_OK);
    ASSERT_EQ(strSize, 11U);
    ASSERT_STREQ(buffer.data(), "hello world");
}

TEST_F(VariableSetValueRefTest, composite_case_3)
{
    ani_namespace result {};
    ASSERT_EQ(env_->FindNamespace("variable_set_value_ref_test.anyns.second", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_variable variable1 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "stringValue", &variable1), ANI_OK);
    ASSERT_NE(variable1, nullptr);

    ani_string string = {};
    ASSERT_EQ(env_->String_NewUTF8("ABC", 3U, &string), ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable1, string), ANI_OK);
    ani_ref getValue1 = nullptr;
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable1, &getValue1), ANI_OK);
    auto str = static_cast<ani_string>(getValue1);
    std::array<char, 10U> buffer {};
    ani_size strSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(str, 0U, 3U, buffer.data(), buffer.size(), &strSize), ANI_OK);
    ASSERT_EQ(strSize, 3U);
    ASSERT_STREQ(buffer.data(), "ABC");

    ani_variable variable2 {};
    ASSERT_EQ(env_->Namespace_FindVariable(result, "aValue", &variable2), ANI_OK);
    ASSERT_NE(variable2, nullptr);

    ASSERT_EQ(env_->String_NewUTF8("EFG", 3U, &string), ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable2, string), ANI_OK);
    ani_ref getValue2 = nullptr;
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable2, &getValue2), ANI_OK);
    str = static_cast<ani_string>(getValue2);
    ASSERT_EQ(env_->String_GetUTF8SubString(str, 0U, 3U, buffer.data(), buffer.size(), &strSize), ANI_OK);
    ASSERT_EQ(strSize, 3U);
    ASSERT_STREQ(buffer.data(), "EFG");
}

TEST_F(VariableSetValueRefTest, composite_case_4)
{
    ani_variable variable1 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "stringValue", &variable1), ANI_OK);
    ASSERT_NE(variable1, nullptr);

    ani_string string = {};
    ASSERT_EQ(env_->String_NewUTF8("XYZ", 3U, &string), ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable1, string), ANI_OK);
    ani_ref getValue1 = nullptr;
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable1, &getValue1), ANI_OK);
    auto str = static_cast<ani_string>(getValue1);
    std::array<char, 10U> buffer {};
    ani_size strSize = 0U;
    ASSERT_EQ(env_->String_GetUTF8SubString(str, 0U, 3U, buffer.data(), buffer.size(), &strSize), ANI_OK);
    ASSERT_EQ(strSize, 3U);
    ASSERT_STREQ(buffer.data(), "XYZ");

    ani_variable variable2 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "testValue", &variable2), ANI_OK);
    ASSERT_NE(variable2, nullptr);

    ASSERT_EQ(env_->String_NewUTF8("UVW", 3U, &string), ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable2, string), ANI_OK);
    ani_ref getValue2 = nullptr;
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable2, &getValue2), ANI_OK);
    str = static_cast<ani_string>(getValue2);
    ASSERT_EQ(env_->String_GetUTF8SubString(str, 0U, 3U, buffer.data(), buffer.size(), &strSize), ANI_OK);
    ASSERT_EQ(strSize, 3U);
    ASSERT_STREQ(buffer.data(), "UVW");
}

TEST_F(VariableSetValueRefTest, check_initialization)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "stringValue", &variable), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("variable_set_value_ref_test.anyns"));
    ani_string string = {};
    ASSERT_EQ(env_->String_NewUTF8("VLG", 3U, &string), ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable, string), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("variable_set_value_ref_test.anyns"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers,
// readability-identifier-naming)