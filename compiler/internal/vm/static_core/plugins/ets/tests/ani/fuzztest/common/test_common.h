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

#ifndef PLUGINS_ETS_TESTS_ANI_FUZZTEST_COMMON_TEST_COMMON_H
#define PLUGINS_ETS_TESTS_ANI_FUZZTEST_COMMON_TEST_COMMON_H

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

    ~FuzzTestEngine()
    {
        env_ = nullptr;
        if (vm_ != nullptr) {
            vm_->DestroyVM();
            vm_ = nullptr;
        }
    }
    void GetAniEnv(ani_env **env)
    {
        *env = env_;
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

class AniFuzzEngine : public FuzzTestEngine {
public:
    static AniFuzzEngine *GetInstance()
    {
        static AniFuzzEngine instanceEngine;
        return &instanceEngine;
    }

    void AniGetIntObj(ani_object *obj)
    {
        ani_class intCls {};
        ani_method intCtorMethod {};
        ani_object intObj {};
        env_->FindClass("std.core.Int", &intCls);
        env_->Class_FindMethod(intCls, "<ctor>", "i:", &intCtorMethod);
        env_->Object_New(intCls, intCtorMethod, &intObj, 1U);
        *obj = intObj;
    }

    void AniGetTuple(ani_tuple_value *tuple)
    {
        ani_object intObj {};
        AniGetIntObj(&intObj);
        ani_class tupleCls {};
        ani_method tupleCtorMethod {};
        ani_object tupleObj {};
        env_->FindClass("std.core.Tuple3", &tupleCls);
        env_->Class_FindMethod(tupleCls, "<ctor>", nullptr, &tupleCtorMethod);
        env_->Object_New(tupleCls, tupleCtorMethod, &tupleObj, intObj, intObj, intObj);
        *tuple = static_cast<ani_tuple_value>(tupleObj);
    }

    template <typename ArrayType>
    void AniGetArray(ArrayType *result, ani_size length = 5)
    {
        if constexpr (std::is_same_v<ArrayType, ani_array>) {
            ani_ref undef {};
            env_->GetUndefined(&undef);
            env_->Array_New(length, undef, result);
        } else {
            static_assert(!std::is_same_v<ArrayType, ArrayType>, "Unsupported array type in AniGetArray");
        }
    }

private:
    AniFuzzEngine() : FuzzTestEngine() {}
};
#endif  // PLUGINS_ETS_TESTS_ANI_FUZZTEST_COMMON_TEST_COMMON_H
