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

class ReferenceIsUndefinedTest : public AniTest {
public:
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr int32_t LOOP_COUNT = 3;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr const char *MODULE_NAME = "reference_is_undefined_test";
};

TEST_F(ReferenceIsUndefinedTest, check_null)
{
    auto ref = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    ani_boolean isUndefined = ANI_TRUE;
    ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_FALSE);
}

TEST_F(ReferenceIsUndefinedTest, check_undefined)
{
    auto ref = CallEtsFunction<ani_ref>(MODULE_NAME, "GetUndefined");
    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);
}

TEST_F(ReferenceIsUndefinedTest, check_object)
{
    auto ref = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
    ani_boolean isUndefined = ANI_TRUE;
    ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_FALSE);
}

TEST_F(ReferenceIsUndefinedTest, invalid_argument)
{
    auto ref = CallEtsFunction<ani_ref>(MODULE_NAME, "GetNull");
    ASSERT_EQ(env_->Reference_IsUndefined(ref, nullptr), ANI_INVALID_ARGS);
    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Reference_IsUndefined(nullptr, ref, &isUndefined), ANI_INVALID_ARGS);
}

TEST_F(ReferenceIsUndefinedTest, check_undefined_loop)
{
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        auto ref = CallEtsFunction<ani_ref>(MODULE_NAME, "GetUndefined");
        ani_boolean isUndefined = ANI_FALSE;
        ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndefined), ANI_OK);
        ASSERT_EQ(isUndefined, ANI_TRUE);
    }
}

TEST_F(ReferenceIsUndefinedTest, mix_test)
{
    auto ref = CallEtsFunction<ani_ref>(MODULE_NAME, "GetUndefined");
    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);

    auto objectRef = CallEtsFunction<ani_ref>(MODULE_NAME, "getObject");
    ASSERT_EQ(env_->Reference_IsUndefined(objectRef, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_FALSE);

    ani_boolean isEquals = ANI_TRUE;
    ASSERT_EQ(env_->Reference_Equals(ref, objectRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

static void IsUndefinedHelper(ani_env *env, ani_ref ref, ani_boolean *result)
{
    ASSERT_EQ(env->Reference_IsUndefined(ref, result), ANI_OK);
    ani_ref undefinedRef {};
    ASSERT_EQ(env->GetUndefined(&undefinedRef), ANI_OK);
    ani_boolean isStrictEqualToUndefined = ANI_FALSE;
    ASSERT_EQ(env->Reference_StrictEquals(ref, undefinedRef, &isStrictEqualToUndefined), ANI_OK);
    ASSERT_EQ(*result, isStrictEqualToUndefined);
}

static ani_boolean IsUndefinedImpl(ani_env *env, ani_ref ref)
{
    ani_boolean isUndefined = ANI_FALSE;
    IsUndefinedHelper(env, ref, &isUndefined);
    return isUndefined;
}

TEST_F(ReferenceIsUndefinedTest, CheckUndefinedReceivedFromManaged)
{
    ani_module mod {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &mod), ANI_OK);
    ani_native_function fn {"isUndefined", "X{C{std.core.Object}C{std.core.Null}}:z",
                            reinterpret_cast<void *>(IsUndefinedImpl)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(mod, &fn, 1), ANI_OK);

    auto isCorrectUndefined = CallEtsFunction<ani_boolean>(MODULE_NAME, "checkUndefinedReference");

    ani_boolean hasPendingError = ANI_FALSE;
    ASSERT_EQ(env_->ExistUnhandledError(&hasPendingError), ANI_OK);
    ASSERT_EQ(hasPendingError, ANI_FALSE);

    ASSERT_EQ(isCorrectUndefined, ANI_TRUE);
}

static ani_ref GetUndefinedFromNativeImpl(ani_env *env)
{
    ani_ref undef {};
    ani_status status = env->GetUndefined(&undef);
    LOG_IF(status != ANI_OK, FATAL, ANI) << "Cannot get undefined ani_ref";
    return undef;
}

TEST_F(ReferenceIsUndefinedTest, CheckUndefinedFromNative)
{
    ani_module mod {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &mod), ANI_OK);
    ani_native_function fn {"getUndefinedFromNative", ":C{std.core.Object}",
                            reinterpret_cast<void *>(GetUndefinedFromNativeImpl)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(mod, &fn, 1), ANI_OK);

    auto isCorrectUndefined = CallEtsFunction<ani_boolean>(MODULE_NAME, "checkUndefinedFromNative");

    ani_boolean hasPendingError = ANI_FALSE;
    ASSERT_EQ(env_->ExistUnhandledError(&hasPendingError), ANI_OK);
    ASSERT_EQ(hasPendingError, ANI_FALSE);

    ASSERT_EQ(isCorrectUndefined, ANI_TRUE);
}

TEST_F(ReferenceIsUndefinedTest, get_undef_test)
{
    ani_ref ref {};
    ani_boolean isUndef = ANI_FALSE;
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndef), ANI_OK);
    ASSERT_EQ(isUndef, ANI_TRUE);
}

TEST_F(ReferenceIsUndefinedTest, get_null_test)
{
    ani_ref ref {};
    ani_boolean isUndef = ANI_FALSE;
    ASSERT_EQ(env_->GetNull(&ref), ANI_OK);
    ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndef), ANI_OK);
    ASSERT_EQ(isUndef, ANI_FALSE);
}

}  // namespace ark::ets::ani::testing
