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

#ifndef PLUGINS_ETS_TESTS_ETS_SDK_FUZZTEST_COMMON_TEST_COMMON_H
#define PLUGINS_ETS_TESTS_ETS_SDK_FUZZTEST_COMMON_TEST_COMMON_H

#include "ani.h"

#include <string>
#include <vector>
#include <iostream>

#define MAX_INPUT_SIZE 256

class FuzzTestEngine {
public:
    FuzzTestEngine()
    {
        std::string bootFileString = BuildOptionString();
        ani_option bootFileOption = {bootFileString.data(), nullptr};
        std::vector<ani_option> options = {bootFileOption};
        // Add extra test-special ANI options
        std::vector<ani_option> extraOptions = GetExtraAniOptions();
        for (auto &opt : extraOptions) {
            options.push_back(opt);
        }

        ani_options optionsPtr = {options.size(), options.data()};
        if (ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm_) != ANI_OK) {
            std::cerr << "Failed to create ani_vm" << std::endl;
        }

        if (vm_->GetEnv(ANI_VERSION_1, &env_) != ANI_OK) {
            std::cerr << "Failed to get ani_env" << std::endl;
        }
    }

    virtual ~FuzzTestEngine()
    {
        env_ = nullptr;
        if (vm_ != nullptr) {
            vm_->DestroyVM();
            vm_ = nullptr;
        }
    }

protected:
    virtual std::vector<ani_option> GetExtraAniOptions()
    {
        return {};
    }

    std::string BuildOptionString()
    {
        const char *stdlibPath = "/system/etc/etsstdlib.abc";
        std::string bootFileString("--ext:boot-panda-files=");
        bootFileString += stdlibPath;

        const char *abcPath = std::getenv("ANI_GTEST_ABC_PATH");
        if (abcPath != nullptr) {
            bootFileString += ":";
            bootFileString += abcPath;
        }

        return bootFileString;
    }

    ani_vm *vm_ {nullptr};
    ani_env *env_ {nullptr};
};

#endif  // PLUGINS_ETS_TESTS_ETS_SDK_FUZZTEST_COMMON_TEST_COMMON_H
