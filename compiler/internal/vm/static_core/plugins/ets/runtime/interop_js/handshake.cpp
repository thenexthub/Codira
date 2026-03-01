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

#include "plugins/ets/runtime/interop_js/handshake.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "hybrid/ecma_vm_interface.h"

#include "interfaces/inner_api/napi/native_node_hybrid_api.h"

napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_vm_handshake([[maybe_unused]] napi_env env, [[maybe_unused]] void *stsIface, [[maybe_unused]] void **ecmaIface)
{
    INTEROP_LOG(FATAL) << "ETS_INTEROP_GTEST_PLUGIN: " << __func__
                       << " is implemented in later versions of OHOS, please update." << std::endl;
    return napi_ok;
}

namespace ark::ets::interop::js {

void Handshake::VmHandshake(napi_env env, InteropCtx *ctx)
{
    void *jsvmIface;
    auto status = napi_vm_handshake(env, ctx->GetSTSVMInterface(), &jsvmIface);
    if (status != napi_status::napi_ok) {
        InteropCtx::ThrowJSError(env, "Handshake error: napi_vm_handshake failed");
        return;
    }
    if (jsvmIface == nullptr) {
        InteropCtx::ThrowJSError(env, "Handshake error: got null VMInterfaceType");
        return;
    }
    auto *iface = static_cast<platform::VMInterface *>(jsvmIface);
    if (iface->GetVMType() != platform::VMInterface::VMInterfaceType::ECMA_VM_IFACE) {
        InteropCtx::ThrowJSError(env, "Handshake error: got wrong VMInterfaceType");
        return;
    }
    ctx->CreateXGCVmAdaptor<XGCVmAdaptor>(env, static_cast<platform::EcmaVMInterface *>(jsvmIface));
}

}  // namespace ark::ets::interop::js
