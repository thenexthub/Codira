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

#include "method_data_accessor.h"

namespace panda::panda_file {

MethodDataAccessor::MethodDataAccessor(const File &panda_file, File::EntityId method_id)
    : panda_file_(panda_file), method_id_(method_id)
{
    auto sp = panda_file_.GetSpanFromId(method_id);

    class_idx_ = helpers::Read<IDX_SIZE>(&sp);
    proto_idx_ = helpers::Read<IDX_SIZE>(&sp);

    class_off_ = panda_file.ResolveClassIndex(method_id, class_idx_).GetOffset();
    if (proto_idx_ != INVALID_INDEX_16) {
        proto_off_ = panda_file.ResolveProtoIndex(method_id, proto_idx_).GetOffset();
    }

    name_off_ = helpers::Read<ID_SIZE>(&sp);
    access_flags_ = helpers::ReadULeb128(&sp);

    is_external_ = panda_file_.IsExternal(method_id);

    if (!is_external_) {
        tagged_values_sp_ = sp;
        size_ = 0;
    } else {
        size_ = panda_file_.GetIdFromPointer(sp.data()).GetOffset() - method_id_.GetOffset();
    }
}

}  // namespace panda::panda_file
