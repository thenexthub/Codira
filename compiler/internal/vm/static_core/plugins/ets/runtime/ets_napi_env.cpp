/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "runtime/include/managed_thread.h"
#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ani/verify/verify_ani_interaction_api.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets {
Expected<PandaEtsNapiEnv *, const char *> PandaEtsNapiEnv::Create(EtsCoroutine *coroutine,
                                                                  mem::InternalAllocatorPtr allocator)
{
    auto etsVm = coroutine->GetVM();
    auto referenceStorage = MakePandaUnique<EtsReferenceStorage>(etsVm->GetGlobalObjectStorage(), allocator, false);
    if (!referenceStorage || !referenceStorage->GetAsReferenceStorage()->Init()) {
        return Unexpected("Cannot allocate EtsReferenceStorage");
    }

    auto *etsNapiEnv = allocator->New<PandaEtsNapiEnv>(coroutine, std::move(referenceStorage));
    if (etsNapiEnv == nullptr) {
        return Unexpected("Cannot allocate PandaEtsNapiEnv");
    }

    return Expected<PandaEtsNapiEnv *, const char *>(etsNapiEnv);
}

PandaEtsNapiEnv *PandaEtsNapiEnv::GetCurrent()
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    return coro->GetEtsNapiEnv();
}

PandaEtsNapiEnv::PandaEtsNapiEnv(EtsCoroutine *coroutine, PandaUniquePtr<EtsReferenceStorage> referenceStorage)
    : ani_env {ani::GetInteractionAPI()}, coroutine_(coroutine), referenceStorage_(std::move(referenceStorage))
{
    if (coroutine->GetPandaVM()->IsVerifyANI()) {
        ani::verify::ANIVerifier *verifier = GetEtsVM()->GetANIVerifier();
        envANIVerifier_ = MakePandaUnique<ani::verify::EnvANIVerifier>(verifier, c_api);
        c_api = ani::verify::GetVerifyInteractionAPI();
    }
}

PandaEtsVM *PandaEtsNapiEnv::GetEtsVM() const
{
    return coroutine_->GetPandaVM();
}

void PandaEtsNapiEnv::FreeInternalMemory()
{
    referenceStorage_.reset();
}

void PandaEtsNapiEnv::CleanUp()
{
    referenceStorage_->CleanUp();
}

void PandaEtsNapiEnv::ReInitialize()
{
    referenceStorage_->ReInitialize();
}

void PandaEtsNapiEnv::SetException(EtsThrowable *thr)
{
    ASSERT_MANAGED_CODE();
    coroutine_->SetException(thr->GetCoreType());
}

EtsThrowable *PandaEtsNapiEnv::GetThrowable() const
{
    ASSERT_MANAGED_CODE();
    return reinterpret_cast<EtsThrowable *>(coroutine_->GetException());
}

bool PandaEtsNapiEnv::HasPendingException() const
{
    return coroutine_->HasPendingException();
}

void PandaEtsNapiEnv::ClearException()
{
    ASSERT_MANAGED_CODE();
    coroutine_->ClearException();
}

}  // namespace ark::ets
