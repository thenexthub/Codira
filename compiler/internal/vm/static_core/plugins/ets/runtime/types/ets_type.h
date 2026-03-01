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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPE_H_

#include "plugins/ets/runtime/types/ets_primitives.h"
#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "runtime/include/mem/panda_string.h"

namespace ark::ets {

// Must be synchronized with
// plugins/ecmascript/es2panda/compiler/scripts/signatures.yaml
static constexpr char ARRAY_TYPE_PREFIX = '[';
static constexpr char CLASS_TYPE_PREFIX = 'L';
static constexpr char UNION_OR_ENUM_TYPE_PREFIX = '{';
static constexpr char METHOD_PREFIX = 'M';
static constexpr const char *TYPE_API_UNDEFINED_TYPE_DESC = "__TYPE_API_UNDEFINED";
static constexpr const char *INVOKE_METHOD_NAME = "$_invoke";
static constexpr const char *CONSTRUCTOR_NAME = "constructor";
static constexpr char TYPE_DESC_DELIMITER = ';';
//--------------
static constexpr const char *PROPERTY = "%%property-";
static constexpr const uint8_t PROPERTY_PREFIX_LENGTH = 11;
//--------------
static constexpr const char *GETTER_BEGIN = "%%get-";
static constexpr const char *SETTER_BEGIN = "%%set-";
static constexpr const uint8_t SETTER_GETTER_PREFIX_LENGTH = 6;
//--------------
static constexpr const char *ITERATOR_METHOD = "$_iterator";
static constexpr const char *GET_INDEX_METHOD = "$_get";
static constexpr const char *SET_INDEX_METHOD = "$_set";

static constexpr const char *STD_CORE_FUNCTION_PREFIX = "std.core.Function";
static constexpr const char *STD_CORE_FUNCTION_UNSAFECALL_METHOD = "unsafeCall";
static constexpr const char *STD_CORE_FUNCTION_INVOKE_PREFIX = "invoke";
static constexpr const char *STD_CORE_FUNCTION_INVOKE_R_PREFIX = "invokeR";

enum class EtsType { BOOLEAN, BYTE, CHAR, SHORT, INT, LONG, FLOAT, DOUBLE, OBJECT, UNKNOWN, VOID };

// CC-OFFNXT(G.FUD.06) switch-case, ODR
inline EtsType ConvertPandaTypeToEtsType(panda_file::Type type)
{
    switch (type.GetId()) {
        case panda_file::Type::TypeId::INVALID:
            return EtsType::UNKNOWN;
        case panda_file::Type::TypeId::VOID:
            return EtsType::VOID;
        case panda_file::Type::TypeId::U1:
            return EtsType::BOOLEAN;
        case panda_file::Type::TypeId::I8:
            return EtsType::BYTE;
        case panda_file::Type::TypeId::U16:
            return EtsType::CHAR;
        case panda_file::Type::TypeId::I16:
            return EtsType::SHORT;
        case panda_file::Type::TypeId::I32:
            return EtsType::INT;
        case panda_file::Type::TypeId::I64:
            return EtsType::LONG;
        case panda_file::Type::TypeId::F32:
            return EtsType::FLOAT;
        case panda_file::Type::TypeId::F64:
            return EtsType::DOUBLE;
        case panda_file::Type::TypeId::REFERENCE:
            return EtsType::OBJECT;
        case panda_file::Type::TypeId::TAGGED:
            return EtsType::UNKNOWN;
        default:
            return EtsType::UNKNOWN;
    }
    UNREACHABLE();
}

// CC-OFFNXT(G.FUD.06) switch-case, ODR
inline panda_file::Type ConvertEtsTypeToPandaType(const EtsType type)
{
    switch (type) {
        case EtsType::VOID:
            return panda_file::Type(panda_file::Type::TypeId::VOID);
        case EtsType::BOOLEAN:
            return panda_file::Type(panda_file::Type::TypeId::U1);
        case EtsType::BYTE:
            return panda_file::Type(panda_file::Type::TypeId::I8);
        case EtsType::CHAR:
            return panda_file::Type(panda_file::Type::TypeId::U16);
        case EtsType::SHORT:
            return panda_file::Type(panda_file::Type::TypeId::I16);
        case EtsType::INT:
            return panda_file::Type(panda_file::Type::TypeId::I32);
        case EtsType::LONG:
            return panda_file::Type(panda_file::Type::TypeId::I64);
        case EtsType::FLOAT:
            return panda_file::Type(panda_file::Type::TypeId::F32);
        case EtsType::DOUBLE:
            return panda_file::Type(panda_file::Type::TypeId::F64);
        case EtsType::OBJECT:
            return panda_file::Type(panda_file::Type::TypeId::REFERENCE);
        default:
            return panda_file::Type(panda_file::Type::TypeId::INVALID);
    }
}

// CC-OFFNXT(G.FUD.06) switch-case, ODR
inline PandaString EtsTypeToString(const EtsType type)
{
    switch (type) {
        case EtsType::VOID:
            return "void";
        case EtsType::BOOLEAN:
            return "boolean";
        case EtsType::BYTE:
            return "byte";
        case EtsType::CHAR:
            return "char";
        case EtsType::SHORT:
            return "short";
        case EtsType::INT:
            return "int";
        case EtsType::LONG:
            return "long";
        case EtsType::FLOAT:
            return "float";
        case EtsType::DOUBLE:
            return "double";
        case EtsType::OBJECT:
            return "object";
        default:
            return "invalid";
    }
    UNREACHABLE();
}

template <typename T>
constexpr EtsType GetEtsTypeByPrimitive()
{
    if constexpr (std::is_same_v<T, EtsBoolean>) {
        return EtsType::BOOLEAN;
    } else if constexpr (std::is_same_v<T, EtsChar>) {
        return EtsType::CHAR;
    } else if constexpr (std::is_same_v<T, EtsByte>) {
        return EtsType::BYTE;
    } else if constexpr (std::is_same_v<T, EtsShort>) {
        return EtsType::SHORT;
    } else if constexpr (std::is_same_v<T, EtsInt>) {
        return EtsType::INT;
    } else if constexpr (std::is_same_v<T, EtsLong>) {
        return EtsType::LONG;
    } else if constexpr (std::is_same_v<T, EtsFloat>) {
        return EtsType::FLOAT;
    } else if constexpr (std::is_same_v<T, EtsDouble>) {
        return EtsType::DOUBLE;
    } else {
        static_assert(true, "Unsupported Ets primitive");
    }
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPE_H_
