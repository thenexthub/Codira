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

#ifndef LIBPANDAFILE_FIELD_DATA_ACCESSOR_H_
#define LIBPANDAFILE_FIELD_DATA_ACCESSOR_H_

#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "libarkfile/modifiers.h"
#include <libarkfile/include/type.h>

#include <variant>
#include <vector>
#include <optional>

namespace ark::panda_file {

class FieldDataAccessor {
public:
    PANDA_PUBLIC_API FieldDataAccessor(const File &pandaFile, File::EntityId fieldId);

    ~FieldDataAccessor() = default;

    NO_COPY_SEMANTIC(FieldDataAccessor);
    NO_MOVE_SEMANTIC(FieldDataAccessor);

    // quick way to get type id
    static File::EntityId GetTypeId(const File &pandaFile, File::EntityId fieldId);

    static File::EntityId GetNameId(const File &pandaFile, File::EntityId fieldId);

    bool IsExternal() const
    {
        return isExternal_;
    }

    File::EntityId GetClassId() const
    {
        return File::EntityId(classOff_);
    }

    File::EntityId GetNameId() const
    {
        return File::EntityId(nameOff_);
    }

    uint32_t GetType() const
    {
        return typeOff_;
    }

    uint32_t GetAccessFlags() const
    {
        return accessFlags_;
    }

    bool IsStatic() const
    {
        return (accessFlags_ & ACC_STATIC) != 0;
    }

    bool IsVolatile() const
    {
        return (accessFlags_ & ACC_VOLATILE) != 0;
    }

    bool IsPublic() const
    {
        return (accessFlags_ & ACC_PUBLIC) != 0;
    }

    bool IsPrivate() const
    {
        return (accessFlags_ & ACC_PRIVATE) != 0;
    }

    bool IsProtected() const
    {
        return (accessFlags_ & ACC_PROTECTED) != 0;
    }

    bool IsFinal() const
    {
        return (accessFlags_ & ACC_FINAL) != 0;
    }

    bool IsReadonly() const
    {
        return (accessFlags_ & ACC_READONLY) != 0;
    }

    bool IsTransient() const
    {
        return (accessFlags_ & ACC_TRANSIENT) != 0;
    }

    bool IsSynthetic() const
    {
        return (accessFlags_ & ACC_SYNTHETIC) != 0;
    }

    bool IsEnum() const
    {
        return (accessFlags_ & ACC_ENUM) != 0;
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
        return pandaFile_;
    }

    File::EntityId GetFieldId() const
    {
        return fieldId_;
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

    template <class T>
    inline T GetValueImpl(FieldValue &fieldValue);

    template <class T>
    inline T GetValueIntegral(FieldValue &fieldValue);

    template <class T>
    inline T GetValueNonIntegral(FieldValue &fieldValue);

    const File &pandaFile_;
    File::EntityId fieldId_;

    bool isExternal_;

    uint32_t classOff_;
    uint32_t typeOff_;
    uint32_t nameOff_;
    uint32_t accessFlags_;

    Span<const uint8_t> taggedValuesSp_ {nullptr, nullptr};
    Span<const uint8_t> runtimeAnnotationsSp_ {nullptr, nullptr};
    Span<const uint8_t> annotationsSp_ {nullptr, nullptr};
    Span<const uint8_t> runtimeTypeAnnotationsSp_ {nullptr, nullptr};
    Span<const uint8_t> typeAnnotationsSp_ {nullptr, nullptr};

    size_t size_;
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_FIELD_DATA_ACCESSOR_H_
