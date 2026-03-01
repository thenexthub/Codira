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

namespace ark::ets::interop::js {

class TestModule {
public:
    NO_COPY_SEMANTIC(TestModule);
    NO_MOVE_SEMANTIC(TestModule);
    TestModule() = delete;
    ~TestModule() = default;

    static napi_value CallJsBuiltinTest(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        return callTestMethod(env, "callJsBuiltinTest");
    }

    static napi_value LoadJsModuleTest(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        return callTestMethod(env, "loadJsModuleTest");
    }

    static napi_value CallJsFunctionTest(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        return callTestMethod(env, "callJsFunctionTest");
    }

    static napi_value CallJsAsyncFunctionTest(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        return callTestMethod(env, "callJsAsyncFunctionTest");
    }

    static napi_value CallJsAsyncFunctionWithAwaitTest(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        return callTestMethod(env, "callJsAsyncFunctionWithAwaitTest");
    }

    static napi_value Init(napi_env env, napi_value exports)
    {
        const std::array desc = {
            napi_property_descriptor {"callJsBuiltinTest", 0, CallJsBuiltinTest, 0, 0, 0, napi_enumerable, 0},
            napi_property_descriptor {"loadJsModuleTest", 0, LoadJsModuleTest, 0, 0, 0, napi_enumerable, 0},
            napi_property_descriptor {"callJsFunctionTest", 0, CallJsFunctionTest, 0, 0, 0, napi_enumerable, 0},
            napi_property_descriptor {"callJsAsyncFunctionTest", 0, CallJsAsyncFunctionTest, 0, 0, 0, napi_enumerable,
                                      0},
            napi_property_descriptor {"callJsAsyncFunctionWithAwaitTest", 0, CallJsAsyncFunctionWithAwaitTest, 0, 0, 0,
                                      napi_enumerable, 0},
        };

        napi_define_properties(env, exports, desc.size(), desc.data());

        return exports;
    }

private:
    static ani_vm *GetEtsVm()
    {
        ani_vm *etsVM;
        ani_size vmCount;
        [[maybe_unused]] auto res = ANI_GetCreatedVMs(&etsVM, 1, &vmCount);
        ASSERT(res == ANI_OK);
        ASSERT(vmCount == 1);
        return etsVM;
    }

    static napi_value callTestMethod(napi_env env, const char *name)
    {
        napi_value result;
        napi_get_undefined(env, &result);

        ani_vm *etsVM = GetEtsVm();

        auto event = os::memory::Event();
        auto t = std::thread([&event, etsVM, &name] {
            ani_env *etsEnv {nullptr};
            ani_option interopEnabled {"--interop=enable", nullptr};
            ani_options aniArgs {1, &interopEnabled};
            etsVM->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &etsEnv);
            ASSERT(etsEnv != nullptr);
            event.Fire();

            ani_class cls;
            [[maybe_unused]] ani_status status = etsEnv->FindClass("attach_test.TestClass", &cls);
            ASSERT(status == ANI_OK);
            ani_static_method method;
            status = etsEnv->Class_FindStaticMethod(cls, name, nullptr, &method);
            ASSERT(status == ANI_OK);
            ani_int val;
            etsEnv->c_api->Class_CallStaticMethod_Int(etsEnv, cls, method, &val);
            ASSERT(val == 0);

            etsVM->DetachCurrentThread();
        });

        event.Wait();
        t.join();

        return result;
    }
};

NAPI_MODULE(TEST_MODULE, ark::ets::interop::js::TestModule::Init)

}  // namespace ark::ets::interop::js
