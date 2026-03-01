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

#ifndef PANDA_PLUGINS_ETS_ANI_GTEST_H
#define PANDA_PLUGINS_ETS_ANI_GTEST_H

#include <gtest/gtest.h>
#include <cstdlib>
#include <optional>
#include <vector>

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ani/ani_mangle.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"

namespace ark::ets::ani::testing {

class AniTest : public ::testing::Test {
public:
    void SetUp() override
    {
        const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
        ASSERT_NE(stdlib, nullptr);

        // Create boot-panda-files options
        std::string bootFileString = "--ext:boot-panda-files=" + std::string(stdlib);
        const char *abcPath = std::getenv("ANI_GTEST_ABC_PATH");
        if (abcPath != nullptr) {
            bootFileString += ":";
            bootFileString += abcPath;
        }

        ani_option bootFileOption = {bootFileString.data(), nullptr};

        std::vector<ani_option> options;
        options.push_back(bootFileOption);

        // Add extra test-specific ANI options
        for (auto &aniOpt : GetExtraAniOptions()) {
            options.push_back(aniOpt);
        }

        ani_options optionsPtr = {options.size(), options.data()};
        ASSERT_TRUE(ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm_) == ANI_OK);

        // Get ANI API
        ASSERT_TRUE(vm_->GetEnv(ANI_VERSION_1, &env_) == ANI_OK) << "Cannot get ani env";
        uint32_t aniVersion;
        ASSERT_TRUE(env_->GetVersion(&aniVersion) == ANI_OK) << "Cannot get ani version";
        ASSERT_TRUE(aniVersion == ANI_VERSION_1) << "Incorrect ani version";
    }

    void TearDown() override
    {
        ASSERT_TRUE(vm_->DestroyVM() == ANI_OK) << "Cannot destroy ANI VM";
    }

    /// Call function with name `fnName` from module denoted by `moduleName`
    template <typename R, typename... Args>
    R CallEtsFunction(const std::string &moduleName, const std::string &fnName, Args &&...args)
    {
        std::optional<R> result;
        CallEtsFunctionImpl(&result, moduleName, fnName, std::forward<Args>(args)...);
        if constexpr (!std::is_same_v<R, void>) {
            return result.value();
        }
    }

    class NativeFunction {
    public:
        template <typename FuncT>
        NativeFunction(const char *moduleDescriptor, const char *functionName, FuncT nativeFunction)
            : moduleDescriptor_(moduleDescriptor),
              functionName_(functionName),
              nativeFunction_(reinterpret_cast<void *>(nativeFunction))
        {
        }

        const char *GetModule() const
        {
            return moduleDescriptor_;
        }

        const char *GetName() const
        {
            return functionName_;
        }

        void *GetNativePtr() const
        {
            return nativeFunction_;
        }

    private:
        const char *moduleDescriptor_ {nullptr};
        const char *functionName_ {nullptr};
        void *nativeFunction_ {nullptr};
    };

    template <typename R, typename... Args>
    R CallEtsNativeMethod(const NativeFunction &fn, Args &&...args)
    {
        std::optional<R> result;

        CallEtsNativeMethodImpl(&result, fn, std::forward<Args>(args)...);

        if constexpr (!std::is_same_v<R, void>) {
            return result.value();
        }
    }

    void GetStdString(ani_string str, std::string &result)
    {
        GetStdString(env_, str, result);
    }

    static void GetStdString(ani_env *env, ani_string str, std::string &result)
    {
        ani_size sz {};
        ASSERT_EQ(env->String_GetUTF8Size(str, &sz), ANI_OK);

        result.resize(sz + 1);
        ASSERT_EQ(env->String_GetUTF8SubString(str, 0, sz, result.data(), result.size(), &sz), ANI_OK);
        result.resize(sz);
    }

    bool IsRuntimeClassInitialized(std::string_view classDescriptor, bool isClass = true)
    {
        std::optional<PandaString> desc;
        if (isClass) {
            desc = Mangle::ConvertDescriptor(classDescriptor, true);
        } else {
            desc = Mangle::ConvertDescriptor(std::string(classDescriptor) + ".ETSGLOBAL", true);
        }
        ASSERT(desc.has_value());
        PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env_);
        ScopedManagedCodeFix s(pandaEnv);

        EtsClassLinker *classLinker = pandaEnv->GetEtsVM()->GetClassLinker();
        EtsClass *klass = classLinker->GetClass(desc.value().c_str(), true, GetClassLinkerContext(s.GetCoroutine()));
        ASSERT(klass);

        return klass->IsInitialized();
    }

    void ThrowError()
    {
        ani_ref undef {};
        ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("escompat.Error", &cls), ANI_OK);
        ani_method ctor {};
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C{std.core.String}C{escompat.ErrorOptions}:", &ctor), ANI_OK);

        ani_object err {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env_->Object_New(cls, ctor, &err, undef, undef), ANI_OK);
        ASSERT_EQ(env_->ThrowError(static_cast<ani_error>(err)), ANI_OK);

        ASSERT_EQ(env_->Reference_Delete(err), ANI_OK);
        ASSERT_EQ(env_->Reference_Delete(undef), ANI_OK);
    }

protected:
    virtual std::vector<ani_option> GetExtraAniOptions()
    {
        return {};
    }

private:
    static std::string GetFindModuleFailureMsg(const std::string &moduleDescriptor)
    {
        std::stringstream ss;
        ss << "Failed to find module `" << moduleDescriptor << "`.";
        return ss.str();
    }

    static std::string GetFindFunctionFailureMsg(const std::string &moduleDescriptor, const std::string &fnName)
    {
        std::stringstream ss;
        ss << "Failed to find function `" << fnName << "` in `" << moduleDescriptor << "`.";
        return ss.str();
    }

    template <typename R, typename... Args>
    void CallEtsFunctionImpl(std::optional<R> *result, const std::string &moduleDescriptor, const std::string &fnName,
                             Args &&...args)
    {
        ani_module mod {};
        ASSERT_EQ(env_->FindModule(moduleDescriptor.c_str(), &mod), ANI_OK)
            << GetFindModuleFailureMsg(moduleDescriptor);
        ani_function fn {};
        ASSERT_EQ(env_->Module_FindFunction(mod, fnName.c_str(), nullptr, &fn), ANI_OK)
            << GetFindFunctionFailureMsg(moduleDescriptor, fnName);
        DoCallFunction(result, fn, std::forward<Args>(args)...);
    }

    template <typename R, typename... Args>
    void CallEtsNativeMethodImpl(std::optional<R> *result, const NativeFunction &fn, Args &&...args)
    {
        const auto *moduleDescriptor = fn.GetModule();
        const auto *fnName = fn.GetName();
        ani_module mod {};
        ASSERT_EQ(env_->FindModule(moduleDescriptor, &mod), ANI_OK) << GetFindModuleFailureMsg(moduleDescriptor);
        ani_function aniFn {};
        ASSERT_EQ(env_->Module_FindFunction(mod, fnName, nullptr, &aniFn), ANI_OK)
            << GetFindFunctionFailureMsg(moduleDescriptor, fnName);

        ani_native_function nativeFn = {fnName, nullptr, fn.GetNativePtr()};

        ASSERT_EQ(env_->Module_BindNativeFunctions(mod, &nativeFn, 1), ANI_OK)
            << "Failed to register native function `" << fnName << "` in module " << moduleDescriptor << ".";

        DoCallFunction(result, aniFn, std::forward<Args>(args)...);
    }

    // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
    template <typename R, typename... Args>
    void DoCallFunction(std::optional<R> *result, ani_function fn, Args &&...args)
    {
        std::conditional_t<std::is_same_v<R, void>, std::nullopt_t, R> value {};
        ani_status status;

        if constexpr (std::is_same_v<R, ani_boolean>) {
            status = env_->Function_Call_Boolean(fn, &value, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ani_byte>) {
            status = env_->Function_Call_Byte(fn, &value, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ani_char>) {
            status = env_->Function_Call_Char(fn, &value, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ani_short>) {
            status = env_->Function_Call_Short(fn, &value, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ani_int>) {
            status = env_->Function_Call_Int(fn, &value, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ani_long>) {
            status = env_->Function_Call_Long(fn, &value, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ani_float>) {
            status = env_->Function_Call_Float(fn, &value, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ani_double>) {
            status = env_->Function_Call_Double(fn, &value, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, void>) {
            status = env_->Function_Call_Void(fn, std::forward<Args>(args)...);
            value = std::nullopt;
        } else if constexpr (std::is_same_v<R, ani_ref> || std::is_same_v<R, ani_tuple_value> ||
                             std::is_same_v<R, ani_object>) {
            ani_ref resultRef {};
            status = env_->Function_Call_Ref(fn, &resultRef, std::forward<Args>(args)...);
            value = static_cast<R>(resultRef);
        } else {
            enum { INCORRECT_TEMPLATE_TYPE = false };
            static_assert(INCORRECT_TEMPLATE_TYPE, "Incorrect template type");
        }

        if (status == ANI_PENDING_ERROR) {
            ASSERT_EQ(env_->DescribeError(), ANI_OK);
        }
        ASSERT_EQ(status, ANI_OK);

        result->emplace(value);
    }
    // NOLINTEND(cppcoreguidelines-pro-type-vararg)

    static ClassLinkerContext *GetClassLinkerContext(ark::ets::EtsCoroutine *coroutine)
    {
        auto stack = StackWalker::Create(coroutine);
        if (!stack.HasFrame()) {
            return nullptr;
        }

        auto *method = ark::ets::EtsMethod::FromRuntimeMethod(stack.GetMethod());
        if (method != nullptr) {
            return method->GetClass()->GetLoadContext();
        }
        return nullptr;
    }

protected:
    ani_env *env_ {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
    ani_vm *vm_ {nullptr};    // NOLINT(misc-non-private-member-variables-in-classes)
};

}  // namespace ark::ets::ani::testing

#endif  // PANDA_PLUGINS_ETS_ANI_GTEST_H
