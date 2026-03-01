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

// NOLINTBEGIN(modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class FindNamespaceTest : public AniTest {
public:
    static constexpr ani_int VAL1 = 5;
    static constexpr ani_int VAL2 = 6;
};

TEST_F(FindNamespaceTest, has_namespace)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("find_namespace_test.geometry", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
}

TEST_F(FindNamespaceTest, namespace_not_found)
{
    ani_namespace ns;
    ASSERT_EQ(env_->FindNamespace("find_namespace_test.bla-bla-bla", &ns), ANI_NOT_FOUND);
}

TEST_F(FindNamespaceTest, invalid_arguments)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace(nullptr, &ns), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->FindNamespace("find_namespace_test.geometry", nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->FindNamespace(nullptr, "geometry", &ns), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->FindNamespace("", &ns), ANI_INVALID_DESCRIPTOR);
    ASSERT_EQ(env_->FindNamespace("\t", &ns), ANI_NOT_FOUND);
}

TEST_F(FindNamespaceTest, namespace_is_not_class)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("find_namespace_test.geometry", &cls), ANI_NOT_FOUND);
}

TEST_F(FindNamespaceTest, find_namespace_combine_scenes_002)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("find_namespace_test.nameA.nameB", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "int_method", "ii:i", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);

    ani_value args[2U];
    args[0].i = VAL1;
    args[1].i = VAL2;
    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int_A(fn, &value, args), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(FindNamespaceTest, find_namespace_combine_scenes_003)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("find_namespace_test.spaceA.spaceB.spaceC", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "int_method", "ii:i", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);

    ani_value args[2U];
    args[0].i = VAL1;
    args[1].i = VAL2;
    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int_A(fn, &value, args), ANI_OK);
    ASSERT_EQ(value, VAL1 * VAL2);
}

TEST_F(FindNamespaceTest, check_initialization)
{
    ASSERT_FALSE(IsRuntimeClassInitialized("find_namespace_test.geometry"));
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("find_namespace_test.geometry", &ns), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("find_namespace_test.geometry"));
}

TEST_F(FindNamespaceTest, bad_descriptor)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("find_namespace_test/geometry", &ns), ANI_INVALID_DESCRIPTOR);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(modernize-avoid-c-arrays)