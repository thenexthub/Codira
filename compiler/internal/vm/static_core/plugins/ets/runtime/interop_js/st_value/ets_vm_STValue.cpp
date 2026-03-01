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

#include <node_api.h>
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include "ets_coroutine.h"
#include "include/mem/panda_containers.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm_api.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_proxy.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"

#include "compiler_options.h"
#include "compiler/compiler_logger.h"
#include "interop_js/napi_impl/napi_impl.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "runtime/include/runtime.h"

#include "plugins/ets/runtime/interop_js/st_value/ets_vm_STValue.h"
#include "plugins/ets/runtime/interop_js/st_value/ets_vm_STValue_impl.h"
#include "plugins/ets/runtime/interop_js/interop_context_api.h"

// NOLINTBEGIN(readability-identifier-naming, readability-redundant-declaration)
// CC-OFFNXT(G.FMT.10-CPP) project code style
napi_status __attribute__((weak)) napi_define_properties(napi_env env, napi_value object, size_t property_count,
                                                         const napi_property_descriptor *properties);
// NOLINTEND(readability-identifier-naming, readability-redundant-declaration)

namespace ark::ets::interop::js {
void AniExpectOK(ani_status status)
{
    if (status != ANI_OK) {
        INTEROP_LOG(FATAL) << "The ANI call must succeed, but got status: " << status;
    }
}

void STValueThrowJSError(napi_env env, const std::string &msg)
{
    INTEROP_LOG(INFO) << "ThrowJSError: " << msg;
    ASSERT(!NapiIsExceptionPending(env));
    NAPI_CHECK_FATAL(napi_throw_error(env, nullptr, msg.c_str()));
}

static std::string GetErrorMessage(ani_env *aniEnv, ani_error aniError)
{
    std::string propertyValue;
    ani_type errorType = nullptr;
    const char *property = "message";
    AniExpectOK(aniEnv->Object_GetType(aniError, &errorType));

    ani_method getterMethod = nullptr;
    AniExpectOK(aniEnv->Class_FindGetter(static_cast<ani_class>(errorType), property, &getterMethod));

    ani_ref aniRef = nullptr;
    AniExpectOK(aniEnv->Object_CallMethod_Ref(aniError, getterMethod, &aniRef));

    auto aniString = reinterpret_cast<ani_string>(aniRef);
    ani_size sz {};
    AniExpectOK(aniEnv->String_GetUTF8Size(aniString, &sz));

    propertyValue.resize(sz + 1);
    AniExpectOK(aniEnv->String_GetUTF8SubString(aniString, 0, sz, propertyValue.data(), propertyValue.size(), &sz));

    propertyValue.resize(sz);
    return propertyValue;
}

bool AniCheckAndThrowToDynamic(napi_env env, ani_status status)
{
    if (status == ANI_OK) {
        return true;
    }

    ani_env *aniEnv = GetAniEnv();
    ani_boolean hasError = false;
    AniExpectOK(aniEnv->ExistUnhandledError(&hasError));
    if (!hasError) {
        STValueThrowJSError(env, "Unknown ANI error occurred, status: " + std::to_string(status));
        return false;
    }

    ani_error error = nullptr;
    AniExpectOK(aniEnv->GetUnhandledError(&error));
    AniExpectOK(aniEnv->ResetError());

    std::string errorDescription = GetErrorMessage(aniEnv, error);
    std::string errorMessage = "ANI error occurred, got error: " + errorDescription;
    STValueThrowJSError(env, errorMessage);
    return false;
}

bool AniCheckAndThrowToDynamic(napi_env env, ani_status status, const std::string &errorMsg)
{
    if (status == ANI_OK) {
        return true;
    }

    ani_env *aniEnv = GetAniEnv();
    ani_boolean hasError = false;
    AniExpectOK(aniEnv->ExistUnhandledError(&hasError));
    if (!hasError) {
        STValueThrowJSError(env, errorMsg);
        return false;
    }

    // if error exists, clear it and throw to js
    ani_error error = nullptr;
    AniExpectOK(aniEnv->GetUnhandledError(&error));
    AniExpectOK(aniEnv->ResetError());
    STValueThrowJSError(env, errorMsg);
    return false;
}

STValueData::STValueData([[maybe_unused]] napi_env env, ani_ref refData)
{
    // create a global reference to keep the object alive
    auto *aniEnv = GetAniEnv();

    ani_ref globalRef {nullptr};
    AniCheckAndThrowToDynamic(env, aniEnv->GlobalReference_Create(refData, &globalRef));

    this->dataRef = globalRef;
}

STValueData::STValueData([[maybe_unused]] napi_env env, ani_boolean booleanData)
{
    dataRef = booleanData;
}

STValueData::STValueData([[maybe_unused]] napi_env env, ani_char charData)
{
    dataRef = charData;
}

STValueData::STValueData([[maybe_unused]] napi_env env, ani_byte byteData)
{
    dataRef = byteData;
}

STValueData::STValueData([[maybe_unused]] napi_env env, ani_short shortData)
{
    dataRef = shortData;
}

STValueData::STValueData([[maybe_unused]] napi_env env, ani_int intData)
{
    dataRef = intData;
}

STValueData::STValueData([[maybe_unused]] napi_env env, ani_long longData)
{
    dataRef = longData;
}

STValueData::STValueData([[maybe_unused]] napi_env env, ani_float floatData)
{
    dataRef = floatData;
}

STValueData::STValueData([[maybe_unused]] napi_env env, ani_double doubleData)
{
    dataRef = doubleData;
}

STValueData::~STValueData()
{
    if (IsAniRef()) {
        auto dataReference = this->GetAniRef();
        auto *aniEnv = GetAniEnv<false>();
        if (aniEnv == nullptr) {
            return;
        }
        [[maybe_unused]] ani_status status = aniEnv->GlobalReference_Delete(dataReference);
        ASSERT(status == ANI_OK);
    }
}

bool STValueData::IsAniNullOrUndefined(napi_env env) const
{
    auto *aniEnv = GetAniEnv();
    ani_boolean isNullOrUndefined = ANI_FALSE;
    AniCheckAndThrowToDynamic(env, aniEnv->Reference_IsNullishValue(this->GetAniRef(), &isNullOrUndefined));
    return isNullOrUndefined == ANI_TRUE;
}

uintptr_t GetSTValueDataPtr(napi_env env, napi_value jsSTValue)
{
    if (!IsSTValueInstance(env, jsSTValue)) {
        return false;
    }

    napi_value stvalueDataPtrLow {};
    napi_value stvalueDataPtrHigh {};
    NAPI_CHECK_FATAL(napi_get_named_property(env, jsSTValue, STVALUE_DATA_PTR_LOW, &stvalueDataPtrLow));
    NAPI_CHECK_FATAL(napi_get_named_property(env, jsSTValue, STVALUE_DATA_PTR_HIGH, &stvalueDataPtrHigh));

    uint32_t low = 0;
    uint32_t high = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, stvalueDataPtrLow, &low));
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, stvalueDataPtrHigh, &high));

    uint64_t ptr = (static_cast<uint64_t>(static_cast<uint32_t>(high)) << UINT32_BIT_SHIFT) |
                   static_cast<uint64_t>(static_cast<uint32_t>(low));
    return static_cast<uintptr_t>(ptr);
}

bool GetAniValueFromSTValue(napi_env env, napi_value element, ani_value &value)
{
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, element));
    if (data == nullptr) {
        return false;
    }

    if (data->IsAniRef()) {
        value.r = data->GetAniRef();
    } else if (data->IsAniBoolean()) {
        value.z = data->GetAniBoolean();
    } else if (data->IsAniChar()) {
        value.c = data->GetAniChar();
    } else if (data->IsAniByte()) {
        value.b = data->GetAniByte();
    } else if (data->IsAniShort()) {
        value.s = data->GetAniShort();
    } else if (data->IsAniInt()) {
        value.i = data->GetAniInt();
    } else if (data->IsAniLong()) {
        value.l = data->GetAniLong();
    } else if (data->IsAniFloat()) {
        value.f = data->GetAniFloat();
    } else if (data->IsAniDouble()) {
        value.d = data->GetAniDouble();
    } else {
        return false;
    }
    return true;
}

bool CheckNapiIsArray(napi_env env, napi_value jsObject)
{
    bool isArray = false;
    NAPI_CHECK_FATAL(napi_is_array(env, jsObject, &isArray));
    if (!isArray) {
        STValueThrowJSError(env, "arg list is not an Array.");
        return false;
    }
    return true;
}

SType GetTypeFromType(napi_env env, napi_value stNapiType)
{
    uint32_t sType;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, stNapiType, &sType));
    return static_cast<SType>(sType);
}

static void STValueDataFinilizer([[maybe_unused]] napi_env env, void *nativeObject,
                                 [[maybe_unused]] void * /*finalize_hint*/)
{
    auto *data = reinterpret_cast<STValueData *>(nativeObject);
    delete data;
}

static napi_value STValueCtorImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 2U) {
        ThrowJSBadArgCountError(env, jsArgc, 2U);
        return nullptr;
    }

    napi_value jsThis;
    napi_value argv[2];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, argv, &jsThis, nullptr));

    napi_value STValueDataPtrLow = argv[0];
    napi_value STValueDataPtrHigh = argv[1];

    // set property to the instance
    NAPI_CHECK_FATAL(napi_set_named_property(env, jsThis, STVALUE_DATA_PTR_LOW, STValueDataPtrLow));
    NAPI_CHECK_FATAL(napi_set_named_property(env, jsThis, STVALUE_DATA_PTR_HIGH, STValueDataPtrHigh));

    // unwrap the native object and set finalizer
    STValueData *dataPtr = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    NAPI_CHECK_FATAL(napi_wrap(env, jsThis, dataPtr, STValueDataFinilizer, nullptr, nullptr));

    return jsThis;
}

bool IsSTValueInstance(napi_env env, napi_value value)
{
    napi_valuetype valuetype;
    NAPI_CHECK_FATAL(napi_typeof(env, value, &valuetype));
    if (valuetype != napi_object) {
        return false;
    }

    napi_value ptrLowKey;
    napi_value ptrHighKey;
    bool hasLowKey;
    bool hasHighKey;
    NAPI_CHECK_FATAL(napi_create_string_utf8(env, STVALUE_DATA_PTR_LOW, NAPI_AUTO_LENGTH, &ptrLowKey));
    NAPI_CHECK_FATAL(napi_create_string_utf8(env, STVALUE_DATA_PTR_HIGH, NAPI_AUTO_LENGTH, &ptrHighKey));

    NAPI_CHECK_FATAL(napi_has_property(env, value, ptrLowKey, &hasLowKey));
    NAPI_CHECK_FATAL(napi_has_property(env, value, ptrHighKey, &hasHighKey));

    return hasLowKey && hasHighKey;
}

napi_value CreateSTypeObject(napi_env env)
{
    napi_value stype_obj;
    NAPI_CHECK_FATAL(napi_create_object(env, &stype_obj));

    napi_value value;

    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(ark::ets::interop::js::SType::BOOLEAN), &value));
    NAPI_CHECK_FATAL(napi_set_named_property(env, stype_obj, "BOOLEAN", value));

    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(ark::ets::interop::js::SType::BYTE), &value));
    NAPI_CHECK_FATAL(napi_set_named_property(env, stype_obj, "BYTE", value));

    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(ark::ets::interop::js::SType::CHAR), &value));
    NAPI_CHECK_FATAL(napi_set_named_property(env, stype_obj, "CHAR", value));

    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(ark::ets::interop::js::SType::SHORT), &value));
    NAPI_CHECK_FATAL(napi_set_named_property(env, stype_obj, "SHORT", value));
    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(ark::ets::interop::js::SType::INT), &value));
    NAPI_CHECK_FATAL(napi_set_named_property(env, stype_obj, "INT", value));

    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(ark::ets::interop::js::SType::LONG), &value));
    NAPI_CHECK_FATAL(napi_set_named_property(env, stype_obj, "LONG", value));

    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(ark::ets::interop::js::SType::FLOAT), &value));
    NAPI_CHECK_FATAL(napi_set_named_property(env, stype_obj, "FLOAT", value));

    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(ark::ets::interop::js::SType::DOUBLE), &value));
    NAPI_CHECK_FATAL(napi_set_named_property(env, stype_obj, "DOUBLE", value));

    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(ark::ets::interop::js::SType::REFERENCE), &value));
    NAPI_CHECK_FATAL(napi_set_named_property(env, stype_obj, "REFERENCE", value));

    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(ark::ets::interop::js::SType::VOID), &value));
    NAPI_CHECK_FATAL(napi_set_named_property(env, stype_obj, "VOID", value));

    return stype_obj;
}

napi_value GetSTValueClass(napi_env env)
{
    napi_value STValueCtor {};
    thread_local napi_ref stvalueCtorRef {};
    if (stvalueCtorRef != nullptr) {
        NAPI_CHECK_FATAL(napi_get_reference_value(env, stvalueCtorRef, &STValueCtor));
        return STValueCtor;
    }

    const std::array instanceProperties = {
        napi_property_descriptor {"unwrapToNumber", 0, STValueUnwrapToNumberImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"unwrapToString", 0, STValueUnwrapToStringImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"unwrapToBigInt", 0, STValueUnwrapToBigIntImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"unwrapToBoolean", 0, STValueUnwrapToBooleanImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"namespaceGetVariable", 0, STValueNamespaceGetVariableImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"namespaceSetVariable", 0, STValueNamespcaeSetVariableImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"namespaceInvokeFunction", 0, STValueNamespaceInvokeFunctionImpl, 0, 0, 0,
                                  napi_default, 0},

        napi_property_descriptor {"isString", 0, STValueIsStringImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isBigInt", 0, STValueIsBigIntImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isBoolean", 0, STValueIsBooleanImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isByte", 0, STValueIsByteImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isChar", 0, STValueIsCharImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isShort", 0, STValueIsShortImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isInt", 0, STValueIsIntImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isLong", 0, STValueIsLongImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isFloat", 0, STValueIsFloatImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isNumber", 0, STValueIsNumberImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isStrictlyEqualTo", 0, IsStrictlyEqualToImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isNull", 0, IsNullImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isUndefined", 0, IsUndefinedImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"isEqualTo", 0, IsEqualToImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"functionalObjectInvoke", 0, STValueFunctionalObjectInvokeImpl, 0, 0, 0, napi_default,
                                  0},
        napi_property_descriptor {"objectInvokeMethod", 0, STValueObjectInvokeMethodImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"classInvokeStaticMethod", 0, STValueClassInvokeStaticMethodImpl, 0, 0, 0,
                                  napi_default, 0},
        napi_property_descriptor {"objectInstanceOf", 0, STValueObjectInstanceOfImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"objectGetType", 0, STValueObjectGetTypeImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"classInstantiate", 0, STValueClassInstantiateImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"classGetSuperClass", 0, ClassGetSuperClassImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"fixedArrayGetLength", 0, FixedArrayGetLengthImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"arrayGetLength", 0, ArrayGetLengthImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"enumGetIndexByName", 0, EnumGetIndexByNameImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"enumGetValueByName", 0, EnumGetValueByNameImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"classGetStaticField", 0, ClassGetStaticFieldImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"classSetStaticField", 0, ClassSetStaticFieldImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"objectGetProperty", 0, ObjectGetPropertyImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"objectSetProperty", 0, ObjectSetPropertyImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"fixedArrayGet", 0, FixedArrayGetImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"fixedArraySet", 0, FixedArraySetImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"arrayGet", 0, ArrayGetImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"arraySet", 0, ArraySetImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"arrayPush", 0, ArrayPushImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"arrayPop", 0, ArrayPopImpl, 0, 0, 0, napi_default, 0},
    };
    NAPI_CHECK_FATAL(napi_define_class(env, "STValue", NAPI_AUTO_LENGTH, STValueCtorImpl, nullptr,
                                       instanceProperties.size(), instanceProperties.data(), &STValueCtor));

    const std::array staticProperties = {
        napi_property_descriptor {"findClass", 0, STValueFindClassImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"findNamespace", 0, STValueFindNamespaceImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"findEnum", 0, STValueFindEnumImpl, 0, 0, 0, napi_default, 0},

        napi_property_descriptor {"wrapByte", 0, WrapByteImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"wrapChar", 0, WrapCharImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"wrapShort", 0, WrapShortImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"wrapInt", 0, WrapIntImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"wrapLong", 0, WrapLongImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"wrapFloat", 0, WrapFloatImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"wrapNumber", 0, WrapNumberImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"wrapString", 0, WrapStringImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"wrapBoolean", 0, WrapBooleanImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"wrapBigInt", 0, WrapBigIntImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"getNull", 0, GetNullImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"getUndefined", 0, GetUndefinedImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"typeIsAssignableFrom", 0, STValueTypeIsAssignableFromImpl, 0, 0, 0, napi_default, 0},
        napi_property_descriptor {"newFixedArrayPrimitive", 0, STValueNewFixedArrayPrimitiveImpl, 0, 0, 0, napi_default,
                                  0},
        napi_property_descriptor {"newFixedArrayReference", 0, STValueNewFixedArrayReferenceImpl, 0, 0, 0, napi_default,
                                  0},
        napi_property_descriptor {"newArray", 0, STValueNewArrayImpl, 0, 0, 0, napi_default, 0},
    };
    NAPI_CHECK_FATAL(napi_define_properties(env, STValueCtor, staticProperties.size(), staticProperties.data()));

    NAPI_CHECK_FATAL(napi_create_reference(env, STValueCtor, 1, &stvalueCtorRef));

    return STValueCtor;
}

void ThrowJSBadArgCountError(napi_env env, size_t jsArgc, size_t expectedArgc)
{
    STValueThrowJSError(env, "Expect " + std::to_string(expectedArgc) + " args, but got " + std::to_string(jsArgc) +
                                 " args");
}

std::string GetSTypeName(SType stype)
{
    switch (stype) {
        case SType::BOOLEAN:
            return "SType::BOOLEAN";
        case SType::BYTE:
            return "SType::BYTE";
        case SType::CHAR:
            return "SType::CHAR";
        case SType::SHORT:
            return "SType::SHORT";
        case SType::DOUBLE:
            return "SType::DOUBLE";
        case SType::FLOAT:
            return "SType::FLOAT";
        case SType::INT:
            return "SType::INT";
        case SType::LONG:
            return "SType::LONG";
        case SType::REFERENCE:
            return "SType::REFERENCE";
        case SType::VOID:
            return "SType::VOID";
        default:
            break;
    }
    return std::to_string(static_cast<int32_t>(stype));
}

void ThrowUnsupportedSTypeError(napi_env env, SType stype)
{
    STValueThrowJSError(env, "Unsupported SType: " + GetSTypeName(stype));
}

void ThrowTypeCheckError(napi_env env, const std::string &name, const std::string &type)
{
    STValueThrowJSError(env, name + " STValue instance does not wrap a value of type " + type);
}

void ThrowJSNonBooleanError(napi_env env, const std::string &name)
{
    ThrowTypeCheckError(env, name, "boolean");
}

void ThrowJSNonByteError(napi_env env, const std::string &name)
{
    ThrowTypeCheckError(env, name, "byte");
}

void ThrowJSNonCharError(napi_env env, const std::string &name)
{
    ThrowTypeCheckError(env, name, "char");
}

void ThrowJSNonShortError(napi_env env, const std::string &name)
{
    ThrowTypeCheckError(env, name, "short");
}

void ThrowJSNonIntError(napi_env env, const std::string &name)
{
    ThrowTypeCheckError(env, name, "int");
}

void ThrowJSNonLongError(napi_env env, const std::string &name)
{
    ThrowTypeCheckError(env, name, "long");
}

void ThrowJSNonFloatError(napi_env env, const std::string &name)
{
    ThrowTypeCheckError(env, name, "float");
}

void ThrowJSNonDoubleError(napi_env env, const std::string &name)
{
    ThrowTypeCheckError(env, name, "double");
}

void ThrowJSNonObjectError(napi_env env, const std::string &name)
{
    ThrowTypeCheckError(env, name, "reference");
}

void ThrowJSThisNonObjectError(napi_env env)
{
    ThrowJSNonObjectError(env, std::string("\'this\'"));
}

void ThrowJSOtherNonObjectError(napi_env env)
{
    ThrowJSNonObjectError(env, std::string("\'other\'"));
}
}  // namespace ark::ets::interop::js