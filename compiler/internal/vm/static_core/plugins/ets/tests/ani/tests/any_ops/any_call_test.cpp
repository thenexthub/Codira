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

#include "ani.h"
#include "ani_gtest.h"
// NOLINTBEGIN(readability-magic-numbers)
namespace ark::ets::ani::testing {

class AnyCallTest : public AniTest {
protected:
    void CheckANIStr(const ani_string &str, const std::string &expected) const
    {
        const ani_size utfBufferSize = 20;
        std::array<char, utfBufferSize> utfBuffer = {0};
        ani_size resultSize;
        const ani_size offset = 0;
        ASSERT_EQ(env_->String_GetUTF8SubString(str, offset, expected.size(), utfBuffer.data(), utfBuffer.size(),
                                                &resultSize),
                  ANI_OK);
        ASSERT_STREQ(utfBuffer.data(), expected.c_str());
    }
};

TEST_F(AnyCallTest, AnyCall_Valid)
{
    auto fnObj = reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("any_call_test", "GetFnObj"));
    const std::string str = "test";
    ani_string arg1 = {};
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &arg1), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg1)};
    ani_ref result;
    ASSERT_EQ(env_->Any_Call(fnObj, args.size(), args.data(), &result), ANI_OK);
    CheckANIStr(static_cast<ani_string>(result), str);
}

TEST_F(AnyCallTest, AnyCall_NonFunctionObject)
{
    auto obj = CallEtsFunction<ani_ref>("any_call_test", "GetNonFnObj");
    std::string str = "test";
    ani_string arg1 {};
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &arg1), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg1)};
    ani_ref result {};
    ASSERT_EQ(env_->Any_Call(reinterpret_cast<ani_fn_object>(obj), args.size(), args.data(), &result),
              ANI_PENDING_ERROR);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(AnyCallTest, AnyCall_WithClosure)
{
    auto fnObj = reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("any_call_test", "GetFnObjWithClosure"));
    std::string input = "123";
    std::string expected = "test123";
    ani_string arg {};
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &arg), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg)};
    ani_ref result {};
    ASSERT_EQ(env_->Any_Call(fnObj, args.size(), args.data(), &result), ANI_OK);
    CheckANIStr(static_cast<ani_string>(result), expected);
}

TEST_F(AnyCallTest, AnyCall_NestedFunction)
{
    auto fnObj = reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("any_call_test", "GetFnObjNested"));
    std::string input = "ArkTS";
    std::string expected = "hello ArkTS";
    ani_string arg {};
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &arg), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg)};
    ani_ref result {};
    ASSERT_EQ(env_->Any_Call(fnObj, args.size(), args.data(), &result), ANI_OK);
    CheckANIStr(static_cast<ani_string>(result), expected);
}

TEST_F(AnyCallTest, AnyCall_RecursiveFunction)
{
    auto fnObj = reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("any_call_test", "GetFnObjRecursive"));
    std::string input = "hello1";
    std::string expected = "hello1111111111";
    ani_string arg {};
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &arg), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg)};
    ani_ref result {};
    ASSERT_EQ(env_->Any_Call(fnObj, args.size(), args.data(), &result), ANI_OK);
    CheckANIStr(static_cast<ani_string>(result), expected);
}

TEST_F(AnyCallTest, AnyCall_InvalidFunc)
{
    std::string str = "test";
    ani_string arg {};
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &arg), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg)};
    ani_ref result {};

    ASSERT_EQ(env_->Any_Call(nullptr, args.size(), args.data(), &result), ANI_INVALID_ARGS);
}

TEST_F(AnyCallTest, AnyCall_InvalidArgsPointerWhenArgcNotZero)
{
    auto fnObj = reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("any_call_test", "GetFnObj"));
    ani_ref result {};

    ASSERT_EQ(env_->Any_Call(fnObj, 1, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(AnyCallTest, AnyCall_NullResult)
{
    auto fnObj = reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("any_call_test", "GetFnObj"));
    std::string str = "test";
    ani_string arg {};
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &arg), ANI_OK);
    std::vector<ani_ref> args = {static_cast<ani_ref>(arg)};

    ASSERT_EQ(env_->Any_Call(fnObj, args.size(), args.data(), nullptr), ANI_INVALID_ARGS);
}

TEST_F(AnyCallTest, AnyCall_ZeroArgcWithNullArgv)
{
    auto fnObj = reinterpret_cast<ani_fn_object>(CallEtsFunction<ani_ref>("any_call_test", "GetFnObjNoArgs"));
    ani_ref result {};

    ASSERT_EQ(env_->Any_Call(fnObj, 0, nullptr, &result), ANI_OK);
    CheckANIStr(static_cast<ani_string>(result), "test");
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::ets::ani::testing
