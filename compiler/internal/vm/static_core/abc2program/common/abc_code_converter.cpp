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

#include "abc_code_converter.h"
#include "abc_method_processor.h"
#include "libarkbase/os/file.h"
#include "libarkfile/type_helper.h"

namespace ark::abc2program {

AbcCodeConverter::AbcCodeConverter(Abc2ProgramKeyData &keyData) : keyData_(keyData)
{
    file_ = &(keyData_.GetAbcFile());
    stringTable_ = &(keyData_.GetAbcStringTable());
}

std::string AbcCodeConverter::IDToString(BytecodeInstruction bcIns, panda_file::File::EntityId methodId) const
{
    std::stringstream name;

    if (bcIns.HasFlag(BytecodeInstruction::Flags::TYPE_ID)) {
        auto type = pandasm::Type::FromDescriptor(
            stringTable_->GetStringById(file_->ResolveClassIndex(methodId, bcIns.GetId().AsIndex())));
        name.str("");
        name << type.GetPandasmName();
    } else if (bcIns.HasFlag(BytecodeInstruction::Flags::METHOD_ID) ||
               bcIns.HasFlag(BytecodeInstruction::Flags::STATIC_METHOD_ID)) {
        auto id = file_->ResolveMethodIndex(methodId, bcIns.GetId().AsIndex());
        AbcMethodProcessor methodProcessor(id, keyData_);
        name << methodProcessor.GetMethodSignature();
    } else if (bcIns.HasFlag(BytecodeInstruction::Flags::STRING_ID)) {
        const auto offset = bcIns.GetId().AsFileId();
        stringTable_->AddStringId(offset);
        std::string strData = stringTable_->GetStringById(offset);
        name << stringTable_->GetStringById(bcIns.GetId().AsFileId());
    } else if (bcIns.HasFlag(BytecodeInstruction::Flags::FIELD_ID) ||
               bcIns.HasFlag(BytecodeInstruction::Flags::STATIC_FIELD_ID)) {
        auto id = file_->ResolveFieldIndex(methodId, bcIns.GetId().AsIndex());
        panda_file::FieldDataAccessor fieldAccessor(*file_, id);

        auto recordName = keyData_.GetFullRecordNameById(fieldAccessor.GetClassId());
        name << recordName << "." << stringTable_->GetStringById(fieldAccessor.GetNameId());
    } else if (bcIns.HasFlag(BytecodeInstruction::Flags::LITERALARRAY_ID)) {
        auto index = bcIns.GetId().AsIndex();
        name << "array_" << index;
    }

    return name.str();
}

}  // namespace ark::abc2program
