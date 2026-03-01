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

#include "ets_interop_js_gtest.h"

#include <gtest/gtest.h>
namespace ark::ets::interop::js::testing {

std::string GetPathFromPlugin(const std::string &path)
{
    size_t pluginPos = path.find("/plugin");
    if (pluginPos != std::string::npos) {
        return path.substr(pluginPos);
    }
    return std::string();
}

std::string GetABCPath()
{
    const char *abcSource = std::getenv("ARK_ETS_INTEROP_JS_GTEST_SOURCES");
    const char *targetPackageName = std::getenv("ARK_ETS_INTEROP_JS_TARGET_GTEST_PACKAGE");
    const char *binPath = std::getenv("OLDPWD");

    if (abcSource == nullptr || targetPackageName == nullptr || binPath == nullptr) {
        return std::string();
    }

    std::string abcPath = binPath + GetPathFromPlugin(abcSource) + "/" + targetPackageName + "/src/classes.abc";

    return abcPath;
}

void SetRuntimeLinker(ani_env *env, const std::string &modulePath)
{
    ani_ref undefinedRef {};
    env->GetUndefined(&undefinedRef);

    ani_string aniStr {};
    env->String_NewUTF8(modulePath.c_str(), modulePath.size(), &aniStr);

    ani_array refArray;
    env->Array_New(1, undefinedRef, &refArray);
    env->Array_Set(refArray, 0, aniStr);

    std::string clsDescriptor = "std.core.AbcRuntimeLinker";
    ani_class cls {};
    env->FindClass(clsDescriptor.c_str(), &cls);

    ani_method ctor {};
    env->Class_FindMethod(cls, "<ctor>", "C{std.core.RuntimeLinker}C{std.core.Array}:", &ctor);

    ani_object obj {};
    // NOLINTNEXTLINE
    env->Object_New(cls, ctor, &obj, undefinedRef, refArray);

    ani_class contextCls {};
    env->FindClass("std.interop.InteropContext", &contextCls);

    // NOLINTNEXTLINE
    env->Class_CallStaticMethodByName_Void(contextCls, "setDefaultInteropLinker", "C{std.core.RuntimeLinker}:", obj);
}

class EtsRetsRuntimeLinkerEtsToTsTest : public EtsInteropTest {
public:
    EtsRetsRuntimeLinkerEtsToTsTest()
    {
        ani_env *env {};
        ani_vm *vm {};
        ani_size vmCount;
        ANI_GetCreatedVMs(&vm, 1, &vmCount);
        ASSERT(vmCount == 1);
        vm->GetEnv(ANI_VERSION_1, &env);

        const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
        if (stdlib != nullptr) {
            SetRuntimeLinker(env, stdlib);
        }

        std::string modulePath = GetABCPath();
        SetRuntimeLinker(env, modulePath);
    }
};

TEST_F(EtsRetsRuntimeLinkerEtsToTsTest, check_RuntimeLinker_ets_to_ts)
{
    ASSERT_TRUE(RunJsTestSuite("test_runtimelinker.ts"));
}

}  // namespace ark::ets::interop::js::testing
