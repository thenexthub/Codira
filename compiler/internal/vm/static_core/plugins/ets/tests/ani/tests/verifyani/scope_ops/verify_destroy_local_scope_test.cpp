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

class DestroyLocalScopeTest : public VerifyAniTest {};

TEST_F(DestroyLocalScopeTest, wrong_env)
{
    ASSERT_EQ(env_->c_api->DestroyLocalScope(nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyLocalScope", testLines);
}

TEST_F(DestroyLocalScopeTest, has_unhandled_error)
{
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);
    ThrowError();

    ASSERT_EQ(env_->c_api->DestroyLocalScope(env_), ANI_ERROR);

    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyLocalScope", testLines);
}

TEST_F(DestroyLocalScopeTest, success)
{
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);

    ASSERT_EQ(env_->c_api->DestroyLocalScope(env_), ANI_OK);
}

TEST_F(DestroyLocalScopeTest, call_without_scope_and_wrong_env)
{
    ASSERT_EQ(env_->c_api->DestroyLocalScope(nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DestroyLocalScope", testLines);
}

TEST_F(DestroyLocalScopeTest, call_without_scope)
{
    ASSERT_EQ(env_->c_api->DestroyLocalScope(env_), ANI_ERROR);
    ASSERT_ERROR_ANI_MSG("DestroyLocalScope",
                         "Illegal call DestroyLocalScope() without first calling CreateLocalScope()");
}

TEST_F(DestroyLocalScopeTest, call_after_call_CreateEscapeLocalScope)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(1), ANI_OK);

    ASSERT_EQ(env_->c_api->DestroyLocalScope(env_), ANI_ERROR);
    ASSERT_ERROR_ANI_MSG("DestroyLocalScope",
                         "Illegal call DestroyLocalScope(), after calling CreateEscapeLocalScope()");

    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->DestroyEscapeLocalScope(ref, &ref), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
