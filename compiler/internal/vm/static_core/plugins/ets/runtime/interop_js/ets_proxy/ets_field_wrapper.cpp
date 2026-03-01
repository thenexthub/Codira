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

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_field_wrapper.h"

#include "libarkfile/file.h"
#include "plugins/ets/runtime/ets_class_root.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/mem/local_object_handle.h"

#include "runtime/mem/vm_handle-inl.h"

namespace ark::ets::interop::js::ets_proxy {

template <bool IS_STATIC>
static EtsObject *EtsAccessorsHandleThis(EtsFieldWrapper *fieldWrapper, EtsCoroutine *coro, InteropCtx *ctx,
                                         napi_env env, napi_value jsThis)
{
    if constexpr (IS_STATIC) {
        EtsClass *etsClass = fieldWrapper->GetOwner()->GetEtsClass();
        if (UNLIKELY(!coro->GetPandaVM()->GetClassLinker()->InitializeClass(coro, etsClass))) {
            ctx->ForwardEtsException(coro);
            return nullptr;
        }
        return etsClass->AsObject();
    }

    if (UNLIKELY(IsNullOrUndefined<true>(env, jsThis))) {
        ctx->ThrowJSTypeError(env, "ets this in set accessor cannot be null or undefined");
        return nullptr;
    }

    EtsObject *etsThis = fieldWrapper->GetOwner()->UnwrapEtsProxy(ctx, jsThis);
    if (UNLIKELY(etsThis == nullptr)) {
        if (coro->HasPendingException()) {
            ctx->ForwardEtsException(coro);
        }
        return nullptr;
    }
    return etsThis;
}

template <typename FieldAccessor, bool IS_STATIC>
static napi_value EtsFieldGetter(napi_env env, napi_callback_info cinfo)
{
    size_t argc = 0;
    napi_value jsThis;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, nullptr, &jsThis, &data));
    if (UNLIKELY(argc != 0)) {
        InteropCtx::ThrowJSError(env, "getter called in wrong context");
        return napi_value {};
    }

    auto etsFieldWrapper = reinterpret_cast<EtsFieldWrapper *>(data);
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS_TO_ETS(coro);
    ScopedManagedCodeThread managedScope(coro);

    EtsObject *etsThis = EtsAccessorsHandleThis<IS_STATIC>(etsFieldWrapper, coro, ctx, env, jsThis);
    if (UNLIKELY(etsThis == nullptr)) {
        ASSERT(ctx->SanityJSExceptionPending());
        return nullptr;
    }

    napi_value res = FieldAccessor::Getter(ctx, env, etsThis, etsFieldWrapper);
    if (UNLIKELY(res == nullptr)) {
        if (coro->HasPendingException()) {
            ctx->ForwardEtsException(coro);
        }
    }
    ASSERT(res != nullptr || ctx->SanityJSExceptionPending());
    return res;
}

template <typename FieldAccessor, bool IS_STATIC>
static napi_value EtsFieldSetter(napi_env env, napi_callback_info cinfo)
{
    size_t argc = 1;
    napi_value jsValue;
    napi_value jsThis;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, &jsValue, &jsThis, &data));
    if (UNLIKELY(argc != 1)) {
        InteropCtx::ThrowJSError(env, "setter called in wrong context");
        return napi_value {};
    }

    auto etsFieldWrapper = reinterpret_cast<EtsFieldWrapper *>(data);
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS_TO_ETS(coro);
    ScopedManagedCodeThread managedScope(coro);

    EtsObject *etsThis = EtsAccessorsHandleThis<IS_STATIC>(etsFieldWrapper, coro, ctx, env, jsThis);
    if (UNLIKELY(etsThis == nullptr)) {
        ASSERT(ctx->SanityJSExceptionPending());
        return nullptr;
    }

    LocalObjectHandle<EtsObject> etsThisHandle(coro, etsThis);
    auto res = FieldAccessor::Setter(ctx, env, EtsHandle<EtsObject>(VMHandle<EtsObject>(etsThisHandle)),
                                     etsFieldWrapper, jsValue);
    if (UNLIKELY(!res)) {
        if (coro->HasPendingException()) {
            ctx->ForwardEtsException(coro);
        }
        ASSERT(ctx->SanityJSExceptionPending());
    }
    return nullptr;
}

struct EtsFieldAccessorREFERENCE {
    static napi_value Getter(InteropCtx *ctx, napi_env env, EtsObject *etsObject, EtsFieldWrapper *etsFieldWrapper)
    {
        INTEROP_TRACE();
        EtsObject *etsValue = etsObject->GetFieldObject(etsFieldWrapper->GetObjOffset());
        if (etsValue == nullptr) {
            return GetUndefined(env);
        }
        auto refconv = JSRefConvertResolve(ctx, etsValue->GetClass()->GetRuntimeClass());
        if (refconv == nullptr) {
            return nullptr;
        }
        return refconv->Wrap(ctx, etsValue);
    }

    static bool Setter(InteropCtx *ctx, napi_env env, EtsHandle<EtsObject> etsObject, EtsFieldWrapper *etsFieldWrapper,
                       napi_value jsValue)
    {
        INTEROP_TRACE();
        EtsObject *etsValue;
        if (IsUndefined<true>(env, jsValue)) {
            etsValue = nullptr;
        } else if (IsNull<true>(env, jsValue)) {
            etsValue = ctx->GetNullValue();
        } else {
            JSRefConvert *refconv = etsFieldWrapper->GetRefConvert<true>(ctx);
            if (UNLIKELY(refconv == nullptr)) {
                return false;
            }
            etsValue = refconv->Unwrap(ctx, jsValue);
            if (UNLIKELY(etsValue == nullptr)) {
                return false;
            }
        }
        ASSERT(etsObject.GetPtr() != nullptr);
        etsObject->SetFieldObject(etsFieldWrapper->GetObjOffset(), etsValue);
        return true;
    }
};

template <typename Convertor>
struct EtsFieldAccessorPRIMITIVE {
    using PrimitiveType = typename Convertor::cpptype;

    static napi_value Getter(InteropCtx * /*ctx*/, napi_env env, EtsObject *etsObject, EtsFieldWrapper *etsFieldWrapper)
    {
        INTEROP_TRACE();
        auto etsValue = etsObject->GetFieldPrimitive<PrimitiveType>(etsFieldWrapper->GetObjOffset());
        return Convertor::Wrap(env, etsValue);
    }

    // NOTE(vpukhov): elide etsObject handle
    static bool Setter(InteropCtx *ctx, napi_env env, EtsHandle<EtsObject> etsObject, EtsFieldWrapper *etsFieldWrapper,
                       napi_value jsValue)
    {
        INTEROP_TRACE();
        std::optional<PrimitiveType> etsValue = Convertor::Unwrap(ctx, env, jsValue);
        if (LIKELY(etsValue.has_value())) {
            ASSERT(etsObject.GetPtr() != nullptr);
            etsObject->SetFieldPrimitive<PrimitiveType>(etsFieldWrapper->GetObjOffset(), etsValue.value());
        }
        return etsValue.has_value();
    }
};

template <bool ALLOW_INIT>
JSRefConvert *EtsFieldWrapper::GetRefConvert(InteropCtx *ctx)
{
    if (LIKELY(lazyRefconvertLink_.IsResolved())) {
        return lazyRefconvertLink_.GetResolved();
    }

    const Field *field = lazyRefconvertLink_.GetUnresolved();
    ASSERT(field->GetTypeId() == panda_file::Type::TypeId::REFERENCE);

    const auto *pandaFile = field->GetPandaFile();
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    Class *fieldClass =
        classLinker->GetClass(*pandaFile, panda_file::FieldDataAccessor::GetTypeId(*pandaFile, field->GetFileId()),
                              ctx->LinkerCtx(), nullptr);

    JSRefConvert *refconv = JSRefConvertResolve<ALLOW_INIT>(ctx, fieldClass);
    if (UNLIKELY(refconv == nullptr)) {
        return nullptr;
    }
    lazyRefconvertLink_.Set(refconv);  // Update link
    return refconv;
}

// Explicit instantiation
template JSRefConvert *EtsFieldWrapper::GetRefConvert<false>(InteropCtx *ctx);
template JSRefConvert *EtsFieldWrapper::GetRefConvert<true>(InteropCtx *ctx);

template <bool IS_STATIC>
static napi_property_descriptor DoMakeNapiProperty(EtsFieldWrapper *wrapper)
{
    Field *field = wrapper->GetField();
    napi_property_descriptor prop {};
    prop.utf8name = utf::Mutf8AsCString(field->GetName().data);
    prop.attributes = IS_STATIC ? EtsClassWrapper::STATIC_FIELD_ATTR : EtsClassWrapper::FIELD_ATTR;
    prop.data = wrapper;

    // NOTE(vpukhov): apply the same rule to instance fields?
    ASSERT(!IS_STATIC || wrapper->GetOwner()->GetEtsClass()->GetRuntimeClass() == field->GetClass());

    auto setupAccessors = [&prop](auto accessorTag) {
        using Accessor = typename decltype(accessorTag)::type;
        prop.getter = EtsFieldGetter<Accessor, IS_STATIC>;
        prop.setter = EtsFieldSetter<Accessor, IS_STATIC>;
        return prop;
    };

    panda_file::Type type = field->GetType();
    switch (type.GetId()) {
        case panda_file::Type::TypeId::U1:
            return setupAccessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertU1>>());
        case panda_file::Type::TypeId::I8:
            return setupAccessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertI8>>());
        case panda_file::Type::TypeId::U8:
            return setupAccessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertU8>>());
        case panda_file::Type::TypeId::I16:
            return setupAccessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertI16>>());
        case panda_file::Type::TypeId::U16:
            return setupAccessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertU16>>());
        case panda_file::Type::TypeId::I32:
            return setupAccessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertI32>>());
        case panda_file::Type::TypeId::U32:
            return setupAccessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertU32>>());
        case panda_file::Type::TypeId::I64:
            return setupAccessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertI64>>());
        case panda_file::Type::TypeId::U64:
            return setupAccessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertU64>>());
        case panda_file::Type::TypeId::F32:
            return setupAccessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertF32>>());
        case panda_file::Type::TypeId::F64:
            return setupAccessors(helpers::TypeIdentity<EtsFieldAccessorPRIMITIVE<JSConvertF64>>());
        case panda_file::Type::TypeId::REFERENCE:
            return setupAccessors(helpers::TypeIdentity<EtsFieldAccessorREFERENCE>());
        default:
            InteropCtx::Fatal(std::string("DoMakeNapiProperty: unsupported typeid ") +
                              panda_file::Type::GetSignatureByTypeId(type));
    }
    UNREACHABLE();
}

napi_property_descriptor EtsFieldWrapper::MakeInstanceProperty(EtsClassWrapper *owner, Field *field)
{
    new (this) EtsFieldWrapper(owner, field);
    /*IS_STATIC=false*/
    return DoMakeNapiProperty<false>(this);
}

napi_property_descriptor EtsFieldWrapper::MakeStaticProperty(EtsClassWrapper *owner, Field *field)
{
    new (this) EtsFieldWrapper(owner, field);
    /*IS_STATIC=true*/
    return DoMakeNapiProperty<true>(this);
}

}  // namespace ark::ets::interop::js::ets_proxy
