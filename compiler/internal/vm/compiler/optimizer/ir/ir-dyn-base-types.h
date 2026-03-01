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

#ifndef COMPILER_OPTIMIZER_IR_IR_DYN_BASE_TYPES_H
#define COMPILER_OPTIMIZER_IR_IR_DYN_BASE_TYPES_H

#include "compiler/optimizer/ir/datatype.h"
#include "source_languages.h"

#include <optional>

namespace panda::compiler {
inline AnyBaseType NumericDataTypeToAnyType(panda::compiler::DataType::Type type,
                                            [[maybe_unused]] panda::compiler::SourceLanguage language)
{
    switch (type) {
        case panda::compiler::DataType::Type::INT32:
            return panda::compiler::AnyBaseType::ECMASCRIPT_INT_TYPE;
        case panda::compiler::DataType::Type::UINT32:
        case panda::compiler::DataType::Type::INT64:
        case panda::compiler::DataType::Type::UINT64:
            return panda::compiler::AnyBaseType::UNDEFINED_TYPE;
        case panda::compiler::DataType::Type::FLOAT64:
            return panda::compiler::AnyBaseType::ECMASCRIPT_DOUBLE_TYPE;
        default:
            UNREACHABLE();
    }
}

inline AnyBaseType GetAnyStringType([[maybe_unused]] panda::compiler::SourceLanguage language)
{
    return panda::compiler::AnyBaseType::ECMASCRIPT_STRING_TYPE;
}

/*
 * Checks that the exact type of `any` value that is subtype of `type` also can be subtype of `super_type`.
 * Returns `true` if the `type` is equal to or subtype of `super_type` (i.e `type` is `STRING_TYPE` and
 * `super_type` is `OBJECT_TYPE`).
 * Returns `false` if there is no subtype relationship beetween `type` an `super_type` (i.e `type` is `INT_TYPE`
 * and `super_type` is `OBJECT_TYPE`)
 * Return `nullopt` if the `super_type` is subtype of `type` (i.e `type` is `OBJECT` and `super_type` is `STRING`).
 * In this case we need to check exact type at the runtime.
 */
inline std::optional<bool> IsAnyTypeCanBeSubtypeOf(AnyBaseType super_type, AnyBaseType type,
                                                   [[maybe_unused]] panda::compiler::SourceLanguage language)
{
    if (super_type == type) {
        return true;
    }
    switch (super_type) {
        case panda::compiler::AnyBaseType::ECMASCRIPT_OBJECT_TYPE:
            return type == panda::compiler::AnyBaseType::ECMASCRIPT_STRING_TYPE;
        case panda::compiler::AnyBaseType::ECMASCRIPT_STRING_TYPE:
            if (type == panda::compiler::AnyBaseType::ECMASCRIPT_OBJECT_TYPE) {
                return std::nullopt;
            }
            return false;
        default:
            break;
    }
    return false;
}

inline panda::compiler::DataType::Type AnyBaseTypeToDataType([[maybe_unused]] AnyBaseType any_type)
{
    return panda::compiler::DataType::Type::ANY;
}

inline const char *AnyTypeTypeToString(AnyBaseType any_type)
{
    return "UNDEFINED_TYPE";
}

} // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_IR_DYN_BASE_TYPES_H
