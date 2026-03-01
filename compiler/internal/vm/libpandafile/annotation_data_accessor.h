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

#ifndef LIBPANDAFILE_ANNOTATION_DATA_ACCESSOR_H
#define LIBPANDAFILE_ANNOTATION_DATA_ACCESSOR_H

#include "file.h"
#include "helpers.h"
#include "value.h"

namespace panda::panda_file {

class AnnotationDataAccessor {
public:
    class Elem {
    public:
        Elem(const File &panda_file, File::EntityId name_id, uint32_t value)
            : panda_file_(panda_file), name_id_(name_id), value_(value)
        {
        }
        ~Elem() = default;

        NO_COPY_SEMANTIC(Elem);
        NO_MOVE_SEMANTIC(Elem);

        File::EntityId GetNameId() const
        {
            return name_id_;
        }

        ScalarValue GetScalarValue() const
        {
            return ScalarValue(panda_file_, value_);
        }

        ArrayValue GetArrayValue() const
        {
            return ArrayValue(panda_file_, File::EntityId(value_));
        }

    private:
        const File &panda_file_;
        File::EntityId name_id_;
        uint32_t value_;
    };

    class Tag {
    public:
        explicit Tag(char item) : item_(item) {}
        ~Tag() = default;

        NO_COPY_SEMANTIC(Tag);
        NO_MOVE_SEMANTIC(Tag);

        char GetItem() const
        {
            return item_;
        }

    private:
        char item_;
    };

    PANDA_PUBLIC_API AnnotationDataAccessor(const File &panda_file, File::EntityId annotation_id);
    ~AnnotationDataAccessor() = default;

    NO_COPY_SEMANTIC(AnnotationDataAccessor);
    NO_MOVE_SEMANTIC(AnnotationDataAccessor);

    File::EntityId GetClassId() const
    {
        return File::EntityId(class_off_);
    }

    uint32_t GetCount() const
    {
        return count_;
    }

    PANDA_PUBLIC_API Elem GetElement(size_t i) const;

    PANDA_PUBLIC_API Tag GetTag(size_t i) const;

    size_t GetSize() const
    {
        return size_;
    }

    File::EntityId GetAnnotationId() const
    {
        return annotation_id_;
    }

private:
    static constexpr size_t COUNT_SIZE = sizeof(uint16_t);
    static constexpr size_t VALUE_SIZE = sizeof(uint32_t);
    static constexpr size_t TYPE_TAG_SIZE = sizeof(uint8_t);

    const File &panda_file_;
    File::EntityId annotation_id_;

    uint32_t class_off_;
    uint32_t count_;
    Span<const uint8_t> elements_sp_ {nullptr, nullptr};
    Span<const uint8_t> elements_tags_ {nullptr, nullptr};
    size_t size_;
};

}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_ANNOTATION_DATA_ACCESSOR_H
