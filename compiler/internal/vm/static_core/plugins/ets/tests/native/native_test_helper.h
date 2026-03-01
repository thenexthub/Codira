/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_TESTS_NATIVE_NATIVE_TEST_HELPER_H
#define PANDA_PLUGINS_ETS_TESTS_NATIVE_NATIVE_TEST_HELPER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/ani.h"

namespace ark::ets::test {

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// CC-OFFNXT(G.PRE.02) should be with define
#define CHECK_PTR_ARG(arg) ANI_CHECK_RETURN_IF_EQ(arg, nullptr, ANI_INVALID_ARGS)

// CC-OFFNXT(G.PRE.02) should be with define
#define CHECK_ENV(env)                                                                           \
    do {                                                                                         \
        ANI_CHECK_RETURN_IF_EQ(env, nullptr, ANI_INVALID_ARGS);                                  \
        bool hasPendingException = ::ark::ets::PandaEnv::FromAniEnv(env)->HasPendingException(); \
        ANI_CHECK_RETURN_IF_EQ(hasPendingException, true, ANI_PENDING_ERROR);                    \
    } while (false)

// NOLINTEND(cppcoreguidelines-macro-usage)

class EtsNapiTestBaseClass : public testing::Test {
public:
    template <typename R, typename... Args>
    void CallEtsFunction(R *ret, std::string_view className, std::string_view methodName, Args &&...args)
    {
        ani_module module;
        auto status = env_->FindModule(className.data(), &module);
        ASSERT_EQ(status, ANI_OK);
        ani_function fn;
        status = env_->Module_FindFunction(module, methodName.data(), nullptr, &fn);
        ASSERT_EQ(status, ANI_OK);

        // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
        if constexpr (std::is_same_v<R, ani_boolean>) {
            status = env_->Function_Call_Boolean(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_byte>) {
            status = env_->Function_Call_Byte(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_char>) {
            status = env_->Function_Call_Char(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_short>) {
            status = env_->Function_Call_Short(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_int>) {
            status = env_->Function_Call_Int(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_long>) {
            status = env_->Function_Call_Long(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_float>) {
            status = env_->Function_Call_Float(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_double>) {
            status = env_->Function_Call_Double(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_ref *> || std::is_same_v<R, ani_array> ||
                             std::is_same_v<R, ani_arraybuffer>) {
            status = env_->Function_Call_Ref(fn, reinterpret_cast<ani_ref *>(ret), args...);
        } else if constexpr (std::is_same_v<R, void>) {
            // do nothing
        } else {
            enum { INCORRECT_TEMPLATE_TYPE = false };
            static_assert(INCORRECT_TEMPLATE_TYPE, "Incorrect template type");
        }
        // NOLINTEND(cppcoreguidelines-pro-type-vararg)
    }

protected:
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

        // clang-format off
        std::vector<ani_option> optionsVector{
            {bootFileString.data(), nullptr},
            {"--ext:compiler-enable-jit=false", nullptr},
        };
        // clang-format on
        ani_options optionsPtr = {optionsVector.size(), optionsVector.data()};

        ASSERT_EQ(ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm_), ANI_OK) << "Cannot create ETS VM";
        ASSERT_EQ(vm_->GetEnv(ANI_VERSION_1, &env_), ANI_OK) << "Cannot get ani env";
    }

    void TearDown() override
    {
        ASSERT_EQ(vm_->DestroyVM(), ANI_OK) << "Cannot destroy ETS VM";
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    const char *abcPath_ {nullptr};
    ani_env *env_ {nullptr};
    ani_vm *vm_ {nullptr};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

inline void CheckUnhandledError(ani_env *env, ani_boolean shouldExist = ANI_FALSE)
{
    ani_boolean exists = ANI_FALSE;
    ASSERT_EQ(env->ExistUnhandledError(&exists), ANI_OK);
    ASSERT_EQ(exists, shouldExist);
}

}  // namespace ark::ets::test

#endif  // PANDA_PLUGINS_ETS_TESTS_NATIVE_NATIVE_TEST_HELPER_H
