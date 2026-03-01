/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

class EnsureEnoughRef : public VerifyAniTest {};

TEST_F(EnsureEnoughRef, wrong_env)
{
    ani_size cnt {};
    ASSERT_EQ(env_->c_api->EnsureEnoughReferences(nullptr, cnt), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"nrRefs", "ani_size"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnsureEnoughReferences", testLines);
}

TEST_F(EnsureEnoughRef, success)
{
    ani_size cnt = 5U;
    ASSERT_EQ(env_->c_api->EnsureEnoughReferences(env_, cnt), ANI_OK);
}

TEST_F(EnsureEnoughRef, throw_error)
{
    ThrowError();
    ani_size cnt = 5U;
    ASSERT_EQ(env_->c_api->EnsureEnoughReferences(env_, cnt), ANI_OK);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
