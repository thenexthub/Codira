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

#include <thread>
#include "plugins/ets/tests/ani/ani_gtest/verify_ani_gtest.h"

namespace ark::ets::ani::verify::testing {

class AttachCurrentThreadTest : public VerifyAniTest {};

TEST_F(AttachCurrentThreadTest, wrong_vm)
{
    ani_env *env {};
    ASSERT_EQ(vm_->c_api->AttachCurrentThread(nullptr, nullptr, ANI_VERSION_1, &env), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *", "wrong VM pointer"},
        {"options", "ani_options *"},
        {"version", "uint32_t"},
        {"result", "ani_env **"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("AttachCurrentThread", testLines);
}

TEST_F(AttachCurrentThreadTest, wrong_options_0)
{
    ani_options opts = {1, nullptr};
    ani_env *env {};
    ASSERT_EQ(vm_->c_api->AttachCurrentThread(vm_, &opts, ANI_VERSION_1, &env), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *"},
        {"options", "ani_options *", "wrong 'options' pointer, options->options == NULL"},
        {"version", "uint32_t"},
        {"result", "ani_env **"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("AttachCurrentThread", testLines);
}

TEST_F(AttachCurrentThreadTest, wrong_options_1)
{
    std::array optsArray = {
        ani_option {"--ext:gc-type=g1-gc", nullptr},
        ani_option {nullptr, nullptr},
    };
    ani_options opts = {optsArray.size(), optsArray.data()};
    ani_env *env {};
    ASSERT_EQ(vm_->c_api->AttachCurrentThread(vm_, &opts, ANI_VERSION_1, &env), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *"},
        {"options", "ani_options *", "wrong 'option' pointer, options->options[1].option == NULL"},
        {"version", "uint32_t"},
        {"result", "ani_env **"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("AttachCurrentThread", testLines);
}

TEST_F(AttachCurrentThreadTest, wrong_options_2)
{
    std::array optsArray = {
        ani_option {"--ext:gc-type=g1-gc", nullptr},
    };
    const size_t sz = 8345;
    ani_options opts = {sz, optsArray.data()};
    ani_env *env {};
    ASSERT_EQ(vm_->c_api->AttachCurrentThread(vm_, &opts, ANI_VERSION_1, &env), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *"},
        {"options", "ani_options *", "'nr_options' value is too large. options->nr_options == 8345"},
        {"version", "uint32_t"},
        {"result", "ani_env **"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("AttachCurrentThread", testLines);
}

TEST_F(AttachCurrentThreadTest, wrong_options_3)
{
    std::array optsArray = {
        ani_option {"--ext:gc-type=g1-gc", nullptr},
        ani_option {"--ext:--log-level=info", nullptr},
    };
    ani_options opts = {optsArray.size(), optsArray.data()};
    ani_env *env {};
    ASSERT_EQ(vm_->c_api->AttachCurrentThread(vm_, &opts, ANI_VERSION_1, &env), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *"},
        {"options", "ani_options *", "wrong 'option' value, options->options[1].option == --ext:--log-level=info"},
        {"version", "uint32_t"},
        {"result", "ani_env **"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("AttachCurrentThread", testLines);
}

TEST_F(AttachCurrentThreadTest, wrong_version)
{
    const uint32_t fakeANIVersion = 0xfe3d;
    ani_env *env {};
    ASSERT_EQ(vm_->c_api->AttachCurrentThread(vm_, nullptr, fakeANIVersion, &env), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *"},
        {"options", "ani_options *"},
        {"version", "uint32_t", "unsupported ANI version"},
        {"result", "ani_env **"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("AttachCurrentThread", testLines);
}

TEST_F(AttachCurrentThreadTest, wrong_result_ptr)
{
    ASSERT_EQ(vm_->c_api->AttachCurrentThread(vm_, nullptr, ANI_VERSION_1, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *"},
        {"options", "ani_options *"},
        {"version", "uint32_t"},
        {"result", "ani_env **", "wrong pointer for storing 'ani_env *'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("AttachCurrentThread", testLines);
}

TEST_F(AttachCurrentThreadTest, invalid_all_args)
{
    const uint32_t fakeANIVersion = 0xdeadbeaf;
    ani_options opts = {1, nullptr};
    ASSERT_EQ(vm_->c_api->AttachCurrentThread(nullptr, &opts, fakeANIVersion, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"vm", "ani_vm *", "wrong VM pointer"},
        {"options", "ani_options *", "wrong 'options' pointer, options->options == NULL"},
        {"version", "uint32_t", "unsupported ANI version"},
        {"result", "ani_env **", "wrong pointer for storing 'ani_env *'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("AttachCurrentThread", testLines);
}

TEST_F(AttachCurrentThreadTest, success)
{
    std::thread([this]() {
        ani_env *env {};
        ASSERT_EQ(vm_->c_api->AttachCurrentThread(vm_, nullptr, ANI_VERSION_1, &env), ANI_OK);
        ASSERT_NO_ABORT_MESSAGE();
        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();
}

}  // namespace ark::ets::ani::verify::testing
