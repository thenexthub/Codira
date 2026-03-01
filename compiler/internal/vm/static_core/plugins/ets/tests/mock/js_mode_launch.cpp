/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "runtime/include/runtime.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_napi_env.h"

namespace ark::ets::test {

static int64_t GetCoroId([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_class cls)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(PandaEtsNapiEnv::FromAniEnv(env)->GetEtsCoroutine() == coro);
    return reinterpret_cast<ani_long>(coro);
}

class JsModeLaunchTest : public testing::Test {
protected:
    void SetUp() override
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});
        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib, "js_mode_launch.abc"});
        options.SetCoroutineImpl("stackful");

        Runtime::Create(options);

        ani_env *env = PandaEtsNapiEnv::GetCurrent();
        ani_class jsCoroutineClass {};
        ASSERT_EQ(env->FindClass("js_mode_launch.JSCoroutine", &jsCoroutineClass), ANI_OK);
        ani_native_function fn {"getCoroutineId", ":l", reinterpret_cast<void *>(GetCoroId)};
        ASSERT_EQ(env->Class_BindStaticNativeMethods(jsCoroutineClass, &fn, 1), ANI_OK);
    }

    void TearDown() override
    {
        Runtime::Destroy();
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    const std::string abcFile_ = "js_mode_launch.abc";
};

TEST_F(JsModeLaunchTest, Call)
{
    const std::string mainFunc = "js_mode_launch.ETSGLOBAL::main";
    auto res = Runtime::GetCurrent()->ExecutePandaFile(abcFile_.c_str(), mainFunc.c_str(), {});
    ASSERT_EQ(0, res.Value());
}
}  // namespace ark::ets::test
