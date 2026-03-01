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

#include "ani/ani.h"
#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class IsAssignableFromTest : public AniTest {
public:
    template <bool IS_ASSIGNABLE>
    void CheckIsAssignableFrom(const char *fromClsName, const char *toClsName)
    {
        ani_class fromCls;
        ASSERT_EQ(env_->FindClass(fromClsName, &fromCls), ANI_OK);
        ASSERT_NE(fromCls, nullptr);

        ani_class toCls;
        ASSERT_EQ(env_->FindClass(toClsName, &toCls), ANI_OK);
        ASSERT_NE(toCls, nullptr);

        ani_boolean result;
        ASSERT_EQ(env_->Type_IsAssignableFrom(fromCls, toCls, &result), ANI_OK);

        if constexpr (IS_ASSIGNABLE) {
            ASSERT_TRUE(result);
        } else {
            ASSERT_FALSE(result);
        }
    }
};

TEST_F(IsAssignableFromTest, is_assignable)
{
    CheckIsAssignableFrom<true>("is_assignable_from_test.A", "is_assignable_from_test.A");
    CheckIsAssignableFrom<true>("is_assignable_from_test.B", "is_assignable_from_test.A");
    CheckIsAssignableFrom<true>("is_assignable_from_test.A", "std.core.Object");
    CheckIsAssignableFrom<true>("is_assignable_from_test.B", "std.core.Object");
    CheckIsAssignableFrom<true>("is_assignable_from_test.A", "is_assignable_from_test.I");
    CheckIsAssignableFrom<true>("is_assignable_from_test.B", "is_assignable_from_test.I");
}

TEST_F(IsAssignableFromTest, not_assignable)
{
    CheckIsAssignableFrom<false>("is_assignable_from_test.A", "is_assignable_from_test.B");
    CheckIsAssignableFrom<false>("std.core.Object", "is_assignable_from_test.A");
    CheckIsAssignableFrom<false>("std.core.Object", "is_assignable_from_test.B");
    CheckIsAssignableFrom<false>("is_assignable_from_test.I", "is_assignable_from_test.B");
    CheckIsAssignableFrom<false>("is_assignable_from_test.I", "is_assignable_from_test.A");
}

TEST_F(IsAssignableFromTest, ani_invalid_args)
{
    ani_class clsA;

    ASSERT_EQ(env_->FindClass("is_assignable_from_test.A", &clsA), ANI_OK);
    ASSERT_NE(clsA, nullptr);

    ani_boolean result;
    ASSERT_EQ(env_->Type_IsAssignableFrom(nullptr, clsA, &result), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Type_IsAssignableFrom(clsA, nullptr, &result), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Type_IsAssignableFrom(clsA, clsA, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Type_IsAssignableFrom(nullptr, clsA, clsA, &result), ANI_INVALID_ARGS);
}

TEST_F(IsAssignableFromTest, is_assignable_combind_scenes_001)
{
    CheckIsAssignableFrom<true>("is_assignable_from_test.BaseA", "is_assignable_from_test.BaseA");
    CheckIsAssignableFrom<true>("is_assignable_from_test.SubB", "is_assignable_from_test.SubB");
    CheckIsAssignableFrom<true>("is_assignable_from_test.SubB", "is_assignable_from_test.BaseA");
    CheckIsAssignableFrom<true>("is_assignable_from_test.SubC", "is_assignable_from_test.SubC");
    CheckIsAssignableFrom<true>("is_assignable_from_test.SubC", "is_assignable_from_test.SubB");
    CheckIsAssignableFrom<true>("is_assignable_from_test.SubC", "is_assignable_from_test.BaseA");
    CheckIsAssignableFrom<true>("is_assignable_from_test.D", "is_assignable_from_test.D");

    CheckIsAssignableFrom<false>("is_assignable_from_test.BaseA", "is_assignable_from_test.SubB");
    CheckIsAssignableFrom<false>("is_assignable_from_test.BaseA", "is_assignable_from_test.SubC");
    CheckIsAssignableFrom<false>("is_assignable_from_test.SubB", "is_assignable_from_test.SubC");
    CheckIsAssignableFrom<false>("is_assignable_from_test.SubC", "is_assignable_from_test.D");
    CheckIsAssignableFrom<false>("is_assignable_from_test.SubB", "is_assignable_from_test.D");
    CheckIsAssignableFrom<false>("is_assignable_from_test.BaseA", "is_assignable_from_test.D");
}

TEST_F(IsAssignableFromTest, is_assignable_from_multiple_inheritance)
{
    CheckIsAssignableFrom<true>("is_assignable_from_test.InterfaceA", "is_assignable_from_test.InterfaceA");
    CheckIsAssignableFrom<true>("is_assignable_from_test.InterfaceB", "is_assignable_from_test.InterfaceB");
    CheckIsAssignableFrom<true>("is_assignable_from_test.InterfaceC", "is_assignable_from_test.InterfaceC");
    CheckIsAssignableFrom<true>("is_assignable_from_test.MyClass", "is_assignable_from_test.MyClass");
    CheckIsAssignableFrom<true>("is_assignable_from_test.MyClass", "is_assignable_from_test.InterfaceC");
    CheckIsAssignableFrom<true>("is_assignable_from_test.MyClass", "is_assignable_from_test.InterfaceB");
    CheckIsAssignableFrom<true>("is_assignable_from_test.MyClass", "is_assignable_from_test.InterfaceA");
    CheckIsAssignableFrom<true>("is_assignable_from_test.InterfaceC", "is_assignable_from_test.InterfaceA");
    CheckIsAssignableFrom<true>("is_assignable_from_test.InterfaceC", "is_assignable_from_test.InterfaceB");

    CheckIsAssignableFrom<false>("is_assignable_from_test.InterfaceC", "is_assignable_from_test.MyClass");
    CheckIsAssignableFrom<false>("is_assignable_from_test.InterfaceA", "is_assignable_from_test.InterfaceB");
    CheckIsAssignableFrom<false>("is_assignable_from_test.InterfaceB", "is_assignable_from_test.InterfaceA");
    CheckIsAssignableFrom<false>("is_assignable_from_test.InterfaceB", "is_assignable_from_test.InterfaceC");
    CheckIsAssignableFrom<false>("is_assignable_from_test.InterfaceB", "is_assignable_from_test.MyClass");
    CheckIsAssignableFrom<false>("is_assignable_from_test.InterfaceA", "is_assignable_from_test.InterfaceC");
    CheckIsAssignableFrom<false>("is_assignable_from_test.InterfaceA", "is_assignable_from_test.MyClass");
}

TEST_F(IsAssignableFromTest, check_initialization)
{
    ani_class fromCls;
    ASSERT_EQ(env_->FindClass("is_assignable_from_test.InterfaceA", &fromCls), ANI_OK);

    ani_class toCls;
    ASSERT_EQ(env_->FindClass("is_assignable_from_test.InterfaceB", &toCls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("is_assignable_from_test.InterfaceA"));
    ASSERT_FALSE(IsRuntimeClassInitialized("is_assignable_from_test.InterfaceB"));
    ani_boolean result;
    ASSERT_EQ(env_->Type_IsAssignableFrom(fromCls, toCls, &result), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("is_assignable_from_test.InterfaceB"));
    ASSERT_FALSE(IsRuntimeClassInitialized("is_assignable_from_test.InterfaceA"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
