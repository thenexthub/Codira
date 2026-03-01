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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_LITERALS_H
#define PANDA_ASSEMBLER_ASSEMBLY_LITERALS_H

#include <string>
#include <vector>

#include "libarkfile/literal_data_accessor-inl.h"

namespace ark::pandasm {

struct LiteralArray {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    struct Literal {
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        panda_file::LiteralTag tag;
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        std::variant<bool, uint8_t, uint16_t, uint32_t, uint64_t, float, double, std::string> value;

        bool IsSigned() const
        {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_I8:
                case panda_file::LiteralTag::ARRAY_I16:
                case panda_file::LiteralTag::ARRAY_I32:
                case panda_file::LiteralTag::ARRAY_I64:
                case panda_file::LiteralTag::INTEGER:
                case panda_file::LiteralTag::BIGINT:
                    return true;
                default:
                    return false;
            }
        }

        bool IsArray() const
        {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_U1:
                case panda_file::LiteralTag::ARRAY_U8:
                case panda_file::LiteralTag::ARRAY_I8:
                case panda_file::LiteralTag::ARRAY_U16:
                case panda_file::LiteralTag::ARRAY_I16:
                case panda_file::LiteralTag::ARRAY_U32:
                case panda_file::LiteralTag::ARRAY_I32:
                case panda_file::LiteralTag::ARRAY_U64:
                case panda_file::LiteralTag::ARRAY_I64:
                case panda_file::LiteralTag::ARRAY_F32:
                case panda_file::LiteralTag::ARRAY_F64:
                case panda_file::LiteralTag::ARRAY_STRING:
                    return true;
                default:
                    return false;
            }
        }

        bool IsBoolValue() const
        {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_U1:
                case panda_file::LiteralTag::BOOL:
                    return true;
                default:
                    return false;
            }
        }

        bool IsByteValue() const
        {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_U8:
                case panda_file::LiteralTag::ARRAY_I8:
                case panda_file::LiteralTag::TAGVALUE:
                case panda_file::LiteralTag::ACCESSOR:
                case panda_file::LiteralTag::NULLVALUE:
                    return true;
                default:
                    return false;
            }
        }

        bool IsShortValue() const
        {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_U16:
                case panda_file::LiteralTag::ARRAY_I16:
                    return true;
                default:
                    return false;
            }
        }

        bool IsIntegerValue() const
        {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_U32:
                case panda_file::LiteralTag::ARRAY_I32:
                case panda_file::LiteralTag::INTEGER:
                    return true;
                default:
                    return false;
            }
        }

        bool IsLongValue() const
        {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_U64:
                case panda_file::LiteralTag::ARRAY_I64:
                case panda_file::LiteralTag::BIGINT:
                    return true;
                default:
                    return false;
            }
        }

        bool IsFloatValue() const
        {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_F32:
                case panda_file::LiteralTag::FLOAT:
                    return true;
                default:
                    return false;
            }
        }

        bool IsDoubleValue() const
        {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_F64:
                case panda_file::LiteralTag::DOUBLE:
                    return true;
                default:
                    return false;
            }
        }

        bool IsStringValue() const
        {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_STRING:
                case panda_file::LiteralTag::STRING:
                case panda_file::LiteralTag::METHOD:
                case panda_file::LiteralTag::GENERATORMETHOD:
                case panda_file::LiteralTag::ASYNCGENERATORMETHOD:
                case panda_file::LiteralTag::ASYNCMETHOD:
                    return true;
                default:
                    return false;
            }
        }

        bool IsLiteralArrayValue() const
        {
            return tag == panda_file::LiteralTag::LITERALARRAY;
        }

        bool operator==(const LiteralArray::Literal &literal)
        {
            return tag == literal.tag && value == literal.value;
        }

        bool operator!=(const LiteralArray::Literal &literal)
        {
            return !(*this == literal);
        }
    };

    using LiteralVector = std::vector<ark::pandasm::LiteralArray::Literal>;

    LiteralVector literals;  // NOLINT(misc-non-private-member-variables-in-classes)

    explicit LiteralArray(LiteralVector literalsVec) : literals(std::move(literalsVec)) {}
    explicit LiteralArray() = default;

    static constexpr panda_file::LiteralTag GetArrayTagFromComponentType(panda_file::Type::TypeId type)
    {
        switch (type) {
            case panda_file::Type::TypeId::U1:
                return panda_file::LiteralTag::ARRAY_U1;
            case panda_file::Type::TypeId::U8:
                return panda_file::LiteralTag::ARRAY_U8;
            case panda_file::Type::TypeId::I8:
                return panda_file::LiteralTag::ARRAY_I8;
            case panda_file::Type::TypeId::U16:
                return panda_file::LiteralTag::ARRAY_U16;
            case panda_file::Type::TypeId::I16:
                return panda_file::LiteralTag::ARRAY_I16;
            case panda_file::Type::TypeId::U32:
                return panda_file::LiteralTag::ARRAY_U32;
            case panda_file::Type::TypeId::I32:
                return panda_file::LiteralTag::ARRAY_I32;
            case panda_file::Type::TypeId::U64:
                return panda_file::LiteralTag::ARRAY_U64;
            case panda_file::Type::TypeId::I64:
                return panda_file::LiteralTag::ARRAY_I64;
            case panda_file::Type::TypeId::F32:
                return panda_file::LiteralTag::ARRAY_F32;
            case panda_file::Type::TypeId::F64:
                return panda_file::LiteralTag::ARRAY_F64;
            default:
                UNREACHABLE();
        }
    }

    static constexpr panda_file::LiteralTag GetLiteralTagFromComponentType(panda_file::Type::TypeId type)
    {
        switch (type) {
            case panda_file::Type::TypeId::U1:
                return panda_file::LiteralTag::BOOL;
            case panda_file::Type::TypeId::U8:
            case panda_file::Type::TypeId::I8:
            case panda_file::Type::TypeId::U16:
            case panda_file::Type::TypeId::I16:
            case panda_file::Type::TypeId::U32:
            case panda_file::Type::TypeId::I32:
                return panda_file::LiteralTag::INTEGER;
            case panda_file::Type::TypeId::U64:
            case panda_file::Type::TypeId::I64:
                return panda_file::LiteralTag::BIGINT;
            case panda_file::Type::TypeId::F32:
                return panda_file::LiteralTag::FLOAT;
            case panda_file::Type::TypeId::F64:
                return panda_file::LiteralTag::DOUBLE;
            default:
                UNREACHABLE();
        }
    }
};

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_LITERALS_H
