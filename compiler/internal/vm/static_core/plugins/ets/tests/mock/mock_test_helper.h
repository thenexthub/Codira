/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_TESTS_MOCK_MOCK_TEST_HELPER_H
#define PANDA_PLUGINS_ETS_TESTS_MOCK_MOCK_TEST_HELPER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "plugins/ets/runtime/ani/ani.h"

namespace ark::ets::test {

class MockEtsNapiTestBaseClass : public testing::Test {
protected:
    MockEtsNapiTestBaseClass() = default;
    explicit MockEtsNapiTestBaseClass(const char *fileName) : testBinFileName_(fileName) {};

    static void SetUpTestCase()
    {
        if (std::getenv("PANDA_STD_LIB") == nullptr) {
            std::cerr << "PANDA_STD_LIB not set" << std::endl;
            std::abort();
        }
    }

    void SetUp() override
    {
        const char *stdlib = std::getenv("PANDA_STD_LIB");
        ASSERT_NE(stdlib, nullptr);

        // Create boot-panda-files options
        std::string bootFileString = "--ext:boot-panda-files=" + std::string(stdlib);

        // clang-format off
        std::vector<ani_option> optionsVector{
            {bootFileString.data(), nullptr},
            {"--ext:gc-type=epsilon", nullptr},
            {"--ext:compiler-enable-jit=false", nullptr},
        };
        // clang-format on
        ani_options optionsPtr = {optionsVector.size(), optionsVector.data()};

        ASSERT_EQ(ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm_), ANI_OK) << "Cannot create ETS VM";
        ASSERT_EQ(vm_->GetEnv(ANI_VERSION_1, &env_), ANI_OK) << "Cannot get ani env";
    }

    void TearDown() override
    {
        ASSERT_EQ(vm_->DestroyVM(), ANI_OK) << "Cannot destroy ETS VM";
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    const char *testBinFileName_ {nullptr};
    ani_env *env_ {nullptr};
    ani_vm *vm_ {nullptr};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

}  // namespace ark::ets::test

#endif  // PANDA_PLUGINS_ETS_TESTS_MOCK_MOCK_TEST_HELPER_H
