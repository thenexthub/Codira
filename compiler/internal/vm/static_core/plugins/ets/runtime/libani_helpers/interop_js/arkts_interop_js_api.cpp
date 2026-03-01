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

#include "arkts_interop_js_api.h"
#include "libarkbase/utils/expected.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/interop_js/native_api/arkts_interop_js_api_impl.h"

static ark::Expected<ark::ets::EtsCoroutine *, std::string_view> GetCurrentCoroutine()
{
    auto *thread = ark::Thread::GetCurrent();
    if (thread == nullptr) {
        return ark::Unexpected(std::string_view("Thread is unattached"));
    }
    // After verifying that the current thread is attached to VM, it is valid to get current EtsCoroutine
    auto *coro = ark::ets::EtsCoroutine::GetCurrent();
    if (UNLIKELY(coro == nullptr)) {
        return ark::Unexpected(std::string_view("No current coroutine exists"));
    }
    return coro;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" bool arkts_napi_scope_open(ani_env *env, napi_env *result)
{
    if (UNLIKELY(result == nullptr)) {
        return false;
    }
    if (UNLIKELY(env == nullptr)) {
        LOG(WARNING, RUNTIME) << "Cannot open napi scope, incorrect ani_env";
        return false;
    }

    auto optCoro = GetCurrentCoroutine();
    if (UNLIKELY(!optCoro.HasValue())) {
        LOG(WARNING, RUNTIME) << "Cannot open napi scope, " << optCoro.Error();
        return false;
    }
    auto *coro = ark::ets::PandaEtsNapiEnv::FromAniEnv(env)->GetEtsCoroutine();
    if (UNLIKELY(coro != optCoro.Value())) {
        LOG(WARNING, RUNTIME) << "Cannot open napi scope, input ani_env is taken from another coroutine";
        return false;
    }

    napi_env localJsEnv {};
    if (UNLIKELY(!ark::ets::interop::js::GetCurrentNapiEnv(env, &localJsEnv))) {
        LOG(WARNING, RUNTIME) << "Cannot open napi scope, coroutine cannot interop";
        return false;
    }
    if (UNLIKELY(!ark::ets::interop::js::OpenETSToJSScope(coro, nullptr))) {
        return false;
    }
    *result = localJsEnv;
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" bool arkts_napi_scope_close_n(napi_env env, size_t nValues, napi_value *values, ani_ref *result)
{
    auto optCoro = GetCurrentCoroutine();
    if (UNLIKELY(!optCoro.HasValue())) {
        LOG(WARNING, RUNTIME) << "Cannot close napi scope, " << optCoro.Error();
        return false;
    }
    return ark::ets::interop::js::CloseETSToJSScope(env, nValues, values, result);
}
