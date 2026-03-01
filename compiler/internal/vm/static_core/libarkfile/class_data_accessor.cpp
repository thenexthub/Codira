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

#include "libarkfile/class_data_accessor.h"
#include "libarkfile/helpers.h"

#include "libarkbase/utils/leb128.h"
#include "libarkbase/utils/utf.h"

namespace ark::panda_file {

ClassDataAccessor::ClassDataAccessor(const File &pandaFile, File::EntityId classId)
    : pandaFile_(pandaFile), classId_(classId)
{
    ASSERT(!pandaFile.IsExternal(classId));
    auto sp = pandaFile_.GetSpanFromId(classId_);
    name_.utf16Length = helpers::ReadULeb128(&sp);
    name_.data = sp.data();

    sp = sp.SubSpan(utf::Mutf8Size(name_.data) + 1);  // + 1 for null byte

    superClassOff_ = helpers::Read<ID_SIZE>(&sp);

    accessFlags_ = helpers::ReadULeb128(&sp);
    numFields_ = helpers::ReadULeb128(&sp);
    numMethods_ = helpers::ReadULeb128(&sp);

    auto tag = static_cast<ClassTag>(sp[0]);

    while (tag != ClassTag::NOTHING && tag < ClassTag::SOURCE_LANG) {
        sp = sp.SubSpan(1);

        if (tag == ClassTag::INTERFACES) {
            numIfaces_ = helpers::ReadULeb128(&sp);
            ifacesOffsetsSp_ = sp;
            sp = sp.SubSpan(IDX_SIZE * numIfaces_);
        }

        tag = static_cast<ClassTag>(sp[0]);
    }

    sourceLangSp_ = sp;

    if (tag == ClassTag::NOTHING) {
        annotationsSp_ = sp;
        sourceFileSp_ = sp;
        fieldsSp_ = sp.SubSpan(TAG_SIZE);  // skip NOTHING tag
    }
}

}  // namespace ark::panda_file
