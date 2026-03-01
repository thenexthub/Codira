/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <array>
#include <iostream>
#include <vector>

#include "plugins/ets/runtime/ani/ani.h"
#include "libarkbase/os/file.h"

static ani_fixedarray_byte LoadFile(ani_env *env, [[maybe_unused]] ani_class cls, ani_string path)
{
    ani_size pathLength = 0;
    [[maybe_unused]] auto status = env->String_GetUTF8Size(path, &pathLength);
    ASSERT(status == ANI_OK);
    ASSERT(pathLength != 0);
    std::string pathToFile(pathLength, 0);

    ani_size writtenSize = 0;
    status = env->String_GetUTF8SubString(path, 0, pathLength, pathToFile.data(), pathLength + 1, &writtenSize);
    ASSERT(status == ANI_OK);
    ASSERT(writtenSize == pathLength);

    auto undefinedFactory = [env, pathToFile = std::as_const(pathToFile)](std::string_view message) {
        std::cerr << message << " '" << pathToFile << "'";
        ani_ref undefined;
        [[maybe_unused]] auto s = env->GetUndefined(&undefined);
        ASSERT(s == ANI_OK);
        return static_cast<ani_fixedarray_byte>(undefined);
    };

    auto file = ark::os::file::Open(pathToFile, ark::os::file::Mode::READONLY);
    if (!file.IsValid()) {
        return undefinedFactory("Failed to open file");
    }
    ark::os::file::FileHolder fhHolder(file);

    auto res = file.GetFileSize();
    if (!res) {
        return undefinedFactory("Failed to get size of file");
    }

    std::vector<ani_byte> buffer(res.Value());
    if (!file.ReadAll(buffer.data(), buffer.size())) {
        return undefinedFactory("Failed to read file");
    }

    ani_fixedarray_byte rawFile;
    status = env->FixedArray_New_Byte(buffer.size(), &rawFile);
    ASSERT(status == ANI_OK);
    status = env->FixedArray_SetRegion_Byte(rawFile, 0, buffer.size(), buffer.data());
    ASSERT(status == ANI_OK);
    return rawFile;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    static constexpr const char *CLASS_DESCRIPTOR = "runtime_linker_extensions.Test";
    ani_class cls;
    if (ANI_OK != env->FindClass(CLASS_DESCRIPTOR, &cls)) {
        std::cerr << "Not found '" << CLASS_DESCRIPTOR << "'" << std::endl;
        return ANI_ERROR;
    }

    ani_native_function method {"loadFile", "C{std.core.String}:A{b}", reinterpret_cast<void *>(LoadFile)};
    if (ANI_OK != env->Class_BindStaticNativeMethods(cls, &method, 1)) {
        std::cerr << "Cannot bind native methods to '" << CLASS_DESCRIPTOR << "'" << std::endl;
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}
