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

#include "plugins/ets/runtime/interop_js/js_refconvert_tuple.h"

#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_proxy/js_proxy.h"
#include "plugins/ets/runtime/types/ets_tuple.h"

namespace ark::ets::interop::js {

template class JSRefConvertTuple<false>;
template class JSRefConvertTuple<true>;

using JsProxyMethodCallBack = napi_value (*)(napi_env, size_t, napi_value *);

template <JsProxyMethodCallBack CALL_BACK>
static napi_value JsProxyMethodCallBackWrapper(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = ark::ets::interop::js::InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS_TO_ETS(coro);

    size_t argc;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &argc, jsArgs->data(), nullptr, nullptr));

    return CALL_BACK(env, argc, jsArgs->data());
}

static EtsObject *UnwrapJsValue(napi_env env, napi_value jsValue)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);

    EtsObject *etsObject;
    if (IsUndefined<true>(env, jsValue)) {
        etsObject = nullptr;
    } else if (IsNull<true>(env, jsValue)) {
        etsObject = ctx->GetNullValue();
    } else {
        auto refconv = JSRefConvertResolve(ctx, ctx->GetObjectClass());
        ASSERT(refconv != nullptr);
        etsObject = refconv->Unwrap(ctx, jsValue);
    }

    return etsObject;
}

template <bool IS_TUPLEN>
static napi_value TupleGetCallBackHandler(napi_env env, size_t argc, napi_value *argv)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);

    Span<napi_value> argsSpan {argv, argc};
    napi_value jsThis = argsSpan[0];
    napi_value property = argsSpan[1];

    if (GetValueType(env, property) != napi_number) {
        // if the property name is invalid, return undefined
        return GetUndefined(env);
    }

    uint32_t tupleIdx;
    napi_get_value_uint32(env, property, &tupleIdx);

    if constexpr (!IS_TUPLEN) {
        // handle Tuple0, Tuple1, ..., Tuple16
        ScopedManagedCodeThread managedCode(coro);

        ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
        auto etsThis = storage->GetReference(env, jsThis)->GetEtsObject();

        uint32_t fieldsCnt = etsThis->GetClass()->GetFieldsNumber();
        if (tupleIdx >= fieldsCnt) {
            // if the tuple index is out of bounds, return undefined
            return GetUndefined(env);
        }

        auto etsTupleAccessor = EtsTupleAccessorFromEtsObject(etsThis, fieldsCnt);
        auto elementObject = etsTupleAccessor->GetElement(tupleIdx);

        auto refconv = JSRefConvertResolve(ctx, elementObject->GetClass()->GetRuntimeClass());
        ASSERT(refconv != nullptr);
        return refconv->Wrap(ctx, elementObject);
    } else {
        InteropFatal("Interop: Tuple types with arity >16 are not yet implemented.");
    }
}

template <bool IS_TUPLEN>
static napi_value TupleSetCallBackHandler(napi_env env, size_t argc, napi_value *argv)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);

    Span<napi_value> argsSpan {argv, argc};
    napi_value jsThis = argsSpan[0];
    napi_value property = argsSpan[1];
    napi_value jsValueToSet = argsSpan[2];
    if (GetValueType(env, property) != napi_number) {
        return GetBoolean(env, false);
    }

    uint32_t tupleIdx;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, property, &tupleIdx));

    if constexpr (!IS_TUPLEN) {
        // handle Tuple0, Tuple1, ..., Tuple16
        ScopedManagedCodeThread managedCode(coro);

        ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
        auto etsThis = storage->GetReference(env, jsThis)->GetEtsObject();

        uint32_t fieldsCnt = etsThis->GetClass()->GetFieldsNumber();
        if (tupleIdx >= fieldsCnt) {
            // if the tuple index is out of bounds, throw an error
            InteropCtx::ThrowJSError(env, "Tuple index out of bounds");
            return GetBoolean(env, true);
        }

        auto etsTupleAccessor = EtsTupleAccessorFromEtsObject(etsThis, fieldsCnt);
        EtsObject *etsValueToSet = UnwrapJsValue(env, jsValueToSet);
        etsTupleAccessor->SetElement(tupleIdx, etsValueToSet);

        return GetBoolean(env, true);
    } else {
        // handle TupleN
        InteropFatal("Interop: Tuple types with arity >16 are not yet implemented.");
    }
}

template <bool IS_TUPLEN>
napi_ref JSRefConvertTuple<IS_TUPLEN>::CreateJsProxyHandler(InteropCtx *ctx)
{
    // init jsProxyHandler
    napi_env env = ctx->GetJSEnv();

    napi_value jsProxyHandler;
    NAPI_CHECK_FATAL(napi_create_object(env, &jsProxyHandler));

    napi_value getHandlerFunc;
    NAPI_CHECK_FATAL(napi_create_function(env, nullptr, NAPI_AUTO_LENGTH,
                                          JsProxyMethodCallBackWrapper<TupleGetCallBackHandler<IS_TUPLEN>>, nullptr,
                                          &getHandlerFunc));
    napi_value setHandlerFunc;
    NAPI_CHECK_FATAL(napi_create_function(env, nullptr, NAPI_AUTO_LENGTH,
                                          JsProxyMethodCallBackWrapper<TupleSetCallBackHandler<IS_TUPLEN>>, nullptr,
                                          &setHandlerFunc));

    NAPI_CHECK_FATAL(napi_set_named_property(env, jsProxyHandler, "get", getHandlerFunc));
    NAPI_CHECK_FATAL(napi_set_named_property(env, jsProxyHandler, "set", setHandlerFunc));

    napi_ref jsProxyHandlerRef = nullptr;
    NAPI_CHECK_FATAL(napi_create_reference(env, jsProxyHandler, 1, &jsProxyHandlerRef));
    return jsProxyHandlerRef;
}

template <bool IS_TUPLEN>
napi_value JSRefConvertTuple<IS_TUPLEN>::WrapImpl(InteropCtx *ctx, EtsObject *etsObject)
{
    if constexpr (!IS_TUPLEN) {
        // handle Tuple0, Tuple1, ..., Tuple16
        napi_env env = ctx->GetJSEnv();

        ASSERT(etsObject != nullptr);
        ASSERT(this->klass_ == etsObject->GetClass());

        ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
        if (LIKELY(storage->HasReference(etsObject, env))) {
            return storage->GetJsObject(etsObject, env);
        }

        napi_value jsProxyHandler = nullptr;
        NAPI_CHECK_FATAL(napi_get_reference_value(env, jsProxyHandlerRef_, &jsProxyHandler));

        napi_value targetObject;
        NAPI_CHECK_FATAL(napi_create_object(env, &targetObject));
        std::vector<napi_value> args = {targetObject, jsProxyHandler};

        napi_value proxyObject {};
        napi_value jsProxyCtor = ctx->GetCommonJSObjectCache()->GetProxy();
        NAPI_CHECK_FATAL(napi_new_instance(env, jsProxyCtor, args.size(), args.data(), &proxyObject));

        auto coro = EtsCoroutine::GetCurrent();
        [[maybe_unused]] EtsHandleScope s(coro);
        EtsHandle<EtsObject> objHandle(coro, etsObject);
        storage->CreateETSObjectRef(ctx, objHandle, proxyObject);

        return proxyObject;
    } else {
        // handle TupleN
        InteropFatal("Interop: Tuple types with arity >16 are not yet implemented.");
    }
}

template <bool IS_TUPLEN>
EtsObject *JSRefConvertTuple<IS_TUPLEN>::UnwrapImpl([[maybe_unused]] InteropCtx *ctx,
                                                    [[maybe_unused]] napi_value jsObject)
{
    // just throw exception
    auto coro = EtsCoroutine::GetCurrent();
    InteropCtx::ThrowETSError(coro, "Assigning a dynamic object to a static tuple object is not supported.");
    return nullptr;
}

}  // namespace ark::ets::interop::js
