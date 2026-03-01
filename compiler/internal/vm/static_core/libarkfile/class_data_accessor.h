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

#ifndef LIBPANDAFILE_CLASS_DATA_ACCESSOR_H_
#define LIBPANDAFILE_CLASS_DATA_ACCESSOR_H_

#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "libarkfile/field_data_accessor.h"
#include "libarkfile/method_data_accessor.h"

namespace ark::panda_file {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class ClassDataAccessor {
public:
    PANDA_PUBLIC_API ClassDataAccessor(const File &pandaFile, File::EntityId classId);

    ~ClassDataAccessor() = default;

    File::EntityId GetSuperClassId() const
    {
        return File::EntityId(superClassOff_);
    }

    bool IsInterface() const
    {
        return (accessFlags_ & ACC_INTERFACE) != 0;
    }

    bool IsPublic() const
    {
        return (accessFlags_ & ACC_PUBLIC) != 0;
    }

    bool IsProtected() const
    {
        return (accessFlags_ & ACC_PROTECTED) != 0;
    }

    bool IsPrivate() const
    {
        return (accessFlags_ & ACC_PRIVATE) != 0;
    }

    bool IsFinal() const
    {
        return (accessFlags_ & ACC_FINAL) != 0;
    }

    uint32_t GetAccessFlags() const
    {
        return accessFlags_;
    }

    uint32_t GetFieldsNumber() const
    {
        return numFields_;
    }

    uint32_t GetMethodsNumber() const
    {
        return numMethods_;
    }

    uint32_t GetIfacesNumber() const
    {
        return numIfaces_;
    }

    uint32_t GetAnnotationsNumber();

    uint32_t GetRuntimeAnnotationsNumber();

    File::EntityId GetInterfaceId(size_t idx) const;

    template <class Callback>
    void EnumerateInterfaces(const Callback &cb);

    template <class Callback>
    void EnumerateRuntimeAnnotations(const Callback &cb);

    template <class Callback>
    void EnumerateAnnotations(const Callback &cb);

    template <class Callback>
    void EnumerateTypeAnnotations(const Callback &cb);

    template <class Callback>
    void EnumerateRuntimeTypeAnnotations(const Callback &cb);

    template <class Callback>
    bool EnumerateRuntimeAnnotationsWithEarlyStop(const Callback &cb);

    template <class Callback>
    bool EnumerateAnnotationsWithEarlyStop(const Callback &cb);

    template <class Callback>
    auto EnumerateAnnotation(const char *name, const Callback &cb);

    std::optional<SourceLang> GetSourceLang();

    std::optional<File::EntityId> GetSourceFileId();

    template <class Callback>
    void EnumerateFields(const Callback &cb);

    template <class Callback>
    void EnumerateMethods(const Callback &cb);

    size_t GetSize()
    {
        if (size_ == 0) {
            SkipMethods();
        }

        return size_;
    }

    const File &GetPandaFile() const
    {
        return pandaFile_;
    }

    File::EntityId GetClassId() const
    {
        return classId_;
    }

    const uint8_t *GetDescriptor() const
    {
        return name_.data;
    }

    static std::string DemangledName(panda_file::File::StringData data)
    {
        auto descriptor = data.data;
        switch (*descriptor) {
            case 'V':
                return "void";
            case 'Z':
                return "u1";
            case 'B':
                return "i8";
            case 'H':
                return "u8";
            case 'S':
                return "i16";
            case 'C':
                return "u16";
            case 'I':
                return "i32";
            case 'U':
                return "u32";
            case 'J':
                return "i64";
            case 'Q':
                return "u64";
            case 'F':
                return "f32";
            case 'D':
                return "f64";
            case 'A':
                return "any";
            case 'Y':
                return "Y";
            case 'N':
                return "N";
            default: {
                break;
            }
        }

        std::string name = utf::Mutf8AsCString(descriptor);
        if (name[0] == '[') {
            return name;
        }

        std::replace(name.begin(), name.end(), '/', '.');

        ASSERT(name.size() > 2);  // 2 - L and ;

        name.erase(0, 1);
        name.pop_back();

        return name;
    }

    std::string DemangledName() const
    {
        return DemangledName(name_);
    }

private:
    template <class Callback, class Accessor>
    void EnumerateClassElements(const File &pf, Span<const uint8_t> sp, size_t elemNum, const Callback &cb,
                                Span<const uint8_t> *next);

    void SkipSourceLang();

    void SkipRuntimeAnnotations();

    void SkipAnnotations();

    void SkipSourceFile();

    void SkipFields();

    void SkipMethods();

    void SkipRuntimeTypeAnnotations();

    void SkipTypeAnnotations();

    const File &pandaFile_;
    File::EntityId classId_;

    File::StringData name_ {};
    uint32_t superClassOff_;
    uint32_t accessFlags_;
    uint32_t numFields_ {0};
    uint32_t numMethods_ {0};
    uint32_t numIfaces_ {0};

    Span<const uint8_t> ifacesOffsetsSp_ {nullptr, nullptr};
    Span<const uint8_t> sourceLangSp_ {nullptr, nullptr};
    Span<const uint8_t> runtimeAnnotationsSp_ {nullptr, nullptr};
    Span<const uint8_t> annotationsSp_ {nullptr, nullptr};
    Span<const uint8_t> runtimeTypeAnnotationSp_ {nullptr, nullptr};
    Span<const uint8_t> typeAnnotationSp_ {nullptr, nullptr};
    Span<const uint8_t> sourceFileSp_ {nullptr, nullptr};
    Span<const uint8_t> fieldsSp_ {nullptr, nullptr};
    Span<const uint8_t> methodsSp_ {nullptr, nullptr};

    size_t size_ {0};
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_CLASS_DATA_ACCESSOR_H_
