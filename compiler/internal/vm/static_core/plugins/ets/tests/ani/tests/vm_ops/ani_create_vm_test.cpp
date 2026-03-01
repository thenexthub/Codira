/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class CreateVMTest : public ::testing::Test {
public:
    static std::string GetBootPandaFilesOption()
    {
        const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
        if (stdlib == nullptr) {
            std::cerr << "ARK_ETS_STDLIB_PATH=" << stdlib << std::endl;
            std::abort();
        }
        return "--ext:boot-panda-files=" + std::string(stdlib);
    }

    static ani_options GetOptions()
    {
        static std::string bootFileString = GetBootPandaFilesOption();
        static std::array arrayOpts = {
            ani_option {bootFileString.c_str(), nullptr},
        };
        return {arrayOpts.size(), arrayOpts.data()};
    }
};

TEST_F(CreateVMTest, create_vm_and_destroy_vm)
{
    ani_options options = GetOptions();
    ani_vm *vm = nullptr;
    ASSERT_EQ(ANI_CreateVM(&options, ANI_VERSION_1, &vm), ANI_OK);
    ASSERT_EQ(vm->DestroyVM(), ANI_OK);
}

// NOTE: Enable when #25347 is resolved
TEST_F(CreateVMTest, DISABLED_create_vm_secod_attempt)
{
    ani_options options = GetOptions();
    ani_vm *vm = nullptr;
    ASSERT_EQ(ANI_CreateVM(nullptr, ANI_VERSION_1, &vm), ANI_ERROR);
    ASSERT_EQ(ANI_CreateVM(&options, ANI_VERSION_1, &vm), ANI_OK);
    ASSERT_EQ(vm->DestroyVM(), ANI_OK);
}

TEST_F(CreateVMTest, create_vm_with_invalid_args)
{
    ani_options options = GetOptions();
    ani_vm *vm = nullptr;
    const int32_t versionNew = 2;
    const int32_t versionOld = 0;
    ASSERT_EQ(ANI_CreateVM(&options, versionNew, &vm), ANI_INVALID_VERSION);
    ASSERT_EQ(ANI_CreateVM(&options, versionOld, &vm), ANI_INVALID_VERSION);
    ASSERT_EQ(ANI_CreateVM(&options, ANI_VERSION_1, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CreateVMTest, destroy_vm_with_invalid_args)
{
    ani_options options = GetOptions();
    ani_vm *vm = nullptr;
    ASSERT_EQ(ANI_CreateVM(&options, ANI_VERSION_1, &vm), ANI_OK);
    ASSERT_EQ(vm->c_api->DestroyVM(nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(vm->DestroyVM(), ANI_OK);
}

}  // namespace ark::ets::ani::testing
