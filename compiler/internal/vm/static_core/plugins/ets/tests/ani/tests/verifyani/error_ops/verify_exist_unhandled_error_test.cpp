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

class ExistUnhandledErrorTest : public VerifyAniTest {};

TEST_F(ExistUnhandledErrorTest, wrong_env)
{
    ani_boolean res {};
    ASSERT_EQ(env_->c_api->ExistUnhandledError(nullptr, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ExistUnhandledError", testLines);
}

TEST_F(ExistUnhandledErrorTest, wrong_res)
{
    ASSERT_EQ(env_->c_api->ExistUnhandledError(env_, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"result", "ani_boolean *", "wrong pointer for storing 'ani_boolean'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("ExistUnhandledError", testLines);
}

TEST_F(ExistUnhandledErrorTest, throw_error)
{
    ani_boolean res = ANI_FALSE;
    ThrowError();
    ASSERT_EQ(env_->c_api->ExistUnhandledError(env_, &res), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(ExistUnhandledErrorTest, no_error)
{
    ani_boolean res = ANI_TRUE;
    ASSERT_EQ(env_->c_api->ExistUnhandledError(env_, &res), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

}  // namespace ark::ets::ani::verify::testing
