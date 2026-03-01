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

#ifndef PANDA_ASSEMBLER_EXTENSIONS_ETS_META_H_
#define PANDA_ASSEMBLER_EXTENSIONS_ETS_META_H_

#include "meta.h"

namespace ark::pandasm::extensions::ets {

class AnnotationHelper {
public:
    static bool IsAnnotationRecordAttribute(std::string_view attribute)
    {
        return attribute == "ets.annotation.class";
    }

    static bool IsAnnotationIdAttribute(std::string_view attribute)
    {
        return attribute == "ets.annotation.id";
    }

    static bool IsAnnotationElementTypeAttribute(std::string_view attribute)
    {
        return attribute == "ets.annotation.element.type";
    }

    static bool IsAnnotationElementArrayComponentTypeAttribute(std::string_view attribute)
    {
        return attribute == "ets.annotation.element.array.component.type";
    }

    static bool IsAnnotationElementNameAttribute(std::string_view attribute)
    {
        return attribute == "ets.annotation.element.name";
    }

    static bool IsAnnotationElementValueAttribute(std::string_view attribute)
    {
        return attribute == "ets.annotation.element.value";
    }
};

class RecordMetadata : public pandasm::RecordMetadata {
public:
    std::string GetBase() const override
    {
        auto base = GetAttributeValue("ets.extends");
        if (base) {
            return base.value();
        }

        return "";
    }

    bool SetBase(std::string_view base) override
    {
        if (GetBase() != "std.core.Object") {
            return false;
        }

        RemoveAttributeValue("ets.extends", "std.core.Object");
        return !SetAttributeValue("ets.extends", base);
    }

    std::vector<std::string> GetInterfaces() const override
    {
        return GetAttributeValues("ets.implements");
    }

    bool AddInterface(std::string_view value) override
    {
        return !SetAttributeValue("ets.implements", value);
    }

    bool RemoveInterface(std::string_view value) override
    {
        return !RemoveAttributeValue("ets.implements", value);
    }

    bool IsAnnotation() const override
    {
        return (GetAccessFlags() & ACC_ANNOTATION) != 0;
    }

    bool IsRuntimeAnnotation() const override
    {
        auto type = GetAttributeValue("ets.annotation.type");
        if (type) {
            return type.value() == "runtime";
        }

        return false;
    }

    bool IsTypeAnnotation() const override
    {
        auto type = GetAttributeValue("ets.annotation.type");
        if (type) {
            return type.value() == "type";
        }

        return false;
    }

protected:
    bool IsAnnotationRecordAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationRecordAttribute(attribute);
    }

    bool IsAnnotationIdAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationIdAttribute(attribute);
    }

    bool IsAnnotationElementNameAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementNameAttribute(attribute);
    }

    bool IsAnnotationElementTypeAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementTypeAttribute(attribute);
    }

    bool IsAnnotationElementArrayComponentTypeAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementArrayComponentTypeAttribute(attribute);
    }

    bool IsAnnotationElementValueAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementValueAttribute(attribute);
    }

    std::optional<Error> Validate(std::string_view attribute) const override;

    std::optional<Error> Validate(std::string_view attribute, std::string_view value) const override;

    void SetFlags(std::string_view attribute) override;

    void SetFlags(std::string_view attribute, std::string_view value) override;

    void RemoveFlags(std::string_view attribute) override;

    void RemoveFlags(std::string_view attribute, std::string_view value) override;
};

class FieldMetadata : public pandasm::FieldMetadata {
protected:
    bool IsAnnotationRecordAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationRecordAttribute(attribute);
    }

    bool IsAnnotationIdAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationIdAttribute(attribute);
    }

    bool IsAnnotationElementNameAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementNameAttribute(attribute);
    }

    bool IsAnnotationElementTypeAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementTypeAttribute(attribute);
    }

    bool IsAnnotationElementArrayComponentTypeAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementArrayComponentTypeAttribute(attribute);
    }

    bool IsAnnotationElementValueAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementValueAttribute(attribute);
    }

    std::optional<Error> Validate(std::string_view attribute) const override;

    std::optional<Error> Validate(std::string_view attribute, std::string_view value) const override;

    void SetFlags(std::string_view attribute) override;

    void SetFlags(std::string_view attribute, std::string_view value) override;

    void RemoveFlags(std::string_view attribute) override;

    void RemoveFlags(std::string_view attribute, std::string_view value) override;
};

class FunctionMetadata : public pandasm::FunctionMetadata {
protected:
    bool IsAnnotationRecordAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationRecordAttribute(attribute);
    }

    bool IsAnnotationIdAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationIdAttribute(attribute);
    }

    bool IsAnnotationElementNameAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementNameAttribute(attribute);
    }

    bool IsAnnotationElementTypeAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementTypeAttribute(attribute);
    }

    bool IsAnnotationElementArrayComponentTypeAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementArrayComponentTypeAttribute(attribute);
    }

    bool IsAnnotationElementValueAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementValueAttribute(attribute);
    }

    std::optional<Error> Validate(std::string_view attribute) const override;

    std::optional<Error> Validate(std::string_view attribute, std::string_view value) const override;

    void SetFlags(std::string_view attribute) override;

    void SetFlags(std::string_view attribute, std::string_view value) override;

    void RemoveFlags(std::string_view attribute) override;

    void RemoveFlags(std::string_view attribute, std::string_view value) override;
};

class ParamMetadata : public pandasm::ParamMetadata {
protected:
    bool IsAnnotationRecordAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationRecordAttribute(attribute);
    }

    bool IsAnnotationIdAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationIdAttribute(attribute);
    }

    bool IsAnnotationElementNameAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementNameAttribute(attribute);
    }

    bool IsAnnotationElementTypeAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementTypeAttribute(attribute);
    }

    bool IsAnnotationElementArrayComponentTypeAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementArrayComponentTypeAttribute(attribute);
    }

    bool IsAnnotationElementValueAttribute([[maybe_unused]] std::string_view attribute) const override
    {
        return AnnotationHelper::IsAnnotationElementValueAttribute(attribute);
    }

    std::optional<Error> Validate(std::string_view attribute) const override;

    std::optional<Error> Validate(std::string_view attribute, std::string_view value) const override;

    void SetFlags(std::string_view attribute) override;

    void SetFlags(std::string_view attribute, std::string_view value) override;

    void RemoveFlags(std::string_view attribute) override;

    void RemoveFlags(std::string_view attribute, std::string_view value) override;
};

}  // namespace ark::pandasm::extensions::ets

#endif  // PANDA_ASSEMBLER_EXTENSIONS_ETS_META_H_
