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

class DescribeErrorTest : public VerifyAniTest {};

TEST_F(DescribeErrorTest, wrong_env)
{
    ASSERT_EQ(env_->c_api->DescribeError(nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("DescribeError", testLines);
}

// NOTE: enable describe error tests #29858
TEST_F(DescribeErrorTest, DISABLED_throw_error)
{
    ThrowError();
    ASSERT_EQ(env_->c_api->DescribeError(env_), ANI_OK);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

// NOTE: enable describe error tests #29858
TEST_F(DescribeErrorTest, DISABLED_no_error)
{
    ASSERT_EQ(env_->c_api->DescribeError(env_), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
