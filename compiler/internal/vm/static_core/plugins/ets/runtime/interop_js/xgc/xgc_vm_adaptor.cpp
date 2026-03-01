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

#include "plugins/ets/runtime/interop_js/xgc/xgc_vm_adaptor.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

#ifdef PANDA_JS_ETS_HYBRID_MODE
#include "native_engine/native_reference.h"
#include "interfaces/inner_api/napi/native_node_hybrid_api.h"
#endif  // PANDA_JS_ETS_HYBRID_MODE

namespace ark::ets::interop::js {

void XGCVmAdaptor::MarkFromObject([[maybe_unused]] napi_ref ref)
{
#ifdef PANDA_JS_ETS_HYBRID_MODE
    napi_mark_from_object(env_, ref);
#endif  // PANDA_JS_ETS_HYBRID_MODE
}

bool XGCVmAdaptor::StartXRefMarking()
{
    ASSERT(ecmaVMIface_ != nullptr);
    return ecmaVMIface_->StartXRefMarking();
}

void XGCVmAdaptor::NotifyXGCInterruption()
{
    ASSERT(ecmaVMIface_ != nullptr);
    ecmaVMIface_->NotifyXGCInterruption();
}

napi_status XGCVmAdaptor::NapiDeleteReference(napi_ref ref)
{
    return napi_delete_reference(env_, ref);
}

}  // namespace ark::ets::interop::js
