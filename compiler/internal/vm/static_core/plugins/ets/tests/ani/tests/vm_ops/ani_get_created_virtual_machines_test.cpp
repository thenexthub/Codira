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
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
static ani_int Sum([[maybe_unused]] ani_env *env, ani_int a, ani_int b)
{
    return a + b;
}

class GetCreatedVMsTest : public ::testing::Test {
public:
    std::string BuildOptionString()
    {
        const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
        if (stdlib == nullptr) {
            return "";
        }
        const char *abcPath = std::getenv("ANI_GTEST_ABC_PATH");
        std::string bootFileString = "--ext:--boot-panda-files=" + std::string(stdlib);

        if (abcPath != nullptr) {
            bootFileString += ":" + std::string(abcPath);
        }
        return bootFileString;
    }
};

TEST_F(GetCreatedVMsTest, getCreatedVMs)
{
    ani_vm *vm = nullptr;
    std::string bootFileString = BuildOptionString();
    ani_option bootFileOption = {bootFileString.data(), nullptr};
    std::vector<ani_option> options;
    options.push_back(bootFileOption);
    ani_options optionsPtr = {options.size(), options.data()};
    ASSERT_EQ(ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm), ANI_OK);

    vm = nullptr;
    ani_size size = 0;
    ani_size bufferSize = 1;
    ASSERT_EQ(ANI_GetCreatedVMs(nullptr, bufferSize, &size), ANI_INVALID_ARGS);
    ASSERT_EQ(ANI_GetCreatedVMs(&vm, 0, &size), ANI_INVALID_ARGS);
    ASSERT_EQ(ANI_GetCreatedVMs(&vm, bufferSize, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(ANI_GetCreatedVMs(&vm, bufferSize, &size), ANI_OK);
    ASSERT_NE(vm, nullptr);
    ASSERT_EQ(size, bufferSize);
    ASSERT_EQ(vm->DestroyVM(), ANI_OK) << "Cannot destroy ANI VM";
    ASSERT_EQ(ANI_GetCreatedVMs(&vm, bufferSize, &size), ANI_OK);
    ASSERT_EQ(size, 0);
}

TEST_F(GetCreatedVMsTest, combination)
{
    ani_vm *vm = nullptr;
    std::string bootFileString = BuildOptionString();
    ani_option bootFileOption = {bootFileString.data(), nullptr};
    std::vector<ani_option> options;
    options.push_back(bootFileOption);
    ani_options optionsPtr = {options.size(), options.data()};
    ASSERT_EQ(ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm), ANI_OK);
    ani_size size = 0;
    ani_size bufferSize = 1;
    ASSERT_EQ(ANI_GetCreatedVMs(&vm, bufferSize, &size), ANI_OK);
    ani_env *env = nullptr;
    ASSERT_EQ(vm->GetEnv(ANI_VERSION_1, &env), ANI_OK);
    ASSERT_NE(env, nullptr);
    ASSERT_EQ(env->GetVM(&vm), ANI_OK);
    ani_namespace ns;
    ASSERT_EQ(env->FindNamespace("ani_get_created_virtual_machines_test.ops", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
    std::array functions = {
        ani_native_function {"sum", "ii:i", reinterpret_cast<void *>(Sum)},
    };
    ASSERT_EQ(env->Namespace_BindNativeFunctions(ns, functions.data(), functions.size()), ANI_OK);

    ani_function fn {};
    ASSERT_EQ(env->Namespace_FindFunction(ns, "checkSum", ":z", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
    ani_boolean value = 0U;
    ASSERT_EQ(env->Function_Call_Boolean(fn, &value), ANI_OK);
    ASSERT_EQ(value, true);
    ASSERT_EQ(vm->DestroyVM(), ANI_OK);
    vm = nullptr;
    ASSERT_EQ(ANI_GetCreatedVMs(&vm, bufferSize, &size), ANI_OK);
    ASSERT_EQ(vm, nullptr);
}  // NOLINTEND(cppcoreguidelines-pro-type-vararg)
}  // namespace ark::ets::ani::testing
