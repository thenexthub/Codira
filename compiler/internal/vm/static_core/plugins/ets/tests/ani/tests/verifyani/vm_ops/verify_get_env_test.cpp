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

class GetEnvTest : public VerifyAniTest {};

TEST_F(GetEnvTest, wrong_vm)
{
    ani_env *env {};
    ASSERT_EQ(vm_->c_api->GetEnv(nullptr, ANI_VERSION_1, &env), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *", "wrong VM pointer"},
        {"version", "uint32_t"},
        {"result", "ani_env **"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetEnv", testLines);
}

TEST_F(GetEnvTest, wrong_version)
{
    constexpr uint32_t FAKE_ANI_VERSION = 0xfe3d;
    ani_env *env {};
    ASSERT_EQ(vm_->c_api->GetEnv(vm_, FAKE_ANI_VERSION, &env), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *"},
        {"version", "uint32_t", "unsupported ANI version"},
        {"result", "ani_env **"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetEnv", testLines);
}

TEST_F(GetEnvTest, wrong_result_ptr)
{
    ASSERT_EQ(vm_->c_api->GetEnv(vm_, ANI_VERSION_1, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *"},
        {"version", "uint32_t"},
        {"result", "ani_env **", "wrong pointer for storing 'ani_env *'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetEnv", testLines);
}

TEST_F(GetEnvTest, wrong_all_args)
{
    constexpr uint32_t FAKE_ANI_VERSION = 0xdeadbeaf;
    ASSERT_EQ(vm_->c_api->GetEnv(nullptr, FAKE_ANI_VERSION, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *", "wrong VM pointer"},
        {"version", "uint32_t", "unsupported ANI version"},
        {"result", "ani_env **", "wrong pointer for storing 'ani_env *'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GetEnv", testLines);
}

TEST_F(GetEnvTest, success)
{
    ani_env *env {};
    ASSERT_EQ(vm_->c_api->GetEnv(vm_, ANI_VERSION_1, &env), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
