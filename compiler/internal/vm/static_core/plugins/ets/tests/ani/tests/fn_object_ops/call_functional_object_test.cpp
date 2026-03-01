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

class CallFunctionalObjectTest : public AniTest {
protected:
    void CheckANIStr(const ani_string &str, std::string_view expected) const
    {
        const ani_size utfBufferSize = 20;
        std::array<char, utfBufferSize> utfBuffer = {0};
        ani_size resultSize;
        const ani_size offset = 0;
        ASSERT_EQ(env_->String_GetUTF8SubString(str, offset, expected.size(), utfBuffer.data(), utfBuffer.size(),
                                                &resultSize),
                  ANI_OK);
        ASSERT_STREQ(utfBuffer.data(), expected.data());
    }
};

TEST_F(CallFunctionalObjectTest, functional_object_call_invalid_args)
{
    auto fnObj = reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("call_functional_object_test", "GetFnObj"));
    ani_ref result;
    ASSERT_EQ(env_->FunctionalObject_Call(nullptr, 0, nullptr, &result), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, 0, nullptr, nullptr), ANI_INVALID_ARGS);
    std::array<ani_ref, 1> argv {nullptr};
    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, argv.size(), argv.data(), nullptr), ANI_INVALID_ARGS);
    auto nonFnObj =
        reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("call_functional_object_test", "GetNonFnObj"));
    ASSERT_EQ(env_->FunctionalObject_Call(nonFnObj, 0, nullptr, &result), ANI_INVALID_TYPE);
}

TEST_F(CallFunctionalObjectTest, functional_object_call)
{
    auto fnObj = reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("call_functional_object_test", "GetFnObj"));
    static constexpr std::string_view TEST_STRING = "test";
    ani_string arg1 = {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &arg1), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg1)};
    ani_ref result;
    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, args.size(), args.data(), &result), ANI_OK);
    CheckANIStr(static_cast<ani_string>(result), TEST_STRING);
}

TEST_F(CallFunctionalObjectTest, functional_object_call_with_closure)
{
    auto fnObjWithClosure =
        reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("call_functional_object_test", "GetFnObjWithClosure"));
    static constexpr std::string_view TEST_STRING = "test";
    ani_string arg1 = {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &arg1), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg1)};
    ani_ref result;
    ASSERT_EQ(env_->FunctionalObject_Call(fnObjWithClosure, args.size(), args.data(), &result), ANI_OK);
    std::string expected = std::string(TEST_STRING) + std::string(TEST_STRING);
    CheckANIStr(static_cast<ani_string>(result), expected);
}

TEST_F(CallFunctionalObjectTest, functional_object_call_invalid_args_1)
{
    auto fnObj = reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("call_functional_object_test", "GetFnObj"));
    const std::string str = "test";
    ani_string arg1 = {};
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &arg1), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg1)};
    ani_ref result;
    EXPECT_EQ(env_->c_api->FunctionalObject_Call(nullptr, fnObj, args.size(), args.data(), &result), ANI_INVALID_ARGS);
    EXPECT_EQ(env_->FunctionalObject_Call(nullptr, args.size(), args.data(), &result), ANI_INVALID_ARGS);
    EXPECT_EQ(env_->FunctionalObject_Call(fnObj, 1, nullptr, &result), ANI_INVALID_ARGS);
    EXPECT_EQ(env_->FunctionalObject_Call(fnObj, args.size(), args.data(), nullptr), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, 0, args.data(), &result), ANI_PENDING_ERROR);
    ani_error currentError {};
    ASSERT_EQ(env_->GetUnhandledError(&currentError), ANI_OK);
    ani_class expectedErrorClass {};
    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->FindClass("std.core.ArgumentsUnderapplicationError", &expectedErrorClass), ANI_OK);
    ani_boolean isExpectedClass = ANI_FALSE;
    ASSERT_EQ(env_->Object_InstanceOf(currentError, expectedErrorClass, &isExpectedClass), ANI_OK);
    ASSERT_EQ(isExpectedClass, ANI_TRUE);
}

TEST_F(CallFunctionalObjectTest, functional_object_call_multiple_call)
{
    auto fnObj = reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("call_functional_object_test", "GetFnObj"));
    static constexpr std::string_view TEST_STRING = "test";
    ani_string arg1 = {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &arg1), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg1)};
    ani_ref result;
    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, args.size(), args.data(), &result), ANI_OK);
    CheckANIStr(static_cast<ani_string>(result), TEST_STRING);
    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, args.size(), args.data(), &result), ANI_OK);
    CheckANIStr(static_cast<ani_string>(result), TEST_STRING);
    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, args.size(), args.data(), &result), ANI_OK);
    CheckANIStr(static_cast<ani_string>(result), TEST_STRING);
}

TEST_F(CallFunctionalObjectTest, functional_object_call_nested)
{
    auto fnObj =
        reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("call_functional_object_test", "GetFnObjNested"));
    const std::string str1 = "hello ";
    const std::string str2 = "world";
    ani_string arg1 = {};
    ASSERT_EQ(env_->String_NewUTF8(str2.c_str(), str2.size(), &arg1), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg1)};
    ani_ref result = nullptr;
    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, args.size(), args.data(), &result), ANI_OK);
    ASSERT_NE(result, nullptr);
    CheckANIStr(static_cast<ani_string>(result), str1 + str2);
}

TEST_F(CallFunctionalObjectTest, functional_object_call_recursive)
{
    auto fnObj =
        reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("call_functional_object_test", "GetFnObjRecursive"));
    static constexpr std::string_view TEST_STRING = "hello";

    ani_string arg1 = {};
    ASSERT_EQ(env_->String_NewUTF8(TEST_STRING.data(), TEST_STRING.size(), &arg1), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg1)};
    ani_ref result;
    ASSERT_EQ(env_->FunctionalObject_Call(fnObj, args.size(), args.data(), &result), ANI_OK);
    CheckANIStr(static_cast<ani_string>(result), TEST_STRING);
}
}  // namespace ark::ets::ani::testing
