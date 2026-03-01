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

#include <limits>
#include "ani.h"
#include "array_gtest_helper.h"
#include "libarkbase/macros.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ArrayGetSetTest : public ArrayHelperTest {
public:
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr const char *MODULE_NAME = "array_push_pop_test";

public:
    void GetExtendedArray(ani_array *resultArray)
    {
        auto throwingArray = CallEtsFunction<ani_object>(MODULE_NAME, "getExtendedArray");
        ani_boolean result = ANI_FALSE;
        ASSERT_EQ(env_->Reference_IsNullishValue(throwingArray, &result), ANI_OK);
        ASSERT_EQ(result, ANI_FALSE);
        ani_class escompatArray {};
        ASSERT_EQ(env_->FindClass("std.core.Array", &escompatArray), ANI_OK);
        ASSERT_EQ(env_->Object_InstanceOf(throwingArray, escompatArray, &result), ANI_OK);
        ASSERT_EQ(result, ANI_TRUE);
        *resultArray = static_cast<ani_array>(throwingArray);
    }
};

TEST_F(ArrayGetSetTest, PushTest)
{
    InitTest();

    ani_size arrSize = 5U;
    ani_array array {};
    ASSERT_EQ(env_->Array_New(0U, nullptr, &array), ANI_OK);

    for (ani_size it = 0; it < arrSize; ++it) {
        ani_object boxedInt {};
        env_->Object_New(intClass, intCtor, &boxedInt, it);
        ASSERT_EQ(env_->Array_Push(array, boxedInt), ANI_OK);
        ani_size len = -1;
        ASSERT_EQ(env_->Array_GetLength(array, &len), ANI_OK);
        ASSERT_EQ(len, it + 1U);

        ani_ref val {};
        ASSERT_EQ(env_->Array_Get(array, it, &val), ANI_OK);
        ani_int unboxedInt = -1;
        ASSERT_EQ(env_->Object_CallMethod_Int(reinterpret_cast<ani_object>(val), intUnbox, &unboxedInt), ANI_OK);
        ASSERT_EQ(unboxedInt, it);
    }
}

TEST_F(ArrayGetSetTest, PopTest)
{
    InitTest();

    ani_size arrSize = 5U;
    ani_array array {};
    ASSERT_EQ(env_->Array_New(0U, nullptr, &array), ANI_OK);

    for (ani_size it = 0; it < arrSize; ++it) {
        ani_object boxedInt {};
        env_->Object_New(intClass, intCtor, &boxedInt, it);
        ASSERT_EQ(env_->Array_Push(array, boxedInt), ANI_OK);
    }
    ani_size len = -1;
    ASSERT_EQ(env_->Array_GetLength(array, &len), ANI_OK);
    ASSERT_EQ(len, arrSize);

    for (ani_int it = arrSize - 1; it >= 0; --it) {
        ani_ref boxedInt {};
        ASSERT_EQ(env_->Array_Pop(array, &boxedInt), ANI_OK);
        ani_int unboxedInt = -1;
        ASSERT_EQ(env_->Object_CallMethod_Int(reinterpret_cast<ani_object>(boxedInt), intUnbox, &unboxedInt), ANI_OK);
        ASSERT_EQ(unboxedInt, it);
    }
    ASSERT_EQ(env_->Array_GetLength(array, &len), ANI_OK);
    ASSERT_EQ(len, 0);
}

static void CheckUnhandledError(ani_env *env, std::string_view expectedMessage)
{
    ani_boolean hasUnhandledError = ANI_FALSE;
    ASSERT_EQ(env->ExistUnhandledError(&hasUnhandledError), ANI_OK);
    ASSERT_EQ(hasUnhandledError, ANI_TRUE);
    ani_error pendingError {};
    ASSERT_EQ(env->GetUnhandledError(&pendingError), ANI_OK);
    ASSERT_EQ(env->ResetError(), ANI_OK);
    ani_ref message {};
    ASSERT_EQ(env->Object_CallMethodByName_Ref(pendingError, "%%get-message", ":C{std.core.String}", &message), ANI_OK);
    std::string messageStr;
    AniTest::GetStdString(env, static_cast<ani_string>(message), messageStr);
    ASSERT_STREQ(messageStr.c_str(), expectedMessage.data());
}

TEST_F(ArrayGetSetTest, PushInvalidArgsTest)
{
    InitTest();

    ani_object boxedInt {};
    ASSERT_EQ(env_->Object_New(intClass, intCtor, &boxedInt, 0), ANI_OK);
    ASSERT_EQ(env_->Array_Push(nullptr, boxedInt), ANI_INVALID_ARGS);

    ani_array throwingArray {};
    GetExtendedArray(&throwingArray);
    ASSERT_EQ(env_->Array_Push(static_cast<ani_array>(throwingArray), boxedInt), ANI_PENDING_ERROR);
    CheckUnhandledError(env_, "pushOne was called");
}

TEST_F(ArrayGetSetTest, PopInvalidArgsTest)
{
    InitTest();

    ani_ref res {};
    ASSERT_EQ(env_->Array_Pop(nullptr, &res), ANI_INVALID_ARGS);

    ani_array throwingArray {};
    GetExtendedArray(&throwingArray);
    ASSERT_EQ(env_->Array_Pop(throwingArray, nullptr), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Array_Pop(throwingArray, &res), ANI_PENDING_ERROR);
    CheckUnhandledError(env_, "pop was called");
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
