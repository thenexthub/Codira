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

#ifndef ASSEMBLER_ASSEMBLY_TYPE_H
#define ASSEMBLER_ASSEMBLY_TYPE_H

#include "define.h"
#include "file_items.h"
#include "isa.h"

namespace panda::pandasm {

constexpr std::string_view STRING_CLASS_NAME = "panda.String";

class Type {
public:
    enum VerificationType {
        TYPE_ID_OBJECT,
        TYPE_ID_ARRAY,
        TYPE_ID_ANY_OBJECT,
    };

    Type() = default;
    DEFAULT_MOVE_SEMANTIC(Type);
    DEFAULT_COPY_SEMANTIC(Type);
    ~Type() = default;

    Type(const std::string_view &component_name, size_t rank, bool ignore_primitive = false)
        : component_name_(component_name), rank_(rank)
    {
        name_ = GetName(component_name_, rank_);
        type_id_ = GetId(name_, ignore_primitive);
    }

    Type(const Type &component_type, size_t rank)
        : Type(component_type.GetComponentName(), component_type.GetRank() + rank)
    {
    }

    std::string GetDescriptor(bool ignore_primitive = false) const;

    std::string GetName() const
    {
        return name_;
    }

    std::string GetPandasmName() const
    {
        std::string name_pa {name_};
        std::replace(name_pa.begin(), name_pa.end(), '/', '.');
        return name_pa;
    }

    std::string GetComponentName() const
    {
        return component_name_;
    }

    size_t GetRank() const
    {
        return rank_;
    }

    Type GetComponentType() const
    {
        return Type(component_name_, rank_ > 0 ? rank_ - 1 : 0);
    }

    panda_file::Type::TypeId GetId() const
    {
        return type_id_;
    }

    bool IsArrayContainsPrimTypes() const
    {
        auto elem = GetId(component_name_);
        return elem != panda_file::Type::TypeId::REFERENCE;
    }

    bool IsValid() const
    {
        return !component_name_.empty();
    }

    bool IsArray() const
    {
        return rank_ > 0;
    }

    bool IsObject() const
    {
        return type_id_ == panda_file::Type::TypeId::REFERENCE;
    }

    bool IsTagged() const
    {
        return type_id_ == panda_file::Type::TypeId::TAGGED;
    }

    bool IsIntegral() const
    {
        return type_id_ == panda_file::Type::TypeId::U1 || type_id_ == panda_file::Type::TypeId::U8 ||
               type_id_ == panda_file::Type::TypeId::I8 || type_id_ == panda_file::Type::TypeId::U16 ||
               type_id_ == panda_file::Type::TypeId::I16 || type_id_ == panda_file::Type::TypeId::U32 ||
               type_id_ == panda_file::Type::TypeId::I32 || type_id_ == panda_file::Type::TypeId::U64 ||
               type_id_ == panda_file::Type::TypeId::I64;
    }

    bool FitsInto32() const
    {
        return type_id_ == panda_file::Type::TypeId::U1 || type_id_ == panda_file::Type::TypeId::U8 ||
               type_id_ == panda_file::Type::TypeId::I8 || type_id_ == panda_file::Type::TypeId::U16 ||
               type_id_ == panda_file::Type::TypeId::I16 || type_id_ == panda_file::Type::TypeId::U32 ||
               type_id_ == panda_file::Type::TypeId::I32;
    }

    bool IsFloat32() const
    {
        return type_id_ == panda_file::Type::TypeId::F32;
    }

    bool IsFloat64() const
    {
        return type_id_ == panda_file::Type::TypeId::F64;
    }

    bool IsPrim32() const
    {
        return (IsIntegral() && FitsInto32()) || IsFloat32();
    }

    bool IsPrim64() const
    {
        return (IsIntegral() && !FitsInto32()) || IsFloat64();
    }

    bool IsPrimitive() const
    {
        return IsPrim64() || IsPrim32();
    }

    bool IsVoid() const
    {
        return type_id_ == panda_file::Type::TypeId::VOID;
    }

    PANDA_PUBLIC_API static panda_file::Type::TypeId GetId(const std::string_view &name, bool ignore_primitive = false);

    bool operator==(const Type &type) const
    {
        return name_ == type.name_;
    }

    static PANDA_PUBLIC_API Type FromDescriptor(std::string_view descriptor);

    static PANDA_PUBLIC_API Type FromName(std::string_view name, bool ignore_primitive = false);

    static PANDA_PUBLIC_API bool IsPandaPrimitiveType(const std::string &name);
    static bool IsStringType(const std::string &name, panda::panda_file::SourceLang lang);

private:
    static PANDA_PUBLIC_API std::string GetName(std::string_view component_name, size_t rank);

    std::string component_name_;
    size_t rank_ {0};
    std::string name_;
    panda_file::Type::TypeId type_id_ {panda_file::Type::TypeId::VOID};
};

}  // namespace panda::pandasm

namespace std {

template <>
class hash<panda::pandasm::Type> {
public:
    size_t operator()(const panda::pandasm::Type &type) const
    {
        return std::hash<std::string>()(type.GetName());
    }
};

}  // namespace std

#endif  // ASSEMBLER_ASSEMBLY_TYPE_H
