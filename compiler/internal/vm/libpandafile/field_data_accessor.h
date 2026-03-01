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

#ifndef LIBPANDAFILE_FIELD_DATA_ACCESSOR_H
#define LIBPANDAFILE_FIELD_DATA_ACCESSOR_H

#include "file.h"
#include "file_items.h"
#include "modifiers.h"
#include "type.h"

#include <variant>
#include <vector>
#include <optional>

namespace panda::panda_file {

class FieldDataAccessor {
public:
    PANDA_PUBLIC_API FieldDataAccessor(const File &panda_file, File::EntityId field_id);

    ~FieldDataAccessor() = default;

    NO_COPY_SEMANTIC(FieldDataAccessor);
    NO_MOVE_SEMANTIC(FieldDataAccessor);

    // quick way to get type id
    static File::EntityId GetTypeId(const File &panda_file, File::EntityId field_id);

    static File::EntityId GetNameId(const File &panda_file, File::EntityId field_id);

    bool IsExternal() const
    {
        return is_external_;
    }

    File::EntityId GetClassId() const
    {
        return File::EntityId(class_off_);
    }

    File::EntityId GetNameId() const
    {
        return File::EntityId(name_off_);
    }

    uint32_t GetType() const
    {
        return type_off_;
    }

    uint32_t GetAccessFlags() const
    {
        return access_flags_;
    }

    bool IsStatic() const
    {
        return (access_flags_ & ACC_STATIC) != 0;
    }

    bool IsVolatile() const
    {
        return (access_flags_ & ACC_VOLATILE) != 0;
    }

    bool IsPublic() const
    {
        return (access_flags_ & ACC_PUBLIC) != 0;
    }

    bool IsPrivate() const
    {
        return (access_flags_ & ACC_PRIVATE) != 0;
    }

    bool IsProtected() const
    {
        return (access_flags_ & ACC_PROTECTED) != 0;
    }

    bool IsFinal() const
    {
        return (access_flags_ & ACC_FINAL) != 0;
    }

    bool IsTransient() const
    {
        return (access_flags_ & ACC_TRANSIENT) != 0;
    }

    bool IsSynthetic() const
    {
        return (access_flags_ & ACC_SYNTHETIC) != 0;
    }

    bool IsEnum() const
    {
        return (access_flags_ & ACC_ENUM) != 0;
    }

    template <class T>
    std::optional<T> GetValue();

    template <class Callback>
    void EnumerateRuntimeAnnotations(const Callback &cb);

    template <class Callback>
    void EnumerateAnnotations(const Callback &cb);

    template <class Callback>
    void EnumerateRuntimeTypeAnnotations(const Callback &cb);

    template <class Callback>
    void EnumerateTypeAnnotations(const Callback &cb);

    template <class Callback>
    bool EnumerateRuntimeAnnotationsWithEarlyStop(const Callback &cb);

    template <class Callback>
    bool EnumerateAnnotationsWithEarlyStop(const Callback &cb);

    size_t GetSize()
    {
        if (size_ == 0) {
            SkipTypeAnnotations();
        }

        return size_;
    }

    const File &GetPandaFile() const
    {
        return panda_file_;
    }

    File::EntityId GetFieldId() const
    {
        return field_id_;
    }

    uint32_t GetAnnotationsNumber();
    uint32_t GetRuntimeAnnotationsNumber();
    uint32_t GetRuntimeTypeAnnotationsNumber();
    uint32_t GetTypeAnnotationsNumber();

private:
    using FieldValue = std::variant<uint32_t, uint64_t>;

    PANDA_PUBLIC_API std::optional<FieldValue> GetValueInternal();

    void SkipValue();

    void SkipRuntimeAnnotations();

    void SkipAnnotations();

    void SkipRuntimeTypeAnnotations();

    void SkipTypeAnnotations();

    const File &panda_file_;
    File::EntityId field_id_;

    bool is_external_;

    uint32_t class_off_;
    uint32_t type_off_;
    uint32_t name_off_;
    uint32_t access_flags_;

    Span<const uint8_t> tagged_values_sp_ {nullptr, nullptr};
    Span<const uint8_t> runtime_annotations_sp_ {nullptr, nullptr};
    Span<const uint8_t> annotations_sp_ {nullptr, nullptr};
    Span<const uint8_t> runtime_type_annotations_sp_ {nullptr, nullptr};
    Span<const uint8_t> type_annotations_sp_ {nullptr, nullptr};

    size_t size_;
};

}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_FIELD_DATA_ACCESSOR_H
