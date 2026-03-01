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

#include "test_util/execute_util.h"

#ifndef PANDA_TARGET_WINDOWS

#include <sstream>
#include <string>
#include <vector>

#include "libarkbase/mem/mem_config.h"
#include "libarkbase/mem/pool_manager.h"
#include "plugins/ets/runtime/ani/ani.h"

#include "logger.h"

#ifndef GUARD_ETS_STD_LIB
#define GUARD_ETS_STD_LIB ""
#endif

namespace ark::guard {
bool ExecuteUtil::isInitialized_ = false;

ani_options GenerateVmOptions(const std::string &abcPath)
{
    static std::string bootFileString; // CC-OFF(G.EXP.09) for test purposes
    bootFileString = "--ext:boot-panda-files=" + std::string(GUARD_ETS_STD_LIB);
    if (!abcPath.empty()) {
        bootFileString += ":";
        bootFileString += abcPath;
    }

    static std::vector<ani_option> optionsVector; // CC-OFF(G.EXP.09) for test purposes
    optionsVector = {{bootFileString.data(), nullptr},
                     {"--ext:gc-trigger-type=heap-trigger", nullptr},
                     {"--ext:compiler-enable-jit=false", nullptr},
                     {"--ext:interpreter-type=cpp", nullptr}};
    return {optionsVector.size(), optionsVector.data()};
}

bool ExecuteTargetFunction(ani_env *env, const std::string &moduleName, const std::string &fnName,
                           std::stringstream &ss)
{
    ani_module module {};
    auto status = env->FindModule(moduleName.c_str(), &module);
    if (status == ANI_OK) {
        ani_function fn {};
        status = env->Module_FindFunction(module, fnName.c_str(), nullptr, &fn);
        if (status != ANI_OK) {
            LOG_E << "Failed to find function " << fnName << " in module " << moduleName;
            return false;
        }

        CoutRedirect coutRedirect(ss.rdbuf());
        status = env->Function_Call_Void(fn, nullptr);
        if (status != ANI_OK) {
            LOG_E << "Failed to call function " << fnName << " in module " << moduleName;
            return false;
        }
        return true;
    }
    ani_class klass {};
    auto klassName = moduleName + ".ETSGLOBAL";
    status = env->FindClass(klassName.c_str(), &klass);
    if (status != ANI_OK) {
        LOG_E << "Failed to find class " << klassName << " in module " << moduleName;
        return false;
    }

    ani_static_method method {};
    status = env->Class_FindStaticMethod(klass, fnName.c_str(), nullptr, &method);
    if (status != ANI_OK) {
        LOG_E << "Failed to find static method " << fnName << " in class " << klassName << " in module " << moduleName;
        return false;
    }

    CoutRedirect coutRedirect(ss.rdbuf());
    status = env->Class_CallStaticMethod_Void(klass, method, nullptr);
    if (status != ANI_OK) {
        LOG_E << "Failed to call static method " << fnName << " in class " << klassName << " in module " << moduleName;
        return false;
    }
    return true;
}
}  // namespace ark::guard

void ark::guard::ExecuteUtil::InitEnvironment()
{
    if (isInitialized_) {
        return;
    }
    PoolManager::Finalize();
    mem::MemConfig::Finalize();
    isInitialized_ = true;
}

std::string ark::guard::ExecuteUtil::ExecuteStaticAbc(const std::string &abcPath, const std::string &moduleName,
                                                      const std::string &fnName)
{
    LOG_D << "ExecuteStaticAbc: " << abcPath << ' ' << moduleName << ' ' << fnName;
    ani_options optionsPtr = GenerateVmOptions(abcPath);
    InitEnvironment();

    ani_env *env {nullptr};
    ani_vm *vm {nullptr};
    auto status = ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm);
    if (status != ANI_OK) {
        LOG_E << "Failed to create vm";
        return "";
    }
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        LOG_E << "Failed to get env";
        return "";
    }

    std::stringstream ss;
    ExecuteTargetFunction(env, moduleName, fnName, ss);

    vm->DestroyVM();
    LOG_D << "Output:\n" << ss.str();
    return ss.str();
}

#endif