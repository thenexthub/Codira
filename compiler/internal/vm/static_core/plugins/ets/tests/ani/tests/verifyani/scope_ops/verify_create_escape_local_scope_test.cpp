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

class CreateEscapeLocalScopeTest : public VerifyAniTest {};

TEST_F(CreateEscapeLocalScopeTest, wrong_env)
{
    ASSERT_EQ(env_->c_api->CreateEscapeLocalScope(nullptr, 1), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"nr_refs", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("CreateEscapeLocalScope", testLines);
}

TEST_F(CreateEscapeLocalScopeTest, wrong_nr_refs_0)
{
    ASSERT_EQ(env_->c_api->CreateEscapeLocalScope(env_, 0), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"nr_refs", "ani_size", "wrong value"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("CreateEscapeLocalScope", testLines);
}

TEST_F(CreateEscapeLocalScopeTest, wrong_nr_refs_1)
{
    ASSERT_EQ(env_->c_api->CreateEscapeLocalScope(env_, ani_size(std::numeric_limits<uint16_t>::max()) + 1), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"nr_refs", "ani_size", "it is too big"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("CreateEscapeLocalScope", testLines);
}

TEST_F(CreateEscapeLocalScopeTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->CreateEscapeLocalScope(nullptr, 0), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"nr_refs", "ani_size", "wrong value"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("CreateEscapeLocalScope", testLines);
}

TEST_F(CreateEscapeLocalScopeTest, success)
{
    ASSERT_EQ(env_->c_api->CreateEscapeLocalScope(env_, 1), ANI_OK);
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->DestroyEscapeLocalScope(ref, &ref), ANI_OK);
}

TEST_F(CreateEscapeLocalScopeTest, call_in_LocalScope)
{
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);

    ASSERT_EQ(env_->c_api->CreateEscapeLocalScope(env_, 1), ANI_OK);
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->DestroyEscapeLocalScope(ref, &ref), ANI_OK);

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(CreateEscapeLocalScopeTest, call_in_EscapeLocalScope)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(1), ANI_OK);

    ASSERT_EQ(env_->c_api->CreateEscapeLocalScope(env_, 1), ANI_OK);
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_EQ(env_->DestroyEscapeLocalScope(ref, &ref), ANI_OK);

    ASSERT_EQ(env_->DestroyEscapeLocalScope(ref, &ref), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
