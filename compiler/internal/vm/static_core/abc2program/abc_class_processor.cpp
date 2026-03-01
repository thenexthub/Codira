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
#include <iostream>
#include "abc_method_processor.h"
#include "abc_field_processor.h"
#include "mangling.h"
#include "abc_annotation_processor.h"

namespace ark::abc2program {

AbcClassProcessor::AbcClassProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData)
    : AbcFileEntityProcessor(entityId, keyData)
{
    FillProgramData();
}

void AbcClassProcessor::FillProgramData()
{
    FillRecord();
}

void AbcClassProcessor::FillRecord()
{
    pandasm::Record record("", keyData_.GetFileLanguage());
    record.name = keyData_.GetFullRecordNameById(entityId_);
    FillRecordMetaData(record);
    if (!file_->IsExternal(entityId_)) {
        FillRecordData(record);
    }
    std::string name = record.name;
    keyData_.GetRecordNameToIdTable().emplace(name, entityId_);
    program_->recordTable.emplace(name, std::move(record));
}

void AbcClassProcessor::FillRecordAnnotations(pandasm::Record &record)
{
    classDataAccessor_->EnumerateAnnotations([&](panda_file::File::EntityId annotationId) {
        AbcAnnotationProcessor annotationProcessor(annotationId, keyData_, record);
        annotationProcessor.FillProgramData();
    });
}

void AbcClassProcessor::FillRecordData(pandasm::Record &record)
{
    classDataAccessor_ = std::make_unique<panda_file::ClassDataAccessor>(*file_, entityId_);
    FillFields(record);
    FillFunctions();
    FillRecordSourceFile(record);
    FillRecordAnnotations(record);
}

void AbcClassProcessor::FillRecordMetaData(pandasm::Record &record)
{
    LOG(DEBUG, ABC2PROGRAM) << "[getting metadata]\nrecord id: " << entityId_ << " (0x" << std::hex << entityId_ << ")";

    auto external = file_->IsExternal(entityId_);

    SetEntityAttribute(
        record, [external]() { return external; }, "external");

    if (!external) {
        auto cda = panda_file::ClassDataAccessor {*file_, entityId_};
        SetEntityAttributeValue(
            record, [&cda]() { return cda.IsPublic(); }, "access.record", "public");

        SetEntityAttributeValue(
            record, [&cda]() { return cda.IsProtected(); }, "access.record", "protected");

        SetEntityAttributeValue(
            record, [&cda]() { return cda.IsPrivate(); }, "access.record", "private");

        SetEntityAttribute(
            record, [&cda]() { return cda.IsFinal(); }, "final");
    }
}

void AbcClassProcessor::FillFields(pandasm::Record &record)
{
    classDataAccessor_->EnumerateFields([&](panda_file::FieldDataAccessor &fda) -> void {
        panda_file::File::EntityId fieldId = fda.GetFieldId();
        AbcFieldProcessor fieldProcessor(fieldId, keyData_, record);
    });
}

void AbcClassProcessor::FillRecordSourceFile(pandasm::Record &record)
{
    std::optional<panda_file::File::EntityId> sourceFileId = classDataAccessor_->GetSourceFileId();
    if (sourceFileId.has_value()) {
        record.sourceFile = stringTable_->GetStringById(sourceFileId.value());
    } else {
        record.sourceFile = "";
    }
}

void AbcClassProcessor::FillFunctions()
{
    classDataAccessor_->EnumerateMethods([&](panda_file::MethodDataAccessor &mda) -> void {
        panda_file::File::EntityId methodId = mda.GetMethodId();
        AbcMethodProcessor methodProcessor(methodId, keyData_);
    });
}

}  // namespace ark::abc2program
