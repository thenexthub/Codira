/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class ReferenceStrictEqualsTest : public AniTest {
public:
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr const char *MODULE_NAME = "reference_strict_equals_test";
};

TEST_F(ReferenceStrictEqualsTest, check_null_and_null)
{
    auto nullRef1 = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    auto nullRef2 = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(nullRef1, nullRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceStrictEqualsTest, check_null_and_undefined)
{
    auto nullRef = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    auto undefinedRef = CallEtsFunction<ani_ref>(MODULE_NAME, "GetUndefined");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(nullRef, undefinedRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceStrictEqualsTest, check_null_and_object)
{
    auto nullRef = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    auto objectRef = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(nullRef, objectRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceStrictEqualsTest, check_undefined_and_undefined)
{
    auto undefinedRef1 = CallEtsFunction<ani_ref>(MODULE_NAME, "GetUndefined");
    auto undefinedRef2 = CallEtsFunction<ani_ref>(MODULE_NAME, "GetUndefined");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(undefinedRef1, undefinedRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceStrictEqualsTest, check_undefined_and_object)
{
    auto undefinedRef = CallEtsFunction<ani_ref>(MODULE_NAME, "GetUndefined");
    auto objectRef = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(undefinedRef, objectRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceStrictEqualsTest, check_object_and_object)
{
    auto objectRef1 = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
    auto objectRef2 = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(objectRef1, objectRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceStrictEqualsTest, invalid_argument)
{
    auto ref = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    ASSERT_EQ(env_->Reference_StrictEquals(ref, ref, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ReferenceStrictEqualsTest, invalid_env)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Reference_StrictEquals(nullptr, undefinedRef, undefinedRef, &isEquals), ANI_INVALID_ARGS);
}

TEST_F(ReferenceStrictEqualsTest, invalid_result)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ASSERT_EQ(env_->Reference_StrictEquals(undefinedRef, undefinedRef, nullptr), ANI_INVALID_ARGS);
}

static void AreReferencesStrictEqualHelper(ani_env *env, ani_ref lhs, ani_ref rhs, ani_boolean *result)
{
    ASSERT_EQ(env->Reference_StrictEquals(lhs, rhs, result), ANI_OK);
}

static ani_boolean AreReferencesStrictEqualImpl(ani_env *env, ani_ref lhs, ani_ref rhs)
{
    ani_boolean areEqual = ANI_FALSE;
    AreReferencesStrictEqualHelper(env, lhs, rhs, &areEqual);
    return areEqual;
}

TEST_F(ReferenceStrictEqualsTest, CheckStrictEqualityWithNullishValues)
{
    ani_module mod {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &mod), ANI_OK);
    ani_native_function fn {"areReferencesStrictEqual",
                            "X{C{std.core.Object}C{std.core.Null}}X{C{std.core.Object}C{std.core.Null}}:z",
                            reinterpret_cast<void *>(AreReferencesStrictEqualImpl)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(mod, &fn, 1), ANI_OK);

    auto isCorrect = CallEtsFunction<ani_boolean>(MODULE_NAME, "checkNullishValuesStrictEquality");

    ani_boolean hasPendingError = ANI_FALSE;
    ASSERT_EQ(env_->ExistUnhandledError(&hasPendingError), ANI_OK);
    ASSERT_EQ(hasPendingError, ANI_FALSE);

    ASSERT_EQ(isCorrect, ANI_TRUE);
}

TEST_F(ReferenceStrictEqualsTest, ptr_equal_double_nan_is_false)
{
    auto dnan = CallEtsFunction<ani_ref>(MODULE_NAME, "GetDoubleNaN");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(dnan, dnan, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceStrictEqualsTest, ptr_equal_float_nan_is_false)
{
    auto fnan = CallEtsFunction<ani_ref>(MODULE_NAME, "GetFloatNaN");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(fnan, fnan, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceStrictEqualsTest, ptr_equal_object_nan_is_false)
{
    auto onan = CallEtsFunction<ani_ref>(MODULE_NAME, "GetObjectNaN");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(onan, onan, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

}  // namespace ark::ets::ani::testing
