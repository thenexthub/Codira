/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include <string>
#include <array>

#include <node_api.h>

#include "libarkbase/macros.h"

namespace ark::ets::interop::js {

class TestModule {
public:
    NO_COPY_SEMANTIC(TestModule);
    NO_MOVE_SEMANTIC(TestModule);
    TestModule() = delete;
    ~TestModule() = default;

    static napi_value info(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        napi_value result;
        napi_get_boolean(env, true, &result);
        return result;
    }

    static napi_value Init(napi_env env, napi_value exports)
    {
        const std::array desc = {napi_property_descriptor {"info", 0, info, 0, 0, 0, napi_enumerable, 0}};

        napi_define_properties(env, exports, desc.size(), desc.data());
        return exports;
    }
};

NAPI_MODULE(TEST_MODULE, TestModule::Init)
}  // namespace ark::ets::interop::js