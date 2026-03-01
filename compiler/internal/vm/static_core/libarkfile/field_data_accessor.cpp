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

#include "libarkfile/field_data_accessor.h"
#include "libarkfile/helpers.h"

#include "libarkbase/utils/leb128.h"

namespace ark::panda_file {

FieldDataAccessor::FieldDataAccessor(const File &pandaFile, File::EntityId fieldId)
    : pandaFile_(pandaFile), fieldId_(fieldId)
{
    auto sp = pandaFile_.GetSpanFromId(fieldId_);

    auto classIdx = helpers::Read<IDX_SIZE>(&sp);
    auto typeIdx = helpers::Read<IDX_SIZE>(&sp);

    classOff_ = pandaFile.ResolveClassIndex(fieldId, classIdx).GetOffset();
    typeOff_ = pandaFile.ResolveClassIndex(fieldId, typeIdx).GetOffset();

    nameOff_ = helpers::Read<ID_SIZE>(&sp);
    accessFlags_ = helpers::ReadULeb128(&sp);

    isExternal_ = pandaFile_.IsExternal(fieldId_);
    if (!isExternal_) {
        taggedValuesSp_ = sp;
        size_ = 0;
    } else {
        size_ = pandaFile_.GetIdFromPointer(sp.data()).GetOffset() - fieldId_.GetOffset();
    }
}

std::optional<FieldDataAccessor::FieldValue> FieldDataAccessor::GetValueInternal()
{
    auto sp = taggedValuesSp_;
    auto tag = static_cast<FieldTag>(sp[0]);
    FieldValue value;

    if (tag == FieldTag::INT_VALUE) {
        sp = sp.SubSpan(1);
        value = static_cast<uint32_t>(helpers::ReadLeb128(&sp));
    } else if (tag == FieldTag::VALUE) {
        sp = sp.SubSpan(1);

        switch (GetType()) {
            case Type(Type::TypeId::F32).GetFieldEncoding(): {
                value = static_cast<uint32_t>(helpers::Read<sizeof(uint32_t)>(&sp));
                break;
            }
            case Type(Type::TypeId::I64).GetFieldEncoding():
            case Type(Type::TypeId::U64).GetFieldEncoding():
            case Type(Type::TypeId::F64).GetFieldEncoding(): {
                auto offset = static_cast<uint32_t>(helpers::Read<sizeof(uint32_t)>(&sp));
                auto valueSp = pandaFile_.GetSpanFromId(File::EntityId(offset));
                value = static_cast<uint64_t>(helpers::Read<sizeof(uint64_t)>(valueSp));
                break;
            }
            default: {
                value = static_cast<uint32_t>(helpers::Read<sizeof(uint32_t)>(&sp));
                break;
            }
        }
    }

    runtimeAnnotationsSp_ = sp;

    if (tag == FieldTag::INT_VALUE || tag == FieldTag::VALUE) {
        return value;
    }

    return {};
}

}  // namespace ark::panda_file
