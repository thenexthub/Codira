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

#include "abc_field_processor.h"
#include "abc2program_log.h"
#include "abc_annotation_processor.h"

namespace ark::abc2program {

AbcFieldProcessor::AbcFieldProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData,
                                     pandasm::Record &record, bool isExternal)
    : AbcFileEntityProcessor(entityId, keyData), record_(record), isExternal_(isExternal)
{
    fieldDataAccessor_ = std::make_unique<panda_file::FieldDataAccessor>(*file_, entityId_);
    typeConverter_ = std::make_unique<AbcTypeConverter>(*stringTable_);
    FillProgramData();
}

void AbcFieldProcessor::FillProgramData()
{
    pandasm::Field field(keyData_.GetFileLanguage());
    FillFieldData(field);
    if (isExternal_) {
        auto &fieldList =
            keyData_.GetExternalFieldTable()[keyData_.GetFullRecordNameById(fieldDataAccessor_->GetClassId())];
        auto retField = std::find_if(fieldList.begin(), fieldList.end(), [&field](pandasm::Field &fieldFromList) {
            return field.name == fieldFromList.name && field.IsStatic() == fieldFromList.IsStatic();
            ;
        });
        if (retField == fieldList.end()) {
            fieldList.push_back(std::move(field));
        }
        return;
    }
    record_.fieldList.emplace_back(std::move(field));
}

void AbcFieldProcessor::FillFieldData(pandasm::Field &field)
{
    FillFieldName(field);
    FillFieldType(field);
    FillFieldMetaData(field);
    fieldDataAccessor_->EnumerateAnnotations([&](panda_file::File::EntityId annotationId) {
        AbcAnnotationProcessor annotationProcessor(annotationId, keyData_, field);
        annotationProcessor.FillProgramData();
    });
}

void AbcFieldProcessor::FillFieldName(pandasm::Field &field)
{
    panda_file::File::EntityId fieldNameId = fieldDataAccessor_->GetNameId();
    field.name = stringTable_->GetStringById(fieldNameId);
}

void AbcFieldProcessor::FillFieldType(pandasm::Field &field)
{
    uint32_t fieldType = fieldDataAccessor_->GetType();
    field.type = typeConverter_->FieldTypeToPandasmType(fieldType);
}

void AbcFieldProcessor::FillFieldMetaData(pandasm::Field &field)
{
    LOG(DEBUG, ABC2PROGRAM) << "[getting metadata]\nfield id: " << entityId_ << " (0x" << std::hex << entityId_ << ")";

    panda_file::FieldDataAccessor fieldAccessor(*file_, entityId_);

    SetEntityAttribute(
        field, [&fieldAccessor]() { return fieldAccessor.IsExternal(); }, "external");

    SetEntityAttribute(
        field, [&fieldAccessor]() { return fieldAccessor.IsStatic(); }, "static");

    SetEntityAttributeValue(
        field, [&fieldAccessor]() { return fieldAccessor.IsPublic(); }, "access.field", "public");

    SetEntityAttributeValue(
        field, [&fieldAccessor]() { return fieldAccessor.IsProtected(); }, "access.field", "protected");

    SetEntityAttributeValue(
        field, [&fieldAccessor]() { return fieldAccessor.IsPrivate(); }, "access.field", "private");

    SetEntityAttribute(
        field, [&fieldAccessor]() { return fieldAccessor.IsFinal(); }, "final");

    // NOTE: Default values are not suppported #25313
    if (field.type.GetId() == panda_file::Type::TypeId::REFERENCE && field.type.GetName() == "std.core.String") {
        std::optional<uint32_t> stringOffsetVal = fieldAccessor.GetValue<uint32_t>();
        if (stringOffsetVal.has_value()) {
            std::string_view val {reinterpret_cast<const char *>(
                file_->GetStringData(panda_file::File::EntityId(stringOffsetVal.value())).data)};
            field.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::STRING>(val));
        }
    } else if (field.type.GetId() == panda_file::Type::TypeId::I32) {
        std::optional<uint32_t> intVal = fieldAccessor.GetValue<uint32_t>();
        if (intVal.has_value()) {
            field.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::I32>(intVal.value()));
        }
    } else if (field.type.GetId() == panda_file::Type::TypeId::U32) {
        std::optional<uint32_t> uintVal = fieldAccessor.GetValue<uint32_t>();
        if (uintVal.has_value()) {
            field.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::U32>(uintVal.value()));
        }
    } else if (field.type.GetId() == panda_file::Type::TypeId::I64) {
        std::optional<uint64_t> longVal = fieldAccessor.GetValue<uint64_t>();
        if (longVal.has_value()) {
            field.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::I64>(longVal.value()));
        }
    } else if (field.type.GetId() == panda_file::Type::TypeId::U64) {
        std::optional<uint64_t> ulongVal = fieldAccessor.GetValue<uint64_t>();
        if (ulongVal.has_value()) {
            field.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::U64>(ulongVal.value()));
        }
    } else if (field.type.GetId() == panda_file::Type::TypeId::U1) {
        std::optional<uint32_t> boolVal = fieldAccessor.GetValue<uint32_t>();
        if (boolVal.has_value()) {
            field.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::U1>(boolVal.value() != 0));
        }
    } else if (field.type.GetId() == panda_file::Type::TypeId::F32) {
        std::optional<uint32_t> floatVal = fieldAccessor.GetValue<uint32_t>();
        if (floatVal.has_value()) {
            float val = *reinterpret_cast<float *>(&floatVal.value());
            field.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::F32>(val));
        }
    } else if (field.type.GetId() == panda_file::Type::TypeId::F64) {
        std::optional<uint64_t> doubleVal = fieldAccessor.GetValue<uint64_t>();
        if (doubleVal.has_value()) {
            double val = *reinterpret_cast<double *>(&doubleVal.value());
            field.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::F64>(val));
        }
    }
}

}  // namespace ark::abc2program
