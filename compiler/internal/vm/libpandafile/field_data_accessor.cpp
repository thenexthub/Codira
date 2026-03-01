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

#include "field_data_accessor.h"

namespace panda::panda_file {

FieldDataAccessor::FieldDataAccessor(const File &panda_file, File::EntityId field_id)
    : panda_file_(panda_file), field_id_(field_id)
{
    auto sp = panda_file_.GetSpanFromId(field_id_);

    auto class_idx = helpers::Read<IDX_SIZE>(&sp);
    auto type_idx = helpers::Read<IDX_SIZE>(&sp);

    class_off_ = panda_file.ResolveClassIndex(field_id, class_idx).GetOffset();
    type_off_ = panda_file.ResolveClassIndex(field_id, type_idx).GetOffset();

    name_off_ = helpers::Read<ID_SIZE>(&sp);

    is_external_ = panda_file_.IsExternal(field_id_);

    if (!is_external_) {
        access_flags_ = helpers::ReadULeb128(&sp);
        tagged_values_sp_ = sp;
        size_ = 0;
    } else {
        access_flags_ = 0;
        size_ = panda_file_.GetIdFromPointer(sp.data()).GetOffset() - field_id_.GetOffset();
    }
}

std::optional<FieldDataAccessor::FieldValue> FieldDataAccessor::GetValueInternal()
{
    panda_file_.ThrowIfWithCheck(tagged_values_sp_.Size() == 0U, File::INVALID_FILE_OFFSET, File::FIELD_DATA_ACCESSOR);

    auto sp = tagged_values_sp_;
    auto tag = static_cast<FieldTag>(sp[0]);
    FieldValue value;

    if (tag == FieldTag::INT_VALUE) {
        panda_file_.ThrowIfWithCheck(sp.Size() == 0U, File::INVALID_FILE_OFFSET, File::FIELD_DATA_ACCESSOR);
        sp = sp.SubSpan(1);
        value = static_cast<uint32_t>(helpers::ReadLeb128(&sp));
    } else if (tag == FieldTag::VALUE) {
        panda_file_.ThrowIfWithCheck(sp.Size() == 0U, File::INVALID_FILE_OFFSET, File::FIELD_DATA_ACCESSOR);
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
                auto value_sp = panda_file_.GetSpanFromId(File::EntityId(offset));
                value = static_cast<uint64_t>(helpers::Read<sizeof(uint64_t)>(value_sp));
                break;
            }
            default: {
                value = static_cast<uint32_t>(helpers::Read<sizeof(uint32_t)>(&sp));
                break;
            }
        }
    }

    runtime_annotations_sp_ = sp;

    if (tag == FieldTag::INT_VALUE || tag == FieldTag::VALUE) {
        return value;
    }

    return {};
}

}  // namespace panda::panda_file
