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

#include <ani.h>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <node_api.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include "ets_coroutine.h"
#include "ets_vm_STValue.h"
#include "include/mem/panda_containers.h"
#include "interop_js/interop_context.h"
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

#include "plugins/ets/runtime/interop_js/interop_context_api.h"
#include "plugins/ets/runtime/interop_js/st_value/ets_vm_STValue.h"
#include "plugins/ets/runtime/interop_js/st_value/ets_vm_STValue_param_getter.h"

// NOLINTBEGIN(readability-identifier-naming, readability-redundant-declaration)
// CC-OFFNXT(G.FMT.10-CPP) project code style
napi_status __attribute__((weak)) napi_define_properties(napi_env env, napi_value object, size_t property_count,
                                                         const napi_property_descriptor *properties);
// NOLINTEND(readability-identifier-naming, readability-redundant-declaration)

namespace ark::ets::interop::js {

// classGetSuperClass(): STValue
napi_value ClassGetSuperClassImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;
    size_t jsArgc = 0;
    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    auto *aniEnv = GetAniEnv();

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_type currentType = reinterpret_cast<ani_type>(data->GetAniRef());

    ani_class superClass = nullptr;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Type_GetSuperClass(currentType, &superClass));

    if (superClass == nullptr) {
        napi_value jsNull;
        NAPI_CHECK_FATAL(napi_get_null(env, &jsNull));
        return jsNull;
    }

    return CreateSTValueInstance(env, superClass);
}

// fixedArrayGetLength(): number
napi_value FixedArrayGetLengthImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    auto *aniEnv = GetAniEnv();

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_fixedarray currentArray = reinterpret_cast<ani_fixedarray>(data->GetAniRef());

    ani_size length = 0;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->FixedArray_GetLength(currentArray, &length));

    napi_value jsLength;
    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(length), &jsLength));

    return jsLength;
}

// arrayGetLength(): number
napi_value ArrayGetLengthImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    auto *aniEnv = GetAniEnv();

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_array currentArray = reinterpret_cast<ani_array>(data->GetAniRef());

    ani_size length = 0;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Array_GetLength(currentArray, &length));

    napi_value jsLength;
    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(length), &jsLength));

    return jsLength;
}

// enumGetIndexByName(name: string): number
napi_value EnumGetIndexByNameImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 1) {
        ThrowJSBadArgCountError(env, jsArgc, 1);
        return nullptr;
    }

    napi_value jsThis;
    napi_value jsArgv[1];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    std::string memberName;
    if (!GetString(env, jsArgv[0], "memberName", memberName)) {
        return nullptr;
    }
    auto *aniEnv = GetAniEnv();

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_enum currentEnum = reinterpret_cast<ani_enum>(data->GetAniRef());

    ani_enum_item enumItem = nullptr;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Enum_GetEnumItemByName(currentEnum, memberName.c_str(), &enumItem));

    ani_size index = 0;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->EnumItem_GetIndex(enumItem, &index));

    napi_value jsIndex;
    NAPI_CHECK_FATAL(napi_create_uint32(env, static_cast<uint32_t>(index), &jsIndex));

    return jsIndex;
}

// enumGetValueByName(name: string, valueType: SType): STValue
napi_value EnumGetValueByNameImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));
    if (jsArgc != 2U) {
        ThrowJSBadArgCountError(env, jsArgc, 2U);
        return nullptr;
    }

    napi_value jsThis;
    napi_value jsArgv[2];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    std::string memberName;
    if (!GetString(env, jsArgv[0], "memberName", memberName)) {
        return nullptr;
    }
    uint32_t valueTypeInt = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[1], &valueTypeInt));
    SType valueType = static_cast<SType>(valueTypeInt);

    auto *aniEnv = GetAniEnv();
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_enum currentEnum = reinterpret_cast<ani_enum>(data->GetAniRef());

    ani_enum_item enumItem = nullptr;
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Enum_GetEnumItemByName(currentEnum, memberName.c_str(), &enumItem));

    switch (valueType) {
        case SType::INT: {
            ani_int value = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->EnumItem_GetValue_Int(enumItem, &value));
            return CreateSTValueInstance(env, value);
        }
        case SType::REFERENCE: {
            ani_string aniValue = nullptr;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->EnumItem_GetValue_String(enumItem, &aniValue));
            return CreateSTValueInstance(env, aniValue);
        }
        default: {
            ThrowUnsupportedSTypeError(env, valueType);
            return nullptr;
        }
    }
}

// classGetStaticField(name: string, fieldType: SType): STValue
napi_value ClassGetStaticFieldImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 2U) {
        ThrowJSBadArgCountError(env, jsArgc, 2U);
        return nullptr;
    }

    napi_value jsArgv[2];
    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    std::string fieldName;
    if (!GetString(env, jsArgv[0], "fieldName", fieldName)) {
        return nullptr;
    }
    uint32_t fieldTypeInt = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[1], &fieldTypeInt));
    SType fieldType = static_cast<SType>(fieldTypeInt);

    auto *aniEnv = GetAniEnv();

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_class currentClass = reinterpret_cast<ani_class>(data->GetAniRef());

    switch (fieldType) {
        case SType::BOOLEAN: {
            ani_boolean value = 0;
            auto status = aniEnv->Class_GetStaticFieldByName_Boolean(currentClass, fieldName.c_str(), &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::BYTE: {
            ani_byte value = 0;
            auto status = aniEnv->Class_GetStaticFieldByName_Byte(currentClass, fieldName.c_str(), &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::CHAR: {
            // problem: how to cast uint16_t to char?  ans: use uint16_t
            ani_char value = 0;
            auto status = aniEnv->Class_GetStaticFieldByName_Char(currentClass, fieldName.c_str(), &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::SHORT: {
            ani_short value = 0;
            auto status = aniEnv->Class_GetStaticFieldByName_Short(currentClass, fieldName.c_str(), &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::INT: {
            ani_int value = 0;
            auto status = aniEnv->Class_GetStaticFieldByName_Int(currentClass, fieldName.c_str(), &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::LONG: {
            ani_long value = 0;
            auto status = aniEnv->Class_GetStaticFieldByName_Long(currentClass, fieldName.c_str(), &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::FLOAT: {
            ani_float value = 0.0f;
            auto status = aniEnv->Class_GetStaticFieldByName_Float(currentClass, fieldName.c_str(), &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::DOUBLE: {
            ani_double value = 0;
            auto status = aniEnv->Class_GetStaticFieldByName_Double(currentClass, fieldName.c_str(), &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::REFERENCE: {
            ani_ref value = nullptr;
            auto status = aniEnv->Class_GetStaticFieldByName_Ref(currentClass, fieldName.c_str(), &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            if (value == nullptr) {
                napi_value jsNull = nullptr;
                NAPI_CHECK_FATAL(napi_get_null(env, &jsNull));
                return jsNull;
            }
            return CreateSTValueInstance(env, value);
        }
        default: {
            ThrowUnsupportedSTypeError(env, fieldType);
            return nullptr;
        }
    }
}

// classSetStaticField(name: string, val: STValue, fieldType: SType): void
napi_value ClassSetStaticFieldImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 3U) {
        ThrowJSBadArgCountError(env, jsArgc, 3U);
        return nullptr;
    }

    napi_value jsThis;
    napi_value jsArgv[3];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    std::string fieldName;
    if (!GetString(env, jsArgv[0], "fieldName", fieldName)) {
        return nullptr;
    }
    napi_value jsValue = jsArgv[1];

    uint32_t fieldTypeInt = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[2U], &fieldTypeInt));
    SType fieldType = static_cast<SType>(fieldTypeInt);

    auto *aniEnv = GetAniEnv();
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_class currentClass = reinterpret_cast<ani_class>(data->GetAniRef());

    STValueData *valueData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsValue));
    ani_status status = ANI_OK;
    auto *name = fieldName.c_str();

    switch (fieldType) {
        case SType::BOOLEAN: {
            if (!valueData->IsAniBoolean()) {
                ThrowJSNonBooleanError(env, "value");
                return nullptr;
            }
            status = aniEnv->Class_SetStaticFieldByName_Boolean(currentClass, name, valueData->GetAniBoolean());
            break;
        }
        case SType::BYTE: {
            if (!valueData->IsAniByte()) {
                ThrowJSNonByteError(env, "value");
                return nullptr;
            }
            status = aniEnv->Class_SetStaticFieldByName_Byte(currentClass, name, valueData->GetAniByte());
            break;
        }
        case SType::CHAR: {
            if (!valueData->IsAniChar()) {
                ThrowJSNonCharError(env, "value");
                return nullptr;
            }
            status = aniEnv->Class_SetStaticFieldByName_Char(currentClass, name, valueData->GetAniChar());
            break;
        }
        case SType::SHORT: {
            if (!valueData->IsAniShort()) {
                ThrowJSNonShortError(env, "value");
                return nullptr;
            }
            status = aniEnv->Class_SetStaticFieldByName_Short(currentClass, name, valueData->GetAniShort());
            break;
        }
        case SType::INT: {
            if (!valueData->IsAniInt()) {
                ThrowJSNonIntError(env, "value");
                return nullptr;
            }
            status = aniEnv->Class_SetStaticFieldByName_Int(currentClass, name, valueData->GetAniInt());
            break;
        }
        case SType::LONG: {
            if (!valueData->IsAniLong()) {
                ThrowJSNonLongError(env, "value");
                return nullptr;
            }
            status = aniEnv->Class_SetStaticFieldByName_Long(currentClass, name, valueData->GetAniLong());
            break;
        }
        case SType::FLOAT: {
            if (!valueData->IsAniFloat()) {
                ThrowJSNonFloatError(env, "value");
                return nullptr;
            }
            status = aniEnv->Class_SetStaticFieldByName_Float(currentClass, name, valueData->GetAniFloat());
            break;
        }
        case SType::DOUBLE: {
            if (!valueData->IsAniDouble()) {
                ThrowJSNonDoubleError(env, "value");
                return nullptr;
            }
            status = aniEnv->Class_SetStaticFieldByName_Double(currentClass, name, valueData->GetAniDouble());
            break;
        }
        case SType::REFERENCE: {
            if (!valueData->IsAniRef()) {
                ThrowJSNonObjectError(env, "value");
                return nullptr;
            }
            status = aniEnv->Class_SetStaticFieldByName_Ref(currentClass, name, valueData->GetAniRef());
            break;
        }
        default: {
            ThrowUnsupportedSTypeError(env, fieldType);
            return nullptr;
        }
    }
    ANI_CHECK_ERROR_RETURN(env, status);

    napi_value undefined;
    NAPI_CHECK_FATAL(napi_get_undefined(env, &undefined));
    return undefined;
}

// objectGetProperty(name: string, propType: SType): STValue
napi_value ObjectGetPropertyImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 2U) {
        ThrowJSBadArgCountError(env, jsArgc, 2U);
        return nullptr;
    }

    napi_value jsThis;
    napi_value jsArgv[2];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    std::string propNameStr;
    if (!GetString(env, jsArgv[0], "propNameStr", propNameStr)) {
        return nullptr;
    }
    auto propName = propNameStr.c_str();

    uint32_t propTypeInt = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[1], &propTypeInt));
    SType propType = static_cast<SType>(propTypeInt);

    auto aniEnv = GetAniEnv();
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_object currentObject = reinterpret_cast<ani_object>(data->GetAniRef());

    switch (propType) {
        case SType::BOOLEAN: {
            ani_boolean value = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_GetPropertyByName_Boolean(currentObject, propName, &value));
            return CreateSTValueInstance(env, value);
        }
        case SType::BYTE: {
            ani_byte value = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_GetPropertyByName_Byte(currentObject, propName, &value));
            return CreateSTValueInstance(env, value);
        }
        case SType::CHAR: {
            ani_char value = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_GetPropertyByName_Char(currentObject, propName, &value));
            return CreateSTValueInstance(env, value);
        }
        case SType::SHORT: {
            ani_short value = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_GetPropertyByName_Short(currentObject, propName, &value));
            return CreateSTValueInstance(env, value);
        }
        case SType::INT: {
            ani_int value = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_GetPropertyByName_Int(currentObject, propName, &value));
            return CreateSTValueInstance(env, value);
        }
        case SType::LONG: {
            ani_long value = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_GetPropertyByName_Long(currentObject, propName, &value));
            return CreateSTValueInstance(env, value);
        }
        case SType::FLOAT: {
            ani_float value = 0.0f;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_GetPropertyByName_Float(currentObject, propName, &value));
            return CreateSTValueInstance(env, value);
        }
        case SType::DOUBLE: {
            ani_double value = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_GetPropertyByName_Double(currentObject, propName, &value));
            return CreateSTValueInstance(env, value);
        }
        case SType::REFERENCE: {
            ani_ref value = nullptr;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_GetPropertyByName_Ref(currentObject, propName, &value));
            if (value == nullptr) {
                napi_value jsNull = nullptr;
                NAPI_CHECK_FATAL(napi_get_null(env, &jsNull));
                return jsNull;
            }
            return CreateSTValueInstance(env, value);
        }
        default: {
            ThrowUnsupportedSTypeError(env, propType);
            return nullptr;
        }
    }
}

// objectSetProperty(name: string, val: STValue, propType: SType): void
napi_value ObjectSetPropertyImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 3U) {
        ThrowJSBadArgCountError(env, jsArgc, 3U);
        return nullptr;
    }

    napi_value jsThis;
    napi_value jsArgv[3];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    std::string propNameStr;
    if (!GetString(env, jsArgv[0], "propNameStr", propNameStr)) {
        return nullptr;
    }
    auto *propName = propNameStr.c_str();

    napi_value jsValue = jsArgv[1];
    uint32_t propTypeInt = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[2U], &propTypeInt));
    SType propType = static_cast<SType>(propTypeInt);

    auto aniEnv = GetAniEnv();
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_object currentObject = reinterpret_cast<ani_object>(data->GetAniRef());

    STValueData *valueData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsValue));
    ani_status status = ANI_OK;

    switch (propType) {
        case SType::BOOLEAN: {
            if (!valueData->IsAniBoolean()) {
                ThrowJSNonBooleanError(env, "value");
                return nullptr;
            }
            status = aniEnv->Object_SetPropertyByName_Boolean(currentObject, propName, valueData->GetAniBoolean());
            break;
        }
        case SType::BYTE: {
            if (!valueData->IsAniByte()) {
                ThrowJSNonByteError(env, "value");
                return nullptr;
            }
            status = aniEnv->Object_SetPropertyByName_Byte(currentObject, propName, valueData->GetAniByte());
            break;
        }
        case SType::CHAR: {
            if (!valueData->IsAniChar()) {
                ThrowJSNonCharError(env, "value");
                return nullptr;
            }
            status = aniEnv->Object_SetPropertyByName_Char(currentObject, propName, valueData->GetAniChar());
            break;
        }
        case SType::SHORT: {
            if (!valueData->IsAniShort()) {
                ThrowJSNonShortError(env, "value");
                return nullptr;
            }
            status = aniEnv->Object_SetPropertyByName_Short(currentObject, propName, valueData->GetAniShort());
            break;
        }
        case SType::INT: {
            if (!valueData->IsAniInt()) {
                ThrowJSNonIntError(env, "value");
                return nullptr;
            }
            status = aniEnv->Object_SetPropertyByName_Int(currentObject, propName, valueData->GetAniInt());
            break;
        }
        case SType::LONG: {
            if (!valueData->IsAniLong()) {
                ThrowJSNonLongError(env, "value");
                return nullptr;
            }
            status = aniEnv->Object_SetPropertyByName_Long(currentObject, propName, valueData->GetAniLong());
            break;
        }
        case SType::FLOAT: {
            if (!valueData->IsAniFloat()) {
                ThrowJSNonFloatError(env, "value");
                return nullptr;
            }
            status = aniEnv->Object_SetPropertyByName_Float(currentObject, propName, valueData->GetAniFloat());
            break;
        }
        case SType::DOUBLE: {
            if (!valueData->IsAniDouble()) {
                ThrowJSNonDoubleError(env, "value");
                return nullptr;
            }
            status = aniEnv->Object_SetPropertyByName_Double(currentObject, propName, valueData->GetAniDouble());
            break;
        }
        case SType::REFERENCE: {
            if (!valueData->IsAniRef()) {
                ThrowJSNonObjectError(env, "value");
                return nullptr;
            }
            status = aniEnv->Object_SetPropertyByName_Ref(currentObject, propName, valueData->GetAniRef());
            break;
        }
        default: {
            ThrowUnsupportedSTypeError(env, propType);
            return nullptr;
        }
    }
    ANI_CHECK_ERROR_RETURN(env, status);

    napi_value undefined;
    NAPI_CHECK_FATAL(napi_get_undefined(env, &undefined));
    return undefined;
}

// fixedArrayGet(idx: number, elementType: SType): STValue
napi_value FixedArrayGetImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 2U) {
        ThrowJSBadArgCountError(env, jsArgc, 2U);

        return nullptr;
    }

    napi_value jsThis;
    napi_value jsArgv[2];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    uint32_t index = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[0], &index));
    uint32_t elemTypeInt = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[1], &elemTypeInt));
    SType elemType = static_cast<SType>(elemTypeInt);

    auto *aniEnv = GetAniEnv();
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }

    ani_fixedarray currentArray = reinterpret_cast<ani_fixedarray>(data->GetAniRef());

    ani_status status = ANI_OK;

    switch (elemType) {
        case SType::INT: {
            ani_int value = 0;
            status =
                aniEnv->FixedArray_GetRegion_Int(reinterpret_cast<ani_fixedarray_int>(currentArray), index, 1, &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::BOOLEAN: {
            ani_boolean value = ANI_FALSE;
            status = aniEnv->FixedArray_GetRegion_Boolean(reinterpret_cast<ani_fixedarray_boolean>(currentArray), index,
                                                          1, &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::BYTE: {
            ani_byte value = 0;
            status = aniEnv->FixedArray_GetRegion_Byte(reinterpret_cast<ani_fixedarray_byte>(currentArray), index, 1,
                                                       &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::CHAR: {
            ani_char value = 0;
            status = aniEnv->FixedArray_GetRegion_Char(reinterpret_cast<ani_fixedarray_char>(currentArray), index, 1,
                                                       &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::SHORT: {
            ani_short value = 0;
            status = aniEnv->FixedArray_GetRegion_Short(reinterpret_cast<ani_fixedarray_short>(currentArray), index, 1,
                                                        &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::LONG: {
            ani_long value = 0;
            status = aniEnv->FixedArray_GetRegion_Long(reinterpret_cast<ani_fixedarray_long>(currentArray), index, 1,
                                                       &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::FLOAT: {
            ani_float value = 0;
            status = aniEnv->FixedArray_GetRegion_Float(reinterpret_cast<ani_fixedarray_float>(currentArray), index, 1,
                                                        &value);
            ANI_CHECK_ERROR_RETURN(env, status);
            return CreateSTValueInstance(env, value);
        }
        case SType::REFERENCE: {
            ani_ref value {};
            ANI_CHECK_ERROR_RETURN(
                env, aniEnv->FixedArray_Get_Ref(reinterpret_cast<ani_fixedarray_ref>(currentArray), index, &value));
            if (value == nullptr) {
                napi_value jsNull = nullptr;
                NAPI_CHECK_FATAL(napi_get_null(env, &jsNull));
                return jsNull;
            }
            return CreateSTValueInstance(env, value);
        }
        default: {
            ThrowUnsupportedSTypeError(env, elemType);
            return nullptr;
        }
    }
}

// fixedArraySet(idx: number, val: STValue, elementType: SType): void
napi_value FixedArraySetImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 3U) {
        ThrowJSBadArgCountError(env, jsArgc, 3U);
        return nullptr;
    }

    napi_value jsThis;
    napi_value jsArgv[3];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    uint32_t index = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[0], &index));
    napi_value jsElement = jsArgv[1];

    uint32_t elementTypeInt = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[2U], &elementTypeInt));
    SType elementType = static_cast<SType>(elementTypeInt);

    auto *aniEnv = GetAniEnv();

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_fixedarray currentArray = reinterpret_cast<ani_fixedarray>(data->GetAniRef());

    STValueData *valueData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsElement));
    ani_status status = ANI_OK;

    switch (elementType) {
        case SType::INT: {
            if (!valueData->IsAniInt()) {
                ThrowJSNonIntError(env, "value");
                return nullptr;
            }
            ani_int element = valueData->GetAniInt();
            status = aniEnv->FixedArray_SetRegion_Int(reinterpret_cast<ani_fixedarray_int>(currentArray), index, 1,
                                                      &element);
            break;
        }
        case SType::BOOLEAN: {
            if (!valueData->IsAniBoolean()) {
                ThrowJSNonBooleanError(env, "value");
                return nullptr;
            }
            ani_boolean element = valueData->GetAniBoolean();
            status = aniEnv->FixedArray_SetRegion_Boolean(reinterpret_cast<ani_fixedarray_boolean>(currentArray), index,
                                                          1, &element);
            break;
        }
        case SType::BYTE: {
            if (!valueData->IsAniByte()) {
                ThrowJSNonByteError(env, "value");
                return nullptr;
            }
            ani_byte element = valueData->GetAniByte();
            status = aniEnv->FixedArray_SetRegion_Byte(reinterpret_cast<ani_fixedarray_byte>(currentArray), index, 1,
                                                       &element);
            break;
        }
        case SType::CHAR: {
            if (!valueData->IsAniChar()) {
                ThrowJSNonCharError(env, "value");
                return nullptr;
            }
            ani_char element = valueData->GetAniChar();
            status = aniEnv->FixedArray_SetRegion_Char(reinterpret_cast<ani_fixedarray_char>(currentArray), index, 1,
                                                       &element);
            break;
        }
        case SType::DOUBLE: {
            if (!valueData->IsAniDouble()) {
                ThrowJSNonDoubleError(env, "value");
                return nullptr;
            }
            ani_double element = valueData->GetAniDouble();
            status = aniEnv->FixedArray_SetRegion_Double(reinterpret_cast<ani_fixedarray_double>(currentArray), index,
                                                         1, &element);
            break;
        }
        case SType::FLOAT: {
            if (!valueData->IsAniFloat()) {
                ThrowJSNonFloatError(env, "value");
                return nullptr;
            }
            ani_float element = valueData->GetAniFloat();
            status = aniEnv->FixedArray_SetRegion_Float(reinterpret_cast<ani_fixedarray_float>(currentArray), index, 1,
                                                        &element);
            break;
        }
        case SType::REFERENCE: {
            if (!valueData->IsAniRef()) {
                ThrowJSNonObjectError(env, "value");
                return nullptr;
            }
            ani_ref element = valueData->GetAniRef();
            status = aniEnv->FixedArray_Set_Ref(reinterpret_cast<ani_fixedarray_ref>(currentArray), index, element);
            break;
        }
        default: {
            ThrowUnsupportedSTypeError(env, elementType);
            return nullptr;
        }
    }
    ANI_CHECK_ERROR_RETURN(env, status);

    napi_value undefined;
    NAPI_CHECK_FATAL(napi_get_undefined(env, &undefined));
    return undefined;
}

// arrayGet(idx: number): STValue
napi_value ArrayGetImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 1U) {
        ThrowJSBadArgCountError(env, jsArgc, 1U);
        return nullptr;
    }

    napi_value jsThis;
    napi_value jsArgv[1];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    uint32_t index = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[0], &index));

    auto *aniEnv = GetAniEnv();
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_array currentArray = reinterpret_cast<ani_array>(data->GetAniRef());

    ani_ref value {};
    ani_status status = aniEnv->Array_Get(currentArray, index, &value);
    ANI_CHECK_ERROR_RETURN(env, status);
    if (value == nullptr) {
        napi_value jsNull = nullptr;
        NAPI_CHECK_FATAL(napi_get_null(env, &jsNull));
        return jsNull;
    }
    return CreateSTValueInstance(env, value);
}

// arraySet(idx: number, val: STValue): void
napi_value ArraySetImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 2U) {
        ThrowJSBadArgCountError(env, jsArgc, 2U);
        return nullptr;
    }

    napi_value jsThis;
    napi_value jsArgv[2];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    uint32_t index = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[0], &index));
    napi_value jsElement = jsArgv[1];

    auto *aniEnv = GetAniEnv();

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_array currentArray = reinterpret_cast<ani_array>(data->GetAniRef());

    STValueData *valueData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsElement));
    if (!valueData->IsAniRef()) {
        ThrowJSNonObjectError(env, "value");
        return nullptr;
    }
    ani_ref element = valueData->GetAniRef();
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Array_Set(currentArray, index, element));

    napi_value undefined;
    NAPI_CHECK_FATAL(napi_get_undefined(env, &undefined));
    return undefined;
}

// arrayPush(val: STValue): void
napi_value ArrayPushImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 1U) {
        ThrowJSBadArgCountError(env, jsArgc, 1U);
        return nullptr;
    }

    napi_value jsThis;
    napi_value jsArgv[1];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    napi_value jsElement = jsArgv[0];

    auto *aniEnv = GetAniEnv();

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_array currentArray = reinterpret_cast<ani_array>(data->GetAniRef());

    STValueData *valueData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsElement));
    if (!valueData->IsAniRef()) {
        ThrowJSNonObjectError(env, "value");
        return nullptr;
    }
    ani_ref element = valueData->GetAniRef();
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Array_Push(currentArray, element));

    napi_value undefined;
    NAPI_CHECK_FATAL(napi_get_undefined(env, &undefined));
    return undefined;
}

// arrayPop(): STValue
napi_value ArrayPopImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }

    napi_value jsThis;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr));

    auto *aniEnv = GetAniEnv();

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_array currentArray = reinterpret_cast<ani_array>(data->GetAniRef());

    ani_ref value;
    ani_status status = aniEnv->Array_Pop(currentArray, &value);
    ANI_CHECK_ERROR_RETURN(env, status);
    if (value == nullptr) {
        napi_value jsNull = nullptr;
        NAPI_CHECK_FATAL(napi_get_null(env, &jsNull));
        return jsNull;
    }
    return CreateSTValueInstance(env, value);
}

// NamespaceGetVariableByName(name: string, val: STValue, variableType: SType)
napi_value STValueNamespaceGetVariableImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    napi_value jsThis;
    napi_value jsArgv[2];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 2U) {
        ThrowJSBadArgCountError(env, jsArgc, 2U);
        return nullptr;
    }

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    uint32_t variableInt = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[1], &variableInt));
    SType variableType = static_cast<SType>(variableInt);

    auto *aniEnv = GetAniEnv();

    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_namespace aniNamespace = reinterpret_cast<ani_namespace>(data->GetAniRef());

    napi_value variableNameNapiValue = jsArgv[0];
    std::string variableName;
    if (!GetString(env, variableNameNapiValue, "variableName", variableName)) {
        return nullptr;
    }

    ani_variable getVariable {nullptr};
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Namespace_FindVariable(aniNamespace, variableName.c_str(), &getVariable));

    switch (variableType) {
        case SType::BOOLEAN: {
            ani_boolean boolValue = 0U;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_GetValue_Boolean(getVariable, &boolValue));
            return CreateSTValueInstance(env, boolValue);
        }
        case SType::BYTE: {
            ani_byte byteValue = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_GetValue_Byte(getVariable, &byteValue));
            return CreateSTValueInstance(env, byteValue);
        }
        case SType::CHAR: {
            ani_char charValue = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_GetValue_Char(getVariable, &charValue));
            return CreateSTValueInstance(env, charValue);
        }
        case SType::SHORT: {
            ani_short shortValue = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_GetValue_Short(getVariable, &shortValue));
            return CreateSTValueInstance(env, shortValue);
        }
        case SType::INT: {
            ani_int intValue = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_GetValue_Int(getVariable, &intValue));
            return CreateSTValueInstance(env, intValue);
        }
        case SType::LONG: {
            ani_long longValue = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_GetValue_Long(getVariable, &longValue));
            return CreateSTValueInstance(env, longValue);
        }
        case SType::FLOAT: {
            ani_float floatValue = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_GetValue_Float(getVariable, &floatValue));
            return CreateSTValueInstance(env, floatValue);
        }
        case SType::DOUBLE: {
            ani_double doubleValue = 0;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_GetValue_Double(getVariable, &doubleValue));
            return CreateSTValueInstance(env, doubleValue);
        }
        case SType::REFERENCE: {
            ani_ref refValue = nullptr;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_GetValue_Ref(getVariable, &refValue));
            if (refValue == nullptr) {
                napi_value jsNull = nullptr;
                NAPI_CHECK_FATAL(napi_get_null(env, &jsNull));
                return jsNull;
            }
            return CreateSTValueInstance(env, refValue);
        }
        default: {
            ThrowUnsupportedSTypeError(env, variableType);
            return nullptr;
        }
    }
}

// NamespaceSetVariableByName(name: string, val: STValue, variableType: SType)
napi_value STValueNamespcaeSetVariableImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    napi_value jsThis;
    napi_value jsArgv[3];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 3U) {
        ThrowJSBadArgCountError(env, jsArgc, 3U);
        return nullptr;
    }

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    uint32_t variableInt = 0;
    NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsArgv[2U], &variableInt));
    SType variableType = static_cast<SType>(variableInt);

    auto *aniEnv = GetAniEnv();
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (!data->IsAniRef() || data->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_namespace aniNamespace = reinterpret_cast<ani_namespace>(data->GetAniRef());

    napi_value variableNameNapiValue = jsArgv[0];
    std::string variableName;
    if (!GetString(env, variableNameNapiValue, "variableName", variableName)) {
        return nullptr;
    }

    napi_value newValue = jsArgv[1];
    STValueData *newValueData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, newValue));

    ani_variable getVariable {nullptr};
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Namespace_FindVariable(aniNamespace, variableName.c_str(), &getVariable));

    switch (variableType) {
        case SType::BOOLEAN: {
            if (!newValueData->IsAniBoolean()) {
                ThrowJSNonBooleanError(env, "value");
                return nullptr;
            }
            ani_boolean boolValue = newValueData->GetAniBoolean();
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_SetValue_Boolean(getVariable, boolValue));
            break;
        }
        case SType::BYTE: {
            if (!newValueData->IsAniByte()) {
                ThrowJSNonByteError(env, "value");
                return nullptr;
            }
            ani_byte byteValue = newValueData->GetAniByte();
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_SetValue_Byte(getVariable, byteValue));
            break;
        }
        case SType::CHAR: {
            if (!newValueData->IsAniChar()) {
                ThrowJSNonCharError(env, "value");
                return nullptr;
            }
            ani_char charValue = newValueData->GetAniChar();
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_SetValue_Char(getVariable, charValue));
            break;
        }
        case SType::SHORT: {
            if (!newValueData->IsAniShort()) {
                ThrowJSNonShortError(env, "value");
                return nullptr;
            }
            ani_short shortValue = newValueData->GetAniShort();
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_SetValue_Short(getVariable, shortValue));
            break;
        }
        case SType::INT: {
            if (!newValueData->IsAniInt()) {
                ThrowJSNonIntError(env, "value");
                return nullptr;
            }
            ani_int IntValue = newValueData->GetAniInt();
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_SetValue_Int(getVariable, IntValue));
            break;
        }
        case SType::LONG: {
            if (!newValueData->IsAniLong()) {
                ThrowJSNonLongError(env, "value");
                return nullptr;
            }
            ani_long longValue = newValueData->GetAniLong();
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_SetValue_Long(getVariable, longValue));
            break;
        }
        case SType::FLOAT: {
            if (!newValueData->IsAniFloat()) {
                ThrowJSNonFloatError(env, "value");
                return nullptr;
            }
            ani_float floatValue = newValueData->GetAniFloat();
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_SetValue_Float(getVariable, floatValue));
            break;
        }
        case SType::DOUBLE: {
            if (!newValueData->IsAniDouble()) {
                ThrowJSNonDoubleError(env, "value");
                return nullptr;
            }
            ani_double doubleValue = newValueData->GetAniDouble();
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_SetValue_Double(getVariable, doubleValue));
            break;
        }
        case SType::REFERENCE: {
            if (!newValueData->IsAniRef()) {
                ThrowJSNonObjectError(env, "value");
                return nullptr;
            }
            ani_ref refValue = newValueData->GetAniRef();
            ANI_CHECK_ERROR_RETURN(env, aniEnv->Variable_SetValue_Ref(getVariable, refValue));
            break;
        }
        default: {
            ThrowUnsupportedSTypeError(env, variableType);
            return nullptr;
        }
    }

    napi_value js_undefined;
    NAPI_CHECK_FATAL(napi_get_undefined(env, &js_undefined));
    return js_undefined;
}

// objectGetType(): STValue
napi_value STValueObjectGetTypeImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;
    auto *aniEnv = GetAniEnv();

    size_t jsArgc = 0;
    napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr);
    if (jsArgc != 0) {
        ThrowJSBadArgCountError(env, jsArgc, 0);
        return nullptr;
    }
    napi_value jsThis {};
    napi_get_cb_info(env, info, &jsArgc, nullptr, &jsThis, nullptr);

    STValueData *objectData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (objectData == nullptr || !objectData->IsAniRef() || objectData->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_object objObject = static_cast<ani_object>(objectData->GetAniRef());

    ani_type resType {};
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_GetType(objObject, &resType));
    return CreateSTValueInstance(env, resType);
}

static napi_value STValueTemplateFindElement(napi_env env, napi_callback_info info,
                                             const std::function<ani_ref(const std::string &)> &findElement)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    // 1. get elementName
    size_t jsArgc = 0;
    napi_value jsElementName {};
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));
    if (jsArgc != 1) {
        ThrowJSBadArgCountError(env, jsArgc, 1);
        return nullptr;
    }

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, &jsElementName, nullptr, nullptr));

    // 2. get elementName and check it
    std::string elementName;
    if (!GetString(env, jsElementName, "elementName", elementName)) {
        return nullptr;
    }

    ani_ref aniElement = findElement(elementName);
    if (aniElement == nullptr) {
        return nullptr;
    }

    // 3. return result
    return CreateSTValueInstance<ani_ref>(env, std::move(aniElement));
}

ani_object CreateBoolean(ani_env *env, ani_boolean boo)
{
    static constexpr const char *booleanClassName = "std.core.Boolean";
    ani_class booleanCls {};
    AniExpectOK(env->FindClass(booleanClassName, &booleanCls));
    ani_method ctor {};
    AniExpectOK(env->Class_FindMethod(booleanCls, "<ctor>", "z:", &ctor));
    ani_object obj {};
    AniExpectOK(env->Object_New(booleanCls, ctor, &obj, boo));
    return obj;
}

ani_status GetClassFromInteropDefaultLinker([[maybe_unused]] napi_env napiEnv, ani_env *env,
                                            const std::string &elementName, ani_ref *ref)
{
    INTEROP_TRACE();
    ani_string elementStr = nullptr;
    AniExpectOK(env->String_NewUTF8(elementName.c_str(), elementName.size(), &elementStr));

    static ani_method loadMethod {};
    if (loadMethod == nullptr) {
        std::string clsDescriptor = "std.core.AbcRuntimeLinker";
        ani_class cls {};
        AniExpectOK(env->FindClass(clsDescriptor.c_str(), &cls));
        AniExpectOK(env->Class_FindMethod(cls, "loadClass", "C{std.core.String}C{std.core.Boolean}:C{std.core.Class}",
                                          &loadMethod));
    }

    static ani_ref linkerRef {};
    if (linkerRef == nullptr) {
        ani_class contextCls {};
        AniExpectOK(env->FindClass("std.interop.InteropContext", &contextCls));
        AniExpectOK(env->Class_CallStaticMethodByName_Ref(contextCls, "getInteropRuntimeLinker",
                                                          ":C{std.core.RuntimeLinker}", &linkerRef));
    }
    static ani_object boolObj = CreateBoolean(env, ANI_FALSE);
    ani_ref clsRef = nullptr;
    ani_status status =
        env->Object_CallMethod_Ref(static_cast<ani_object>(linkerRef), loadMethod, &clsRef, elementStr, boolObj);
    if (status != ani_status::ANI_OK) {
        return status;
    }

    *ref = clsRef;
    return ani_status::ANI_OK;
}

napi_value STValueFindClassImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    auto findClass = [env](const std::string &className) -> ani_ref {
        auto aniEnv = GetAniEnv();

        // 1. find it by default classlinker
        ani_class aniClass = nullptr;
        ani_status aniStatus = aniEnv->FindClass(className.c_str(), &aniClass);
        if (aniStatus == ani_status::ANI_OK && aniClass != nullptr) {
            return aniClass;
        }

        // 2. check error
        if (aniStatus == ani_status::ANI_INVALID_DESCRIPTOR) {
            AniCheckAndThrowToDynamic(env, aniStatus,
                                      "FindClass failed, invalid className: className=" + className +
                                          ", aniStatus=" + std::to_string(aniStatus));
            return nullptr;
        }

        if (aniStatus != ani_status::ANI_NOT_FOUND) {
            AniCheckAndThrowToDynamic(env, aniStatus,
                                      "FindClass failed: className=" + className +
                                          ", aniStatus=" + std::to_string(aniStatus));
            return nullptr;
        }

        // 3. find it by interop runtime linker
        ani_ref ref = nullptr;
        aniStatus = GetClassFromInteropDefaultLinker(env, aniEnv, className, &ref);
        if (aniStatus == ani_status::ANI_OK && ref != nullptr) {
            return ref;
        }

        AniCheckAndThrowToDynamic(
            env, aniStatus, "FindClass failed: className=" + className + ", aniStatus=" + std::to_string(aniStatus));
        return nullptr;
    };

    return STValueTemplateFindElement(env, info, findClass);
}

napi_value STValueFindNamespaceImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    auto findNamespace = [env](const std::string &namespaceName) -> ani_ref {
        // 1. find it by default classlinker
        auto aniEnv = GetAniEnv();
        ani_namespace aniNamespace = nullptr;
        ani_status aniStatus = aniEnv->FindNamespace(namespaceName.c_str(), &aniNamespace);
        if (aniStatus == ani_status::ANI_OK && aniNamespace != nullptr) {
            return aniNamespace;
        }

        // 2. check error
        if (aniStatus == ani_status::ANI_INVALID_DESCRIPTOR) {
            AniCheckAndThrowToDynamic(env, aniStatus,
                                      "FindNamespace failed, invalid namespaceName: namespaceName=" + namespaceName +
                                          ", aniStatus=" + std::to_string(aniStatus));
            return nullptr;
        }

        if (aniStatus != ani_status::ANI_NOT_FOUND) {
            AniCheckAndThrowToDynamic(env, aniStatus,
                                      "FindNamespace failed: namespaceName=" + namespaceName +
                                          ", aniStatus=" + std::to_string(aniStatus));
            return nullptr;
        }

        // 3. find it by interop runtime linker
        ani_ref ref = nullptr;
        aniStatus = GetClassFromInteropDefaultLinker(env, aniEnv, namespaceName, &ref);
        if (aniStatus == ani_status::ANI_OK && ref != nullptr) {
            return ref;
        }

        AniCheckAndThrowToDynamic(env, aniStatus,
                                  "FindNamespace failed: namespaceName=" + namespaceName +
                                      ", aniStatus=" + std::to_string(aniStatus));
        return nullptr;
    };

    return STValueTemplateFindElement(env, info, findNamespace);
}

napi_value STValueFindEnumImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    auto findEnum = [env](const std::string &enumName) -> ani_ref {
        // 1. find it by default classlinker
        auto aniEnv = GetAniEnv();
        ani_enum aniEnum = nullptr;
        ani_status aniStatus = aniEnv->FindEnum(enumName.c_str(), &aniEnum);
        if (aniStatus == ani_status::ANI_OK && aniEnum != nullptr) {
            return aniEnum;
        }

        // 2. check error
        if (aniStatus == ani_status::ANI_INVALID_DESCRIPTOR) {
            AniCheckAndThrowToDynamic(env, aniStatus,
                                      "FindEnum failed, invalid enumName: enumName=" + enumName +
                                          ", aniStatus=" + std::to_string(aniStatus));
            return nullptr;
        }

        if (aniStatus != ani_status::ANI_NOT_FOUND) {
            AniCheckAndThrowToDynamic(
                env, aniStatus, "FindEnum failed: enumName=" + enumName + ", aniStatus=" + std::to_string(aniStatus));
            return nullptr;
        }

        // 3. find it by interop runtime linker
        ani_ref ref = nullptr;
        aniStatus = GetClassFromInteropDefaultLinker(env, aniEnv, enumName, &ref);
        if (aniStatus == ani_status::ANI_OK && ref != nullptr) {
            return ref;
        }

        AniCheckAndThrowToDynamic(env, aniStatus,
                                  "FindEnum failed: enumName=" + enumName + ", aniStatus=" + std::to_string(aniStatus));
        return nullptr;
    };

    return STValueTemplateFindElement(env, info, findEnum);
}
}  // namespace ark::ets::interop::js