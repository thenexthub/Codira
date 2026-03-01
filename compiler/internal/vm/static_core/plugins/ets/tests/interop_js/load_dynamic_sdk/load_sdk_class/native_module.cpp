/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <string>
#include <array>

#include <node_api.h>

#include "interop_js/interop_common.h"
#include "js_native_api.h"
#include "js_native_api_types.h"
#include "libarkbase/macros.h"

namespace ark::ets::interop::js {

class TestModule {
public:
    NO_COPY_SEMANTIC(TestModule);
    NO_MOVE_SEMANTIC(TestModule);
    TestModule() = delete;
    ~TestModule() = default;

    static napi_value constructor(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        napi_value thisVar;
        size_t argc = 0;
        napi_status status = napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr);
        if (status != napi_ok || thisVar == nullptr) {
            napi_throw_error(env, nullptr, "Failed to get callback info");
            return nullptr;
        }
        return thisVar;
    }

    static napi_value Init(napi_env env, [[maybe_unused]] napi_value exports)
    {
        napi_value ctor = nullptr;
        const char *clsName = "HashMap";
        NAPI_CHECK_FATAL(napi_define_class(env, clsName, NAPI_AUTO_LENGTH, constructor, nullptr, 0, nullptr, &ctor));
        NAPI_CHECK_FATAL(napi_set_named_property(env, exports, "HashMap", ctor));
        return exports;
    }
};

NAPI_MODULE(TEST_MODULE, TestModule::Init)
}  // namespace ark::ets::interop::js