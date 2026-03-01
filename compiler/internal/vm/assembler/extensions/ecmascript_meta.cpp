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

#include "ecmascript_meta.h"

namespace panda::pandasm::extensions::ecmascript {

std::optional<Metadata::Error> RecordMetadata::Validate(const std::string_view &attribute) const
{
    if (attribute == "ecmascript.annotation") {
        if (HasAttribute(attribute)) {
            return Error("Attribute 'ecmascript.annotation' already defined",
                         Error::Type::MULTIPLE_ATTRIBUTE);
        }
        return {};
    }

    if (attribute == "ecmascript.extends") {
        return Error("Attribute 'ecmascript.extends' must have a value",
                     Error::Type::MISSING_VALUE);
    }

    return pandasm::RecordMetadata::Validate(attribute);
}

std::optional<Metadata::Error> RecordMetadata::Validate(const std::string_view &attribute,
                                                        const std::string_view &value) const
{
    if (attribute == "ecmascript.extends") {
        if (HasAttribute(attribute)) {
            return Error("Attribute 'ecmascript.extends' already defined",
                         Error::Type::MULTIPLE_ATTRIBUTE);
        }
        return {};
    }

    if (attribute == "ecmascript.annotation") {
        return Error("Attribute 'ecmascript.annotation' must not have a value",
                     Error::Type::UNEXPECTED_VALUE);
    }

    return pandasm::RecordMetadata::Validate(attribute, value);
}

std::optional<Metadata::Error> FieldMetadata::Validate(const std::string_view &attribute) const
{
    return pandasm::FieldMetadata::Validate(attribute);
}

std::optional<Metadata::Error> FieldMetadata::Validate(const std::string_view &attribute,
                                                       const std::string_view &value) const
{
    return pandasm::FieldMetadata::Validate(attribute, value);
}

std::optional<Metadata::Error> FunctionMetadata::Validate(const std::string_view &attribute) const
{
    return pandasm::FunctionMetadata::Validate(attribute);
}

std::optional<Metadata::Error> FunctionMetadata::Validate(const std::string_view &attribute,
                                                          const std::string_view &value) const
{
    return pandasm::FunctionMetadata::Validate(attribute, value);
}

std::optional<Metadata::Error> ParamMetadata::Validate(const std::string_view &attribute) const
{
    return pandasm::ParamMetadata::Validate(attribute);
}

std::optional<Metadata::Error> ParamMetadata::Validate(const std::string_view &attribute,
                                                       const std::string_view &value) const
{
    return pandasm::ParamMetadata::Validate(attribute, value);
}

void RecordMetadata::SetFlags(const std::string_view &attribute)
{
    if (attribute == "ecmascript.annotation") {
        SetAccessFlags(GetAccessFlags() | ACC_ANNOTATION);
    }
    pandasm::RecordMetadata::SetFlags(attribute);
}

void RecordMetadata::SetFlags(const std::string_view &attribute, const std::string_view &value)
{
    pandasm::RecordMetadata::SetFlags(attribute, value);
}

void RecordMetadata::RemoveFlags(const std::string_view &attribute)
{
    if (attribute == "ecmascript.annotation") {
        if ((GetAccessFlags() & ACC_ANNOTATION) != 0) {
            SetAccessFlags(GetAccessFlags() ^ (ACC_ANNOTATION));
        }
    }
    pandasm::RecordMetadata::RemoveFlags(attribute);
}

void RecordMetadata::RemoveFlags(const std::string_view &attribute, const std::string_view &value)
{
    pandasm::RecordMetadata::RemoveFlags(attribute, value);
}

void FieldMetadata::SetFlags(const std::string_view &attribute)
{
    pandasm::FieldMetadata::SetFlags(attribute);
}

void FieldMetadata::SetFlags(const std::string_view &attribute, const std::string_view &value)
{
    pandasm::FieldMetadata::SetFlags(attribute, value);
}

void FieldMetadata::RemoveFlags(const std::string_view &attribute)
{
    pandasm::FieldMetadata::RemoveFlags(attribute);
}

void FieldMetadata::RemoveFlags(const std::string_view &attribute, const std::string_view &value)
{
    pandasm::FieldMetadata::RemoveFlags(attribute, value);
}

void FunctionMetadata::SetFlags(const std::string_view &attribute)
{
    pandasm::FunctionMetadata::SetFlags(attribute);
}

void FunctionMetadata::SetFlags(const std::string_view &attribute, const std::string_view &value)
{
    pandasm::FunctionMetadata::SetFlags(attribute, value);
}

void FunctionMetadata::RemoveFlags(const std::string_view &attribute)
{
    pandasm::FunctionMetadata::RemoveFlags(attribute);
}

void FunctionMetadata::RemoveFlags(const std::string_view &attribute, const std::string_view &value)
{
    pandasm::FunctionMetadata::RemoveFlags(attribute, value);
}

void ParamMetadata::SetFlags(const std::string_view &attribute)
{
    pandasm::ParamMetadata::SetFlags(attribute);
}

void ParamMetadata::SetFlags(const std::string_view &attribute, const std::string_view &value)
{
    pandasm::ParamMetadata::SetFlags(attribute, value);
}

void ParamMetadata::RemoveFlags(const std::string_view &attribute)
{
    pandasm::ParamMetadata::RemoveFlags(attribute);
}

void ParamMetadata::RemoveFlags(const std::string_view &attribute, const std::string_view &value)
{
    pandasm::ParamMetadata::RemoveFlags(attribute, value);
}

}  // namespace panda::pandasm::extensions::ecmascript
