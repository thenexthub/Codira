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

#include <sstream>

#include "libarkbase/macros.h"
#include "libarkbase/os/system_environment.h"
#include "plugins/ets/runtime/ani/ani.h"

namespace {

std::vector<ani_string> SplitString(ani_env *env, std::string_view from, char delim)
{
    std::vector<ani_string> list;
    std::istringstream iss {from.data()};
    std::string part;
    while (std::getline(iss, part, delim)) {
        ani_string newString;
        env->String_NewUTF8(part.data(), part.size(), &newString);
        list.emplace_back(newString);
    }
    return list;
}

}  // namespace

extern "C" {
// NOLINTNEXTLINE(readability-identifier-naming)
ANI_EXPORT ani_array getAppAbcFiles(ani_env *env, [[maybe_unused]] ani_class /*unused*/)
{
    auto appAbcFiles = ark::os::system_environment::GetEnvironmentVar("APP_ABC_FILES");
    const auto paths = SplitString(env, appAbcFiles, ':');
    ASSERT(!paths.empty());

    ani_class stringClass;
    env->FindClass("std.core.String", &stringClass);
    ASSERT(stringClass != nullptr);
    ani_array pathsArray;
    env->Array_New(paths.size(), paths[0], &pathsArray);
    for (size_t i = 0; i < paths.size(); ++i) {
        env->Array_Set(pathsArray, i, paths[i]);
    }
    return pathsArray;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *version)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    ani_env *env;
    *version = ANI_VERSION_1;
    if (ANI_OK != vm->GetEnv(*version, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    ani_native_function function {"getAppAbcFiles", ":C{std.core.Array}", reinterpret_cast<void *>(getAppAbcFiles)};

    std::string_view mdl = "@spawn.spawn";
    ani_module spawnModule;
    if (ANI_OK != env->FindModule(mdl.data(), &spawnModule)) {
        std::cerr << "Not found '" << mdl << "'" << std::endl;
        return ANI_ERROR;
    }

    if (ANI_OK != env->Module_BindNativeFunctions(spawnModule, &function, 1)) {
        std::cerr << "Failed binds an array of methods to the specified class" << std::endl;
        return ANI_ERROR;
    }

    return ANI_OK;
}
}
