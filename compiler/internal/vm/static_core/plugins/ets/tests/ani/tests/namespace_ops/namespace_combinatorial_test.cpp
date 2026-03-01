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

class NamespaceCombinatorialTest : public AniTest {};
TEST_F(NamespaceCombinatorialTest, find04)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_combinatorial_test.combinatorialA", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function result {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "fun", ":i", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceCombinatorialTest, find05)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_combinatorial_test.combinatorialB", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function result {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "funA", ":i", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceCombinatorialTest, find06)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_combinatorial_test.combinatorialB", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function result {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "funC", ":i", &result), ANI_NOT_FOUND);
}

TEST_F(NamespaceCombinatorialTest, find07)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_combinatorial_test.combinatorial", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->FindNamespace("namespace_combinatorial_test.combinatorial.sub", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_function result1 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "fun", ":i", &result1), ANI_OK);
    ASSERT_NE(result1, nullptr);
}

TEST_F(NamespaceCombinatorialTest, find08)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_combinatorial_test.combinatorialC", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ASSERT_EQ(env_->FindNamespace("namespace_combinatorial_test.combinatorialC.subB", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function result2 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "fun", ":i", &result2), ANI_OK);
    ASSERT_NE(result2, nullptr);
}

TEST_F(NamespaceCombinatorialTest, find09)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_combinatorial_test.combinatorialC", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->FindNamespace("namespace_combinatorial_test.combinatorialC.subc", &tmp), ANI_NOT_FOUND);

    ani_function result2 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "funC", ":i", &result2), ANI_NOT_FOUND);
}

TEST_F(NamespaceCombinatorialTest, find10)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_combinatorial_test.combinatorialD", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->FindNamespace("namespace_combinatorial_test.combinatorialD.subA", &tmp), ANI_OK);
    ASSERT_NE(tmp, nullptr);

    ani_function result2 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "fun", ":i", &result2), ANI_OK);
    ASSERT_NE(result2, nullptr);

    ani_function result5 {};
    ASSERT_EQ(env_->Namespace_FindFunction(nullptr, "fun", ":i", &result5), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing