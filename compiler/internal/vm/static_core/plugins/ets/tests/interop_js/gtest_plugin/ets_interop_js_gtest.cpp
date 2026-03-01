/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include "ets_interop_js_gtest.h"
#include <iostream>
#include <memory>

namespace ark::ets::interop::js::testing {

napi_env EtsInteropTest::jsEnv_ = {};

class JsEnvAccessor {
public:
    static void SetJsEnv(napi_env env)
    {
        EtsInteropTest::jsEnv_ = env;
    }

    static void ResetJsEnv()
    {
        EtsInteropTest::jsEnv_ = {};
    }
};

static napi_value Main(napi_env env, napi_callback_info info)
{
    size_t jsArgc = 0;
    [[maybe_unused]] napi_status status = napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr);
    ASSERT(status == napi_ok);

    if (jsArgc != 1) {
        std::cerr << "ETS_INTEROP_GTEST_PLUGIN: Incorrect argc, argc=" << jsArgc << std::endl;
        std::abort();
    }

    std::vector<napi_value> jsArgv(jsArgc);

    napi_value thisArg = nullptr;
    void *data = nullptr;
    status = napi_get_cb_info(env, info, &jsArgc, jsArgv.data(), &thisArg, &data);
    ASSERT(status == napi_ok);

    napi_value jsArray = jsArgv.front();
    uint32_t jsArrayLength = 0;
    status = napi_get_array_length(env, jsArray, &jsArrayLength);
    ASSERT(status == napi_ok);

    std::vector<std::string> argv;
    for (uint32_t i = 0; i < jsArrayLength; ++i) {
        napi_value jsArrayElement;
        status = napi_get_element(env, jsArray, i, &jsArrayElement);
        ASSERT(status == napi_ok);

        size_t strLength = 0;
        status = napi_get_value_string_utf8(env, jsArrayElement, nullptr, 0, &strLength);
        ASSERT(status == napi_ok);

        std::string utf8Str(++strLength, '\0');
        size_t copied = 0;
        status = napi_get_value_string_utf8(env, jsArrayElement, utf8Str.data(), utf8Str.size(), &copied);
        ASSERT(status == napi_ok);

        argv.emplace_back(std::move(utf8Str));
    }

    int argc = jsArrayLength;
    std::vector<char *> argvChar;
    argvChar.reserve(argv.size());

    for (auto &arg : argv) {
        argvChar.emplace_back(arg.data());
    }

    ASSERT(size_t(argc) == argvChar.size());
    ::testing::InitGoogleTest(&argc, argvChar.data());

    // Run tests
    JsEnvAccessor::SetJsEnv(env);
    int retValue = RUN_ALL_TESTS();
    JsEnvAccessor::ResetJsEnv();

    napi_value jsRetValue;
    status = napi_create_int32(env, retValue, &jsRetValue);
    ASSERT(status == napi_ok);
    return jsRetValue;
}

static napi_value Init(napi_env env, napi_value exports)
{
    const std::array desc = {
        napi_property_descriptor {"main", 0, Main, 0, 0, 0, napi_enumerable, 0},
    };

    [[maybe_unused]] napi_status status = napi_define_properties(env, exports, desc.size(), desc.data());
    ASSERT(status == napi_ok);

    return exports;
}

}  // namespace ark::ets::interop::js::testing

NAPI_MODULE(GTEST_ADDON, ark::ets::interop::js::testing::Init)
