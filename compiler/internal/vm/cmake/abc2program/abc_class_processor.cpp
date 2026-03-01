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

#include "abc_class_processor.h"
#include "abc_field_processor.h"
#include "abc_literal_array_processor.h"
#include "abc_method_processor.h"

namespace panda::abc2program {

AbcClassProcessor::AbcClassProcessor(panda_file::File::EntityId entity_id, Abc2ProgramEntityContainer &entity_container)
    : AbcFileEntityProcessor(entity_id, entity_container), record_(pandasm::Record("", DEFUALT_SOURCE_LANG))
{
    if (!file_->IsExternal(entity_id_)) {
        class_data_accessor_ = std::make_unique<panda_file::ClassDataAccessor>(*file_, entity_id_);
    }
}

void AbcClassProcessor::FillProgramData()
{
    FillRecord();
    FillFunctions();
    FillLiteralArrayTable();
}

void AbcClassProcessor::FillRecord()
{
    FillRecordName();
    /*
    * All array types like f64[] are records because of assembler implementation.
    * AbcClassProcessor must only collect record name for future purposes
    * and not to collect any other data about it
    */
    if (AbcFileUtils::IsArrayTypeName(record_.name)) {
        return;
    }
    if (!file_->IsExternal(entity_id_)) {
        record_.metadata->SetAccessFlags(class_data_accessor_->GetAccessFlags());
        record_.language = class_data_accessor_->GetSourceLang().value_or(DEFUALT_SOURCE_LANG);
    } else {
        record_.language = DEFUALT_SOURCE_LANG;
    }
    ASSERT(program_->record_table.count(record_.name) == 0);
    FillRecordData();
    program_->record_table.emplace(record_.name, std::move(record_));
}

void AbcClassProcessor::FillRecordName()
{
    record_.name = entity_container_.GetFullRecordNameById(entity_id_);
}

void AbcClassProcessor::FillRecordData()
{
    FillRecordMetaData();
    FillFields();
    FillRecordSourceFile();
}

void AbcClassProcessor::FillRecordMetaData()
{
    FillRecordAttributes();
}

void AbcClassProcessor::FillRecordAttributes()
{
    if (file_->IsExternal(entity_id_)) {
        record_.metadata->SetAttribute(ABC_ATTR_EXTERNAL);
    }
}

void AbcClassProcessor::FillRecordAnnotations()
{
    log::Unimplemented(__PRETTY_FUNCTION__);
}

void AbcClassProcessor::FillRecordSourceFile()
{
    if (file_->IsExternal(entity_id_)) {
        return;
    }
    std::optional<panda_file::File::EntityId> source_file_id = class_data_accessor_->GetSourceFileId();
    if (source_file_id.has_value()) {
        record_.source_file = GetStringById(source_file_id.value());
    } else {
        record_.source_file = "";
    }
}

void AbcClassProcessor::FillFields()
{
    if (file_->IsExternal(entity_id_)) {
        return;
    }
    class_data_accessor_->EnumerateFields([&](panda_file::FieldDataAccessor &fda) -> void {
        panda_file::File::EntityId field_id = fda.GetFieldId();
        AbcFieldProcessor field_processor(field_id, entity_container_, record_);
        field_processor.FillProgramData();
    });
}

void AbcClassProcessor::FillFunctions()
{
    if (file_->IsExternal(entity_id_)) {
        return;
    }
    class_data_accessor_->EnumerateMethods([&](panda_file::MethodDataAccessor &mda) -> void {
        panda_file::File::EntityId method_id = mda.GetMethodId();
        AbcMethodProcessor method_processor(method_id, entity_container_);
        method_processor.FillProgramData();
    });
}

void AbcClassProcessor::FillLiteralArrayTable()
{
    literal_data_accessor_ = std::make_unique<panda_file::LiteralDataAccessor>(*file_, file_->GetLiteralArraysId());
    FillModuleLiteralArrays();
    FillUnnestedLiteralArrays();
    FillNestedLiteralArrays();
}

void AbcClassProcessor::FillModuleLiteralArrays()
{
    for (uint32_t module_literal_array_id : entity_container_.GetMouleLiteralArrayIdSet()) {
        FillModuleLiteralArrayById(module_literal_array_id);
    }

    for (uint32_t module_literal_array_id : entity_container_.GetModuleRequestPhaseIdSet()) {
        FillModuleRequestPhaseById(module_literal_array_id);
    }
}

void AbcClassProcessor::FillUnnestedLiteralArrays()
{
    for (uint32_t unnested_literal_array_id : entity_container_.GetUnnestedLiteralArrayIdSet()) {
        FillLiteralArrayById(unnested_literal_array_id);
    }
}

void AbcClassProcessor::FillNestedLiteralArrays()
{
    auto &unprocessed_literal_array_id_set = entity_container_.GetUnprocessedNestedLiteralArrayIdSet();
    while (!unprocessed_literal_array_id_set.empty()) {
        auto id = unprocessed_literal_array_id_set.begin();
        entity_container_.AddProcessedNestedLiteralArrayId(*id);
        FillLiteralArrayById(*id);
        unprocessed_literal_array_id_set.erase(id);
    }
}

void AbcClassProcessor::FillModuleLiteralArrayById(uint32_t module_literal_array_id)
{
    panda_file::File::EntityId entity_id(module_literal_array_id);
    AbcModuleArrayProcessor module_array_processor(entity_id, entity_container_);
    module_array_processor.FillProgramData();
}

void AbcClassProcessor::FillLiteralArrayById(uint32_t literal_array_id)
{
    panda_file::File::EntityId entity_id(literal_array_id);
    AbcLiteralArrayProcessor literal_array_processor(entity_id, entity_container_, *literal_data_accessor_);
    literal_array_processor.FillProgramData();
}

void AbcClassProcessor::FillModuleRequestPhaseById(uint32_t module_literal_array_id)
{
    panda_file::File::EntityId entity_id(module_literal_array_id);
    AbcLiteralArrayProcessor literal_array_processor(entity_id, entity_container_, *literal_data_accessor_);
    literal_array_processor.FillModuleRequestPhase();
}

}  // namespace panda::abc2program
