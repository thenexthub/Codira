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

#include "plugins/ets/runtime/interop_js/xref_object_operator.h"

#include <string>

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::interop::js {

using JSConvertEtsObject = interop::js::JSConvertEtsObject;

XRefObjectOperator::XRefObjectOperator(EtsHandle<EtsObject> &etsObject) : etsObject_(etsObject) {}

XRefObjectOperator XRefObjectOperator::FromEtsObject(EtsHandle<EtsObject> &etsObject)
{
    ASSERT(etsObject.GetPtr() == nullptr || etsObject->GetClass()->GetRuntimeClass()->IsXRefClass());
    return XRefObjectOperator(etsObject);
}

EtsObject *XRefObjectOperator::GetProperty(EtsCoroutine *coro, const PandaString &name) const
{
    INTEROP_TRACE();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    napi_value jsThis = this->GetNapiValue(coro);
    TaggedType *resultTaggedType = nullptr;
    {
        ScopedNativeCodeThread nativeScope(coro);
        resultTaggedType =
            common::DynamicObjectAccessorUtil::GetProperty(ArkNapiHelper::ToBaseObject(jsThis), name.c_str());
    }
    if (NapiIsExceptionPending(env)) {
        ctx->ForwardJSException(coro);
        return {};
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, ArkNapiHelper::ToNapiValue(resultTaggedType)).value();
}

EtsObject *XRefObjectOperator::GetProperty(EtsCoroutine *coro, EtsHandle<EtsObject> &keyObject) const
{
    INTEROP_TRACE();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsKey = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, keyObject);
    napi_value jsVal = nullptr;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_get_property(env, jsThis, jsKey, &jsVal);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return {};
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, jsVal).value();
}

EtsObject *XRefObjectOperator::GetProperty(EtsCoroutine *coro, const uint32_t index) const
{
    INTEROP_TRACE();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsVal = nullptr;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_get_element(env, jsThis, index, &jsVal);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return {};
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, jsVal).value();
}

bool XRefObjectOperator::SetProperty(EtsCoroutine *coro, const std::string &name,
                                     EtsHandle<EtsObject> &valueObject) const
{
    INTEROP_TRACE();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    ASSERT_MANAGED_CODE();
    napi_value jsValue = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, valueObject);
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_set_named_property(env, jsThis, name.c_str(), jsValue);
    }

    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }
    return true;
}

bool XRefObjectOperator::SetProperty(EtsCoroutine *coro, EtsHandle<EtsObject> &keyObject,
                                     EtsHandle<EtsObject> &valueObject) const
{
    INTEROP_TRACE();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_status jsStatus;
    napi_value jsKey = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, keyObject);
    napi_value jsValue = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, valueObject);
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_set_property(env, jsThis, jsKey, jsValue);
    }

    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }
    return true;
}

bool XRefObjectOperator::SetProperty(EtsCoroutine *coro, uint32_t index, EtsHandle<EtsObject> &valueObject) const
{
    INTEROP_TRACE();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsValue = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, valueObject);
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_set_element(env, jsThis, index, jsValue);
    }

    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }
    return true;
}

bool XRefObjectOperator::IsInstanceOf(EtsCoroutine *coro, const XRefObjectOperator &rhsObject) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsRhs = rhsObject.GetNapiValue(coro);
    bool isInstanceOf = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_instanceof(env, jsThis, jsRhs, &isInstanceOf);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }

    return isInstanceOf;
}

EtsObject *XRefObjectOperator::Invoke(EtsCoroutine *coro, Span<VMHandle<ObjectHeader>> args) const
{
    INTEROP_TRACE();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    napi_value jsFunc = this->GetNapiValue(coro);

    // convert static args to dynamic args
    PandaVector<TaggedType> dynamicArgs;
    for (auto &objHeader : args) {
        EtsObject *arg = EtsObject::FromCoreType(objHeader.GetPtr());
        auto dynamicArg = JSConvertEtsObject::WrapWithNullCheck(env, arg);
        if (dynamicArg == nullptr) {
            ctx->ForwardJSException(coro);
            return nullptr;
        }
        auto dynamicArgTaggedType = ArkNapiHelper::GetTaggedType(dynamicArg);
        dynamicArgs.push_back(dynamicArgTaggedType);
    }

    auto undefinedTaggedType = ArkNapiHelper::GetTaggedType(GetUndefined(env));
    auto jsFuncTaggedType = ArkNapiHelper::GetTaggedType(jsFunc);
    TaggedType *jsRetTaggedType = nullptr;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsRetTaggedType = common::DynamicObjectAccessorUtil::CallFunction(undefinedTaggedType, jsFuncTaggedType,
                                                                          dynamicArgs.size(), dynamicArgs.data());
    }
    if (NapiIsExceptionPending(env)) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, ArkNapiHelper::ToNapiValue(jsRetTaggedType)).value();
}

EtsObject *XRefObjectOperator::InvokeMethod(EtsCoroutine *coro, const std::string &name,
                                            Span<VMHandle<ObjectHeader>> args) const
{
    INTEROP_TRACE();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    napi_value jsThis = this->GetNapiValue(coro);

    // convert static args to dynamic args
    PandaVector<napi_value> dynamicArgs;
    for (auto &objHeader : args) {
        EtsObject *arg = EtsObject::FromCoreType(objHeader.GetPtr());
        auto dynamicArg = JSConvertEtsObject::WrapWithNullCheck(env, arg);
        dynamicArgs.push_back(dynamicArg);
    }

    napi_value jsMethod = nullptr;
    napi_status jsStatus;
    {
        // get js function property
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_get_named_property(env, jsThis, name.c_str(), &jsMethod);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }

    napi_value jsRet = nullptr;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_call_function(env, jsThis, jsMethod, dynamicArgs.size(), dynamicArgs.data(), &jsRet);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, jsRet).value();
}

EtsObject *XRefObjectOperator::InvokeMethod(EtsCoroutine *coro, EtsHandle<EtsObject> &methodObject,
                                            Span<VMHandle<ObjectHeader>> args) const
{
    INTEROP_TRACE();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsMethod = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, methodObject);

    // convert static args to dynamic args
    PandaVector<TaggedType> dynamicArgs;
    for (auto &objHeader : args) {
        EtsObject *arg = EtsObject::FromCoreType(objHeader.GetPtr());
        auto dynamicArg = JSConvertEtsObject::WrapWithNullCheck(env, arg);
        if (dynamicArg == nullptr) {
            ctx->ForwardJSException(coro);
            return nullptr;
        }
        auto dynamicArgTaggedType = ArkNapiHelper::GetTaggedType(dynamicArg);
        dynamicArgs.push_back(dynamicArgTaggedType);
    }

    auto jsThisTaggedType = ArkNapiHelper::GetTaggedType(jsThis);
    auto jsMethodTaggedType = ArkNapiHelper::GetTaggedType(jsMethod);
    TaggedType *jsRetTaggedType = nullptr;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsRetTaggedType = common::DynamicObjectAccessorUtil::CallFunction(jsThisTaggedType, jsMethodTaggedType,
                                                                          dynamicArgs.size(), dynamicArgs.data());
    }
    if (NapiIsExceptionPending(env)) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, ArkNapiHelper::ToNapiValue(jsRetTaggedType)).value();
}

bool XRefObjectOperator::HasProperty(EtsCoroutine *coro, const std::string &name, bool isOwnProperty) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    bool res = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        if (isOwnProperty) {
            napi_value jsKey = nullptr;
            NAPI_CHECK_FATAL(napi_create_string_utf8(env, name.c_str(), name.size(), &jsKey));

            jsStatus = napi_has_own_property(env, jsThis, jsKey, &res);
        } else {
            jsStatus = napi_has_named_property(env, jsThis, name.c_str(), &res);
        }
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }
    return res;
}

bool XRefObjectOperator::HasProperty(EtsCoroutine *coro, EtsHandle<EtsObject> &keyObject, bool isOwnProperty) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsKey = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, keyObject);
    bool res = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        if (isOwnProperty) {
            jsStatus = napi_has_own_property(env, jsThis, jsKey, &res);
        } else {
            jsStatus = napi_has_property(env, jsThis, jsKey, &res);
        }
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }
    return res;
}

bool XRefObjectOperator::HasProperty(EtsCoroutine *coro, const uint32_t index) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    bool res = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_has_element(env, jsThis, index, &res);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }
    return res;
}

EtsObject *XRefObjectOperator::Instantiate(EtsCoroutine *coro, Span<VMHandle<ObjectHeader>> args) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    napi_value jsCtor = this->GetNapiValue(coro);

    // convert static args to dynamic args
    PandaVector<napi_value> dynamicArgs;
    for (auto &objHeader : args) {
        EtsObject *arg = EtsObject::FromCoreType(objHeader.GetPtr());
        auto dynamicArg = JSConvertEtsObject::WrapWithNullCheck(env, arg);
        dynamicArgs.push_back(dynamicArg);
    }

    napi_value jsRet = nullptr;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_new_instance(env, jsCtor, dynamicArgs.size(), dynamicArgs.data(), &jsRet);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, jsRet).value();
}

napi_valuetype XRefObjectOperator::GetValueType(EtsCoroutine *coro, EtsObject *obj)
{
    // 1. If it is nullptr, then we return napi_undefined
    if (obj == nullptr) {
        return napi_undefined;
    }

    // 2. If it is a JSValue, then we use the JSValue's GetNapiValue method
    // Note(MockMockBlack, #ICQS8L): this is a workaround and will be removed
    if (obj->GetClass() == PlatformTypes(coro)->interopJSValue) {
        return JSValue::FromEtsObject(obj)->GetType();
    }

    // 3. Otherwise, we assume that it is a XRefObject
    // In this case, this object is either a function or an object.
    if (obj->GetClass()->IsFunction()) {
        return napi_function;
    }
    return napi_object;
}

std::string XRefObjectOperator::TypeOf(EtsCoroutine *coro, EtsObject *obj)
{
    napi_valuetype valueType = GetValueType(coro, obj);
    switch (valueType) {
        // NOTE(MockMockBlack): moved this code from JSValue::TypeOf
        case napi_string:
            return "string";
        case napi_undefined:
            return "undefined";
        case napi_null:
            return "object";
        case napi_number:
            return "number";
        case napi_bigint:
            return "bigint";
        case napi_function:
            return "function";
        default:
            return "object";
    }
}

bool XRefObjectOperator::IsTrue(EtsCoroutine *coro, EtsObject *obj)
{
    // 1. If it is nullptr, then we return false
    if (obj == nullptr) {
        return false;
    }

    // 2. If it is a JSValue, then we use the JSValue's IsTrue method
    // Note(MockMockBlack, #ICQS8L): this is a workaround and will be removed
    if (obj->GetClass() == PlatformTypes(coro)->interopJSValue) {
        return JSValue::FromEtsObject(obj)->IsTrue();
    }

    // 3. Otherwise, we assume that it is a XRefObject
    // In this case, this object is either a function or an object,
    // so a `true` value is returned.
    return true;
}

napi_value XRefObjectOperator::GetNapiValue(EtsCoroutine *coro) const
{
    // NOTE(MockMockBlack): will just get from shared_refenece_table
    // after jsvalue it put into shared_reference_table
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();

    // 1. If it is undefiend(nullptr in native),
    // then return an undefined value
    if (this->etsObject_.GetPtr() == nullptr) {
        return GetUndefined(env);
    }

    // 2. If it is already in shared_reference_table,
    // get it from there
    ASSERT_MANAGED_CODE();
    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    if (UNLIKELY(storage->HasReference(this->etsObject_.GetPtr(), env))) {
        auto jsObject = storage->GetJsObject(this->etsObject_.GetPtr(), env);
        return jsObject;
    }

    // 3. otherwise, we assume that it is a jsvalue object,
    // so we will treat it as a JSValue
    auto jsValueObject = JSValue::FromEtsObject(this->etsObject_.GetPtr());
    EtsHandle<JSValue> jsValueHandle(coro, jsValueObject);
    auto jsObject = JSValue::GetNapiValue(coro, ctx, jsValueHandle);
    // NOTE(MockMockBlack): will be removed after this is stable
    if (UNLIKELY(jsObject == nullptr)) {
        InteropCtx::Fatal("Failed to get NAPI value for XRefObject");
    }

    return jsObject;
}

bool XRefObjectOperator::StrictEquals(EtsCoroutine *coro, EtsObject *obj1, EtsObject *obj2)
{
    // 1. obj1 or obj2 is both nullptr,
    // so we can just return false
    if (obj1 == nullptr && obj2 == nullptr) {
        return true;
    }

    // 2. if one and only one of them is nullptr,
    // then we can just return false
    if (obj1 == nullptr || obj2 == nullptr) {
        return false;
    }

    // 3. If both of them are JSValue,
    // then we can use JSValue's StrictEquals method
    // Note(MockMockBlack, #ICQS8L): this is a workaround and will be removed
    if (obj1->GetClass() == PlatformTypes(coro)->interopJSValue &&
        obj2->GetClass() == PlatformTypes(coro)->interopJSValue) {
        return JSValue::StrictEquals(JSValue::FromEtsObject(obj1), JSValue::FromEtsObject(obj2));
    }

    // 4. Otherwise, we just compare their pointers
    return obj1 == obj2;
}

napi_value XRefObjectOperator::ConvertStaticObjectToDynamic(EtsCoroutine *coro, EtsHandle<EtsObject> &object)
{
    INTEROP_TRACE();
    auto ctx = interop::js::InteropCtx::Current(coro);

    // static undefined is nullptr in runtime
    // handle undefined case
    if (UNLIKELY(object.GetPtr() == nullptr)) {
        return interop::js::GetUndefined(ctx->GetJSEnv());
    }

    // if it a XRefObject, then we can just return its napi value
    if (object->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(object);
        return xRefObjectOperator.GetNapiValue(coro);
    }

    auto converter = JSRefConvertResolve(ctx, object->GetClass()->GetRuntimeClass());
    if (converter == nullptr) {
        return interop::js::GetUndefined(ctx->GetJSEnv());
    }
    return converter->Wrap(ctx, object.GetPtr());
}
}  // namespace ark::ets::interop::js