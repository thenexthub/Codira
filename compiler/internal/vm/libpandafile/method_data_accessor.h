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

#ifndef LIBPANDAFILE_METHOD_DATA_ACCESSOR_H
#define LIBPANDAFILE_METHOD_DATA_ACCESSOR_H

#include "file.h"
#include "file_items.h"
#include "modifiers.h"

namespace panda::panda_file {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class MethodDataAccessor {
public:
    PANDA_PUBLIC_API MethodDataAccessor(const File &panda_file, File::EntityId method_id);

    ~MethodDataAccessor() = default;

    // quick way to get name id
    static File::EntityId GetNameId(const File &panda_file, File::EntityId method_id);

    // quick way to get method name
    static panda_file::File::StringData GetName(const File &panda_file, File::EntityId method_id);

    // quick way to get proto id
    static File::EntityId GetProtoId(const File &panda_file, File::EntityId method_id);

    // quick way to get class id
    static File::EntityId GetClassId(const File &panda_file, File::EntityId method_id);

    bool IsExternal() const
    {
        return is_external_;
    }

    bool IsStatic() const
    {
        return (access_flags_ & ACC_STATIC) != 0;
    }

    bool IsAbstract() const
    {
        return (access_flags_ & ACC_ABSTRACT) != 0;
    }

    bool IsNative() const
    {
        return (access_flags_ & ACC_NATIVE) != 0;
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

    bool IsSynthetic() const
    {
        return (access_flags_ & ACC_SYNTHETIC) != 0;
    }

    File::EntityId GetClassId() const
    {
        return File::EntityId(class_off_);
    }

    File::Index GetClassIdx() const
    {
        return class_idx_;
    }

    File::EntityId GetNameId() const
    {
        return File::EntityId(name_off_);
    };

    panda_file::File::StringData GetName() const;

    File::EntityId GetProtoId() const
    {
        return File::EntityId(proto_off_);
    }

    File::Index GetProtoIdx() const
    {
        return proto_idx_;
    }

    uint32_t GetAccessFlags() const
    {
        return access_flags_;
    }

    std::optional<File::EntityId> GetCodeId();

    std::optional<SourceLang> GetSourceLang();

    template <class Callback>
    void EnumerateRuntimeAnnotations(Callback cb);

    template <typename Callback>
    void EnumerateTypesInProto(Callback cb, bool skip_this = false);

    std::optional<File::EntityId> GetRuntimeParamAnnotationId();

    std::optional<File::EntityId> GetDebugInfoId();

    template <class Callback>
    void EnumerateAnnotations(Callback cb);

    template <class Callback>
    bool EnumerateRuntimeAnnotationsWithEarlyStop(Callback cb);

    template <class Callback>
    bool EnumerateAnnotationsWithEarlyStop(Callback cb);

    template <class Callback>
    void EnumerateTypeAnnotations(Callback cb);

    template <class Callback>
    void EnumerateRuntimeTypeAnnotations(Callback cb);

    std::optional<File::EntityId> GetParamAnnotationId();

    size_t GetSize()
    {
        if (size_ == 0) {
            SkipRuntimeTypeAnnotation();
        }

        return size_;
    }

    const File &GetPandaFile() const
    {
        return panda_file_;
    }

    File::EntityId GetMethodId() const
    {
        return method_id_;
    }

    bool HasValidProto() const
    {
        return proto_idx_ != INVALID_INDEX_16;
    }

    uint32_t GetAnnotationsNumber();
    uint32_t GetRuntimeAnnotationsNumber();
    uint32_t GetTypeAnnotationsNumber();
    uint32_t GetRuntimeTypeAnnotationsNumber();

    uint32_t GetNumericalAnnotation(uint32_t field_id);

private:
    void SkipCode();

    void SkipSourceLang();

    void SkipRuntimeAnnotations();

    void SkipRuntimeParamAnnotation();

    void SkipDebugInfo();

    void SkipAnnotations();

    void SkipParamAnnotation();

    void SkipTypeAnnotation();

    void SkipRuntimeTypeAnnotation();

    const File &panda_file_;
    File::EntityId method_id_;

    bool is_external_;

    uint16_t class_idx_;
    uint16_t proto_idx_;
    uint32_t class_off_;
    uint32_t proto_off_ {0};
    uint32_t name_off_;
    uint32_t access_flags_;

    Span<const uint8_t> tagged_values_sp_ {nullptr, nullptr};
    Span<const uint8_t> source_lang_sp_ {nullptr, nullptr};
    Span<const uint8_t> runtime_annotations_sp_ {nullptr, nullptr};
    Span<const uint8_t> runtime_param_annotation_sp_ {nullptr, nullptr};
    Span<const uint8_t> debug_sp_ {nullptr, nullptr};
    Span<const uint8_t> annotations_sp_ {nullptr, nullptr};
    Span<const uint8_t> param_annotation_sp_ {nullptr, nullptr};
    Span<const uint8_t> type_annotation_sp_ {nullptr, nullptr};
    Span<const uint8_t> runtime_type_annotation_sp_ {nullptr, nullptr};

    size_t size_;
};

}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_METHOD_DATA_ACCESSOR_H
