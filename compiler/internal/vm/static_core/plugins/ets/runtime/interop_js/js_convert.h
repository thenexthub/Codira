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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_H

#include "plugins/ets/runtime/interop_js/js_convert_base.h"
#include "plugins/ets/runtime/interop_js/js_convert_stdlib.h"
#include "plugins/ets/runtime/interop_js/napi_impl/ark_napi_helper.h"
#include "common_interfaces/objects/dynamic_object_accessor_util.h"

namespace ark::ets::interop::js {

template <typename Cpptype>
struct JSConvertNumeric : public JSConvertBase<JSConvertNumeric<Cpptype>, Cpptype> {
    static constexpr const char *TYPE_NAME = "number";

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_integral_v<P>, napi_value> WrapImpl(napi_env env, Cpptype etsVal)
    {
        napi_value jsVal;
        ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
        if constexpr (sizeof(Cpptype) >= sizeof(int32_t)) {
            NAPI_CHECK_FATAL(napi_create_int64(env, etsVal, &jsVal));
        } else if constexpr (std::is_signed_v<Cpptype>) {
            NAPI_CHECK_FATAL(napi_create_int32(env, etsVal, &jsVal));
        } else {
            NAPI_CHECK_FATAL(napi_create_uint32(env, etsVal, &jsVal));
        }
        return jsVal;
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_integral_v<P>, std::optional<Cpptype>> UnwrapImpl([[maybe_unused]] InteropCtx *ctx,
                                                                                      napi_env env, napi_value jsVal)
    {
        ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
        napi_valuetype valueType = GetValueType(env, jsVal);
        napi_value result = jsVal;
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
            JSConvertNumeric::TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_number)) {
            JSConvertNumeric::TypeCheckFailed();
            return {};
        }
        Cpptype etsVal;
        if constexpr (sizeof(Cpptype) >= sizeof(int32_t)) {
            int64_t val;
            NAPI_CHECK_FATAL(napi_get_value_int64(env, result, &val));
            etsVal = static_cast<Cpptype>(val);
        } else if constexpr (std::is_signed_v<Cpptype>) {
            int32_t val;
            NAPI_CHECK_FATAL(napi_get_value_int32(env, result, &val));
            etsVal = static_cast<Cpptype>(val);
        } else {
            uint32_t val;
            NAPI_CHECK_FATAL(napi_get_value_uint32(env, result, &val));
            etsVal = static_cast<Cpptype>(val);
        }
        return etsVal;
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_floating_point_v<P>, napi_value> WrapImpl(napi_env env, Cpptype etsVal)
    {
        napi_value jsVal;
        ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
        NAPI_CHECK_FATAL(napi_create_double(env, etsVal, &jsVal));
        return jsVal;
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_floating_point_v<P>, std::optional<Cpptype>> UnwrapImpl(
        [[maybe_unused]] InteropCtx *ctx, napi_env env, napi_value jsVal)
    {
        ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
        napi_valuetype valueType = GetValueType(env, jsVal);
        napi_value result = jsVal;
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
            JSConvertNumeric::TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_number)) {
            JSConvertNumeric::TypeCheckFailed();
            return {};
        }
        double val;
        NAPI_CHECK_FATAL(napi_get_value_double(env, result, &val));
        return val;
    }
};

using JSConvertI8 = JSConvertNumeric<int8_t>;
using JSConvertU8 = JSConvertNumeric<uint8_t>;
using JSConvertI16 = JSConvertNumeric<int16_t>;
using JSConvertI32 = JSConvertNumeric<int32_t>;
using JSConvertU32 = JSConvertNumeric<uint32_t>;
using JSConvertI64 = JSConvertNumeric<int64_t>;
using JSConvertU64 = JSConvertNumeric<uint64_t>;
using JSConvertF32 = JSConvertNumeric<float>;
using JSConvertF64 = JSConvertNumeric<double>;

JSCONVERT_DEFINE_TYPE(U1, bool);
JSCONVERT_WRAP(U1)
{
    napi_value jsVal;
    NAPI_CHECK_FATAL(napi_get_boolean(env, static_cast<bool>(etsVal), &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(U1)
{
    if (IsUndefined<true>(env, jsVal)) {
        TypeCheckFailed();
        return {};
    }
    auto objVal = JSConvertStdlibBoolean::UnwrapWithNullCheck(ctx, env, jsVal);
    if (!objVal || (objVal.has_value() && objVal.value() == nullptr)) {
        return {};
    }
    if (objVal.has_value()) {
        return EtsBoxPrimitive<EtsBoolean>::FromCoreType(objVal.value())->GetValue();
    }
    return {};
}

JSCONVERT_DEFINE_TYPE(U16, char16_t);
JSCONVERT_WRAP(U16)
{
    napi_value jsVal;
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    NAPI_CHECK_FATAL(napi_create_string_utf16(env, &etsVal, 1, &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(U16)
{
    if (IsUndefined<true>(env, jsVal)) {
        TypeCheckFailed();
        return {};
    }
    auto objVal = JSConvertStdlibChar::UnwrapWithNullCheck(ctx, env, jsVal);
    if (!objVal || (objVal.has_value() && objVal.value() == nullptr)) {
        return {};
    }
    if (objVal.has_value()) {
        return EtsBoxPrimitive<EtsChar>::FromCoreType(objVal.value())->GetValue();
    }
    return {};
}

JSCONVERT_DEFINE_TYPE(String, EtsString *);
JSCONVERT_WRAP(String)
{
    PandaVector<uint8_t> tree8Buf;
    PandaVector<uint16_t> tree16Buf;
    napi_value jsVal;
    if (UNLIKELY(etsVal->IsUtf16())) {
        auto str = reinterpret_cast<char16_t *>(etsVal->IsTreeString() ? etsVal->GetTreeStringDataUtf16(tree16Buf)
                                                                       : etsVal->GetDataUtf16());
        auto length = etsVal->GetUtf16Length();
        ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
        NAPI_CHECK_FATAL(napi_create_string_utf16(env, str, length, &jsVal));
    } else {
        auto str = utf::Mutf8AsCString(etsVal->IsTreeString() ? etsVal->GetTreeStringDataMUtf8(tree8Buf)
                                                              : etsVal->GetDataMUtf8());
        // -1 for NULL terminated Mutf8 string!!!
        auto length = etsVal->GetMUtf8Length();
        ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
        NAPI_CHECK_FATAL(napi_create_string_utf8(env, str, length - 1, &jsVal));
    }
    return jsVal;
}
JSCONVERT_UNWRAP(String)
{
    std::string value;
    {
        ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
        napi_value result = jsVal;
        napi_valuetype valueType = GetValueType(env, jsVal);
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_STRING, &result)) {
            TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_string)) {
            TypeCheckFailed();
            return {};
        }
        value = GetString(env, result);
    }
    return EtsString::CreateFromUtf8(value.data(), value.length());
}

JSCONVERT_DEFINE_TYPE(BigInt, EtsBigInt *);
JSCONVERT_WRAP(BigInt)
{
    auto size = etsVal->GetBytes()->GetLength();
    auto data = etsVal->GetBytes();

    std::vector<uint32_t> etsArray;
    etsArray.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        etsArray.emplace_back(static_cast<uint32_t>(data->Get(i)));
    }

    SmallVector<uint64_t, 4U> jsArray;
    if (size > 0) {
        jsArray = ConvertBigIntArrayFromEtsToJs(etsArray);
    } else {
        jsArray = {0};
    }

    napi_value jsVal;
    int sign = etsVal->GetSign() < 0 ? 1 : 0;
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    NAPI_CHECK_FATAL(napi_create_bigint_words(env, sign, jsArray.size(), jsArray.data(), &jsVal));

    return jsVal;
}
JSCONVERT_UNWRAP(BigInt)
{
    if (UNLIKELY(GetValueType<true>(env, jsVal) != napi_bigint)) {
        TypeCheckFailed();
        return {};
    }

    auto [words, signBit] = GetBigInt(env, jsVal);
    std::vector<EtsInt> array = ConvertBigIntArrayFromJsToEts(words);

    auto *etsIntArray = EtsIntArray::Create(array.size());
    ASSERT(etsIntArray != nullptr);
    for (uint32_t i = 0; i < array.size(); ++i) {
        etsIntArray->Set(i, array[i]);
    }

    auto coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle etsIntArrayHandle(coro, etsIntArray);

    auto bigintKlass = EtsClass::FromRuntimeClass(ctx->GetBigIntClass());
    auto bigInt = EtsBigInt::FromEtsObject(EtsObject::Create(bigintKlass));
    bigInt->SetFieldObject(EtsBigInt::GetBytesOffset(), reinterpret_cast<EtsObject *>(etsIntArrayHandle.GetPtr()));
    bigInt->SetFieldPrimitive(EtsBigInt::GetSignOffset(), array.empty() ? 0 : signBit == 0 ? 1 : -1);

    return bigInt;
}

JSCONVERT_DEFINE_TYPE(JSValue, JSValue *);
JSCONVERT_WRAP(JSValue)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }

    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<JSValue> etsValHandle(coro, etsVal);
    return JSValue::GetNapiValue(coro, ctx, etsValHandle);
}
JSCONVERT_UNWRAP(JSValue)
{
    return JSValue::Create(EtsCoroutine::GetCurrent(), ctx, jsVal);
}

JSCONVERT_DEFINE_TYPE(EtsObject, EtsObject *);
JSCONVERT_WRAP(EtsObject)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    auto objConv = JSRefConvertResolve(ctx, etsVal->GetClass()->GetRuntimeClass());
    if (objConv == nullptr) {
        ctx->ForwardEtsException(coro);
        return nullptr;
    }
    return objConv->Wrap(ctx, etsVal);
}
JSCONVERT_UNWRAP(EtsObject)
{
    auto objectConverter = ctx->GetEtsClassWrappersCache()->Lookup(PlatformTypes()->coreObject);
    return objectConverter->Unwrap(ctx, jsVal);
}

// ESError convertors are supposed to box JSValue objects, do not treat them in any other way
JSCONVERT_DEFINE_TYPE(ESError, EtsObject *);
JSCONVERT_WRAP(ESError)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }

    ASSERT_MANAGED_CODE();
    auto klass = etsVal->GetClass();
    INTEROP_FATAL_IF(klass->GetRuntimeClass() != ctx->GetESErrorClass());

    auto fieldIdx = etsVal->GetClass()->GetFieldIndexByName("err_");
    auto field = etsVal->GetClass()->GetFieldByIndex(fieldIdx);
    auto etsObject = etsVal->GetFieldObject(field);
    ASSERT(etsObject != nullptr);

    auto method = etsObject->GetClass()->GetInstanceMethod("unwrap", nullptr);
    std::array args {ark::Value(etsObject->GetCoreType())};
    Value res = method->GetPandaMethod()->Invoke(coro, args.data());
    EtsObject *errObject = EtsObject::FromCoreType(res.GetAs<ObjectHeader *>());

    interop::js::ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    if (LIKELY(storage->HasReference(errObject, env))) {
        auto jsThis = storage->GetJsObject(errObject, env);
        return jsThis;
    }
    return JSConvertEtsObject::WrapWithNullCheck(env, errObject);

    UNREACHABLE();
}
JSCONVERT_UNWRAP(ESError)
{
    auto coro = EtsCoroutine::GetCurrent();
    EtsObject *etsObject = nullptr;
    bool isError = false;
    {
        ScopedNativeCodeThread nativeScope(coro);
        NAPI_CHECK_FATAL(napi_is_error(env, jsVal, &isError));
    }
    if (isError) {
        auto jsValueObj = JSValue::Create(coro, ctx, jsVal);
        etsObject = jsValueObj->AsObject();
    } else {
        etsObject = JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, jsVal).value();
    }

    if (UNLIKELY(etsObject == nullptr)) {
        return {};
    }
    auto res = ctx->CreateETSCoreESError(coro, etsObject);
    if (UNLIKELY(res == nullptr)) {
        return {};
    }
    return res;
}

JSCONVERT_DEFINE_TYPE(Promise, EtsPromise *);

JSCONVERT_WRAP(Promise)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    // SharedReferenceStorage uses object's MarkWord to store interop hash.
    // Also runtime may lock a Promise object (this operation also requires MarkWord modification).
    // Interop hash and locks cannot co-exist. That is why a separate object (EtsPromiseRef) is used for
    // mapping between js and ets Promise objects.
    // When a ets Promise object goes to JS we should get the corresponding EtsPromiseRef object.
    // So the ets Promise object should know about EtsPromiseRef which is stored in 'interopObject' field.
    EtsObject *interopObj = etsVal->GetInteropObject(coro);
    if (interopObj != nullptr && storage->HasReference(interopObj, env)) {
        return storage->GetJsObject(interopObj, env);
    }

    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<EtsPromise> hpromise(coro, etsVal);
    ASSERT(hpromise.GetPtr() != nullptr);
    napi_deferred deferred;
    napi_value jsPromise;
    NAPI_CHECK_FATAL(napi_create_promise(env, &deferred, &jsPromise));

    hpromise->Lock();
    // NOTE(alimovilya, #23064) This if should be removed. Only else branch should remain.
    if (!hpromise->IsPending() && !hpromise->IsLinked()) {  // it will never get PENDING again
        EtsHandle<EtsObject> value(coro, hpromise->GetValue(coro));
        napi_value completionValue;
        if (value.GetPtr() == nullptr) {
            completionValue = GetUndefined(env);
        } else {
            auto refconv = JSRefConvertResolve(ctx, value->GetClass()->GetRuntimeClass());
            completionValue = refconv->Wrap(ctx, value.GetPtr());
        }
        ScopedNativeCodeThread nativeScope(coro);
        if (hpromise->IsResolved()) {
            NAPI_CHECK_FATAL(napi_resolve_deferred(env, deferred, completionValue));
        } else {
            NAPI_CHECK_FATAL(napi_reject_deferred(env, deferred, completionValue));
        }
    } else {
        // connect->Invoke calls EtsPromiseSubmitCallback that acquires the mutex and checks the state again
        hpromise->Unlock();
        ASSERT_MANAGED_CODE();
        std::array<Value, 2U> args = {Value(hpromise.GetPtr()), Value(reinterpret_cast<int64_t>(deferred))};
        ctx->GetPromiseInteropConnectMethod()->Invoke(coro, args.data());
        hpromise->Lock();
    }
    EtsPromiseRef *ref = EtsPromiseRef::Create(coro);
    ref->SetTarget(coro, hpromise->AsObject());
    hpromise->SetInteropObject(coro, ref);
    EtsHandle<EtsObject> refHandle(coro, ref->AsObject());
    [[maybe_unused]] auto *sharedRef = storage->CreateETSObjectRef(ctx, refHandle, jsPromise);
    ASSERT(sharedRef != nullptr);
    hpromise->Unlock();
    return jsPromise;
}

JSCONVERT_UNWRAP(Promise)
{
#ifndef NDEBUG
    bool isPromise = false;
    NAPI_CHECK_FATAL(napi_is_promise(env, jsVal, &isPromise));
    ASSERT(isPromise);
#endif
    auto coro = EtsCoroutine::GetCurrent();
    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    ets_proxy::SharedReference *sharedRef = storage->GetReference(env, jsVal);
    if (sharedRef != nullptr) {
        auto *ref = reinterpret_cast<EtsPromiseRef *>(sharedRef->GetEtsObject());
        ASSERT(ref->GetTarget(coro) != nullptr);
        return EtsPromise::FromEtsObject(ref->GetTarget(coro));
    }

    [[maybe_unused]] EtsHandleScope s(coro);
    auto *promise = EtsPromise::Create(coro);
    EtsHandle<EtsPromise> hpromise(coro, promise);
    ASSERT(hpromise.GetPtr() != nullptr);
    EtsHandle<EtsPromiseRef> href(coro, EtsPromiseRef::Create(coro));
    href->SetTarget(coro, hpromise->AsObject());
    hpromise->SetInteropObject(coro, href.GetPtr());
    EtsHandle<EtsObject> etsObject(coro, href->AsObject());
    storage->CreateJSObjectRef(ctx, etsObject, jsVal);
    hpromise->SetLinkedPromise(coro, href->AsObject());
    EtsPromise::CreateLink(hpromise->GetLinkedPromise(coro), hpromise.GetPtr());
    return hpromise.GetPtr();
}

JSCONVERT_DEFINE_TYPE(EtsNull, EtsObject *);
JSCONVERT_WRAP(EtsNull)
{
    return GetNull(env);
}

JSCONVERT_UNWRAP(EtsNull)
{
    if (UNLIKELY(!IsNull<true>(env, jsVal))) {
        TypeCheckFailed();
        return {};
    }
    return ctx->GetNullValue();
}

#undef JSCONVERT_DEFINE_TYPE
#undef JSCONVERT_WRAP
#undef JSCONVERT_UNWRAP

template <typename T>
ALWAYS_INLINE inline std::optional<typename T::cpptype> JSValueGetByName(InteropCtx *ctx, JSValue *jsvalue,
                                                                         const char *name)
{
    auto env = ctx->GetJSEnv();
    auto coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<JSValue> jsvalueHandle(coro, jsvalue);
    NapiScope jsHandleScope(env);
    auto jsVal = JSValue::GetNapiValue(coro, ctx, jsvalueHandle);
    TaggedType *result = nullptr;
    {
        ScopedNativeCodeThread nativeScope(coro);
        result = common::DynamicObjectAccessorUtil::GetProperty(ArkNapiHelper::ToBaseObject(jsVal), name);
    }
    if (NapiIsExceptionPending(env)) {
        ctx->ForwardJSException(coro);
        return {};
    }
    return T::UnwrapWithNullCheck(ctx, env, ArkNapiHelper::ToNapiValue(result));
}

template <typename T>
ALWAYS_INLINE inline void JSValueSetByName(InteropCtx *ctx, JSValue *jsvalue, const char *name,
                                           typename T::cpptype etsPropVal)
{
    auto env = ctx->GetJSEnv();
    auto coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<JSValue> jsvalueHandle(coro, jsvalue);
    NapiScope jsHandleScope(env);
    napi_value jsPropVal = T::WrapWithNullCheck(env, etsPropVal);
    if (UNLIKELY(jsPropVal == nullptr)) {
        return;
    }
    auto jsVal = JSValue::GetNapiValue(coro, ctx, jsvalueHandle);
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_set_named_property(env, jsVal, name, jsPropVal);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
    }
}

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_H
