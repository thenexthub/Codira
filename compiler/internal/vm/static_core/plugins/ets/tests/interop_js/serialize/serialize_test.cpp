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

#include <string>
#include <vector>
#include <node_api.h>

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_vm_api.h"

napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_serialize_hybrid([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value object,
                      [[maybe_unused]] napi_value transfer_list, [[maybe_unused]] napi_value clone_list,
                      [[maybe_unused]] void **result);

napi_status __attribute__((weak)) napi_deserialize_hybrid([[maybe_unused]] napi_env env, [[maybe_unused]] void *buffer,
                                                          [[maybe_unused]] napi_value *object);

namespace ark::ets::interop::js {

class TestModule {
public:
    NO_COPY_SEMANTIC(TestModule);
    NO_MOVE_SEMANTIC(TestModule);
    TestModule() = delete;
    ~TestModule() = default;

    static napi_value NapiSerialize(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        size_t argc = 1;
        napi_value argv[1];
        [[maybe_unused]] napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
        ASSERT(status == napi_ok);
        napi_value firstArg = argv[0];
        napi_value undefined = nullptr;
        status = napi_get_undefined(env, &undefined);
        ASSERT(status == napi_ok);
        void *serializedData;
        status = napi_serialize_hybrid(env, firstArg, undefined, undefined, &serializedData);
        ASSERT(status == napi_ok);
        napi_value v;
        status = napi_create_bigint_int64(env, reinterpret_cast<intptr_t>(serializedData), &v);
        ASSERT(status == napi_ok);
        return v;
    }

    static napi_value NapiDeserialize(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        size_t argc = 1;
        napi_value argv[1];
        [[maybe_unused]] napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
        ASSERT(status == napi_ok);
        int64_t ptr;
        bool loss = false;
        status = napi_get_value_bigint_int64(env, argv[0], &ptr, &loss);
        ASSERT(status == napi_ok);
        napi_value ret;
        status = napi_deserialize_hybrid(env, reinterpret_cast<void *>(ptr), &ret);
        ASSERT(status == napi_ok);
        return ret;
    }

    static napi_value Init(napi_env env, napi_value exports)
    {
        const std::array desc = {
            napi_property_descriptor {"napiSerialize", 0, NapiSerialize, 0, 0, 0, napi_enumerable, 0},
            napi_property_descriptor {"napiDeserialize", 0, NapiDeserialize, 0, 0, 0, napi_enumerable, 0},
        };

        napi_define_properties(env, exports, desc.size(), desc.data());
        return exports;
    }
};

NAPI_MODULE(TEST_MODULE, ark::ets::interop::js::TestModule::Init)

}  // namespace ark::ets::interop::js
