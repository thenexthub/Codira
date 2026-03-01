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

#include "plugins/ets/tests/ani/ani_gtest/verify_ani_gtest.h"

namespace ark::ets::ani::verify::testing {

class DestroyEscapeLocalScopeTest : public VerifyAniTest {
protected:
    void CreateEscapeLocalScope()
    {
        ASSERT_EQ(env_->CreateEscapeLocalScope(1), ANI_OK);
    }

    void DestroyEscapeLocalScope()
    {
        ani_ref ref {};
        ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
        ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(env_, ref, &ref), ANI_OK);
    }
};

TEST_F(DestroyEscapeLocalScopeTest, wrong_env)
{
    CreateEscapeLocalScope();
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);

    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(nullptr, ref, &ref), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"ref", "ani_ref"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyEscapeLocalScope", testLines);
    DestroyEscapeLocalScope();
}

TEST_F(DestroyEscapeLocalScopeTest, wrong_ref_0)
{
    CreateEscapeLocalScope();
    ani_ref ref {};
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(env_, nullptr, &ref), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyEscapeLocalScope", testLines);
    DestroyEscapeLocalScope();
}

TEST_F(DestroyEscapeLocalScopeTest, wrong_ref_1)
{
    CreateEscapeLocalScope();
    const long magicNumber = 0xdeadc0de;  // NOLINT(google-runtime-int)
    auto ref = bit_cast<ani_ref>(magicNumber);
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(env_, nullptr, &ref), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyEscapeLocalScope", testLines);
    DestroyEscapeLocalScope();
}

TEST_F(DestroyEscapeLocalScopeTest, wrong_result)
{
    CreateEscapeLocalScope();
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);

    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(env_, ref, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref"},
        {"result", "ani_ref *", "wrong pointer for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyEscapeLocalScope", testLines);
    DestroyEscapeLocalScope();
}

TEST_F(DestroyEscapeLocalScopeTest, wrong_all_args)
{
    CreateEscapeLocalScope();
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(nullptr, nullptr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_ref *", "wrong pointer for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyEscapeLocalScope", testLines);
    DestroyEscapeLocalScope();
}

TEST_F(DestroyEscapeLocalScopeTest, success)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(1), ANI_OK);
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ani_ref escapeRef {};
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(env_, ref, &escapeRef), ANI_OK);
    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsUndefined(escapeRef, &isUndefined), ANI_OK);
}

TEST_F(DestroyEscapeLocalScopeTest, invalid_ref_after_destroy_scope)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(1), ANI_OK);
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ani_ref escapeRef {};
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(env_, ref, &escapeRef), ANI_OK);
    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndefined), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Reference_IsUndefined", testLines);
}

TEST_F(DestroyEscapeLocalScopeTest, call_without_scope)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(env_, ref, &ref), ANI_ERROR);
    ASSERT_ERROR_ANI_MSG("DestroyEscapeLocalScope",
                         "Illegal call DestroyEscapeLocalScope() without first calling CreateEscapeLocalScope()");
}

TEST_F(DestroyEscapeLocalScopeTest, call_without_scope_and_wrong_env)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(nullptr, ref, &ref), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"ref", "ani_ref"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyEscapeLocalScope", testLines);
}

TEST_F(DestroyEscapeLocalScopeTest, call_without_scope_and_wrong_ref)
{
    ani_ref ref {};
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(env_, nullptr, &ref), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyEscapeLocalScope", testLines);
}

TEST_F(DestroyEscapeLocalScopeTest, call_without_scope_and_wrong_result)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(env_, ref, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref"},
        {"result", "ani_ref *", "wrong pointer for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyEscapeLocalScope", testLines);
}

TEST_F(DestroyEscapeLocalScopeTest, call_with_wrong_scope)
{
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);

    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(env_, ref, &ref), ANI_ERROR);
    ASSERT_ERROR_ANI_MSG("DestroyEscapeLocalScope",
                         "Illegal call DestroyEscapeLocalScope() after calling CreateLocalScope()");

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(DestroyEscapeLocalScopeTest, call_with_wrong_scope_and_wrong_env)
{
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);

    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(nullptr, ref, &ref), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"ref", "ani_ref"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyEscapeLocalScope", testLines);

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(DestroyEscapeLocalScopeTest, call_with_wrong_scope_and_wrong_ref)
{
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);

    ani_ref ref {};
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(env_, nullptr, &ref), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyEscapeLocalScope", testLines);

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(DestroyEscapeLocalScopeTest, call_with_wrong_scope_and_wrong_result)
{
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);

    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->c_api->DestroyEscapeLocalScope(env_, ref, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref"},
        {"result", "ani_ref *", "wrong pointer for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyEscapeLocalScope", testLines);

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
