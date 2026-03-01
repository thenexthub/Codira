/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
namespace ark::ets::ani::testing {

class ReferenceEqualsTest : public AniTest {
public:
    void GetMethodData(ani_ref *objectRef, ani_ref *methodRef, const char *className, const char *newClassName,
                       const char *signature)
    {
        ani_class cls {};
        // Locate the class in the environment.
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        // Emulate allocation an instance of class.
        ani_static_method newMethod {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, newClassName, signature, &newMethod), ANI_OK);
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, objectRef), ANI_OK);

        const char *methodSignature = "C{std.core.String}C{std.core.String}:C{std.core.String}";
        ani_method concat {};
        ASSERT_EQ(env_->Class_FindMethod(cls, "concat", methodSignature, &concat), ANI_OK);
        ASSERT_NE(concat, nullptr);

        ani_string s0 {};
        ASSERT_EQ(env_->String_NewUTF8("abc", 3U, &s0), ANI_OK);
        ani_string s1 {};
        ASSERT_EQ(env_->String_NewUTF8("def", 3U, &s1), ANI_OK);

        ASSERT_EQ(env_->Object_CallMethod_Ref(static_cast<ani_object>(*objectRef), concat, methodRef, s0, s1), ANI_OK);
    }

    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr int32_t LOOP_COUNT = 3;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr const char *MODULE_NAME = "reference_equals_test";
};

TEST_F(ReferenceEqualsTest, check_null_and_null)
{
    auto nullRef1 = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    auto nullRef2 = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(nullRef1, nullRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceEqualsTest, check_null_and_undefined)
{
    auto nullRef = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    auto undefinedRef = CallEtsFunction<ani_ref>(MODULE_NAME, "GetUndefined");
    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(nullRef, undefinedRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceEqualsTest, check_null_and_object)
{
    auto nullRef = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    auto objectRef = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
    ani_boolean isEquals = ANI_TRUE;
    ASSERT_EQ(env_->Reference_Equals(nullRef, objectRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceEqualsTest, check_undefined_and_undefined)
{
    auto undefinedRef1 = CallEtsFunction<ani_ref>(MODULE_NAME, "GetUndefined");
    auto undefinedRef2 = CallEtsFunction<ani_ref>(MODULE_NAME, "GetUndefined");
    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(undefinedRef1, undefinedRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceEqualsTest, check_undefined_and_object)
{
    auto undefinedRef = CallEtsFunction<ani_ref>(MODULE_NAME, "GetUndefined");
    auto objectRef = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
    ani_boolean isEquals = ANI_TRUE;
    ASSERT_EQ(env_->Reference_Equals(undefinedRef, objectRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceEqualsTest, check_object_and_object)
{
    auto objectRef1 = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
    auto objectRef2 = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(objectRef1, objectRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceEqualsTest, invalid_argument)
{
    auto ref = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    ASSERT_EQ(env_->Reference_Equals(ref, ref, nullptr), ANI_INVALID_ARGS);
    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Reference_Equals(nullptr, ref, ref, &isEquals), ANI_INVALID_ARGS);
}

TEST_F(ReferenceEqualsTest, check_custom_object)
{
    auto packRef1 = CallEtsFunction<ani_ref>(MODULE_NAME, "newPackObject");
    auto packRef2 = CallEtsFunction<ani_ref>(MODULE_NAME, "newPackObject");
    ani_boolean isEquals = ANI_TRUE;
    ASSERT_EQ(env_->Reference_Equals(packRef1, packRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceEqualsTest, check_custom_and_string)
{
    auto packRef = CallEtsFunction<ani_ref>(MODULE_NAME, "newPackObject");
    auto objectRef = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
    ani_boolean isEquals = ANI_TRUE;
    ASSERT_EQ(env_->Reference_Equals(packRef, objectRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceEqualsTest, check_reference_equals_loop)
{
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        auto objectRef1 = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
        auto objectRef2 = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
        ani_boolean isEquals = ANI_FALSE;
        ASSERT_EQ(env_->Reference_Equals(objectRef1, objectRef2, &isEquals), ANI_OK);
        ASSERT_EQ(isEquals, ANI_TRUE);

        auto packRef = CallEtsFunction<ani_ref>(MODULE_NAME, "newPackObject");
        auto objectRef = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
        ASSERT_EQ(env_->Reference_Equals(packRef, objectRef, &isEquals), ANI_OK);
        ASSERT_EQ(isEquals, ANI_FALSE);
    }
}

TEST_F(ReferenceEqualsTest, check_object_and_method)
{
    ani_ref objectARef = nullptr;
    ani_ref methodARef = nullptr;
    GetMethodData(&objectARef, &methodARef, "reference_equals_test.A", "new_A", ":C{reference_equals_test.A}");

    ani_ref objectBRef = nullptr;
    ani_ref methodBRef = nullptr;
    GetMethodData(&objectBRef, &methodBRef, "reference_equals_test.B", "new_B", ":C{reference_equals_test.B}");

    ani_boolean isEquals = ANI_TRUE;
    ASSERT_EQ(env_->Reference_Equals(objectARef, objectBRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);

    ASSERT_EQ(env_->Reference_Equals(methodARef, methodBRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceEqualsTest, check_nullptr)
{
    ani_boolean isEquals = ANI_FALSE;
    ani_ref undefinedRef1;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef1), ANI_OK);

    ani_ref undefinedRef2;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef2), ANI_OK);

    ASSERT_EQ(env_->Reference_Equals(undefinedRef1, undefinedRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);

    auto ref = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    ASSERT_EQ(env_->Reference_Equals(ref, undefinedRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);

    ASSERT_EQ(env_->Reference_Equals(undefinedRef1, ref, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

static void AreReferencesEqualHelper(ani_env *env, ani_ref lhs, ani_ref rhs, ani_boolean *result)
{
    ASSERT_EQ(env->Reference_Equals(lhs, rhs, result), ANI_OK);
}

static ani_boolean AreReferencesEqualImpl(ani_env *env, ani_ref lhs, ani_ref rhs)
{
    ani_boolean areEqual = ANI_FALSE;
    AreReferencesEqualHelper(env, lhs, rhs, &areEqual);
    return areEqual;
}

TEST_F(ReferenceEqualsTest, CheckEqualityWithNullishValues)
{
    ani_module mod {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &mod), ANI_OK);
    ani_native_function fn {"areReferencesEqual",
                            "X{C{std.core.Object}C{std.core.Null}}X{C{std.core.Object}C{std.core.Null}}:z",
                            reinterpret_cast<void *>(AreReferencesEqualImpl)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(mod, &fn, 1), ANI_OK);

    auto isCorrect = CallEtsFunction<ani_boolean>(MODULE_NAME, "checkNullishValuesEquality");

    ani_boolean hasPendingError = ANI_FALSE;
    ASSERT_EQ(env_->ExistUnhandledError(&hasPendingError), ANI_OK);
    ASSERT_EQ(hasPendingError, ANI_FALSE);

    ASSERT_EQ(isCorrect, ANI_TRUE);
}

TEST_F(ReferenceEqualsTest, ptr_equal_double_nan_is_false)
{
    auto dnan = CallEtsFunction<ani_ref>(MODULE_NAME, "GetDoubleNaN");
    ani_boolean isEquals = ANI_TRUE;
    ASSERT_EQ(env_->Reference_Equals(dnan, dnan, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceEqualsTest, ptr_equal_float_nan_is_false)
{
    auto fnan = CallEtsFunction<ani_ref>(MODULE_NAME, "GetFloatNaN");
    ani_boolean isEquals = ANI_TRUE;
    ASSERT_EQ(env_->Reference_Equals(fnan, fnan, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceEqualsTest, ptr_equal_object_nan_is_false)
{
    auto onan = CallEtsFunction<ani_ref>(MODULE_NAME, "GetObjectNaN");
    ani_boolean isEquals = ANI_TRUE;
    ASSERT_EQ(env_->Reference_Equals(onan, onan, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceEqualsTest, ptr_equal_double_value_is_true)
{
    auto d42 = CallEtsFunction<ani_ref>(MODULE_NAME, "GetDouble42");
    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(d42, d42, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceEqualsTest, non_ptr_equal_double_same_value_is_true)
{
    auto d1 = CallEtsFunction<ani_ref>(MODULE_NAME, "GetDouble42");
    auto d2 = CallEtsFunction<ani_ref>(MODULE_NAME, "GetDouble42");
    ani_boolean isEquals = ANI_FALSE;
    ASSERT_NE(d1, d2);
    ASSERT_EQ(env_->Reference_Equals(d1, d2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
