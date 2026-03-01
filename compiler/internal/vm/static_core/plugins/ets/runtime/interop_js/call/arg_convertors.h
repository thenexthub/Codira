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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CALL_ARG_CONVERTORS_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CALL_ARG_CONVERTORS_H

#include "plugins/ets/runtime/interop_js/call/proto_reader.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"

namespace ark::ets::interop::js {

template <typename Convertor, typename FStore>
static ALWAYS_INLINE bool UnwrapVal(InteropCtx *ctx, napi_env env, napi_value jsVal, FStore &storeRes)
{
    using cpptype = typename Convertor::cpptype;  // NOLINT(readability-identifier-naming)
    auto res = Convertor::Unwrap(ctx, env, jsVal);
    if (UNLIKELY(!res.has_value())) {
        return false;
    }
    if constexpr (std::is_pointer_v<cpptype>) {
        ASSERT(res.value() != nullptr);
        storeRes(AsEtsObject(res.value())->GetCoreType());
    } else {
        storeRes(Value(res.value()).GetAsLong());
    }
    return true;
}

template <typename FStore>
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertRefArgToEts(InteropCtx *ctx, Class *klass, FStore &storeRes,
                                                                  napi_value jsVal)
{
    INTEROP_TRACE();
    auto env = ctx->GetJSEnv();

    // start fastpath
    if (IsUndefined<true>(env, jsVal)) {
        storeRes(nullptr);
        return true;
    }
    if (IsNull<true>(env, jsVal)) {
        if (LIKELY(klass->IsAssignableFrom(ctx->GetNullValueClass()))) {
            storeRes(ctx->GetNullValue()->GetCoreType());
            return true;
        }
    }
    if (klass == ctx->GetJSValueClass()) {
        return UnwrapVal<JSConvertJSValue>(ctx, env, jsVal, storeRes);
    }
    if (klass->IsStringClass()) {
        return UnwrapVal<JSConvertString>(ctx, env, jsVal, storeRes);
    }
    // start slowpath
    auto refconv = JSRefConvertResolve<true>(ctx, klass);
    if (UNLIKELY(refconv == nullptr)) {
        return false;
    }
    ObjectHeader *res = refconv->Unwrap(ctx, jsVal)->GetCoreType();
    storeRes(res);
    return res != nullptr;
}

template <typename FStore>
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertPrimArgToEts(InteropCtx *ctx, panda_file::Type::TypeId id,
                                                                   FStore &storeRes, napi_value jsVal)
{
    INTEROP_TRACE();
    auto env = ctx->GetJSEnv();

    auto unwrapVal = [&ctx, &env, &jsVal, &storeRes](auto convTag) {
        using Convertor = typename decltype(convTag)::type;  // convTag acts as lambda template parameter
        return UnwrapVal<Convertor>(ctx, env, jsVal, storeRes);
    };
    switch (id) {
        case panda_file::Type::TypeId::VOID: {
            return true;  // do nothing
        }
        case panda_file::Type::TypeId::U1:
            return unwrapVal(helpers::TypeIdentity<JSConvertU1>());
        case panda_file::Type::TypeId::I8:
            return unwrapVal(helpers::TypeIdentity<JSConvertI8>());
        case panda_file::Type::TypeId::U8:
            return unwrapVal(helpers::TypeIdentity<JSConvertU8>());
        case panda_file::Type::TypeId::I16:
            return unwrapVal(helpers::TypeIdentity<JSConvertI16>());
        case panda_file::Type::TypeId::U16:
            return unwrapVal(helpers::TypeIdentity<JSConvertU16>());
        case panda_file::Type::TypeId::I32:
            return unwrapVal(helpers::TypeIdentity<JSConvertI32>());
        case panda_file::Type::TypeId::U32:
            return unwrapVal(helpers::TypeIdentity<JSConvertU32>());
        case panda_file::Type::TypeId::I64:
            return unwrapVal(helpers::TypeIdentity<JSConvertI64>());
        case panda_file::Type::TypeId::U64:
            return unwrapVal(helpers::TypeIdentity<JSConvertU64>());
        case panda_file::Type::TypeId::F32:
            return unwrapVal(helpers::TypeIdentity<JSConvertF32>());
        case panda_file::Type::TypeId::F64:
            return unwrapVal(helpers::TypeIdentity<JSConvertF64>());
        default:
            UNREACHABLE();
    }
}

template <typename FStore, typename GetClass>
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertArgToEts(InteropCtx *ctx, panda_file::Type type, FStore &storeRes,
                                                               const GetClass &getClass, napi_value jsVal)
{
    INTEROP_TRACE();
    auto id = type.GetId();
    if (id == panda_file::Type::TypeId::REFERENCE) {
        return ConvertRefArgToEts(ctx, getClass(), storeRes, jsVal);
    }
    return ConvertPrimArgToEts(ctx, id, storeRes, jsVal);
}

template <typename FStore>
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertArgToEts(InteropCtx *ctx, ProtoReader &protoReader,
                                                               FStore &storeRes, napi_value jsVal)
{
    return ConvertArgToEts(
        ctx, protoReader.GetType(), storeRes, [&protoReader]() { return protoReader.GetClass(); }, jsVal);
}

template <typename RestParamsArray>
static ObjectHeader **DoPackRestParameters(EtsCoroutine *coro, InteropCtx *ctx, ProtoReader &protoReader,
                                           Span<napi_value> jsargv)
{
    const size_t numRestParams = jsargv.size();

    RestParamsArray *objArr = [&]() {
        if constexpr (std::is_same_v<RestParamsArray, EtsObjectArray>) {
            EtsClass *etsClass = EtsClass::FromRuntimeClass(protoReader.GetClass()->GetComponentType());
            return RestParamsArray::Create(etsClass, numRestParams);
        } else {
            return RestParamsArray::Create(numRestParams);
        }
    }();

    auto convertValue = [](auto val) -> typename RestParamsArray::ValueType {
        constexpr bool IS_VAL_PTR = std::is_pointer_v<decltype(val)> || std::is_null_pointer_v<decltype(val)>;
        constexpr bool IS_OBJ_ARR = std::is_same_v<RestParamsArray, EtsObjectArray>;
        // Clang-tidy gives false positive error.
        if constexpr (IS_OBJ_ARR) {
            if constexpr (IS_VAL_PTR) {
                return EtsObject::FromCoreType(static_cast<ObjectHeader *>(val));
            }
        }
        if constexpr (!IS_OBJ_ARR) {
            if constexpr (!IS_VAL_PTR) {
                return *reinterpret_cast<typename RestParamsArray::ValueType *>(&val);
            }
        }
        UNREACHABLE();
    };

    VMHandle<RestParamsArray> restArgsArray(coro, objArr->GetCoreType());
    for (uint32_t restArgIdx = 0; restArgIdx < numRestParams; ++restArgIdx) {
        auto jsVal = jsargv[restArgIdx];
        auto store = [&convertValue, restArgIdx, &restArgsArray](auto val) {
            restArgsArray.GetPtr()->Set(restArgIdx, convertValue(val));
        };
        auto klass = protoReader.GetClass()->GetComponentType();
        auto klassCb = [klass]() { return klass; };
        if (UNLIKELY(!ConvertArgToEts(ctx, klass->GetType(), store, klassCb, jsVal))) {
            if (coro->HasPendingException()) {
                ctx->ForwardEtsException(coro);
            }
            ASSERT(ctx->SanityJSExceptionPending());
            return nullptr;
        }
    }
    return reinterpret_cast<ObjectHeader **>(restArgsArray.GetAddress());
}

// CC-OFFNXT(G.FMT.06-CPP, huge_depth) solid logic
[[maybe_unused]] static ObjectHeader **PackRestParameters(EtsCoroutine *coro, InteropCtx *ctx, ProtoReader &protoReader,
                                                          Span<napi_value> jsargv)
{
    INTEROP_TRACE();
    if (!protoReader.GetClass()->IsArrayClass()) {
        ASSERT(protoReader.GetClass() == ctx->GetArrayClass());
        const size_t numRestParams = jsargv.size();

        EtsHandle<EtsEscompatArray> restArgsArray(coro, EtsEscompatArray::Create(coro, numRestParams));
        if (UNLIKELY(restArgsArray.GetPtr() == nullptr)) {
            ASSERT(coro->HasPendingException());
            return nullptr;
        }
        for (uint32_t restArgIdx = 0; restArgIdx < numRestParams; ++restArgIdx) {
            auto jsVal = jsargv[restArgIdx];
            auto store = [restArgIdx, &restArgsArray](ObjectHeader *val) {
                restArgsArray->EscompatArraySetUnsafe(restArgIdx, EtsObject::FromCoreType(val));
            };
            if (UNLIKELY(!ConvertRefArgToEts(ctx, ctx->GetObjectClass(), store, jsVal))) {
                if (coro->HasPendingException()) {
                    ctx->ForwardEtsException(coro);
                }
                ASSERT(ctx->SanityJSExceptionPending());
                return nullptr;
            }
        }
        return reinterpret_cast<ObjectHeader **>(restArgsArray.GetAddress());
    }

    panda_file::Type restParamsItemType = protoReader.GetClass()->GetComponentType()->GetType();
    switch (restParamsItemType.GetId()) {
        case panda_file::Type::TypeId::U1:
            return DoPackRestParameters<EtsBooleanArray>(coro, ctx, protoReader, jsargv);
        case panda_file::Type::TypeId::I8:
            return DoPackRestParameters<EtsByteArray>(coro, ctx, protoReader, jsargv);
        case panda_file::Type::TypeId::I16:
            return DoPackRestParameters<EtsShortArray>(coro, ctx, protoReader, jsargv);
        case panda_file::Type::TypeId::U16:
            return DoPackRestParameters<EtsCharArray>(coro, ctx, protoReader, jsargv);
        case panda_file::Type::TypeId::I32:
            return DoPackRestParameters<EtsIntArray>(coro, ctx, protoReader, jsargv);
        case panda_file::Type::TypeId::I64:
            return DoPackRestParameters<EtsLongArray>(coro, ctx, protoReader, jsargv);
        case panda_file::Type::TypeId::F32:
            return DoPackRestParameters<EtsFloatArray>(coro, ctx, protoReader, jsargv);
        case panda_file::Type::TypeId::F64:
            return DoPackRestParameters<EtsDoubleArray>(coro, ctx, protoReader, jsargv);
        case panda_file::Type::TypeId::REFERENCE:
            return DoPackRestParameters<EtsObjectArray>(coro, ctx, protoReader, jsargv);
        default:
            UNREACHABLE();
    }
}

template <typename FRead>
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertRefArgToJS(InteropCtx *ctx, napi_value *resSlot, FRead &readVal)
{
    INTEROP_TRACE();
    ASSERT(ctx != nullptr);
    auto env = ctx->GetJSEnv();
    auto setResult = [resSlot](napi_value res) {
        *resSlot = res;
        return res != nullptr;
    };
    auto wrapRef = [&env, setResult](auto convTag, ObjectHeader *ref) -> bool {
        using Convertor = typename decltype(convTag)::type;  // conv_tag acts as lambda template parameter
        using cpptype = typename Convertor::cpptype;         // NOLINT(readability-identifier-naming)
        cpptype value = std::remove_pointer_t<cpptype>::FromEtsObject(EtsObject::FromCoreType(ref));
        return setResult(Convertor::Wrap(env, value));
    };

    ObjectHeader *ref = readVal(helpers::TypeIdentity<ObjectHeader *>());
    if (UNLIKELY(ref == nullptr)) {
        *resSlot = GetUndefined(env);
        return true;
    }
    if (UNLIKELY(ref == ctx->GetNullValue()->GetCoreType())) {
        *resSlot = GetNull(env);
        return true;
    }

    auto klass = ref->template ClassAddr<Class>();
    // start fastpath
    if (klass == ctx->GetJSValueClass()) {
        return wrapRef(helpers::TypeIdentity<JSConvertJSValue>(), ref);
    }
    if (klass->IsStringClass()) {
        return wrapRef(helpers::TypeIdentity<JSConvertString>(), ref);
    }
    // start slowpath
    VMHandle<ObjectHeader> handle(EtsCoroutine::GetCurrent(), ref);
    auto refconv = JSRefConvertResolve(ctx, klass);
    if (refconv == nullptr) {
        return false;
    }
    return setResult(refconv->Wrap(ctx, EtsObject::FromCoreType(handle.GetPtr())));
}

template <typename FRead>
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertArgToJS(InteropCtx *ctx, ProtoReader &protoReader,
                                                              napi_value *resSlot, FRead &readVal)
{
    INTEROP_TRACE();
    auto env = ctx->GetJSEnv();

    auto wrapPrim = [&env, &readVal, resSlot](auto convTag) -> bool {
        INTEROP_TRACE();
        using Convertor = typename decltype(convTag)::type;  // convTag acts as lambda template parameter
        using cpptype = typename Convertor::cpptype;         // NOLINT(readability-identifier-naming)
        napi_value res = Convertor::Wrap(env, readVal(helpers::TypeIdentity<cpptype>()));
        *resSlot = res;
        return res != nullptr;
    };

    switch (protoReader.GetType().GetId()) {
        case panda_file::Type::TypeId::VOID: {
            *resSlot = GetUndefined(env);
            return true;
        }
        case panda_file::Type::TypeId::U1:
            return wrapPrim(helpers::TypeIdentity<JSConvertU1>());
        case panda_file::Type::TypeId::I8:
            return wrapPrim(helpers::TypeIdentity<JSConvertI8>());
        case panda_file::Type::TypeId::U8:
            return wrapPrim(helpers::TypeIdentity<JSConvertU8>());
        case panda_file::Type::TypeId::I16:
            return wrapPrim(helpers::TypeIdentity<JSConvertI16>());
        case panda_file::Type::TypeId::U16:
            return wrapPrim(helpers::TypeIdentity<JSConvertU16>());
        case panda_file::Type::TypeId::I32:
            return wrapPrim(helpers::TypeIdentity<JSConvertI32>());
        case panda_file::Type::TypeId::U32:
            return wrapPrim(helpers::TypeIdentity<JSConvertU32>());
        case panda_file::Type::TypeId::I64:
            return wrapPrim(helpers::TypeIdentity<JSConvertI64>());
        case panda_file::Type::TypeId::U64:
            return wrapPrim(helpers::TypeIdentity<JSConvertU64>());
        case panda_file::Type::TypeId::F32:
            return wrapPrim(helpers::TypeIdentity<JSConvertF32>());
        case panda_file::Type::TypeId::F64:
            return wrapPrim(helpers::TypeIdentity<JSConvertF64>());
        case panda_file::Type::TypeId::REFERENCE:
            return ConvertRefArgToJS(ctx, resSlot, readVal);
        default:
            UNREACHABLE();
    }
}

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CALL_ARG_CONVERTORS_H
