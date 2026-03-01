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
#include "array_gtest_helper.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ArrayGetSetTest : public ArrayHelperTest {
public:
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr const char *MODULE_NAME = "array_get_set_test";

public:
    void GetExtendedArray(ani_array *result)
    {
        auto extendedArray = CallEtsFunction<ani_ref>(MODULE_NAME, "getExtendedArray");
        ani_boolean isCorrect = ANI_FALSE;
        ASSERT_EQ(env_->ExistUnhandledError(&isCorrect), ANI_OK);
        ASSERT_EQ(isCorrect, ANI_FALSE);
        ASSERT_EQ(env_->Reference_IsNullishValue(extendedArray, &isCorrect), ANI_OK);
        ASSERT_EQ(isCorrect, ANI_FALSE);
        ani_class escompatArray {};
        ASSERT_EQ(env_->FindClass("std.core.Array", &escompatArray), ANI_OK);
        ASSERT_EQ(env_->Object_InstanceOf(static_cast<ani_object>(extendedArray), escompatArray, &isCorrect), ANI_OK);
        ASSERT_EQ(isCorrect, ANI_TRUE);
        *result = static_cast<ani_array>(extendedArray);
    }
};

TEST_F(ArrayGetSetTest, GetTest)
{
    InitTest();

    ani_size arrSize = 5U;

    ani_ref undefined {};
    ASSERT_EQ(env_->GetUndefined(&undefined), ANI_OK);
    ani_array array {};
    ASSERT_EQ(env_->Array_New(arrSize, undefined, &array), ANI_OK);
    ani_ref val {};
    for (ani_size it = 0; it < arrSize; ++it) {
        ASSERT_EQ(env_->Array_Get(array, it, &val), ANI_OK);
        ani_boolean res = ANI_FALSE;
        ASSERT_EQ(env_->Reference_IsUndefined(val, &res), ANI_OK);
        ASSERT_EQ(res, ANI_TRUE);
    }

    ani_object boxedBoolean {};
    env_->Object_New(booleanClass, booleanCtor, &boxedBoolean, ANI_TRUE);
    ani_array arrayBoolean {};
    ASSERT_EQ(env_->Array_New(arrSize, boxedBoolean, &arrayBoolean), ANI_OK);
    for (ani_size it = 0; it < arrSize; ++it) {
        ASSERT_EQ(env_->Array_Get(arrayBoolean, it, &val), ANI_OK);
        ani_boolean unboxedBoolean = ANI_FALSE;
        ASSERT_EQ(env_->Object_CallMethod_Boolean(reinterpret_cast<ani_object>(val), booleanUnbox, &unboxedBoolean),
                  ANI_OK);
        ASSERT_EQ(unboxedBoolean, ANI_TRUE);
    }

    ani_object boxedInt {};
    // NOLINTNEXTLINE(readability-magic-numbers)
    env_->Object_New(intClass, intCtor, &boxedInt, 10U);
    ani_array arrayInt {};
    ASSERT_EQ(env_->Array_New(arrSize, boxedInt, &arrayInt), ANI_OK);
    for (ani_size it = 0; it < arrSize; ++it) {
        ASSERT_EQ(env_->Array_Get(arrayInt, it, &val), ANI_OK);
        ani_int unboxedInt = -1;
        ASSERT_EQ(env_->Object_CallMethod_Int(reinterpret_cast<ani_object>(val), intUnbox, &unboxedInt), ANI_OK);
        ASSERT_EQ(unboxedInt, 10U);
    }
}

TEST_F(ArrayGetSetTest, SetTest)
{
    InitTest();

    ani_size arrSize = 5U;

    ani_object boxedInt {};

    ani_ref undefined {};
    ASSERT_EQ(env_->GetUndefined(&undefined), ANI_OK);
    ani_array array {};
    ASSERT_EQ(env_->Array_New(arrSize, undefined, &array), ANI_OK);
    ani_ref val {};
    for (ani_size it = 0; it < arrSize; ++it) {
        env_->Object_New(intClass, intCtor, &boxedInt, it);
        ASSERT_EQ(env_->Array_Set(array, it, boxedInt), ANI_OK);
        ASSERT_EQ(env_->Array_Get(array, it, &val), ANI_OK);
        ani_int unboxedInt = -1;
        ASSERT_EQ(env_->Object_CallMethod_Int(reinterpret_cast<ani_object>(val), intUnbox, &unboxedInt), ANI_OK);
        ASSERT_EQ(unboxedInt, it);
    }

    ani_object boxedInt2 {};
    ani_array arrayInt {};
    ASSERT_EQ(env_->Array_New(arrSize, boxedInt, &arrayInt), ANI_OK);
    for (ani_size it = 0; it < arrSize; ++it) {
        env_->Object_New(intClass, intCtor, &boxedInt2, it);
        ASSERT_EQ(env_->Array_Set(arrayInt, it, boxedInt2), ANI_OK);
        ASSERT_EQ(env_->Array_Get(arrayInt, it, &val), ANI_OK);
        ani_int unboxedInt = -1;
        ASSERT_EQ(env_->Object_CallMethod_Int(reinterpret_cast<ani_object>(val), intUnbox, &unboxedInt), ANI_OK);
        ASSERT_EQ(unboxedInt, it);
    }
}

TEST_F(ArrayGetSetTest, GetSetInvalidArgsTest)
{
    InitTest();

    ani_object boxedInt {};
    env_->Object_New(intClass, intCtor, &boxedInt, 0);
    ASSERT_EQ(env_->Array_Set(nullptr, 0, boxedInt), ANI_INVALID_ARGS);
    ani_ref res {};
    ASSERT_EQ(env_->Array_Get(nullptr, 0, &res), ANI_INVALID_ARGS);

    ani_array array {};
    ASSERT_EQ(env_->Array_New(0U, nullptr, &array), ANI_OK);
    ASSERT_EQ(env_->Array_Get(array, 0, nullptr), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->GetUndefined(&res), ANI_OK);
    ASSERT_EQ(env_->Array_Set(array, 0, res), ANI_PENDING_ERROR);
    ani_boolean hasPendingError = ANI_FALSE;
    ASSERT_EQ(env_->ExistUnhandledError(&hasPendingError), ANI_OK);
    ASSERT_EQ(hasPendingError, ANI_TRUE);
    ASSERT_EQ(env_->ResetError(), ANI_OK);

    ASSERT_EQ(env_->Array_Get(array, 0, &res), ANI_PENDING_ERROR);
    ASSERT_EQ(env_->ExistUnhandledError(&hasPendingError), ANI_OK);
    ASSERT_EQ(hasPendingError, ANI_TRUE);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

template <bool GETTER>
static void CheckArrayCounter(ani_env *env, ani_array extendedArray, ani_int expected)
{
    const char *fieldName = "setCounter";
    if constexpr (GETTER) {
        fieldName = "getCounter";
    }
    ani_int value = 0;
    ASSERT_EQ(env->Object_GetFieldByName_Int(extendedArray, fieldName, &value), ANI_OK);
    ASSERT_EQ(value, expected);
}

TEST_F(ArrayGetSetTest, ExtendedArrayLength)
{
    ani_array extendedArray {};
    GetExtendedArray(&extendedArray);

    ani_size sz = 0;
    ASSERT_EQ(env_->Array_GetLength(extendedArray, &sz), ANI_OK);
    // Extended array always returns zero length
    ASSERT_EQ(sz, 0);
}

TEST_F(ArrayGetSetTest, ExtendedArrayGet)
{
    ani_array extendedArray {};
    GetExtendedArray(&extendedArray);

    ani_ref res {};
    ASSERT_EQ(env_->Array_Get(extendedArray, 0, &res), ANI_OK);
    ani_boolean isCorrect = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsNull(res, &isCorrect), ANI_OK);
    ASSERT_EQ(isCorrect, ANI_TRUE);
    CheckArrayCounter<true>(env_, extendedArray, 1U);

    ASSERT_EQ(env_->Array_Get(extendedArray, 1U, &res), ANI_OK);
    ASSERT_EQ(env_->Reference_IsUndefined(res, &isCorrect), ANI_OK);
    ASSERT_EQ(isCorrect, ANI_TRUE);
    CheckArrayCounter<true>(env_, extendedArray, 2U);

    ASSERT_EQ(env_->Array_Get(extendedArray, 2U, &res), ANI_OK);
    ASSERT_EQ(env_->Reference_IsNullishValue(res, &isCorrect), ANI_OK);
    ASSERT_EQ(isCorrect, ANI_FALSE);
    ani_class coreString {};
    ASSERT_EQ(env_->FindClass("std.core.String", &coreString), ANI_OK);
    ASSERT_EQ(env_->Object_InstanceOf(static_cast<ani_object>(res), coreString, &isCorrect), ANI_OK);
    ASSERT_EQ(isCorrect, ANI_TRUE);
    std::string str;
    GetStdString(static_cast<ani_string>(res), str);
    ASSERT_STREQ(str.c_str(), "sample");
    CheckArrayCounter<true>(env_, extendedArray, 3U);
}

TEST_F(ArrayGetSetTest, ExtendedArraySet)
{
    ani_array extendedArray {};
    GetExtendedArray(&extendedArray);

    ani_class coreInt {};
    ASSERT_EQ(env_->FindClass("std.core.Int", &coreInt), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(coreInt, "<ctor>", "i:", &ctor), ANI_OK);

    for (size_t i = 0; i < 3U; ++i) {
        ani_object value {};
        ASSERT_EQ(env_->Object_New(coreInt, ctor, &value, i), ANI_OK);
        ASSERT_EQ(env_->Array_Set(extendedArray, i, value), ANI_OK);
        CheckArrayCounter<false>(env_, extendedArray, i + 1);
    }

    auto isCorrect = CallEtsFunction<ani_boolean>(MODULE_NAME, "checkChangedArray", extendedArray);
    ani_boolean hasError = ANI_FALSE;
    ASSERT_EQ(env_->ExistUnhandledError(&hasError), ANI_OK);
    ASSERT_EQ(hasError, ANI_FALSE);
    ASSERT_EQ(isCorrect, ANI_TRUE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
