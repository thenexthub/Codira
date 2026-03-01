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

#ifndef TEST_MODULE_H
#define TEST_MODULE_H

#include <string>
#include <vector>
#include "runtime/mem/gc/g1/xgc-extension-data.h"
#include "plugins/ets/runtime/interop_js/sts_vm_interface_impl.h"

namespace ark::ets::interop::js {

class TestModule {
public:
    NO_COPY_SEMANTIC(TestModule);
    NO_MOVE_SEMANTIC(TestModule);
    TestModule() = delete;
    ~TestModule() = default;

    static napi_value Setup(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        auto *ctx = InteropCtx::Current();
        ctx->CreateXGCVmAdaptor<TestXGCEcmaVmAdaptor>(env, &g_xgcAdaptorValues);
        ctx->GetSTSVMInterface()->OnVMDetach();

        mem::GC *gc = ets::PandaEtsVM::GetCurrent()->GetGC();
        gc->AddListener(&g_gcListener);

        napi_value result;
        napi_get_undefined(env, &result);

        return result;
    }

    static napi_value Check(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        auto &gcErrors = g_gcListener.GetErrors();
        auto ecmaVmIfaceErrors = g_xgcAdaptorValues.GetErrors();
        if (!gcErrors.empty() || !ecmaVmIfaceErrors.empty()) {
            std::stringstream totalErrorMsg;
            totalErrorMsg << "Test failed:\n";
            for (const std::string &err : gcErrors) {
                totalErrorMsg << "* " << err << "\n";
            }
            for (const std::string &err : ecmaVmIfaceErrors) {
                totalErrorMsg << "* " << err << "\n";
            }
            napi_throw_error(env, nullptr, totalErrorMsg.str().c_str());
        }

        napi_value result;
        napi_get_undefined(env, &result);

        return result;
    }

    static napi_value Init(napi_env env, napi_value exports)
    {
        const std::array desc = {
            napi_property_descriptor {"setup", 0, Setup, 0, 0, 0, napi_enumerable, 0},
            napi_property_descriptor {"check", 0, Check, 0, 0, 0, napi_enumerable, 0},
        };

        napi_define_properties(env, exports, desc.size(), desc.data());

        return exports;
    }
};
}  // namespace ark::ets::interop::js

NAPI_MODULE(TEST_MODULE, ark::ets::interop::js::TestModule::Init)

#endif  // TEST_MODULE_H
