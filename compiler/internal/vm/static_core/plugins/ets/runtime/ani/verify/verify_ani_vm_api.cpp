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

#include "plugins/ets/runtime/ani/verify/verify_ani_vm_api.h"

#include "ani.h"
#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/ani_checkers.h"
#include "plugins/ets/runtime/ani/ani_vm_api.h"
#include "plugins/ets/runtime/ani/verify/types/vvm.h"
#include "plugins/ets/runtime/ani/verify/verify_ani_cast_api.h"
#include "plugins/ets/runtime/ani/verify/verify_ani_checker.h"
#include "plugins/ets/runtime/ets_napi_env.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define VERIFY_ANI_ARGS(...)                                   \
    do {                                                       \
        bool success = VerifyANIArgs(__func__, {__VA_ARGS__}); \
        ANI_CHECK_RETURN_IF_EQ(success, false, ANI_ERROR);     \
    } while (false)

namespace ark::ets::ani::verify {

static const __ani_vm_api *g_vmApi = GetVMAPI();

NO_UB_SANITIZE static ani_status DestroyVM(VVm *vvm)
{
    VERIFY_ANI_ARGS(ANIArg::MakeForVm(vvm, "vm"));
    return g_vmApi->DestroyVM(vvm->GetVm());
}

NO_UB_SANITIZE static ani_status GetEnv(VVm *vvm, uint32_t version, VEnv **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForVm(vvm, "vm"),
        ANIArg::MakeForEnvVersion(version, "version"),
        ANIArg::MakeForEnvStorage(vresult, "result"),
    );
    // clang-format on
    ani_env *result {};
    ani_status status = g_vmApi->GetEnv(vvm->GetVm(), version, &result);
    if (LIKELY(status == ANI_OK)) {
        *vresult = PandaEnv::GetCurrent()->GetEnvANIVerifier()->GetEnv(result);
    }
    return status;
}

NO_UB_SANITIZE static ani_status AttachCurrentThread(VVm *vvm, const ani_options *options, uint32_t version,
                                                     VEnv **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForVm(vvm, "vm"),
        ANIArg::MakeForOptions(options, "options"),
        ANIArg::MakeForEnvVersion(version, "version"),
        ANIArg::MakeForEnvStorage(vresult, "result"),
    );
    // clang-format on
    ani_env *result {};
    ani_status status = g_vmApi->AttachCurrentThread(vvm->GetVm(), options, version, &result);
    if (LIKELY(status == ANI_OK)) {
        *vresult = PandaEnv::GetCurrent()->GetEnvANIVerifier()->AttachThread(result);
    }
    return status;
}

NO_UB_SANITIZE static ani_status DetachCurrentThread(VVm *vvm)
{
    VERIFY_ANI_ARGS(ANIArg::MakeForVm(vvm, "vm"));
    return g_vmApi->DetachCurrentThread(vvm->GetVm());
}

// clang-format off
const __ani_vm_api VERIFY_VM_API = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    VERIFY_ANI_CAST_API(DestroyVM),
    VERIFY_ANI_CAST_API(GetEnv),
    VERIFY_ANI_CAST_API(AttachCurrentThread),
    VERIFY_ANI_CAST_API(DetachCurrentThread),
};
// clang-format on

const __ani_vm_api *GetVerifyVMAPI()
{
    return &VERIFY_VM_API;
}

}  // namespace ark::ets::ani::verify
