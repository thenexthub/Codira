/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_XGC_XGC_VM_ADAPTOR_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_XGC_XGC_VM_ADAPTOR_H

#include "hybrid/ecma_vm_interface.h"
#include "libarkbase/macros.h"
#include <node_api.h>

namespace ark::ets::interop::js {

class InteropCtx;

class XGCVmAdaptor {
public:
    XGCVmAdaptor(napi_env env, platform::EcmaVMInterface *ecmaVMIface) : env_(env), ecmaVMIface_(ecmaVMIface) {}
    NO_COPY_SEMANTIC(XGCVmAdaptor);
    NO_MOVE_SEMANTIC(XGCVmAdaptor);
    virtual ~XGCVmAdaptor() = default;
    /**
     * @brief Method checks if specific napi environment is equal to internal one
     * @param env: napi env you want to compare with internal one
     * @returns true if envs are equal, otherwise false
     */
    bool HasSameEnv(napi_env env) const
    {
        return env == env_;
    }
    /**
     * @brief Method uses `napi_delete_reference` to delete specific napi reference
     * @param ref: reference that should be deleted
     * @returns status of `napi_delete_reference` function execution
     */
    napi_status NapiDeleteReference(napi_ref ref);
    /**
     * @brief Method use internal ecma interface to start ecma gc marking from specific reference
     * @param ref: ecma vm napi ref you want to start gc marking with
     */
    virtual void MarkFromObject([[maybe_unused]] napi_ref ref);
    /// @brief Method starts executing of cross reference marking in ecma vm
    virtual bool StartXRefMarking();

    virtual void NotifyXGCInterruption();

private:
    napi_env env_;
    platform::EcmaVMInterface *ecmaVMIface_ = nullptr;
};

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_XGC_XGC_VM_ADAPTOR_H
