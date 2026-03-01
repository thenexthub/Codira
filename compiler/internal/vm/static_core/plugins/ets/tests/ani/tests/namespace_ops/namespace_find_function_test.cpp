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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class NamespaceFindFunctionTest : public AniTest {};

TEST_F(NamespaceFindFunctionTest, find_function01)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_function_test.Fnns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialIntValue", ":i", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);
    }

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialStringValue", ":C{std.core.String}", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialDoubleValue", ":d", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialIntValue", ":i", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
}

TEST_F(NamespaceFindFunctionTest, find_function02)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_function_test.Fnns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn1 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialIntValue", ":i", &fn1), ANI_OK);
    ASSERT_NE(fn1, nullptr);

    ani_function fn2 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialStringValue", ":C{std.core.String}", &fn2), ANI_OK);
    ASSERT_NE(fn2, nullptr);

    ani_function fn3 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialDoubleValue", ":d", &fn3), ANI_OK);
    ASSERT_NE(fn3, nullptr);
}

TEST_F(NamespaceFindFunctionTest, find_function03)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_function_test.Fnns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("namespace_find_function_test.Fnns.A", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method result {};
    ani_function fn {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "c", "i:C{std.core.Promise}", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
}

TEST_F(NamespaceFindFunctionTest, find_function04)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_function_test.TestA.A", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "b", ":i", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
}

TEST_F(NamespaceFindFunctionTest, find_function05)
{
    ani_namespace ns {};

    ASSERT_EQ(env_->FindNamespace("namespace_find_function_test.TestA.A", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->c_api->Namespace_FindFunction(nullptr, ns, "b", ":i", &fn), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindFunction(nullptr, "b", ":i", &fn), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, nullptr, ":i", &fn), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "", ":i", &fn), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "bA", ":i", &fn), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "b", nullptr, &fn), ANI_OK);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "b", "", &fn), ANI_INVALID_DESCRIPTOR);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "b", "d:ii", &fn), ANI_INVALID_DESCRIPTOR);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "b", "d:ii", nullptr), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindFunctionTest, duplicate_no_signature)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_function_test.Fnns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "overloaded", nullptr, &fn), ANI_AMBIGUOUS);
}

TEST_F(NamespaceFindFunctionTest, check_initialization)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_function_test.Fnns", &ns), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("namespace_find_function_test.Fnns"));
    ani_function fn {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialIntValue", ":i", &fn), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("namespace_find_function_test.Fnns"));
}

TEST_F(NamespaceFindFunctionTest, wrong_signature)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_function_test.Fnns", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn2 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialStringValue", ":C{std/core/String}", &fn2),
              ANI_INVALID_DESCRIPTOR);
}

}  // namespace ark::ets::ani::testing
