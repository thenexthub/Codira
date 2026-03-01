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

#include "plugins/ets/runtime/ani/verify/types/venv.h"

#include "plugins/ets/runtime/ets_coroutine.h"

namespace ark::ets::ani::verify {

/*static*/
VEnv *VEnv::GetCurrent()
{
    if (Thread::GetCurrent() == nullptr) {
        return nullptr;
    }
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (coro == nullptr) {
        return nullptr;
    }
    ani_env *env = coro->GetEtsNapiEnv();
    return reinterpret_cast<VEnv *>(env);
}

void VEnv::CreateLocalScope()
{
    GetEnvANIVerifier()->CreateLocalScope();
}

std::optional<PandaString> VEnv::DestroyLocalScope()
{
    return GetEnvANIVerifier()->DestroyLocalScope();
}

void VEnv::CreateEscapeLocalScope()
{
    GetEnvANIVerifier()->CreateEscapeLocalScope();
}

std::optional<PandaString> VEnv::DestroyEscapeLocalScope(VRef *vref)
{
    return GetEnvANIVerifier()->DestroyEscapeLocalScope(vref);
}

void VEnv::DeleteLocalVerifiedRef(VRef *vref)
{
    GetEnvANIVerifier()->DeleteLocalVerifiedRef(vref);
}

VRef *VEnv::AddGlobalVerifiedRef(ani_ref gref)
{
    return GetEnvANIVerifier()->AddGlobalVerifiedRef(gref);
}

void VEnv::DeleteGlobalVerifiedRef(VRef *vgref)
{
    GetEnvANIVerifier()->DeleteGlobalVerifiedRef(vgref);
}

bool VEnv::IsValidGlobalVerifiedRef(VRef *vgref)
{
    return GetEnvANIVerifier()->IsValidGlobalVerifiedRef(vgref);
}

VResolver *VEnv::AddGlobalVerifiedResolver(ani_resolver resolver)
{
    return GetEnvANIVerifier()->AddGlobalVerifiedResolver(resolver);
}

void VEnv::DeleteGlobalVerifiedResolver(VResolver *vresolver)
{
    GetEnvANIVerifier()->DeleteGlobalVerifiedResolver(vresolver);
}

bool VEnv::IsValidGlobalVerifiedResolver(VResolver *vresolver)
{
    return GetEnvANIVerifier()->IsValidGlobalVerifiedResolver(vresolver);
}

EnvANIVerifier *VEnv::GetEnvANIVerifier()
{
    return PandaEnv::FromAniEnv(GetEnv())->GetEnvANIVerifier();
}

}  // namespace ark::ets::ani::verify
