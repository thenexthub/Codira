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

#include <ani.h>
#include <iostream>
#include <array>
#include <thread>

static ani_ref BuildData(ani_env *env, [[maybe_unused]] ani_object object)
{
    ani_ref undefined = nullptr;
    [[maybe_unused]] ani_status status = env->GetUndefined(&undefined);

    ani_array array = nullptr;
    const ani_size size = 4;
    status = env->Array_New(size, undefined, &array);
    if (status != ANI_OK) {
        std::cerr << "Failed to create array" << std::endl;
        return undefined;
    }
    for (ani_size i = 0; i < size; i++) {
        ani_string item = nullptr;
        std::string itemStr = "item" + std::to_string(i);
        status = env->String_NewUTF8(itemStr.c_str(), itemStr.size(), &item);
        if (status != ANI_OK) {
            std::cerr << "Failed to create string" << std::endl;
            return undefined;
        }
        status = env->Array_Set(array, i, item);
        if (status != ANI_OK) {
            std::cerr << "Failed to set array item at index " << i << std::endl;
            return undefined;
        }
    }
    return array;
}

static void Apply(ani_env *env, [[maybe_unused]] ani_object object, ani_array data, ani_fn_object fn)
{
    ani_size size = 0;
    ani_status status = env->Array_GetLength(data, &size);
    if (status != ANI_OK) {
        std::cerr << "Failed to get array length" << std::endl;
        return;
    }

    ani_vm *vm;
    if (ANI_OK != env->GetVM(&vm)) {
        std::cerr << "Cannot get VM" << std::endl;
        return;
    }
    std::thread workerThread([vm, data, fn, size]() {
        ani_env *threadEnv;
        if (ANI_OK != vm->AttachCurrentThread(nullptr, ANI_VERSION_1, &threadEnv)) {
            std::cerr << "Cannot attach current thread" << std::endl;
            return;
        }
        for (ani_size i = 0; i < size; i++) {
            ani_ref item = nullptr;
            ani_status threadStatus = threadEnv->Array_Get(data, i, &item);
            if (threadStatus != ANI_OK) {
                std::cerr << "Failed to get array item at index " << i << std::endl;
                return;
            }
            ani_ref unused = nullptr;
            threadStatus = threadEnv->FunctionalObject_Call(fn, 1, &item, &unused);
            if (threadStatus != ANI_OK) {
                std::cerr << "Failed to call function: " << threadStatus << std::endl;
                return;
            }
        }
        if (ANI_OK != vm->DetachCurrentThread()) {
            std::cerr << "Cannot detach current thread" << std::endl;
            return;
        }
    });

    workerThread.join();
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    static const char *className = "plugin.Plugin";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        std::cerr << "Not found '" << className << "'" << std::endl;
        return ANI_ERROR;
    }

    std::array methods = {
        ani_native_function {"BuildData", ":C{std.core.Array}", reinterpret_cast<void *>(BuildData)},
        ani_native_function {"Apply", "C{std.core.Array}C{std.core.Function1}:", reinterpret_cast<void *>(Apply)},
    };

    ani_status status = env->Class_BindNativeMethods(cls, methods.data(), methods.size());
    if (status != ANI_OK) {
        std::cerr << "Cannot bind native methods to '" << className << "'"
                  << ":" << status << std::endl;
        return ANI_ERROR;
    };
    *result = ANI_VERSION_1;
    return ANI_OK;
}

ANI_EXPORT ani_status ANI_Destructor([[maybe_unused]] ani_vm *vm)
{
    return ANI_OK;
}
