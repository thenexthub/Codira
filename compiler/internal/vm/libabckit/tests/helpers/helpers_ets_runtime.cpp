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

#include "helpers_runtime.h"
#include "libabckit/src/logger.h"
#include "static_core/plugins/ets/runtime/ani/ani.h"
#include "mem_manager/mem_manager.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifndef ABCKIT_ETS_STD_LIB
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ABCKIT_ETS_STD_LIB ""
#endif

namespace libabckit::test::helpers {

// CC-OFFNXT(huge_method[C++], G.FUD.05) solid logic (remove this line after #28140 fix)
std::string ExecuteStaticAbc(const std::string &abcPath, const std::string &moduleName, const std::string &fnName)
{
    LIBABCKIT_LOG(DEBUG) << "ExecuteStaticAbc: " << abcPath << ' ' << moduleName << ' ' << fnName << '\n';

    ani_env *env {nullptr};
    ani_vm *vm {nullptr};

    // Create boot-panda-files options
    std::string bootFileString = "--ext:boot-panda-files=" + std::string(ABCKIT_ETS_STD_LIB);
    if (!abcPath.empty()) {
        bootFileString += ":";
        bootFileString += abcPath;
    }

    // clang-format off
    std::vector<ani_option> optionsVector{
        {bootFileString.data(), nullptr},
        {"--ext:gc-trigger-type=heap-trigger", nullptr},
        {"--ext:compiler-enable-jit=false", nullptr},
        {"--ext:interpreter-type=cpp", nullptr}
    };
    // clang-format on
    ani_options optionsPtr = {optionsVector.size(), optionsVector.data()};

    MemManager::Finalize();
    auto status = ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm);
    if (status != ANI_OK) {
        LIBABCKIT_LOG(FATAL) << "Cannot create ETS VM\n";
    }
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        LIBABCKIT_LOG(FATAL) << "Cannot get ani env\n";
    }

    std::stringstream ss;
    ani_module module {};
    status = env->FindModule(moduleName.c_str(), &module);
    if (status == ANI_OK) {
        ani_function fn {};
        status = env->Module_FindFunction(module, fnName.c_str(), nullptr, &fn);
        if (status != ANI_OK) {
            LIBABCKIT_LOG(FATAL) << "No method " << fnName << " in " << moduleName << '\n';
        }

        CoutRedirect coutRedirect(ss.rdbuf());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        auto status = env->Function_Call_Void(fn, nullptr);
        if (status != ANI_OK) {
            LIBABCKIT_LOG(FATAL) << "Calling " << fnName << " failed\n";
        }
    } else {
        // NOTE(srokashevich, #28140): after fix replace this branch with
        // LIBABCKIT_LOG(FATAL) << "No module " << moduleName << " in " << abcPath << '\n';
        ani_class klass {};
        auto klassName = moduleName + ".ETSGLOBAL";
        status = env->FindClass(klassName.c_str(), &klass);
        if (status != ANI_OK) {
            LIBABCKIT_LOG(FATAL) << "No class " << klassName << " in " << abcPath << '\n';
        }

        ani_static_method method {};
        status = env->Class_FindStaticMethod(klass, fnName.c_str(), nullptr, &method);
        if (status != ANI_OK) {
            LIBABCKIT_LOG(FATAL) << "No method " << fnName << " in " << klassName << '\n';
        }

        CoutRedirect coutRedirect(ss.rdbuf());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        auto status = env->Class_CallStaticMethod_Void(klass, method, nullptr);
        if (status != ANI_OK) {
            LIBABCKIT_LOG(FATAL) << "Calling " << fnName << " failed\n";
        }
    }

    vm->DestroyVM();

    LIBABCKIT_LOG(DEBUG) << "Output:\n" << ss.str() << '\n';

    return ss.str();
}

}  // namespace libabckit::test::helpers
