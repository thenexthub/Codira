/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "abc_annotation_processor.h"
#include "abc_file_utils.h"
#include "abc_literal_array_processor.h"
#include "abc_method_processor.h"
#include "libarkfile/file-inl.h"
#include "libarkfile/class_data_accessor.h"
#include "libarkfile/method_data_accessor.h"

namespace ark::abc2program {

AbcAnnotationProcessor::AbcAnnotationProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData,
                                               pandasm::Record &record)
    : AbcFileEntityProcessor(entityId, keyData), target_(&record)
{
    annotationDataAccessor_ = std::make_unique<panda_file::AnnotationDataAccessor>(*file_, entityId);
    std::string stringId = keyData.GetAbcStringTable().GetStringById(annotationDataAccessor_->GetClassId());
    annotationName_ = pandasm::Type::FromDescriptor(stringId).GetName();
    std::replace(annotationName_.begin(), annotationName_.end(), '/', '.');
}

AbcAnnotationProcessor::AbcAnnotationProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData,
                                               pandasm::Function &function)
    : AbcFileEntityProcessor(entityId, keyData), target_(&function)
{
    annotationDataAccessor_ = std::make_unique<panda_file::AnnotationDataAccessor>(*file_, entityId);
    std::string stringId = keyData.GetAbcStringTable().GetStringById(annotationDataAccessor_->GetClassId());
    annotationName_ = pandasm::Type::FromDescriptor(stringId).GetName();
    std::replace(annotationName_.begin(), annotationName_.end(), '/', '.');
}

AbcAnnotationProcessor::AbcAnnotationProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData,
                                               pandasm::Field &field)
    : AbcFileEntityProcessor(entityId, keyData), target_(&field)
{
    annotationDataAccessor_ = std::make_unique<panda_file::AnnotationDataAccessor>(*file_, entityId);
    std::string stringId = keyData.GetAbcStringTable().GetStringById(annotationDataAccessor_->GetClassId());
    annotationName_ = pandasm::Type::FromDescriptor(stringId).GetName();
    std::replace(annotationName_.begin(), annotationName_.end(), '/', '.');
}

void AbcAnnotationProcessor::FillProgramData()
{
    if (annotationName_.empty()) {
        return;
    }
    FillAnnotation();
}

void AbcAnnotationProcessor::FillAnnotation()
{
    std::vector<pandasm::AnnotationElement> elements;
    FillAnnotationElements(elements);
    pandasm::AnnotationData annotationData(annotationName_, elements);
    std::vector<pandasm::AnnotationData> annotations;
    annotations.emplace_back(std::move(annotationData));
    if (std::holds_alternative<pandasm::Record *>(target_)) {
        auto *record = std::get<pandasm::Record *>(target_);
        record->metadata->AddAnnotations(annotations);
    } else if (std::holds_alternative<pandasm::Function *>(target_)) {
        auto *function = std::get<pandasm::Function *>(target_);
        function->metadata->AddAnnotations(annotations);
    } else if (std::holds_alternative<pandasm::Field *>(target_)) {
        auto *field = std::get<pandasm::Field *>(target_);
        field->metadata->AddAnnotations(annotations);
    }
}

void AbcAnnotationProcessor::FillLiteralArrayAnnotation(std::vector<pandasm::AnnotationElement> &elements,
                                                        const std::string &annotationElemName, uint32_t value)
{
    AbcLiteralArrayProcessor litArrayProc(panda_file::File::EntityId {value}, keyData_);
    litArrayProc.FillProgramData();
    std::string name = keyData_.GetLiteralArrayIdName(value);
    pandasm::AnnotationElement annotationElement(
        annotationElemName,
        std::make_unique<pandasm::ScalarValue>(pandasm::ScalarValue::Create<pandasm::Value::Type::LITERALARRAY>(name)));
    elements.emplace_back(annotationElement);
}

template <pandasm::Value::Type P_TYPE, typename VType>
static pandasm::AnnotationElement CreateAnnoElem(std::string &annotationElemName, VType value)
{
    return pandasm::AnnotationElement(
        annotationElemName, std::make_unique<pandasm::ScalarValue>(pandasm::ScalarValue::Create<P_TYPE>(value)));
}

std::optional<pandasm::AnnotationElement> AbcAnnotationProcessor::CreateAnnotationElement(
    const panda_file::AnnotationDataAccessor::Elem &annotationDataAccessorElem, std::string &annotationElemName,
    pandasm::Value::Type valueType)
{
    switch (valueType) {
        case pandasm::Value::Type::U1: {
            auto value = static_cast<uint8_t>(annotationDataAccessorElem.GetScalarValue().Get<bool>());
            return CreateAnnoElem<pandasm::Value::Type::U1, uint8_t>(annotationElemName, value);
        }
        case pandasm::Value::Type::U8: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<uint8_t>();
            return CreateAnnoElem<pandasm::Value::Type::U8, uint8_t>(annotationElemName, value);
        }
        case pandasm::Value::Type::U16: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<uint16_t>();
            return CreateAnnoElem<pandasm::Value::Type::U16, uint16_t>(annotationElemName, value);
        }
        case pandasm::Value::Type::U32: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<uint32_t>();
            return CreateAnnoElem<pandasm::Value::Type::U32, uint32_t>(annotationElemName, value);
        }
        case pandasm::Value::Type::U64: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<uint64_t>();
            return CreateAnnoElem<pandasm::Value::Type::U64, uint64_t>(annotationElemName, value);
        }
        case pandasm::Value::Type::I8: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<int8_t>();
            return CreateAnnoElem<pandasm::Value::Type::I8, int8_t>(annotationElemName, value);
        }
        case pandasm::Value::Type::I16: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<int16_t>();
            return CreateAnnoElem<pandasm::Value::Type::I16, int16_t>(annotationElemName, value);
        }
        case pandasm::Value::Type::I32: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<int32_t>();
            return CreateAnnoElem<pandasm::Value::Type::I32, int32_t>(annotationElemName, value);
        }
        case pandasm::Value::Type::I64: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<int64_t>();
            return CreateAnnoElem<pandasm::Value::Type::I64, int64_t>(annotationElemName, value);
        }
        case pandasm::Value::Type::F32: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<float>();
            return CreateAnnoElem<pandasm::Value::Type::F32, float>(annotationElemName, value);
        }
        case pandasm::Value::Type::F64: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<double>();
            return CreateAnnoElem<pandasm::Value::Type::F64, double>(annotationElemName, value);
        }
        case pandasm::Value::Type::STRING: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<uint32_t>();
            std::string_view stringValue {
                reinterpret_cast<const char *>(file_->GetStringData(panda_file::File::EntityId(value)).data)};
            return CreateAnnoElem<pandasm::Value::Type::STRING, std::string_view>(annotationElemName, stringValue);
        }
        case pandasm::Value::Type::LITERALARRAY: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<uint32_t>();
            AbcLiteralArrayProcessor litArrayProc(panda_file::File::EntityId {value}, keyData_);
            litArrayProc.FillProgramData();
            std::string name = keyData_.GetLiteralArrayIdName(value);
            return CreateAnnoElem<pandasm::Value::Type::LITERALARRAY, std::string>(annotationElemName, name);
        }
        case pandasm::Value::Type::RECORD: {
            // NOTE: Annotations are not fully supported #20663
            auto value = annotationDataAccessorElem.GetScalarValue().Get<uint32_t>();
            panda_file::File::EntityId entityId(value);

            // Check the entity type
            EntityType entityType = AbcFileUtils::GetEntityType(*file_, entityId);
            // Try to get class information if it's a class
            if (entityType == EntityType::CLASS) {
                panda_file::ClassDataAccessor cda(*file_, entityId);
                auto nameData = file_->GetStringData(cda.GetClassId());
                std::string className = keyData_.GetAbcStringTable().StringDataToString(nameData);
                std::replace(className.begin(), className.end(), '/', '.');

                auto recordType = pandasm::Type::FromDescriptor(className);
                return CreateAnnoElem<pandasm::Value::Type::RECORD, pandasm::Type>(annotationElemName, recordType);
            }

            return std::nullopt;  // NOTE: Annotations are not fully supported #20663
        }
        case pandasm::Value::Type::METHOD: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<uint32_t>();
            panda_file::File::EntityId entityId(value);

            // Check the entity type
            EntityType entityType = AbcFileUtils::GetEntityType(*file_, entityId);
            // Try to get method information if it's a method
            if (entityType == EntityType::METHOD) {
                AbcMethodProcessor methodProcessor(entityId, keyData_);
                std::string methodSignature = methodProcessor.GetMethodSignature();
                LOG(DEBUG, ABC2PROGRAM) << "[CreateAnnotationElement] METHOD signature: " << methodSignature;
                return CreateAnnoElem<pandasm::Value::Type::METHOD, std::string>(annotationElemName, methodSignature);
            }

            LOG(DEBUG, ABC2PROGRAM) << "[CreateAnnotationElement] WARNING: EntityType is not METHOD, returning nullopt";
            return std::nullopt;  // NOTE: Annotations are not fully supported #20663
        }
        case pandasm::Value::Type::ARRAY: {
            pandasm::Value::Type cType = pandasm::Value::Type::VOID;
            auto arrayValue = annotationDataAccessorElem.GetArrayValue();
            if (arrayValue.GetCount() != 0) {
                // NOTE: Annotations are not fully supported #20663
                UNREACHABLE();
            }
            std::vector<pandasm::ScalarValue> values;
            return pandasm::AnnotationElement(annotationElemName, std::make_unique<pandasm::ArrayValue>(cType, values));
        }
        default:
            UNREACHABLE();
    }
    return std::nullopt;
}

// Process array annotation elements (for module annotations)
void AbcAnnotationProcessor::ProcessArrayAnnotationElement(std::vector<pandasm::AnnotationElement> &elements,
                                                           const panda_file::AnnotationDataAccessor::Elem &elem,
                                                           const std::string &name, pandasm::Value::Type arrayType)
{
    LOG(DEBUG, ABC2PROGRAM) << "[ProcessArrayAnnotationElement] array type: " << static_cast<int>(arrayType);

    auto arrayValue = elem.GetArrayValue();
    LOG(DEBUG, ABC2PROGRAM) << "[ProcessArrayAnnotationElement] array value count: " << arrayValue.GetCount();

    std::vector<pandasm::ScalarValue> values;
    for (uint32_t j = 0; j < arrayValue.GetCount(); ++j) {
        auto arrayValueElem = arrayValue.Get<uint32_t>(j);
        panda_file::File::EntityId entityId(arrayValueElem);
        EntityType entityType = AbcFileUtils::GetEntityType(*file_, entityId);
        if (entityType == EntityType::CLASS) {
            panda_file::ClassDataAccessor cda(*file_, entityId);
            auto nameData = file_->GetStringData(cda.GetClassId());
            std::string className = keyData_.GetAbcStringTable().StringDataToString(nameData);
            std::replace(className.begin(), className.end(), '/', '.');

            auto recordType = pandasm::Type::FromDescriptor(className);
            auto tmp = pandasm::ScalarValue::Create<pandasm::Value::Type::RECORD>(recordType);
            values.emplace_back(tmp);
        } else if (entityType == EntityType::METHOD) {
            AbcMethodProcessor methodProcessor(entityId, keyData_);
            std::string methodSignature = methodProcessor.GetMethodSignature();
            LOG(DEBUG, ABC2PROGRAM) << "[ProcessArrayAnnotationElement] Array METHOD signature: " << methodSignature;
            auto tmp = pandasm::ScalarValue::Create<pandasm::Value::Type::METHOD>(methodSignature);
            values.emplace_back(tmp);
        }
    }

    auto arrayElement =
        pandasm::AnnotationElement(name, std::make_unique<pandasm::ArrayValue>(pandasm::Value::Type::RECORD, values));
    elements.emplace_back(arrayElement);
    LOG(DEBUG, ABC2PROGRAM) << "[ProcessArrayAnnotationElement] Added array element '" << name << "' with "
                            << values.size() << " values";
}

void AbcAnnotationProcessor::FillAnnotationElements(std::vector<pandasm::AnnotationElement> &elements)
{
    for (uint32_t i = 0; i < annotationDataAccessor_->GetCount(); ++i) {
        auto elem = annotationDataAccessor_->GetElement(i);
        auto name = keyData_.GetAbcStringTable().GetStringById(elem.GetNameId());
        auto tagItem = annotationDataAccessor_->GetTag(i).GetItem();
        auto valueType = pandasm::Value::GetCharAsType(tagItem);
        // Handle array annotations (module annotations)
        if (valueType == pandasm::Value::Type::UNKNOWN) {
            valueType = pandasm::Value::GetCharAsArrayType(tagItem);
            ProcessArrayAnnotationElement(elements, elem, name, valueType);
        } else {
            // Handle standard elements
            LOG(DEBUG, ABC2PROGRAM) << "[FillAnnotationElements] Processing standard element '" << name
                                    << "' with valueType=" << static_cast<int>(valueType);
            auto annoElem = CreateAnnotationElement(elem, const_cast<std::string &>(name), valueType);
            if (annoElem) {
                elements.emplace_back(annoElem.value());
                LOG(DEBUG, ABC2PROGRAM) << "[FillAnnotationElements] Successfully added standard element '" << name
                                        << "'";
            }
        }
    }
}

}  // namespace ark::abc2program
