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

#ifndef PANDA_ASSEMBLER_META_H
#define PANDA_ASSEMBLER_META_H

#include <memory>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "annotation.h"
#include "libarkfile/modifiers.h"

#include "assembly-type.h"

namespace ark::pandasm {

class Metadata {
public:
    class Error {
    public:
        enum class Type {
            INVALID_VALUE,
            MISSING_ATTRIBUTE,
            MISSING_VALUE,
            MULTIPLE_ATTRIBUTE,
            UNEXPECTED_ATTRIBUTE,
            UNEXPECTED_VALUE,
            UNKNOWN_ATTRIBUTE
        };

        Error(std::string msg, Type type) : msg_(std::move(msg)), type_(type) {}
        ~Error() = default;
        DEFAULT_MOVE_SEMANTIC(Error);
        DEFAULT_COPY_SEMANTIC(Error);

        std::string GetMessage() const
        {
            return msg_;
        }

        Type GetType() const
        {
            return type_;
        }

    private:
        std::string msg_;
        Type type_;
    };

    Metadata() = default;

    virtual ~Metadata() = default;

    std::optional<Error> SetAttribute(std::string_view attribute)
    {
        auto err = Validate(attribute);
        if (err) {
            return err;
        }

        SetFlags(attribute);

        return Store(attribute);
    }

    void RemoveAttribute(const std::string &attribute)
    {
        RemoveFlags(attribute);

        setAttributes_.erase(attribute);
    }

    std::optional<Error> RemoveAttributeValue(std::string_view attribute, std::string_view value)
    {
        return RemoveValue(attribute, value);
    }

    bool GetAttribute(const std::string &attribute) const
    {
        return setAttributes_.find(attribute) != setAttributes_.cend();
    }

    virtual std::optional<Error> RemoveValue(std::string_view attribute, std::string_view value)
    {
        std::string key(attribute);
        auto it = attributes_.find(key);
        if (it == attributes_.cend()) {
            return Error("attribute does not exist.", Error::Type::MISSING_ATTRIBUTE);
        }

        std::string valueS(value);

        auto valueIt = std::find(it->second.begin(), it->second.end(), valueS);
        if (valueIt == it->second.cend()) {
            return Error("value does not exist.", Error::Type::MISSING_VALUE);
        }

        it->second.erase(valueIt);
        return {};
    }

    std::optional<Error> SetAttributeValue(std::string_view attribute, std::string_view value)
    {
        auto err = Validate(attribute, value);
        if (err) {
            return err;
        }

        SetFlags(attribute, value);

        return StoreValue(attribute, value);
    }

    void SetAttributeValues(const std::string &attribute, const std::vector<std::string> &values)
    {
        auto it = attributes_.find(attribute);
        if (it == attributes_.cend()) {
            return;
        }

        it->second = values;
    }

    std::vector<std::string> GetAttributeValues(const std::string &attribute) const
    {
        auto it = attributes_.find(attribute);
        if (it == attributes_.cend()) {
            return {};
        }

        return it->second;
    }

    std::optional<std::string> GetAttributeValue(const std::string &attribute) const
    {
        auto values = GetAttributeValues(attribute);
        if (!values.empty()) {
            return values.front();
        }

        return {};
    }

    const std::unordered_set<std::string> &GetBoolAttributes() const
    {
        return setAttributes_;
    }

    const std::unordered_map<std::string, std::vector<std::string>> &GetAttributes() const
    {
        return attributes_;
    }

    virtual std::optional<Error> ValidateData()
    {
        return {};
    }

    DEFAULT_COPY_SEMANTIC(Metadata);

    DEFAULT_MOVE_SEMANTIC(Metadata);

protected:
    virtual std::optional<Error> Validate(std::string_view attribute) const = 0;

    virtual std::optional<Error> Validate(std::string_view attribute, std::string_view value) const = 0;

    virtual std::optional<Error> StoreValue(std::string_view attribute, std::string_view value)
    {
        std::string key(attribute);
        auto it = attributes_.find(key);
        if (it == attributes_.cend()) {
            std::tie(it, std::ignore) = attributes_.try_emplace(key);
        }

        it->second.emplace_back(value);

        return {};
    }

    virtual std::optional<Error> Store(std::string_view attribute)
    {
        setAttributes_.emplace(attribute);

        return {};
    }

    virtual void SetFlags(std::string_view attribute) = 0;

    virtual void SetFlags(std::string_view attribute, std::string_view value) = 0;

    virtual void RemoveFlags(std::string_view attribute) = 0;

    virtual void RemoveFlags(std::string_view attribute, std::string_view value) = 0;

    bool HasAttribute(std::string_view attribute) const
    {
        std::string key(attribute);
        return GetAttribute(key) || GetAttributeValue(key);
    }

    std::optional<Error> ValidateSize(std::string_view value) const;

private:
    std::unordered_set<std::string> setAttributes_;
    std::unordered_map<std::string, std::vector<std::string>> attributes_;
};

class AnnotationMetadata : public Metadata {
public:
    const std::vector<AnnotationData> &GetAnnotations() const
    {
        return annotations_;
    }

    void SetAnnotations(std::vector<AnnotationData> &&annotations)
    {
        ASSERT(annotations_.empty());
        annotations_ = std::forward<std::vector<AnnotationData>>(annotations);
    }

    void AddAnnotations(const std::vector<AnnotationData> &annotations)
    {
        annotations_.insert(annotations_.end(), annotations.begin(), annotations.end());
    }

    void EnumerateAnnotations(const std::function<void(AnnotationData &)> &callback)
    {
        for (auto &annotation : annotations_) {
            callback(annotation);
        }
    }

    void DeleteAnnotationElementByName(std::string_view annotationName, std::string_view annotationElemName)
    {
        auto annotationIter =
            std::find_if(annotations_.begin(), annotations_.end(), [&](pandasm::AnnotationData &annotation) -> bool {
                return annotation.GetName() == annotationName;
            });
        if (annotationIter != annotations_.end()) {
            annotationIter->DeleteAnnotationElementByName(annotationElemName);
        }
    }

    void DeleteAnnotationByName(const std::string_view &annotationName)
    {
        auto annotationIter =
            std::find_if(annotations_.begin(), annotations_.end(), [&](pandasm::AnnotationData &annotation) -> bool {
                return annotation.GetName() == annotationName;
            });
        if (annotationIter != annotations_.end()) {
            (void)annotations_.erase(annotationIter);
        }
    }

    void AddAnnotationElementByName(const std::string_view &annotationName, AnnotationElement &&element)
    {
        auto annotationIter =
            std::find_if(annotations_.begin(), annotations_.end(), [&](pandasm::AnnotationData &annotation) -> bool {
                return annotation.GetName() == annotationName;
            });
        if (annotationIter != annotations_.end()) {
            annotationIter->AddElement(std::move(element));
        }
    }

    std::optional<Error> ValidateData() override;

protected:
    std::optional<Error> Store(std::string_view attribute) override;

    std::optional<Error> StoreValue(std::string_view attribute, std::string_view value) override;

    virtual bool IsAnnotationRecordAttribute([[maybe_unused]] std::string_view attribute) const
    {
        return false;
    }

    virtual bool IsAnnotationIdAttribute([[maybe_unused]] std::string_view attribute) const
    {
        return false;
    }

    virtual bool IsAnnotationElementTypeAttribute([[maybe_unused]] std::string_view attribute) const
    {
        return false;
    }

    virtual bool IsAnnotationElementArrayComponentTypeAttribute([[maybe_unused]] std::string_view attribute) const
    {
        return false;
    }

    virtual bool IsAnnotationElementNameAttribute([[maybe_unused]] std::string_view attribute) const
    {
        return false;
    }

    virtual bool IsAnnotationElementValueAttribute([[maybe_unused]] std::string_view attribute) const
    {
        return false;
    }

private:
    class AnnotationElementBuilder {
    public:
        void Initialize(std::string_view name)
        {
            name_ = name;
            isInitialized_ = true;
        }

        void Reset()
        {
            name_.clear();
            values_.clear();
            type_ = {};
            componentType_ = {};
            isInitialized_ = false;
        }

        void SetType(Value::Type type)
        {
            type_ = type;
        }

        void SetComponentType(Value::Type type)
        {
            ASSERT(type != Value::Type::ARRAY);
            componentType_ = type;
        }

        std::optional<Error> AddValue(
            std::string_view value,
            const std::unordered_map<std::string, std::unique_ptr<AnnotationData>> &annotationIdMap);

        AnnotationElement CreateAnnotationElement()
        {
            if (IsArray()) {
                return AnnotationElement(name_,
                                         std::make_unique<ArrayValue>(componentType_.value(), std::move(values_)));
            }

            return AnnotationElement(name_, std::make_unique<ScalarValue>(values_.front()));
        }

        bool IsArray() const
        {
            return type_.value() == Value::Type::ARRAY;
        }

        bool IsTypeSet() const
        {
            return type_.has_value();
        }

        bool IsComponentTypeSet() const
        {
            return componentType_.has_value();
        }

        bool IsInitialized() const
        {
            return isInitialized_;
        }

        bool IsCompleted() const
        {
            if (!IsTypeSet()) {
                return false;
            }

            if (IsArray() && !IsComponentTypeSet()) {
                return false;
            }

            if (!IsArray() && values_.empty()) {
                return false;
            }

            return true;
        }

    private:
        bool isInitialized_ {false};
        std::string name_;
        std::optional<Value::Type> type_;
        std::optional<Value::Type> componentType_;
        std::vector<ScalarValue> values_;
    };

    class AnnotationBuilder {
    public:
        void Initialize(std::string_view name)
        {
            name_ = name;
            isInitialized_ = true;
        }

        void Reset()
        {
            name_.clear();
            elements_.clear();
            id_ = {};
            isInitialized_ = false;
        }

        void SetId(std::string_view id)
        {
            id_ = id;
        }

        std::string GetId() const
        {
            ASSERT(HasId());
            return id_.value();
        }

        void AddElement(AnnotationElement &&element)
        {
            elements_.push_back(std::forward<AnnotationElement>(element));
        }

        std::unique_ptr<AnnotationData> CreateAnnotationData()
        {
            return std::make_unique<AnnotationData>(name_, std::move(elements_));
        };

        void AddAnnnotationDataToVector(std::vector<AnnotationData> *annotations)
        {
            annotations->emplace_back(name_, std::move(elements_));
        }

        bool HasId() const
        {
            return id_.has_value();
        }

        bool IsInitialized() const
        {
            return isInitialized_;
        }

    private:
        std::string name_;
        std::optional<std::string> id_;
        std::vector<AnnotationElement> elements_;
        bool isInitialized_ {false};
    };

    std::optional<Metadata::Error> MeetExpRecordAttribute(std::string_view attribute, std::string_view value);
    std::optional<Metadata::Error> MeetExpIdAttribute(std::string_view attribute, std::string_view value);
    std::optional<Metadata::Error> MeetExpElementNameAttribute(std::string_view attribute, std::string_view value);
    std::optional<Metadata::Error> MeetExpElementTypeAttribute(std::string_view attribute, std::string_view value);
    std::optional<Metadata::Error> MeetExpElementArrayComponentTypeAttribute(std::string_view attribute,
                                                                             std::string_view value);
    std::optional<Metadata::Error> MeetExpElementValueAttribute(std::string_view attribute, std::string_view value);

    AnnotationElementBuilder *GetOrCreateAnnotationElementBuilder() const
    {
        if (!annotationElementBuilder_) {
            annotationElementBuilder_ = std::make_unique<AnnotationElementBuilder>();
        }
        return annotationElementBuilder_.get();
    }

    AnnotationBuilder *GetOrCreateAnnotationBuilder() const
    {
        if (!annotationBuilder_) {
            annotationBuilder_ = std::make_unique<AnnotationBuilder>();
        }
        return annotationBuilder_.get();
    }

    void InitializeAnnotationBuilder(std::string_view name)
    {
        if (IsParseAnnotation()) {
            ResetAnnotationBuilder();
        }

        GetOrCreateAnnotationBuilder()->Initialize(name);
    }

    void ResetAnnotationBuilder()
    {
        ASSERT(IsParseAnnotation());

        if (IsParseAnnotationElement() && GetOrCreateAnnotationElementBuilder()->IsCompleted()) {
            ResetAnnotationElementBuilder();
        }

        if (GetOrCreateAnnotationBuilder()->HasId()) {
            idMap_.insert(
                {GetOrCreateAnnotationBuilder()->GetId(), GetOrCreateAnnotationBuilder()->CreateAnnotationData()});
        } else {
            GetOrCreateAnnotationBuilder()->AddAnnnotationDataToVector(&annotations_);
        }

        GetOrCreateAnnotationBuilder()->Reset();
    }

    bool IsParseAnnotation() const
    {
        return GetOrCreateAnnotationBuilder()->IsInitialized();
    }

    void InitializeAnnotationElementBuilder(std::string_view name)
    {
        if (IsParseAnnotationElement() && GetOrCreateAnnotationElementBuilder()->IsCompleted()) {
            ResetAnnotationElementBuilder();
        }

        GetOrCreateAnnotationElementBuilder()->Initialize(name);
    }

    void ResetAnnotationElementBuilder()
    {
        ASSERT(IsParseAnnotationElement());
        ASSERT(GetOrCreateAnnotationElementBuilder()->IsCompleted());

        GetOrCreateAnnotationBuilder()->AddElement(GetOrCreateAnnotationElementBuilder()->CreateAnnotationElement());

        GetOrCreateAnnotationElementBuilder()->Reset();
    }

    bool IsParseAnnotationElement() const
    {
        return GetOrCreateAnnotationElementBuilder()->IsInitialized();
    }

    mutable std::unique_ptr<AnnotationBuilder> annotationBuilder_;
    mutable std::unique_ptr<AnnotationElementBuilder> annotationElementBuilder_;
    std::vector<AnnotationData> annotations_;
    std::unordered_map<std::string, std::unique_ptr<AnnotationData>> idMap_;
};

class ItemMetadata : public AnnotationMetadata {
public:
    uint32_t GetAccessFlags() const
    {
        return accessFlags_;
    }

    void SetAccessFlags(uint32_t accessFlags)
    {
        accessFlags_ = accessFlags;
    }

    PANDA_PUBLIC_API bool IsForeign() const;

private:
    uint32_t accessFlags_ {0};
};

class RecordMetadata : public ItemMetadata {
public:
    virtual std::string GetBase() const;

    virtual bool SetBase(std::string_view base);

    virtual std::vector<std::string> GetInterfaces() const;

    virtual bool AddInterface(std::string_view value);

    virtual bool RemoveInterface(std::string_view value);

    virtual bool IsAnnotation() const;

    virtual bool IsRuntimeAnnotation() const;

    virtual bool IsTypeAnnotation() const;

    virtual bool IsRuntimeTypeAnnotation() const;

protected:
    std::optional<Error> Validate(std::string_view attribute) const override;

    std::optional<Error> Validate(std::string_view attribute, std::string_view value) const override;

    void SetFlags(std::string_view attribute) override;

    void SetFlags(std::string_view attribute, std::string_view value) override;

    void RemoveFlags(std::string_view attribute) override;

    void RemoveFlags(std::string_view attribute, std::string_view value) override;
};

class FieldMetadata : public ItemMetadata {
public:
    void SetFieldType(const Type &type)
    {
        fieldType_ = type;
    }

    Type GetFieldType() const
    {
        return fieldType_;
    }

    void SetValue(const ScalarValue &value)
    {
        value_ = value;
    }

    std::optional<ScalarValue> GetValue() const
    {
        return value_;
    }

    void ResetValue()
    {
        value_.reset();
    }

protected:
    std::optional<Error> StoreValue(std::string_view attribute, std::string_view value) override;

    std::optional<Error> Validate(std::string_view attribute) const override;

    std::optional<Error> Validate(std::string_view attribute, std::string_view value) const override;

    void SetFlags(std::string_view attribute) override;

    void SetFlags(std::string_view attribute, std::string_view value) override;

    void RemoveFlags(std::string_view attribute) override;

    void RemoveFlags(std::string_view attribute, std::string_view value) override;

    virtual bool IsValueAttribute(std::string_view attribute)
    {
        return attribute == "value";
    }

private:
    Type fieldType_;
    std::optional<ScalarValue> value_;
};

class FunctionMetadata : public ItemMetadata {
public:
    virtual bool HasImplementation() const;

    virtual bool IsCtor() const;

    virtual bool IsCctor() const;

protected:
    std::optional<Error> Validate(std::string_view attribute) const override;

    std::optional<Error> Validate(std::string_view attribute, std::string_view value) const override;

    void SetFlags(std::string_view attribute) override;

    void SetFlags(std::string_view attribute, std::string_view value) override;

    void RemoveFlags(std::string_view attribute) override;

    void RemoveFlags(std::string_view attribute, std::string_view value) override;
};

class ParamMetadata : public AnnotationMetadata {
protected:
    std::optional<Error> Validate(std::string_view attribute) const override;

    std::optional<Error> Validate(std::string_view attribute, std::string_view value) const override;

    void SetFlags(std::string_view attribute) override;

    void SetFlags(std::string_view attribute, std::string_view value) override;

    void RemoveFlags(std::string_view attribute) override;

    void RemoveFlags(std::string_view attribute, std::string_view value) override;
};

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_META_H
