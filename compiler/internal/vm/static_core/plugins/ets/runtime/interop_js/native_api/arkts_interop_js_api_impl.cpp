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

#include "arkts_interop_js_api_impl.h"

#include "ets_object_state_info.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/interop_js/code_scopes-inl.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/js_value.h"

namespace ark::ets::interop::js {

static ani_class g_esvalueClass {};
static ani_method g_isEcmaObjectMethod {};
static ani_field g_evField {};

static bool CacheEsValueClass(ani_env *env)
{
    ani_class esvalueClass {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    [[maybe_unused]] auto status = env->FindClass("std.interop.ESValue", &esvalueClass);
    ASSERT(status == ANI_OK);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (UNLIKELY(env->GlobalReference_Create(esvalueClass, reinterpret_cast<ani_ref *>(&g_esvalueClass)) != ANI_OK)) {
        return false;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Class_FindMethod(esvalueClass, "isECMAObject", ":z", &g_isEcmaObjectMethod);
    ASSERT(status == ANI_OK);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Class_FindField(esvalueClass, "ev", &g_evField);
    ASSERT(status == ANI_OK);
    return true;
}

static bool InitializeGlobalOnce(ani_env *env)
{
    // Guaranteed to be called once
    static bool isInitialized = CacheEsValueClass(env);
    return isInitialized;
}

PANDA_PUBLIC_API bool UnwrapESValue(ani_env *env, ani_object esvalue, void **result)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }
    if (UNLIKELY(!InitializeGlobalOnce(env))) {
        return false;
    }

    auto *coro = EtsCoroutine::GetCurrent();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (UNLIKELY(PandaEtsNapiEnv::FromAniEnv(env)->GetEtsCoroutine() != coro)) {
        return false;
    }
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        return false;
    }

    ani_boolean isEsValue = ANI_FALSE;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    [[maybe_unused]] auto status = env->Object_InstanceOf(esvalue, g_esvalueClass, &isEsValue);
    if (status != ANI_OK || isEsValue == ANI_FALSE) {
        return false;
    }
    ani_boolean isEcmaObject = ANI_FALSE;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_CallMethod_Boolean(esvalue, g_isEcmaObjectMethod, &isEcmaObject);
    if (UNLIKELY(status != ANI_OK || isEcmaObject == ANI_FALSE)) {
        return false;
    }
    ani_ref jsValue {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_GetField_Ref(esvalue, g_evField, &jsValue);
    ASSERT(status == ANI_OK);

    ani::ScopedManagedCodeFix s(env);
    auto *jsValueObject = JSValue::FromEtsObject(s.ToInternalType(jsValue));

    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto jsenv = ctx->GetJSEnv();
    NapiScope jsHandleScope(jsenv);

    auto jsObj = JSConvertJSValue::WrapWithNullCheck(jsenv, jsValueObject);
    void *res = nullptr;
    if (napi_unwrap(jsenv, jsObj, &res) != napi_ok) {
        return false;
    }
    *result = res;
    return true;
}

PANDA_PUBLIC_API bool GetCurrentNapiEnv(ani_env *env, napi_env *result)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }

    auto *coro = EtsCoroutine::GetCurrent();
    if (UNLIKELY(PandaEtsNapiEnv::FromAniEnv(env)->GetEtsCoroutine() != coro)) {
        return false;
    }

    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        return false;
    }

    *result = ctx->GetJSEnv();
    return true;
}

PANDA_PUBLIC_API bool OpenJSToETSScope(EtsCoroutine *coro, char const *descr)
{
    return OpenInteropCodeScope<false>(coro, descr);
}

PANDA_PUBLIC_API bool CloseJSToETSScope(EtsCoroutine *coro)
{
    return CloseInteropCodeScope<true>(coro);
}

PANDA_PUBLIC_API bool OpenETSToJSScope(EtsCoroutine *coro, char const *descr)
{
    return OpenInteropCodeScope<true>(coro, descr);
}

PANDA_PUBLIC_API bool CloseETSToJSScope(EtsCoroutine *coro)
{
    return CloseInteropCodeScope<false>(coro);
}

static bool CreateAnyRef(ani::ScopedManagedCodeFix &s, JSValue *anyRef, ani_ref *result)
{
    EtsObject *etsObj = anyRef->AsObject();
    return s.AddLocalRef(etsObj, result) == ANI_OK;
}

static bool ConvertNativeReferences(EtsCoroutine *coro, InteropCtx *ctx, Span<napi_value> values, Span<ani_ref> results)
{
    ASSERT(values.size() == results.size());

    ani::ScopedManagedCodeFix s(coro->GetEtsNapiEnv());
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    napi_env env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    std::vector<ani_ref> localResults(values.size(), nullptr);
    for (size_t i = 0, end = values.size(); i < end; ++i) {
        auto optAnyRef = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, values[i]);
        if (UNLIKELY(!optAnyRef || optAnyRef.value() == nullptr)) {
            ctx->ForwardJSException(coro);
            return false;
        }
        if (UNLIKELY(!CreateAnyRef(s, optAnyRef.value(), &localResults[i]))) {
            return false;
        }
    }
    std::copy(localResults.begin(), localResults.end(), results.begin());
    return true;
}

static bool CloseJsScopeImpl(napi_env env, Span<napi_value> values, Span<ani_ref> results)
{
    ASSERT(values.size() == results.size());

    auto *coro = EtsCoroutine::GetCurrent();
    auto *ctx = InteropCtx::Current(coro);
    if (UNLIKELY(ctx == nullptr || ctx->GetJSEnv() != env)) {
        return false;
    }

    if (!values.empty()) {
        if (UNLIKELY(!ConvertNativeReferences(coro, ctx, values, results))) {
            return false;
        }
    }
    return CloseETSToJSScope(coro);
}

PANDA_PUBLIC_API bool CloseETSToJSScope(napi_env env, size_t nValues, napi_value *values, ani_ref *result)
{
    if (UNLIKELY(env == nullptr || (nValues != 0 && (values == nullptr || result == nullptr)))) {
        return false;
    }
    return CloseJsScopeImpl(env, Span(values, nValues), Span(result, nValues));
}

}  // namespace ark::ets::interop::js
