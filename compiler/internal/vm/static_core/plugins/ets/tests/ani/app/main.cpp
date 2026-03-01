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

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>

#include <ani.h>

bool CheckAniStatus(ani_status status, const char *message)
{
    if (status != ANI_OK) {
        std::cerr << message << ": " << status << std::endl;
        return true;
    }
    return false;
}
bool CheckAniError(ani_status status, const char *message, ani_env *env)
{
    if (status != ANI_OK) {
        if (status == ANI_PENDING_ERROR) {
            env->DescribeError();
        }
        std::cerr << message << ": " << status << std::endl;
        return true;
    }
    return false;
}
bool CheckAniCondition(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << message << std::endl;
        return true;
    }
    return false;
}

constexpr const char *APP_CLASS_NAME = "app.App";

std::vector<ani_string> SplitString(ani_env *env, const std::string &str, const char delim)
{
    std::vector<ani_string> result;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delim)) {
        ani_string newString;
        env->String_NewUTF8(item.data(), item.size(), &newString);
        result.emplace_back(newString);
    }
    return result;
}

std::vector<ani_string> GetAppAbcFiles(ani_env *env)
{
    const char *abcPath = std::getenv("APP_ABC_FILES");
    if (abcPath == nullptr) {
        return {};
    }

    return SplitString(env, abcPath, ':');
}

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
static ani_object CreateAbcRuntimeLinker(ani_env *env, ani_class abcRuntimeLinkerClass, ani_ref undefined)
{
    auto appAbcFiles = GetAppAbcFiles(env);
    if (CheckAniCondition(!appAbcFiles.empty(), "No app abc files found")) {
        return nullptr;
    }

    ani_class stringClass = nullptr;
    ani_status status = env->FindClass("std.core.String", &stringClass);
    if (CheckAniError(status, "Cannot find String class std.core.String", env)) {
        return nullptr;
    }

    ani_array appAbcFilesArray = nullptr;
    status = env->Array_New(appAbcFiles.size(), appAbcFiles[0], &appAbcFilesArray);
    if (CheckAniStatus(status, "Cannot create array of String")) {
        return nullptr;
    }

    for (size_t i = 0; i < appAbcFiles.size(); ++i) {
        status = env->Array_Set(appAbcFilesArray, i, appAbcFiles[i]);
        if (CheckAniStatus(status, "Cannot set String for arg")) {
            return nullptr;
        }
    }

    ani_method abcRuntimeLinkerCtor;
    status = env->Class_FindMethod(abcRuntimeLinkerClass, "<ctor>",
                                   "C{std.core.RuntimeLinker}C{std.core.Array}:", &abcRuntimeLinkerCtor);
    if (CheckAniStatus(status, "Cannot find abcRuntimeLinker constructor")) {
        return nullptr;
    }

    ani_object abcRuntimeLinkerObject = nullptr;
    status = env->Object_New(abcRuntimeLinkerClass, abcRuntimeLinkerCtor, &abcRuntimeLinkerObject, undefined,
                             appAbcFilesArray);
    if (CheckAniStatus(status, "Cannot create abcRuntimeLinker object")) {
        return nullptr;
    }
    return abcRuntimeLinkerObject;
}

static ani_object CreateBooleanObject(ani_env *env)
{
    ani_class booleanClass = nullptr;
    ani_status status = env->FindClass("std.core.Boolean", &booleanClass);
    if (CheckAniError(status, "Cannot find std.core.Boolean class", env)) {
        return nullptr;
    }

    ani_method booleanClassCtor;
    status = env->Class_FindMethod(booleanClass, "<ctor>", "z:", &booleanClassCtor);
    if (CheckAniStatus(status, "Cannot find booleanClass constructor")) {
        return nullptr;
    }

    ani_object booleanObject = nullptr;
    status = env->Object_New(booleanClass, booleanClassCtor, &booleanObject, ANI_FALSE);
    if (CheckAniStatus(status, "Cannot create boolean object")) {
        return nullptr;
    }
    return booleanObject;
}

static ani_object LoadMainAppClassInstance(ani_env *env, ani_class abcRuntimeLinkerClass,
                                           ani_object abcRuntimeLinkerObject)
{
    ani_method loadClassMethod = nullptr;
    ani_status status =
        env->Class_FindMethod(abcRuntimeLinkerClass, "loadClass",
                              "C{std.core.String}C{std.core.Boolean}:C{std.core.Class}", &loadClassMethod);
    if (CheckAniStatus(status, "Cannot find loadClass method")) {
        return nullptr;
    }

    ani_string appClassNameStr = nullptr;
    status = env->String_NewUTF8(APP_CLASS_NAME, strlen(APP_CLASS_NAME), &appClassNameStr);
    if (CheckAniStatus(status, "Cannot create String for app class name")) {
        return nullptr;
    }

    ani_object booleanObject = CreateBooleanObject(env);
    if (booleanObject == nullptr) {
        return nullptr;
    }

    ani_ref mainAppClassObj = nullptr;
    status = env->Object_CallMethod_Ref(abcRuntimeLinkerObject, loadClassMethod, &mainAppClassObj, appClassNameStr,
                                        booleanObject);
    if (CheckAniStatus(status, "Cannot create app class object")) {
        return nullptr;
    }

    const std::string stdClass = "std.core.Class";
    ani_class stdClassClass = nullptr;
    status = env->FindClass(stdClass.c_str(), &stdClassClass);
    if (CheckAniError(status, "Cannot find std.core.Class class", env)) {
        return nullptr;
    }

    ani_method createInstanceMethod;
    status = env->Class_FindMethod(stdClassClass, "createInstance", ":C{std.core.Object}", &createInstanceMethod);
    if (CheckAniStatus(status, "Cannot find createInstance method")) {
        return nullptr;
    }

    ani_ref mainAppObjectRef = nullptr;
    status =
        env->Object_CallMethod_Ref(static_cast<ani_object>(mainAppClassObj), createInstanceMethod, &mainAppObjectRef);
    if (CheckAniStatus(status, "Cannot create app object object")) {
        return nullptr;
    }
    return static_cast<ani_object>(mainAppObjectRef);
}

int RunApp(ani_vm *vm)
{
    ani_status status = ANI_OK;
    ani_env *env = nullptr;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (CheckAniStatus(status, "Cannot get env")) {
        return 1;
    }

    ani_ref undefined = nullptr;
    status = env->GetUndefined(&undefined);
    if (CheckAniStatus(status, "Cannot get undefined")) {
        return 1;
    }

    ani_class abcRuntimeLinkerClass = nullptr;
    status = env->FindClass("std.core.AbcRuntimeLinker", &abcRuntimeLinkerClass);
    if (CheckAniError(status, "Cannot find class std.core.AbcRuntimeLinker", env)) {
        return 1;
    }

    ani_object abcRuntimeLinkerObject = CreateAbcRuntimeLinker(env, abcRuntimeLinkerClass, undefined);
    if (abcRuntimeLinkerObject == nullptr) {
        return 1;
    }

    auto mainAppObjectObj = LoadMainAppClassInstance(env, abcRuntimeLinkerClass, abcRuntimeLinkerObject);
    if (mainAppObjectObj == nullptr) {
        return 1;
    }

    ani_type mainAppObjectType = nullptr;
    status = env->Object_GetType(mainAppObjectObj, &mainAppObjectType);
    if (CheckAniStatus(status, "Cannot get type of app object")) {
        return 1;
    }

    ani_method mainAppInvoke;
    status = env->Class_FindMethod(static_cast<ani_class>(mainAppObjectType), "invoke", ":", &mainAppInvoke);
    if (CheckAniStatus(status, "Cannot find invoke method")) {
        return 1;
    }

    status = env->Object_CallMethod_Void(mainAppObjectObj, mainAppInvoke);
    if (CheckAniError(status, "Cannot invoke app", env)) {
        return 1;
    }

    return 0;
}
// NOLINTEND(cppcoreguidelines-pro-type-vararg)

int main()
{
    const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
    if (CheckAniCondition(stdlib != nullptr, "Cannot get ARK_ETS_STDLIB_PATH")) {
        return 1;
    }

    std::string bootFileString = std::string("--ext:boot-panda-files=") + stdlib;
    const char *abcPath = std::getenv("ANI_GTEST_ABC_PATH");
    if (abcPath != nullptr) {
        bootFileString += ":";
        bootFileString += abcPath;
    }
    std::array optionsArray = {
        ani_option {bootFileString.c_str(), nullptr},
        ani_option {"--ext:verification-mode=ahead-of-time", nullptr},
        ani_option {"--ext:gc-type=g1-gc", nullptr},
    };
    ani_options options = {optionsArray.size(), optionsArray.data()};

    ani_vm *vm = nullptr;
    auto status = ANI_CreateVM(&options, ANI_VERSION_1, &vm);
    if (CheckAniStatus(status, "Cannot create VM")) {
        return 1;
    }

    return RunApp(vm);
}
