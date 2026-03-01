/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTRINSICS_API_IMPL_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTRINSICS_API_IMPL_H_

#include "code_scopes.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_typeapi_type.h"

namespace ark::ets::interop::js {

void JSRuntimeFinalizationRegistryCallback(EtsObject *cbarg);
JSValue *JSRuntimeNewJSValueDouble(double v);
JSValue *JSRuntimeNewJSValueBoolean(uint8_t v);
JSValue *JSRuntimeNewJSValueString(EtsString *v);
JSValue *JSRuntimeNewJSValueObject(EtsObject *v);
uint8_t JSRuntimeIsJSValue(EtsObject *v);
JSValue *JSRuntimeNewJSValueBigInt(EtsBigInt *v);
double JSRuntimeGetValueDouble(JSValue *etsJsValue);
uint8_t JSRuntimeGetValueBoolean(JSValue *etsJsValue);
EtsString *JSRuntimeGetValueString(JSValue *etsJsValue);
EtsObject *JSRuntimeGetValueObject(EtsObject *etsValue, EtsClass *clsObj);
JSValue *JSRuntimeGetUndefined();
JSValue *JSRuntimeGetNull();
JSValue *JSRuntimeGetGlobal();
JSValue *JSRuntimeCreateObject();
EtsObject *JSRuntimeCreateArray();
uint8_t JSRuntimeInstanceOf(JSValue *object, JSValue *ctor);
uint8_t JSRuntimeInstanceOfDynamic(EtsObject *object, EtsObject *ctor);
uint8_t JSRuntimeInstanceOfStatic(JSValue *etsJsValue, EtsClass *etsCls);
std::pair<std::string_view, std::string_view> ResolveModuleName(std::string_view module);
JSValue *JSRuntimeLoadModule(EtsString *module);
uint8_t JSRuntimeStrictEqual([[maybe_unused]] JSValue *lhs, [[maybe_unused]] JSValue *rhs);
uint8_t JSRuntimeHasProperty(EtsObject *object, EtsString *name);
JSValue *JSRuntimeGetProperty(JSValue *object, JSValue *property);
uint8_t JSRuntimeHasPropertyObject(EtsObject *object, EtsObject *property);
uint8_t JSRuntimeHasElement(EtsObject *object, int index);
uint8_t JSRuntimeHasOwnProperty(EtsObject *object, EtsString *name);
uint8_t JSRuntimeHasOwnPropertyObject(EtsObject *object, EtsObject *property);
EtsString *JSRuntimeTypeOf(JSValue *object);
uint8_t JSRuntimeIsPromise(JSValue *object);
uint8_t JSRuntimeInstanceOfStaticType(JSValue *object, EtsTypeAPIType *paramType);
EtsObject *JSRuntimeInvoke(EtsObject *recv, EtsObject *func, EtsArray *args);
EtsObject *JSRuntimeInstantiate(EtsObject *callable, EtsArray *args);
void ESValueAnyIndexedSetter(EtsObject *etsObject, int32_t index, EtsObject *value);
void ESValueAnyNamedSetter(EtsObject *etsObject, EtsString *etsPropName, EtsObject *value);
EtsObject *ESValueAnyIndexedGetter(EtsObject *etsObject, int32_t index);
EtsObject *ESValueAnyNamedGetter(EtsObject *etsObject, EtsString *etsPropName);
void ESValueAnyStoreByValue(EtsObject *etsObject, EtsObject *key, EtsObject *value);
EtsObject *ESValueAnyLoadByValue(EtsObject *etsObject, EtsObject *value);
uint8_t ESValueAnyHasProperty(EtsObject *etsObject, EtsString *name);
uint8_t ESValueAnyHasElement(EtsObject *etsObject, int idx);
uint8_t ESValueAnyHasPropertyObject(EtsObject *etsObject, EtsObject *property);
uint8_t ESValueAnyInstanceOf(EtsObject *etsObject, EtsObject *ctor);
uint8_t ESValueAnyHasOwnProperty(EtsObject *etsObject, EtsString *name);
uint8_t ESValueAnyHasOwnPropertyObject(EtsObject *etsObject, EtsObject *property);
uint8_t ESValueAnyIsStaticValue(EtsObject *etsObject);
EtsString *JSValueToString(JSValue *object);
napi_value ToLocal(void *value);
void SetPropertyWithObject(JSValue *object, JSValue *property, EtsObject *value);
void SetIndexedPropertyWithObject(JSValue *object, uint32_t index, EtsObject *value);
void SetNamedPropertyWithObject(JSValue *object, const char *key, EtsObject *value);
EtsObject *GetPropertyObject(JSValue *object, JSValue *property);
EtsObject *GetPropertyObjectByString(JSValue *object, const char *property);
EtsObject *GetNamedPropertyObject(JSValue *object, const char *property);
JSValue *GetNamedPropertyJSValue(JSValue *object, const char *property);
EtsObject *InvokeWithObjectReturn(EtsObject *thisObj, EtsObject *function, Span<VMHandle<ObjectHeader>> args);
EtsObject *CreateObject(JSValue *ctor, Span<VMHandle<ObjectHeader>> args);
void *CompilerGetJSNamedProperty(void *val, char *propStr);
void *CompilerGetJSProperty(void *val, void *prop);
void *CompilerGetJSElement(void *val, int32_t index);
void *CompilerJSCallCheck(void *fn);
void *CompilerJSNewInstance(void *fn, uint32_t argc, void *args);
void CreateLocalScope();
void CompilerDestroyLocalScope();
void *CompilerLoadJSConstantPool();
void CompilerInitJSCallClassForCtx(void *klassPtr);
void *CompilerConvertVoidToLocal();
void *CompilerConvertRefTypeToLocal(EtsObject *etsValue);
void PromiseInteropResolve(EtsObject *etsValue, EtsLong deferred);
void PromiseInteropReject(EtsObject *etsValue, EtsLong deferred);
EtsString *JSONStringify(JSValue *jsvalue);
EtsString *CompilerConvertLocalToString(void *value);
EtsObject *CompilerConvertLocalToRefType(void *klassPtr, void *value);
JSValue *JSRuntimeGetPropertyJSValueyByKey(JSValue *objectValue, JSValue *keyValue);
EtsEscompatArrayBuffer *TransferArrayBufferToStatic(ESValue *object);
EtsObject *TransferArrayBufferToDynamic(EtsEscompatArrayBuffer *staticArrayBuffer);
EtsObject *CreateDynamicTypedArray(EtsEscompatArrayBuffer *staticArrayBuffer, int32_t typedArrayType, double length,
                                   double byteOffset);
EtsObject *CreateDynamicDataView(EtsEscompatArrayBuffer *staticArrayBuffer, double byteLength, double byteOffset);
void SetInteropRuntimeLinker(EtsRuntimeLinker *linker);
EtsRuntimeLinker *GetInteropRuntimeLinker();
EtsBoolean IsJSInteropRef(EtsObject *value);
EtsLong SerializeHandle(JSValue *value);
JSValue *DeserializeHandle(EtsLong value);
EtsObject *JSRuntimeInvokeDynamicFunction(EtsObject *functionObject, EtsObjectArray *args);

template <typename T>
typename T::cpptype JSValueNamedGetter(JSValue *etsJsValue, EtsString *etsPropName)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    PandaString propName = etsPropName->GetMutf8();
    auto res = JSValueGetByName<T>(ctx, etsJsValue, propName.c_str());
    if (UNLIKELY(!res)) {
        return {};
    }
    return res.value();
}

template <typename T>
void JSValueNamedSetter(JSValue *etsJsValue, EtsString *etsPropName, typename T::cpptype etsPropVal)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    PandaString propName = etsPropName->GetMutf8();
    JSValueSetByName<T>(ctx, etsJsValue, propName.c_str(), etsPropVal);
}

template <typename T>
typename T::cpptype JSValueIndexedGetter(JSValue *etsJsValue, int64_t index)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<JSValue> etsJsValueHandle(coro, etsJsValue);
    NapiScope jsHandleScope(env);

    napi_value result;
    napi_value jsVal = JSValue::GetNapiValue(coro, ctx, etsJsValueHandle);
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_get_element(env, jsVal, index, &result);
    }

    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return {};
    }

    auto res = T::UnwrapWithNullCheck(ctx, env, result);
    if (!res) {
        ctx->ForwardJSException(coro);
        return {};
    }

    return res.value();
}

template <typename T>
void JSValueIndexedSetter(JSValue *etsJsValue, int32_t index, typename T::cpptype value)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);

        jsStatus = napi_set_element(env, JSConvertJSValue::WrapWithNullCheck(env, etsJsValue), index,
                                    T::WrapWithNullCheck(env, value));
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
    }
}

template <typename T>
void *ConvertToLocal(typename T::cpptype etsValue)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    napi_env env = ctx->GetJSEnv();
    napi_value localJsValue = T::Wrap(env, etsValue);
    if (UNLIKELY(localJsValue == nullptr)) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return localJsValue;
}

template <bool USE_RET>
std::conditional_t<USE_RET, void *, void> CompilerJSCallFunction(void *obj, void *fn, uint32_t argc, void *args)
{
    auto jsThis = ToLocal(obj);
    auto jsFn = ToLocal(fn);
    auto jsArgs = reinterpret_cast<napi_value *>(args);
    auto coro = EtsCoroutine::GetCurrent();

    InteropCtx *ctx = nullptr;
    if constexpr (USE_RET) {
        ctx = InteropCtx::Current(coro);
        if (ctx == nullptr) {
            ThrowNoInteropContextException();
            return nullptr;
        }
    } else {
        ctx = InteropCtx::Current(coro);
        if (ctx == nullptr) {
            ThrowNoInteropContextException();
            return;
        }
    }
    ASSERT(ctx != nullptr);

    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    napi_env env = ctx->GetJSEnv();

    [[maybe_unused]] napi_value jsRet;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        if constexpr (USE_RET) {
            jsStatus = napi_call_function(env, jsThis, jsFn, argc, jsArgs, &jsRet);
        } else {
            jsStatus = napi_call_function(env, jsThis, jsFn, argc, jsArgs, nullptr);
        }
    }

    if (UNLIKELY(jsStatus != napi_ok)) {
        INTEROP_FATAL_IF(jsStatus != napi_pending_exception);
        ctx->ForwardJSException(coro);
        INTEROP_LOG(DEBUG) << "JSValueJSCall: exit with pending exception";
        if constexpr (USE_RET) {
            return nullptr;
        }
    }
    if constexpr (USE_RET) {
        return jsRet;
    }
}

inline void HandleExceptions(napi_env env, InteropCtx *ctx)
{
    if (NapiIsExceptionPending(env)) {
        ctx->ForwardJSException(EtsCoroutine::GetCurrent());
    }
    ASSERT(ctx->SanityETSExceptionPending());
}

template <typename CONVERTOR>
typename CONVERTOR::cpptype ConvertFromLocal(void *value)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        if constexpr (!std::is_pointer_v<typename CONVERTOR::cpptype>) {
            return 0;
        } else {
            return nullptr;
        }
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    napi_env env = ctx->GetJSEnv();
    auto res = CONVERTOR::Unwrap(ctx, env, ToLocal(value));
    if (UNLIKELY(!res.has_value())) {
        HandleExceptions(env, ctx);
        if constexpr (!std::is_pointer_v<typename CONVERTOR::cpptype>) {
            return 0;
        } else {
            return nullptr;
        }
    }
    return res.value();
}

template <>
// CC-OFFNXT(G.FUD.06) solid logic
inline JSValue *ConvertFromLocal<JSConvertJSValue>(void *value)
{
    INTEROP_CODE_SCOPE_ETS_TO_JS(EtsCoroutine::GetCurrent());

    auto ctx = InteropCtx::Current(EtsCoroutine::GetCurrent());
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    if (IsUndefined(ctx->GetJSEnv(), ToLocal(value))) {
        return nullptr;
    }
    if (auto res = JSConvertJSValue::Unwrap(ctx, ctx->GetJSEnv(), ToLocal(value)); res.has_value()) {
        return res.value();
    }
    HandleExceptions(ctx->GetJSEnv(), ctx);
    return nullptr;
}

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTRINSICS_API_IMPL_H_
