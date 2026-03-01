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

class NamespaceFindVariableTest : public AniTest {};

TEST_F(NamespaceFindVariableTest, get_int_variable_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_variable_test.anyns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "x", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
}

TEST_F(NamespaceFindVariableTest, variable_env)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_variable_test.anyns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->c_api->Namespace_FindVariable(nullptr, ns, "x", &variable), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindVariableTest, get_ref_variable)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_variable_test.anyns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "s", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
}

TEST_F(NamespaceFindVariableTest, invalid_nev)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_variable_test.anyns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->c_api->Namespace_FindVariable(nullptr, ns, "s", &variable), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindVariableTest, invalid_namespace)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(nullptr, "s", &variable), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindVariableTest, invalid_variable_name_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_variable_test.anyns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, nullptr, &variable), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindVariableTest, invalid_variable_name_2)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_variable_test.anyns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "", &variable), ANI_NOT_FOUND);
}

TEST_F(NamespaceFindVariableTest, invalid_variable_name_3)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_variable_test.anyns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "sss", &variable), ANI_NOT_FOUND);
}

TEST_F(NamespaceFindVariableTest, invalid_variable_name_4)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_variable_test.anyns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "测试emoji🙂🙂", &variable), ANI_NOT_FOUND);
}

TEST_F(NamespaceFindVariableTest, invalid_args_result)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_variable_test.anyns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ASSERT_EQ(env_->Namespace_FindVariable(ns, "s", nullptr), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindVariableTest, check_initialization)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_variable_test.anyns", &ns), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("namespace_find_variable_test.anyns"));
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "x", &variable), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("namespace_find_variable_test.anyns"));
}

}  // namespace ark::ets::ani::testing
