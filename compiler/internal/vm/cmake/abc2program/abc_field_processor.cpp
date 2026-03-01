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
#include "abc_literal_array_processor.h"
#include "file-inl.h"
#include "literal_data_accessor.h"

namespace panda::abc2program {

AbcFieldProcessor::AbcFieldProcessor(panda_file::File::EntityId entity_id, Abc2ProgramEntityContainer &entity_container,
                                     pandasm::Record &record)
    : AbcFileEntityProcessor(entity_id, entity_container), record_(record),
      type_converter_(entity_container),
      field_(pandasm::Field(record.language))
{
    field_data_accessor_ = std::make_unique<panda_file::FieldDataAccessor>(*file_, entity_id_);
}

void AbcFieldProcessor::FillProgramData()
{
    FillFieldData();
    record_.field_list.emplace_back(std::move(field_));
}

void AbcFieldProcessor::FillFieldData()
{
    FillFieldName();
    FillFieldType();
    FillFieldMetaData();
}

void AbcFieldProcessor::FillFieldName()
{
    panda_file::File::EntityId field_name_id = field_data_accessor_->GetNameId();
    field_.name = GetStringById(field_name_id);
    type_converter_.EntityContainer().ModifyPkgNameForFieldName(field_.name);
}

void AbcFieldProcessor::FillFieldType()
{
    uint32_t field_type = field_data_accessor_->GetType();
    field_.type = type_converter_.FieldTypeToPandasmType(field_type);
}

void AbcFieldProcessor::FillFieldMetaData()
{
    field_.metadata->SetFieldType(field_.type);
    FillFieldAttributes();
    FillMetaDataValue();
}

void AbcFieldProcessor::FillFieldAttributes()
{
    if (field_data_accessor_->IsExternal()) {
        field_.metadata->SetAttribute(ABC_ATTR_EXTERNAL);
    }
    if (field_data_accessor_->IsStatic()) {
        field_.metadata->SetAttribute(ABC_ATTR_STATIC);
    }
}

void AbcFieldProcessor::FillU32MetaData()
{
    uint32_t val = field_data_accessor_->GetValue<uint32_t>().value();
    if (record_.name == ES_MODULE_RECORD || field_.name == MODULE_RECORD_IDX) {
        entity_container_.AddModuleLiteralArrayId(val);
        auto module_literal_array_id_name = entity_container_.GetLiteralArrayIdName(val);
        field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::LITERALARRAY>(
            module_literal_array_id_name));
    } else if (record_.name == ES_SCOPE_NAMES_RECORD || field_.name == SCOPE_NAMES) {
        entity_container_.AddUnnestedLiteralArrayId(val);
        auto literal_array_id_name = entity_container_.GetLiteralArrayIdName(val);
        field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::LITERALARRAY>(
            literal_array_id_name));
    } else if (field_.name == MODULE_REQUEST_PAHSE_IDX) {
        entity_container_.AddModuleRequestPhaseId(val);
        auto module_literal_array_id_name = entity_container_.GetLiteralArrayIdName(val);
        field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::LITERALARRAY>(
            module_literal_array_id_name));
    } else if (field_.name == JSON_FILE_CONTENT) {
        auto json_string_id = panda_file::File::EntityId(val);
        field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::STRING>(
            entity_container_.GetStringById(json_string_id)));
    } else {
        field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::U32>(val));
    }
}

void AbcFieldProcessor::FillStringMetaData()
{
    std::optional<uint32_t> string_offset_val = field_data_accessor_->GetValue<uint32_t>();
    if (string_offset_val.has_value()) {
        std::string_view val {reinterpret_cast<const char *>(
            file_->GetStringData(panda_file::File::EntityId(string_offset_val.value())).data)};
        field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::STRING>(val));
    }
}

void AbcFieldProcessor::FillLiteralArrayMetaData()
{
    std::optional<uint32_t> litarray_offset_val = field_data_accessor_->GetValue<uint32_t>();
    if (litarray_offset_val.has_value()) {
        panda_file::LiteralDataAccessor literal_data_accessor(*file_, file_->GetLiteralArraysId());
        AbcLiteralArrayProcessor abcLitarrayProcessor{panda_file::File::EntityId{litarray_offset_val.value()},
                                                      entity_container_, literal_data_accessor};
        abcLitarrayProcessor.FillProgramData();
        std::string name = entity_container_.GetLiteralArrayIdName(litarray_offset_val.value());
        field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::LITERALARRAY>(
            std::string_view {name}));
    }
}

void AbcFieldProcessor::FillMetaDataValue()
{
    switch (field_.type.GetId()) {
        case panda_file::Type::TypeId::U32: {
            FillU32MetaData();
            break;
        }
        case panda_file::Type::TypeId::U8: {
            const uint8_t val = field_data_accessor_->GetValue<uint8_t>().value();
            field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::U8>(val));
            break;
        }
        case panda_file::Type::TypeId::F64: {
            std::optional<double> val = field_data_accessor_->GetValue<double>();
            if (val.has_value()) {
                field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::F64>(val.value()));
            }
            break;
        }
        case panda_file::Type::TypeId::U1: {
            std::optional<bool> val = field_data_accessor_->GetValue<bool>();
            if (val.has_value()) {
                field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::U1>(val.value()));
            }
            break;
        }
        case panda_file::Type::TypeId::REFERENCE: {
            if (field_.type.GetName() == "panda.String") {
                FillStringMetaData();
            }
            if (field_.type.IsArray()) {
                FillLiteralArrayMetaData();
            }
            break;
        }
        default:
            UNREACHABLE();
    }
}

void AbcFieldProcessor::FillFieldAnnotations()
{
    log::Unimplemented(__PRETTY_FUNCTION__);
}

}  // namespace panda::abc2program
