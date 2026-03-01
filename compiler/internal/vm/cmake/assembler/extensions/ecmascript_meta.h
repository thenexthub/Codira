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

#ifndef ASSEMBLER_EXTENSIONS_ECMASCRIPT_META_H
#define ASSEMBLER_EXTENSIONS_ECMASCRIPT_META_H

#include "meta.h"

namespace panda::pandasm::extensions::ecmascript {

class RecordMetadata : public pandasm::RecordMetadata {
public:
    std::string GetBase() const override
    {
        auto base = GetAttributeValue("ecmascript.extends");
        if (base) {
            return base.value();
        }

        return "";
    }

    std::vector<std::string> GetInterfaces() const override
    {
        return {};
    }

    bool IsAnnotation() const override
    {
        return (GetAccessFlags() & ACC_ANNOTATION) != 0;
    }

    bool IsRuntimeAnnotation() const override
    {
        return false;
    }

protected:
    bool IsAnnotationRecordAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationIdAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementNameAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementTypeAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementArrayComponentTypeAttribute(
        [[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementValueAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    std::optional<Error> Validate(const std::string_view &attribute) const override;

    std::optional<Error> Validate(const std::string_view &attribute, const std::string_view &value) const override;

    void SetFlags(const std::string_view &attribute) override;

    void SetFlags(const std::string_view &attribute, const std::string_view &value) override;

    void RemoveFlags(const std::string_view &attribute) override;

    void RemoveFlags(const std::string_view &attribute, const std::string_view &value) override;
};

class FieldMetadata : public pandasm::FieldMetadata {
protected:
    bool IsAnnotationRecordAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationIdAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementNameAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementTypeAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementArrayComponentTypeAttribute(
        [[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementValueAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    std::optional<Error> Validate(const std::string_view &attribute) const override;

    std::optional<Error> Validate(const std::string_view &attribute, const std::string_view &value) const override;

    void SetFlags(const std::string_view &attribute) override;

    void SetFlags(const std::string_view &attribute, const std::string_view &value) override;

    void RemoveFlags(const std::string_view &attribute) override;

    void RemoveFlags(const std::string_view &attribute, const std::string_view &value) override;
};

class FunctionMetadata : public pandasm::FunctionMetadata {
protected:
    bool IsAnnotationRecordAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationIdAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementNameAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementTypeAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementArrayComponentTypeAttribute(
        [[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementValueAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    std::optional<Error> Validate(const std::string_view &attribute) const override;

    std::optional<Error> Validate(const std::string_view &attribute, const std::string_view &value) const override;

    void SetFlags(const std::string_view &attribute) override;

    void SetFlags(const std::string_view &attribute, const std::string_view &value) override;

    void RemoveFlags(const std::string_view &attribute) override;

    void RemoveFlags(const std::string_view &attribute, const std::string_view &value) override;
};

class ParamMetadata : public pandasm::ParamMetadata {
protected:
    bool IsAnnotationRecordAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationIdAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementNameAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementTypeAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementArrayComponentTypeAttribute(
        [[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    bool IsAnnotationElementValueAttribute([[maybe_unused]] const std::string_view &attribute) const override
    {
        return false;
    }

    std::optional<Error> Validate(const std::string_view &attribute) const override;

    std::optional<Error> Validate(const std::string_view &attribute, const std::string_view &value) const override;

    void SetFlags(const std::string_view &attribute) override;

    void SetFlags(const std::string_view &attribute, const std::string_view &value) override;

    void RemoveFlags(const std::string_view &attribute) override;

    void RemoveFlags(const std::string_view &attribute, const std::string_view &value) override;
};

}  // namespace panda::pandasm::extensions::ecmascript

#endif  // ASSEMBLER_EXTENSIONS_ECMASCRIPT_META_H
