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

#include "libarkfile/annotation_data_accessor.h"
#include "libarkfile/file_items.h"

namespace ark::panda_file {

AnnotationDataAccessor::AnnotationDataAccessor(const File &pandaFile, File::EntityId annotationId)
    : pandaFile_(pandaFile), annotationId_(annotationId)
{
    auto sp = pandaFile_.GetSpanFromId(annotationId_);
    auto classIdx = helpers::Read<IDX_SIZE>(&sp);
    classOff_ = pandaFile_.ResolveClassIndex(annotationId_, classIdx).GetOffset();
    count_ = helpers::Read<COUNT_SIZE>(&sp);
    size_ = ID_SIZE + COUNT_SIZE + count_ * (ID_SIZE + VALUE_SIZE) + count_ * TYPE_TAG_SIZE;

    elementsSp_ = sp;
    elementsTags_ = sp.SubSpan(count_ * (ID_SIZE + VALUE_SIZE));
}

AnnotationDataAccessor::Elem AnnotationDataAccessor::GetElement(size_t i) const
{
    ASSERT(i < count_);
    auto sp = elementsSp_.SubSpan(i * (ID_SIZE + VALUE_SIZE));
    uint32_t name = helpers::Read<ID_SIZE>(&sp);
    uint32_t value = helpers::Read<VALUE_SIZE>(&sp);
    return AnnotationDataAccessor::Elem(pandaFile_, File::EntityId(name), value);
}

AnnotationDataAccessor::Tag AnnotationDataAccessor::GetTag(size_t i) const
{
    ASSERT(i < count_);
    auto sp = elementsTags_.SubSpan(i * TYPE_TAG_SIZE);
    auto item = static_cast<char>(helpers::Read<TYPE_TAG_SIZE>(&sp));
    return AnnotationDataAccessor::Tag(item);
}

}  // namespace ark::panda_file
