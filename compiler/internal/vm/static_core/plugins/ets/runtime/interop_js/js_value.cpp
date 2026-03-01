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

#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/mem/local_object_handle.h"

namespace ark::ets::interop::js {

[[nodiscard]] JSValue *JSValue::AttachFinalizer(EtsCoroutine *coro, JSValue *jsValue)
{
    ASSERT(JSValue::IsFinalizableType(jsValue->GetType()));

    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }

    LocalObjectHandle<JSValue> handle(coro, jsValue);

    JSValue *mirror = AllocUndefined(coro, ctx);
    if (UNLIKELY(mirror == nullptr)) {
        FinalizeETSWeak(ctx, handle.GetPtr());
        return nullptr;
    }
    mirror->type_ = handle->type_;
    mirror->data_ = handle->data_;

    if (UNLIKELY(!ctx->PushOntoFinalizationRegistry(coro, handle.GetPtr(), mirror))) {
        FinalizeETSWeak(ctx, handle.GetPtr());
        return nullptr;
    }
    return handle.GetPtr();
}

void JSValue::FinalizeETSWeak(InteropCtx *ctx, EtsObject *cbarg)
{
    auto jsValue = JSValue::FromEtsObject(cbarg);
    ASSERT(JSValue::IsFinalizableType(jsValue->GetType()));

    auto type = jsValue->GetType();
    switch (type) {
        case napi_string:
            ctx->GetStringStor()->Release(jsValue->GetString());
            return;
        case napi_symbol:
            [[fallthrough]];
        case napi_object:
            [[fallthrough]];
        case napi_function:
            NAPI_CHECK_FATAL(napi_delete_reference(ctx->GetJSEnv(), jsValue->GetNapiRef(ctx->GetJSEnv())));
            return;
        case napi_bigint:
            delete jsValue->GetBigInt();
            return;
        default:
            InteropCtx::Fatal("Finalizer called for non-finalizable type: " + std::to_string(type));
    }
    UNREACHABLE();
}

JSValue *JSValue::CreateByType(InteropCtx *ctx, napi_env env, napi_value nvalue, napi_valuetype jsType,
                               EtsHandle<JSValue> &jsvalue)
{
    switch (jsType) {
        case napi_undefined:
            jsvalue->SetUndefined();
            return jsvalue.GetPtr();
        case napi_null:
            jsvalue->SetNull();
            return jsvalue.GetPtr();
        case napi_boolean: {
            bool v;
            NAPI_ASSERT_OK(napi_get_value_bool(env, nvalue, &v));
            jsvalue->SetBoolean(v);
            return jsvalue.GetPtr();
        }
        case napi_number: {
            double v;
            NAPI_ASSERT_OK(napi_get_value_double(env, nvalue, &v));
            jsvalue->SetNumber(v);
            return jsvalue.GetPtr();
        }
        case napi_string: {
            auto cachedStr = ctx->GetStringStor()->Get(interop::js::GetString(env, nvalue));
            jsvalue->SetString(cachedStr);
            return JSValue::AttachFinalizer(EtsCoroutine::GetCurrent(), jsvalue.GetPtr());
        }
        case napi_bigint: {
            jsvalue->SetBigInt(interop::js::GetBigInt(env, nvalue));
            return JSValue::AttachFinalizer(EtsCoroutine::GetCurrent(), jsvalue.GetPtr());
        }
        case napi_symbol:
            [[fallthrough]];
        case napi_object:
            [[fallthrough]];
        case napi_function:
            [[fallthrough]];
        case napi_external: {
            SetRefValue(ctx, nvalue, jsType, jsvalue);
            return jsvalue.GetPtr();
        }
        default:
            InteropCtx::Fatal("Unsupported JSValue.Type: " + std::to_string(jsType));
    }
    UNREACHABLE();
}
JSValue *JSValue::Create(EtsCoroutine *coro, InteropCtx *ctx, napi_value nvalue)
{
    auto env = ctx->GetJSEnv();
    napi_valuetype jsType = GetValueType<true>(env, nvalue);

    auto jsvalue = AllocUndefined(coro, ctx);
    if (UNLIKELY(jsvalue == nullptr)) {
        return nullptr;
    }
    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<JSValue> jsvalueHandle(coro, jsvalue);
    return CreateByType(ctx, env, nvalue, jsType, jsvalueHandle);
}

napi_value JSValue::GetBooleanValue(EtsCoroutine *coro, napi_env env, EtsHandle<JSValue> jsObject)
{
    napi_value value;
    auto booleanVal = jsObject->GetBoolean();
    ScopedNativeCodeThread nativeScope(coro);
    NAPI_ASSERT_OK(napi_get_boolean(env, booleanVal, &value));
    return value;
}

napi_value JSValue::GetNumberValue(EtsCoroutine *coro, napi_env env, EtsHandle<JSValue> jsObject)
{
    napi_value value;
    auto numVal = jsObject->GetNumber();
    ScopedNativeCodeThread nativeScope(coro);
    NAPI_ASSERT_OK(napi_create_double(env, numVal, &value));
    return value;
}

napi_value JSValue::GetStringValue(EtsCoroutine *coro, napi_env env, EtsHandle<JSValue> jsObject)
{
    napi_value value;
    std::string const *str = jsObject->GetString().Data();
    ScopedNativeCodeThread nativeScope(coro);
    NAPI_ASSERT_OK(napi_create_string_utf8(env, str->data(), str->size(), &value));
    return value;
}

napi_value JSValue::GetBigIntValue(EtsCoroutine *coro, napi_env env, EtsHandle<JSValue> jsObject)
{
    napi_value value;
    auto [words, signBit] = *(jsObject->GetBigInt());
    ScopedNativeCodeThread nativeScope(coro);
    NAPI_ASSERT_OK(napi_create_bigint_words(env, signBit, words.size(), words.data(), &value));
    return value;
}

napi_value JSValue::GetNapiValue(EtsCoroutine *coro, InteropCtx *ctx, EtsHandle<JSValue> &handle)
{
    napi_value jsValue {};
    auto env = ctx->GetJSEnv();

    auto jsType = handle->GetType();
    switch (jsType) {
        case napi_undefined: {
            NAPI_ASSERT_OK(napi_get_undefined(env, &jsValue));
            return jsValue;
        }
        case napi_null: {
            NAPI_ASSERT_OK(napi_get_null(env, &jsValue));
            return jsValue;
        }
        case napi_boolean: {
            return GetBooleanValue(coro, env, handle);
        }
        case napi_number: {
            return GetNumberValue(coro, env, handle);
        }
        case napi_string: {
            return GetStringValue(coro, env, handle);
        }
        case napi_bigint: {
            return GetBigIntValue(coro, env, handle);
        }
        case napi_symbol:
            [[fallthrough]];
        case napi_object:
            [[fallthrough]];
        case napi_function:
            [[fallthrough]];
        case napi_external: {
            return GetRefValue(env, handle);
        }
        default: {
            InteropCtx::Fatal("Unsupported JSValue.Type: " + std::to_string(jsType));
        }
    }
    UNREACHABLE();
}

}  // namespace ark::ets::interop::js
