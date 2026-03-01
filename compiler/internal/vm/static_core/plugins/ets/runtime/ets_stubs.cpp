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

#include "libarkbase/utils/utf.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_base_enum.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"

#ifdef PANDA_ETS_INTEROP_JS
#include "plugins/ets/runtime/interop_js/intrinsics_api_impl.h"
#include "plugins/ets/runtime/interop_js/xref_object_operator.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#endif

namespace ark::ets {
#ifdef PANDA_ETS_INTEROP_JS
using JSValue = interop::js::JSValue;
using JSConvertEtsObject = interop::js::JSConvertEtsObject;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PANDA_ETS_INTEROP_JS_GUARD(code)   \
    do {                                   \
        code                               \
        /* CC-OFFNXT(G.PRE.09) code gen */ \
    } while (0);
#else
// CC-OFFNXT(G.PRE.09) code gen
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PANDA_ETS_INTEROP_JS_GUARD(code)                                     \
    do {                                                                     \
        LOG(FATAL, RUNTIME) << "Found dynamic object without interop build"; \
        UNREACHABLE();                                                       \
        /* CC-OFFNXT(G.PRE.09) code gen */                                   \
    } while (0);
#endif

template <typename T>
static std::optional<T> GetBoxedNumericValue(EtsPlatformTypes const *ptypes, EtsObject *obj)
{
    auto *cls = obj->GetClass();

    auto const getValue = [obj](auto typeId) {
        using Type = typename decltype(typeId)::type;
        return static_cast<T>(EtsBoxPrimitive<Type>::FromCoreType(obj)->GetValue());
    };

    if (cls == ptypes->coreDouble) {
        return getValue(helpers::TypeIdentity<EtsDouble>());
    }
    if (cls == ptypes->coreInt) {
        return getValue(helpers::TypeIdentity<EtsInt>());
    }

    if (cls == ptypes->coreByte) {
        return getValue(helpers::TypeIdentity<EtsByte>());
    }
    if (cls == ptypes->coreShort) {
        return getValue(helpers::TypeIdentity<EtsShort>());
    }
    if (cls == ptypes->coreLong) {
        return getValue(helpers::TypeIdentity<EtsLong>());
    }
    if (cls == ptypes->coreFloat) {
        return getValue(helpers::TypeIdentity<EtsFloat>());
    }
    if (cls == ptypes->coreChar) {
        return getValue(helpers::TypeIdentity<EtsChar>());
    }
    return std::nullopt;
}

inline EtsMethod *FindGetMethod(EtsCoroutine *coro, EtsClass *cls)
{
    auto getMethod = cls->GetInstanceMethod(GET_INDEX_METHOD, INDEXED_INT_GET_METHOD_SIGNATURE.data());
    if (getMethod == nullptr) {
        ThrowEtsMethodNotFoundException(coro, cls->GetDescriptor(), GET_INDEX_METHOD,
                                        INDEXED_INT_GET_METHOD_SIGNATURE.data());
    }
    return getMethod;
}

inline EtsMethod *FindSetMethod(EtsCoroutine *coro, EtsClass *cls)
{
    auto setMethod = cls->GetInstanceMethod(SET_INDEX_METHOD, INDEXED_INT_SET_METHOD_SIGNATURE.data());
    if (setMethod == nullptr) {
        ThrowEtsMethodNotFoundException(coro, cls->GetDescriptor(), SET_INDEX_METHOD,
                                        INDEXED_INT_SET_METHOD_SIGNATURE.data());
        return nullptr;
    }
    return setMethod;
}

inline EtsMethod *FindSetterMethod(EtsClass *cls, const char *methodName)
{
    auto setterMethod = cls->GetInstanceMethod((PandaString(SETTER_BEGIN) + methodName).c_str(), nullptr);
    if (setterMethod == nullptr) {
        return nullptr;
    }
    return setterMethod;
}

inline EtsMethod *FindGetterMethod(EtsClass *cls, const char *methodName)
{
    auto getterMethod = cls->GetInstanceMethod((PandaString(GETTER_BEGIN) + methodName).c_str(), nullptr);
    if (getterMethod == nullptr) {
        return nullptr;
    }
    return getterMethod;
}

bool EtsBigIntEquality(EtsBigInt *obj1, EtsBigInt *obj2)
{
    auto bytes1 = obj1->GetBytes();
    auto bytes2 = obj2->GetBytes();
    ASSERT(bytes1 != nullptr && bytes2 != nullptr);

    auto size1 = bytes1->GetLength();
    auto size2 = bytes2->GetLength();
    if (size1 != size2) {
        return false;
    }

    if (obj1->GetSign() != obj2->GetSign()) {
        return false;
    }

    return (std::memcmp(bytes1->GetCoreType()->GetData(), bytes2->GetCoreType()->GetData(), size1 * sizeof(EtsInt)) ==
            0);
}

template <typename BoxedType>
static bool CompareBoxedPrimitive(EtsObject *obj1, EtsObject *obj2)
{
    return EtsBoxPrimitive<BoxedType>::FromCoreType(obj1)->GetValue() ==
           EtsBoxPrimitive<BoxedType>::FromCoreType(obj2)->GetValue();
}

static bool EqualityResursionAllowed(EtsObject *obj)
{
    return !(obj->GetClass()->IsEtsEnum() || obj->GetClass()->IsFunctionReference());
}

Method *ResolveMethodIfVirtual(Class *recvRc, Method *decl)
{
    ASSERT(!decl->IsStatic());
    if (decl->IsPrivate() || decl->IsConstructor() || decl->IsFinal()) {
        return decl;
    }

    if (auto *m = recvRc->ResolveVirtualMethod(decl)) {
        return m;
    }

    LOG(FATAL, RUNTIME) << "ResolveVirtualMethod failed for method" << decl->GetName().data;
    UNREACHABLE();
}

template <class T>
static inline bool EqPrim(EtsObject *o1, EtsField *f1, EtsObject *o2, EtsField *f2)
{
    return o1->GetFieldPrimitive<T>(f1) == o2->GetFieldPrimitive<T>(f2);
}

static bool PrimitiveFieldsExactEquality(EtsObject *o1, EtsField *f1, EtsObject *o2, EtsField *f2,
                                         panda_file::Type::TypeId id)
{
    ASSERT(id != panda_file::Type::TypeId::REFERENCE);
    ASSERT(id != panda_file::Type::TypeId::TAGGED);

    switch (id) {
        case panda_file::Type::TypeId::U1:
            return EqPrim<EtsBoolean>(o1, f1, o2, f2);
        case panda_file::Type::TypeId::I8:
            return EqPrim<EtsByte>(o1, f1, o2, f2);
        case panda_file::Type::TypeId::U16:
            return EqPrim<EtsChar>(o1, f1, o2, f2);
        case panda_file::Type::TypeId::I16:
            return EqPrim<EtsShort>(o1, f1, o2, f2);
        case panda_file::Type::TypeId::I32:
            return EqPrim<EtsInt>(o1, f1, o2, f2);
        case panda_file::Type::TypeId::I64:
            return EqPrim<EtsLong>(o1, f1, o2, f2);
        case panda_file::Type::TypeId::F32:
            return EqPrim<EtsFloat>(o1, f1, o2, f2);
        case panda_file::Type::TypeId::F64:
            return EqPrim<EtsDouble>(o1, f1, o2, f2);
        case panda_file::Type::TypeId::U8:
            return EqPrim<EtsByte>(o1, f1, o2, f2);
        case panda_file::Type::TypeId::U32:
            return EqPrim<EtsInt>(o1, f1, o2, f2);
        case panda_file::Type::TypeId::U64:
            return EqPrim<EtsLong>(o1, f1, o2, f2);

        default:
            UNREACHABLE();
    }
}

// CC-OFFNXT(huge_cca_cyclomatic_complexity[C++], huge_cyclomatic_complexity[C++], huge_method[C++]) big case
bool EtsValueTypedEquals(EtsCoroutine *coro, EtsObject *obj1, EtsObject *obj2)
{
    auto cls1 = obj1->GetClass();
    auto cls2 = obj2->GetClass();
    ASSERT(cls1->IsValueTyped() && cls2->IsValueTyped());

    auto ptypes = PlatformTypes(coro);
    ASSERT(ptypes != nullptr);
    if (cls1->IsStringClass()) {
        if (UNLIKELY(cls2->IsEtsEnum())) {
            obj2 = EtsBaseEnum::FromEtsObject(obj2)->GetValue();
            cls2 = obj2->GetClass();
        }
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        return cls2->IsStringClass() && coretypes::String::Cast(obj1->GetCoreType())
                                                ->Compare(coretypes::String::Cast(obj2->GetCoreType()), ctx) == 0;
    }
    if (cls1 == ptypes->coreBoolean) {
        return cls2 == ptypes->coreBoolean && CompareBoxedPrimitive<EtsBoolean>(obj1, obj2);
    }
    if (UNLIKELY(cls1->IsBigInt())) {
        return cls2->IsBigInt() && EtsBigIntEquality(EtsBigInt::FromEtsObject(obj1), EtsBigInt::FromEtsObject(obj2));
    }
    if (cls1 == ptypes->coreLong && cls2 == ptypes->coreLong) {
        return CompareBoxedPrimitive<EtsLong>(obj1, obj2);
    }
    if (auto num1 = GetBoxedNumericValue<EtsDouble>(ptypes, obj1); num1.has_value()) {
        if (UNLIKELY(cls2->IsEtsEnum())) {
            obj2 = EtsBaseEnum::FromEtsObject(obj2)->GetValue();
        }
        auto num2 = GetBoxedNumericValue<EtsDouble>(ptypes, obj2);
        return num2.has_value() && num2.value() == num1.value();
    }
    if (cls1->IsEtsEnum()) {
        auto *value1 = EtsBaseEnum::FromEtsObject(obj1)->GetValue();
        auto *value2 = obj2;
        if (LIKELY(cls2->IsEtsEnum())) {
            value2 = EtsBaseEnum::FromEtsObject(obj2)->GetValue();
        }
        if (!EqualityResursionAllowed(value1) || !EqualityResursionAllowed(value2)) {
            return false;
        }
        return EtsReferenceEquals(coro, value1, value2);
    }
    if (cls1->IsFunctionReference()) {
        // Both sides must be FunctionReference, otherwise not equal
        if (UNLIKELY(!cls2->IsFunctionReference())) {
            return false;
        }

        const auto fnum1 = cls1->GetFieldsNumber();
        const auto fnum2 = cls2->GetFieldsNumber();
        if (fnum1 != fnum2) {
            return false;
        }
        // function or static method
        if (fnum1 == 0) {
            // No receiver: directly compare the Method* stored in TypeMetaData
            return cls1->GetTypeMetaData() == cls2->GetTypeMetaData();
        }
        // For instance method, always only have one field as guaranteed by class initialization
        ASSERT(fnum1 == 1);

        // Retrieve the declaring method pointers stored in TypeMetaData
        auto *decl1 = reinterpret_cast<Method *>(cls1->GetTypeMetaData());
        auto *decl2 = reinterpret_cast<Method *>(cls2->GetTypeMetaData());
        ASSERT(decl1 != nullptr && "Function-reference class missing Method* in TypeMetaData");
        ASSERT(decl2 != nullptr && "Function-reference class missing Method* in TypeMetaData");

        EtsField *f1 = cls1->GetFieldByIndex(0);
        EtsField *f2 = cls2->GetFieldByIndex(0);
        auto t1 = f1->GetRuntimeField()->GetType();
        auto t2 = f2->GetRuntimeField()->GetType();

        // Ensure the captured field types match, and reject TAGGED ("any") type
        const auto id1 = t1.GetId();
        const auto id2 = t2.GetId();
        if (id1 != id2 || id1 == panda_file::Type::TypeId::TAGGED) {
            return false;
        }
        if (!t1.IsReference()) {
            // Receiver is a primitive type: compare field values directly
            return PrimitiveFieldsExactEquality(obj1, f1, obj2, f2, id1);
        }

        // Receiver is a reference type
        auto *recv1 = obj1->GetFieldObject(f1);
        auto *recv2 = obj2->GetFieldObject(f2);
        // Disallow recursion for enums and nested function references
        if (!EqualityResursionAllowed(recv1) || !EqualityResursionAllowed(recv2)) {
            return false;
        }
        auto *rc1 = recv1->GetClass()->GetRuntimeClass();
        auto *rc2 = recv2->GetClass()->GetRuntimeClass();
        // Resolve the actual implementation method (handles virtual dispatch)
        Method *impl1 = ResolveMethodIfVirtual(rc1, decl1);
        Method *impl2 = ResolveMethodIfVirtual(rc2, decl2);
        if (impl1 != impl2) {
            return false;
        }

        // Finally, check equality of the receiver objects themselves
        return EtsReferenceEquals(coro, recv1, recv2);
    }

    if (cls1->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            if (UNLIKELY(!cls2->GetRuntimeClass()->IsXRefClass())) {
                return false;
            }
            return interop::js::XRefObjectOperator::StrictEquals(coro, obj1, obj2);
        });
    }
    UNREACHABLE();
}

[[maybe_unused]] static bool DbgIsBoxedClass(EtsClass *cls)
{
    switch (cls->GetBoxedType()) {
        case EtsClass::BoxedType::BOOLEAN:
        case EtsClass::BoxedType::BYTE:
        case EtsClass::BoxedType::CHAR:
        case EtsClass::BoxedType::SHORT:
        case EtsClass::BoxedType::INT:
        case EtsClass::BoxedType::LONG:
        case EtsClass::BoxedType::FLOAT:
        case EtsClass::BoxedType::DOUBLE: {
            return true;
        }
        default: {
            return false;
        }
    }
}

static EtsString *CreateStringFromBoxedClass(EtsClass *cls)
{
    switch (cls->GetBoxedType()) {
        case EtsClass::BoxedType::BOOLEAN: {
            return EtsString::CreateFromMUtf8("boolean");
        }
        case EtsClass::BoxedType::BYTE: {
            return EtsString::CreateFromMUtf8("byte");
        }
        case EtsClass::BoxedType::CHAR: {
            return EtsString::CreateFromMUtf8("char");
        }
        case EtsClass::BoxedType::SHORT: {
            return EtsString::CreateFromMUtf8("short");
        }
        case EtsClass::BoxedType::INT: {
            return EtsString::CreateFromMUtf8("int");
        }
        case EtsClass::BoxedType::LONG: {
            return EtsString::CreateFromMUtf8("long");
        }
        case EtsClass::BoxedType::FLOAT: {
            return EtsString::CreateFromMUtf8("float");
        }
        case EtsClass::BoxedType::DOUBLE: {
            return EtsString::CreateFromMUtf8("number");
        }
        default: {
            UNREACHABLE();
        }
    }
}

EtsString *EtsGetTypeof(EtsCoroutine *coro, EtsObject *obj)
{
    // NOTE(vpukhov): #19799 use string constants
    if (obj == nullptr) {
        return EtsString::CreateFromMUtf8("undefined");
    }
    EtsClass *cls = obj->GetClass();

    // NOTE(vpukhov): re-encode subtyping flags if necessary
    if (!cls->IsValueTyped()) {
        if (cls->IsFunction()) {
            return EtsString::CreateFromMUtf8("function");
        }
        return EtsString::CreateFromMUtf8("object");
    }
    if (cls->IsNullValue()) {
        return EtsString::CreateFromMUtf8("object");
    }
    if (obj->IsStringClass()) {
        return EtsString::CreateFromMUtf8("string");
    }
    if (cls->IsBigInt()) {
        return EtsString::CreateFromMUtf8("bigint");
    }
    if (cls->IsEtsEnum()) {
        auto *value = EtsBaseEnum::FromEtsObject(obj)->GetValue();
        if (UNLIKELY(value->GetClass()->IsEtsEnum())) {
            // This situation is unexpected from language point of view. If BaseEnum object is contained
            // as value of enum, then it's treated as object
            return EtsString::CreateFromMUtf8("object");
        }
        return EtsGetTypeof(coro, EtsBaseEnum::FromEtsObject(obj)->GetValue());
    }
    if (cls->IsFunctionReference()) {
        return EtsString::CreateFromMUtf8("function");
    }
    if (cls->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD(
            { return EtsString::CreateFromMUtf8(interop::js::XRefObjectOperator::TypeOf(coro, obj).c_str()); });
    }

    ASSERT(cls->IsBoxed());
    ASSERT(DbgIsBoxedClass(cls));
    return CreateStringFromBoxedClass(cls);
}

bool EtsGetIstrue(EtsCoroutine *coro, EtsObject *obj)
{
    if (EtsReferenceNullish(coro, obj)) {
        return false;
    }
    EtsClass *cls = obj->GetClass();

    if (!cls->IsValueTyped()) {
        return true;
    }
    if (cls->IsFunctionReference()) {
        return true;
    }
    if (obj->IsStringClass()) {
        return !EtsString::FromEtsObject(obj)->IsEmpty();
    }
    if (cls->IsBigInt()) {
        return EtsBigInt::FromEtsObject(obj)->GetSign() != 0;
    }
    if (cls->IsEtsEnum()) {
        auto *value = EtsBaseEnum::FromEtsObject(obj)->GetValue();
        if (UNLIKELY(value->GetClass()->IsEtsEnum())) {
            // This situation is unexpected from language point of view. If BaseEnum object is contained
            // as value of enum, then it's treated as object
            return true;
        }
        return EtsGetIstrue(coro, value);
    }

    ASSERT(cls->IsBoxed() || cls->GetRuntimeClass()->IsXRefClass());

    auto ptypes = PlatformTypes(coro);
    if (cls == ptypes->coreBoolean) {
        return EtsBoxPrimitive<EtsBoolean>::FromCoreType(obj)->GetValue() != 0;
    }

    ASSERT(DbgIsBoxedClass(cls) || cls->GetRuntimeClass()->IsXRefClass());
    if (auto num = GetBoxedNumericValue<EtsDouble>(ptypes, obj); num.has_value()) {
        return num.value() != 0 && !std::isnan(num.value());
    }

    if (cls->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({ return interop::js::XRefObjectOperator::IsTrue(coro, obj); });
    }
    UNREACHABLE();
}

bool EtsHasPropertyByName([[maybe_unused]] EtsCoroutine *coro, EtsObject *thisObj, [[maybe_unused]] EtsString *name)
{
    PandaVector<uint8_t> tree8Buf;
    auto nameView = name->ConvertToStringView(&tree8Buf);
    PandaString str(nameView);
    if (thisObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> thisObjHandle(coro, thisObj);
            auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(thisObjHandle);
            return xRefObjectOperator.HasProperty(coro, str.c_str());
        })
    } else {
        auto fieldIndex = thisObj->GetClass()->GetFieldIndexByName(str.c_str());
        if (fieldIndex != static_cast<uint32_t>(-1)) {
            return true;
        }
        auto method = thisObj->GetClass()->GetInstanceMethod(str.c_str(), nullptr);
        if (method != nullptr) {
            return true;
        }
        return FindGetterMethod(thisObj->GetClass(), str.c_str()) != nullptr;
    }
}

bool EtsHasPropertyByIdx([[maybe_unused]] EtsCoroutine *coro, EtsObject *thisObj, [[maybe_unused]] uint32_t idx)
{
    if (thisObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> thisObjHandle(coro, thisObj);
            auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(thisObjHandle);
            return xRefObjectOperator.HasProperty(coro, idx);
        })
    } else {
        EtsMethod *method = FindGetMethod(coro, thisObj->GetClass());
        if (method == nullptr) {
            return false;
        }
        std::array args {Value(thisObj->GetCoreType()), Value(idx)};
        Value res = method->GetPandaMethod()->Invoke(coro, args.data());
        EtsObject *obj = EtsObject::FromCoreType(res.GetAs<ObjectHeader *>());
        if (coro->HasPendingException()) {
            return false;
        }
        return obj != nullptr;
    }
}

bool EtsHasOwnPropertyByName([[maybe_unused]] EtsCoroutine *coro, EtsObject *thisObj, [[maybe_unused]] EtsString *name)
{
    PandaVector<uint8_t> tree8Buf;
    auto fieldName = name->ConvertToStringView(&tree8Buf);
    PandaString str(fieldName);
    if (thisObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> thisObjHandle(coro, thisObj);
            auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(thisObjHandle);
            return xRefObjectOperator.HasProperty(coro, str.c_str(), true);
        });
    } else {
        EtsField *etsFiled = thisObj->GetClass()->GetDeclaredFieldIDByName(str.c_str());
        if (etsFiled != nullptr) {
            return true;
        }
        auto etsMethod = thisObj->GetClass()->GetDirectMethod(str.c_str());
        if (etsMethod != nullptr) {
            return true;
        }
        return FindGetterMethod(thisObj->GetClass(), str.c_str()) != nullptr;
    }
}

bool HandleJSValueHasProperty([[maybe_unused]] EtsCoroutine *coro, [[maybe_unused]] EtsObject *thisObj,
                              [[maybe_unused]] EtsObject *property, [[maybe_unused]] bool isOwn)
{
    PANDA_ETS_INTEROP_JS_GUARD({
        [[maybe_unused]] EtsHandleScope s(coro);
        EtsHandle<EtsObject> thisObjHandle(coro, thisObj);
        EtsHandle<EtsObject> propertyHandle(coro, property);
        auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(thisObjHandle);
        return xRefObjectOperator.HasProperty(coro, propertyHandle, isOwn);
    });
}

bool HandleStaticHasProperty(EtsCoroutine *coro, EtsObject *thisObj, EtsObject *property, bool isOwn)
{
    // ASSERTION. LHS is not a JSValue
    // then it is must a static object
    if (property->IsStringClass()) {
        if (isOwn) {
            return EtsHasOwnPropertyByName(coro, thisObj, EtsString::FromEtsObject(property));
        }
        PandaVector<uint8_t> tree8Buf;
        EtsString *propertyStr = EtsString::FromEtsObject(property);
        EtsHandle<EtsObject> thisObjHandle(coro, thisObj);
        return EtsHasPropertyByName(coro, thisObjHandle.GetPtr(), propertyStr);
    }
    auto unboxedValue = GetBoxedNumericValue<int64_t>(PlatformTypes(coro), property);
    if (unboxedValue.has_value()) {
        ASSERT(!isOwn);
        return EtsHasPropertyByIdx(coro, thisObj, unboxedValue.value());
    }
    if (property->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            auto ctx = interop::js::InteropCtx::Current(coro);
            auto env = ctx->GetJSEnv();
            interop::js::ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
            if (LIKELY(storage->HasReference(thisObj, env))) {
                bool res = false;
                if (isOwn) {
                    res = interop::js::JSRuntimeHasOwnPropertyObject(thisObj, property);
                } else {
                    res = interop::js::JSRuntimeHasPropertyObject(thisObj, property);
                }
                return res;
            }
        });
    }
    return false;
}

bool EtsHasPropertyByValue([[maybe_unused]] EtsCoroutine *coro, EtsObject *thisObj,
                           [[maybe_unused]] EtsObject *property, bool isOwn)
{
    if (thisObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        return HandleJSValueHasProperty(coro, thisObj, property, isOwn);
    }
    return HandleStaticHasProperty(coro, thisObj, property, isOwn);
}

EtsObject *EtsLdbyname(EtsCoroutine *coro, EtsObject *thisObj, panda_file::File::StringData name)
{
    auto fieldName = utf::Mutf8AsCString(name.data);
    if (thisObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            INTEROP_TRACE();
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> thisObjHandle(coro, thisObj);
            auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(thisObjHandle);
            return xRefObjectOperator.GetProperty(coro, fieldName);
        });
    } else {
        auto fieldIndex = thisObj->GetClass()->GetFieldIndexByName(fieldName);
        EtsField *field = thisObj->GetClass()->GetFieldByIndex(fieldIndex);
        if (field != nullptr) {
            return GetPropertyValue(coro, thisObj, field);
        }
        auto getMethod = FindGetterMethod(thisObj->GetClass(), fieldName);
        if (getMethod != nullptr) {
            std::array args {ark::Value(thisObj->GetCoreType())};
            Value res = getMethod->GetPandaMethod()->Invoke(coro, args.data());
            EtsObject *resObj {};
            if (res.IsPrimitive()) {
                EtsType returnType = getMethod->GetReturnValueType();
                resObj = GetBoxedValue(coro, res, returnType);
            } else if (res.IsReference()) {
                resObj = EtsObject::FromCoreType(res.GetAs<ObjectHeader *>());
            } else {
                UNREACHABLE();
            }
            return resObj;
        }
        ThrowEtsFieldNotFoundException(coro, thisObj->GetClass()->GetDescriptor(), fieldName);
        return nullptr;
    }
}

bool EtsStbyname(EtsCoroutine *coro, EtsObject *thisObj, panda_file::File::StringData name, EtsObject *value)
{
    auto fieldName = utf::Mutf8AsCString(name.data);
    if (thisObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            INTEROP_TRACE();
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> thisObjHandle(coro, thisObj);
            EtsHandle<EtsObject> valueHandle(coro, value);
            auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(thisObjHandle);
            return xRefObjectOperator.SetProperty(coro, fieldName, valueHandle);
        });
    } else {
        // ASSERTION. LHS is not a JSValue
        // then it is must a static object
        auto fieldIndex = thisObj->GetClass()->GetFieldIndexByName(fieldName);
        EtsField *field = thisObj->GetClass()->GetFieldByIndex(fieldIndex);
        if (field != nullptr) {
            SetPropertyValue(coro, thisObj, field, value);
            return true;
        }

        auto setMethod = FindSetterMethod(thisObj->GetClass(), fieldName);
        if (setMethod != nullptr) {
            std::array args {ark::Value(thisObj->GetCoreType()), ark::Value(value->GetCoreType())};
            setMethod->GetPandaMethod()->Invoke(coro, args.data());
            return true;
        }
        ThrowEtsFieldNotFoundException(coro, thisObj->GetClass()->GetDescriptor(), fieldName);
        return false;
    }
}

EtsObject *EtsLdbyidx(EtsCoroutine *coro, EtsObject *thisObj, [[maybe_unused]] uint32_t index)
{
    if (thisObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            INTEROP_TRACE();
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> thisObjHandle(coro, thisObj);
            auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(thisObjHandle);
            return xRefObjectOperator.GetProperty(coro, index);
        });
    } else {
        auto getMethod = FindGetMethod(coro, thisObj->GetClass());
        if (getMethod == nullptr) {
            return nullptr;
        }
        std::array args {ark::Value(thisObj->GetCoreType()), ark::Value(index)};
        Value res = getMethod->GetPandaMethod()->Invoke(coro, args.data());
        return EtsObject::FromCoreType(res.GetAs<ObjectHeader *>());
    }
}

bool EtsStbyidx(EtsCoroutine *coro, EtsObject *thisObj, uint32_t idx, EtsObject *value)
{
    if (thisObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            INTEROP_TRACE();
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> thisObjHandle(coro, thisObj);
            EtsHandle<EtsObject> valueHandle(coro, value);
            auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(thisObjHandle);
            return xRefObjectOperator.SetProperty(coro, idx, valueHandle);
        });
    } else {
        // ASSERTION. LHS is not a JSValue
        // then it is must a static object
        auto setMethod = FindSetMethod(coro, thisObj->GetClass());
        if (setMethod == nullptr) {
            return false;
        }
        std::array args {ark::Value(thisObj->GetCoreType()), ark::Value(idx), ark::Value(value->GetCoreType())};
        setMethod->GetPandaMethod()->Invoke(coro, args.data());
        return true;
    }
    UNREACHABLE();
}

EtsObject *EtsLdbyval(EtsCoroutine *coro, EtsObject *thisObj, EtsObject *valObj)
{
    if (thisObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            INTEROP_TRACE();
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> thisObjHandle(coro, thisObj);
            auto xRefObjectThis = interop::js::XRefObjectOperator::FromEtsObject(thisObjHandle);
            if (valObj->IsStringClass()) {
                PandaVector<uint8_t> tree8Buf;
                EtsString *valObjStr = EtsString::FromEtsObject(valObj);
                auto nameView = valObjStr->ConvertToStringView(&tree8Buf);
                PandaString str(nameView);
                return xRefObjectThis.GetProperty(coro, str);
            } else {
                EtsHandle<EtsObject> valObjHandle(coro, valObj);
                return xRefObjectThis.GetProperty(coro, valObjHandle);
            }
        });
    } else {
        // ASSERTION. LHS is not a JSValue
        // then it is must a static object
        if (valObj->IsStringClass()) {
            PandaVector<uint8_t> tree8Buf;
            EtsString *valObjStr = EtsString::FromEtsObject(valObj);
            auto nameView = valObjStr->ConvertToStringView(&tree8Buf);
            PandaString str(nameView);
            panda_file::File::StringData propName = {valObjStr->GetUtf16Length(), utf::CStringAsMutf8(str.c_str())};
            return EtsLdbyname(coro, thisObj, propName);
        }
        auto unboxedValue = GetBoxedNumericValue<int64_t>(PlatformTypes(coro), valObj);
        if (unboxedValue.has_value()) {
            return EtsLdbyidx(coro, thisObj, unboxedValue.value());
        }
        if (valObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
            PANDA_ETS_INTEROP_JS_GUARD({
                auto ctx = interop::js::InteropCtx::Current(coro);
                auto env = ctx->GetJSEnv();
                interop::js::ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
                if (LIKELY(storage->HasReference(thisObj, env))) {
                    auto jsThis = storage->GetJsObject(thisObj, env);
                    return interop::js::GetPropertyObject(JSValue::Create(coro, ctx, jsThis),
                                                          JSValue::FromEtsObject(valObj));
                }
            });
        }
        ThrowEtsInvalidKey(coro, valObj->GetClass()->GetDescriptor());
        return nullptr;
    }
}

static ALWAYS_INLINE inline bool EtsStbyvalWithXRefClass([[maybe_unused]] EtsCoroutine *coro,
                                                         [[maybe_unused]] EtsObject *obj,
                                                         [[maybe_unused]] EtsObject *key,
                                                         [[maybe_unused]] EtsObject *value)
{
    PANDA_ETS_INTEROP_JS_GUARD({
        INTEROP_TRACE();
        [[maybe_unused]] EtsHandleScope s(coro);
        EtsHandle<EtsObject> objHandle(coro, obj);
        EtsHandle<EtsObject> valueHandle(coro, value);
        auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(objHandle);
        if (key->IsStringClass()) {
            PandaVector<uint8_t> tree8Buf;
            auto keyStr = EtsString::FromEtsObject(key);
            auto strObj = keyStr->IsTreeString() ? keyStr->GetTreeStringDataMUtf8(tree8Buf) : keyStr->GetDataMUtf8();
            std::string name = utf::Mutf8AsCString(strObj);
            return xRefObjectOperator.SetProperty(coro, name, valueHandle);
        }
        EtsHandle<EtsObject> keyHandle(coro, key);
        return xRefObjectOperator.SetProperty(coro, keyHandle, valueHandle);
    });
}

bool EtsStbyval(EtsCoroutine *coro, EtsObject *obj, EtsObject *key, EtsObject *value)
{
    if (obj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        return EtsStbyvalWithXRefClass(coro, obj, key, value);
    }

    // ASSERTION. LHS is not a JSValue
    // then it is must a static object
    if (key->IsStringClass()) {
        PandaVector<uint8_t> tree8Buf;
        EtsString *keyStr = EtsString::FromEtsObject(key);
        auto nameView = keyStr->ConvertToStringView(&tree8Buf);
        PandaString str(nameView);
        panda_file::File::StringData propName = {keyStr->GetUtf16Length(), utf::CStringAsMutf8(str.c_str())};
        return EtsStbyname(coro, obj, propName, value);
    }
    auto unboxedValue = GetBoxedNumericValue<int64_t>(PlatformTypes(coro), key);
    if (unboxedValue.has_value()) {
        return EtsStbyidx(coro, obj, unboxedValue.value(), value);
    }
    if (value->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            INTEROP_TRACE();
            auto ctx = interop::js::InteropCtx::Current(coro);
            auto env = ctx->GetJSEnv();
            interop::js::ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
            if (LIKELY(storage->HasReference(obj, env))) {
                auto jsThis = storage->GetJsObject(obj, env);
                return interop::js::GetPropertyObject(JSValue::Create(coro, ctx, jsThis),
                                                      JSValue::FromEtsObject(value));
            }
        });
    }
    ThrowEtsInvalidKey(coro, key->GetClass()->GetDescriptor());
    return false;
}

bool EtsIsinstance([[maybe_unused]] EtsCoroutine *coro, EtsObject *lhsObj, EtsObject *rhsObj)
{
    if (lhsObj->GetClass()->GetRuntimeClass()->IsXRefClass() && rhsObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> lhsObjHandle(coro, lhsObj);
            EtsHandle<EtsObject> rhsObjHandle(coro, rhsObj);
            auto lhsXrefObjOperator = interop::js::XRefObjectOperator::FromEtsObject(lhsObjHandle);
            auto rhsXrefObjOperator = interop::js::XRefObjectOperator::FromEtsObject(rhsObjHandle);
            return lhsXrefObjOperator.IsInstanceOf(coro, rhsXrefObjOperator);
        });
    }
    return false;
}

EtsObject *EtsCall([[maybe_unused]] EtsCoroutine *coro, EtsObject *funcObj,
                   [[maybe_unused]] Span<VMHandle<ObjectHeader>> args)
{
    if (funcObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            INTEROP_TRACE();
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> funcObjHandle(coro, funcObj);
            auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(funcObjHandle);
            return xRefObjectOperator.Invoke(coro, args);
        });
    } else {
        if (!funcObj->GetClass()->IsFunction()) {
            ThrowEtsInvalidType(coro, funcObj->GetClass()->GetDescriptor());
            return nullptr;
        }
        constexpr size_t UNSAFE_CALL_PARAM_COUNT = 2;
        auto method = funcObj->GetClass()->GetInstanceMethod("unsafeCall", nullptr);
        EtsHandle<EtsObject> funcObjHandle(coro, funcObj);
        auto restParam = EtsObjectArray::Create(PlatformTypes(coro)->coreObject, args.size());
        for (size_t i = 0; i < args.size(); i++) {
            restParam->Set(i, EtsObject::FromCoreType(args[i].GetPtr()));
        }
        std::array<ark::Value, UNSAFE_CALL_PARAM_COUNT> realArgs {Value(funcObjHandle.GetPtr()->GetCoreType()),
                                                                  Value(restParam->GetCoreType())};
        ark::Value res = method->GetPandaMethod()->Invoke(coro, realArgs.data());
        EtsObject *resObj {};
        if (res.IsPrimitive()) {
            EtsType returnType = method->GetReturnValueType();
            resObj = GetBoxedValue(coro, res, returnType);
        } else if (res.IsReference()) {
            resObj = EtsObject::FromCoreType(res.GetAs<ObjectHeader *>());
        } else {
            UNREACHABLE();
        }
        return resObj;
    }
}

EtsObject *EtsCallThis(EtsCoroutine *coro, EtsObject *thisObj, [[maybe_unused]] panda_file::File::StringData name,
                       [[maybe_unused]] Span<VMHandle<ObjectHeader>> args)
{
    [[maybe_unused]] auto fieldName = utf::Mutf8AsCString(name.data);
    if (thisObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            INTEROP_TRACE();
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> thisObjHandle(coro, thisObj);
            auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(thisObjHandle);
            std::string methodName = utf::Mutf8AsCString(name.data);
            return xRefObjectOperator.InvokeMethod(coro, methodName, args);
        });
    } else {
        // Not supported yet, need rethink for overload case
        ThrowEtsInvalidType(coro, thisObj->GetClass()->GetDescriptor());
        return nullptr;
    }
}

EtsObject *EtsCallThis(EtsCoroutine *coro, EtsObject *thisObj, [[maybe_unused]] EtsObject *funcObj,
                       [[maybe_unused]] Span<VMHandle<ObjectHeader>> args)
{
    if (thisObj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            INTEROP_TRACE();
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> thisObjHandle(coro, thisObj);
            EtsHandle<EtsObject> funcObjHandle(coro, funcObj);
            auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(thisObjHandle);
            return xRefObjectOperator.InvokeMethod(coro, funcObjHandle, args);
        });
    } else {
        // Not supported yet, need rethink for overload case
        ThrowEtsInvalidType(coro, thisObj->GetClass()->GetDescriptor());
        return nullptr;
    }
}

EtsObject *EtsCallNew([[maybe_unused]] EtsCoroutine *coro, EtsObject *ctor,
                      [[maybe_unused]] Span<VMHandle<ObjectHeader>> args)
{
    if (ctor->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        PANDA_ETS_INTEROP_JS_GUARD({
            INTEROP_TRACE();
            [[maybe_unused]] EtsHandleScope s(coro);
            EtsHandle<EtsObject> ctorHandle(coro, ctor);
            auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(ctorHandle);
            return xRefObjectOperator.Instantiate(coro, args);
        });
    } else {
        ThrowEtsInvalidType(coro, ctor->GetClass()->GetDescriptor());
        return nullptr;
    }
}
}  // namespace ark::ets
