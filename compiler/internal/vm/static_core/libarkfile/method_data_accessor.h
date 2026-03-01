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

#ifndef LIBPANDAFILE_METHOD_DATA_ACCESSOR_H_
#define LIBPANDAFILE_METHOD_DATA_ACCESSOR_H_

#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "libarkfile/modifiers.h"

namespace ark::panda_file {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class MethodDataAccessor {
public:
    enum AnnotationField : uint32_t {
        IC_SIZE = 0,
        FUNCTION_LENGTH,
        FUNCTION_NAME,
        STRING_DATA_BEGIN = FUNCTION_NAME,
        STRING_DATA_END = FUNCTION_NAME
    };

    PANDA_PUBLIC_API MethodDataAccessor(const File &pandaFile, File::EntityId methodId);

    ~MethodDataAccessor() = default;

    // quick way to get name id
    static File::EntityId GetNameId(const File &pandaFile, File::EntityId methodId);

    // quick way to get method name
    static panda_file::File::StringData GetName(const File &pandaFile, File::EntityId methodId);

    std::string GetClassName() const;

    std::string GetFullName() const;

    // quick way to get proto id
    static File::EntityId GetProtoId(const File &pandaFile, File::EntityId methodId);

    // quick way to get class id
    static File::EntityId GetClassId(const File &pandaFile, File::EntityId methodId);

    bool IsExternal() const
    {
        return isExternal_;
    }

    bool IsStatic() const
    {
        return (accessFlags_ & ACC_STATIC) != 0;
    }

    bool IsAbstract() const
    {
        return (accessFlags_ & ACC_ABSTRACT) != 0;
    }

    bool IsVarArgs() const
    {
        return (accessFlags_ & ACC_VARARGS) != 0;
    }

    bool IsNative() const
    {
        return (accessFlags_ & ACC_NATIVE) != 0;
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

    bool IsSynthetic() const
    {
        return (accessFlags_ & ACC_SYNTHETIC) != 0;
    }

    File::EntityId GetClassId() const
    {
        return File::EntityId(classOff_);
    }

    File::Index GetClassIdx() const
    {
        return classIdx_;
    }

    File::EntityId GetNameId() const
    {
        return File::EntityId(nameOff_);
    };

    panda_file::File::StringData GetName() const;

    File::EntityId GetProtoId() const
    {
        return File::EntityId(protoOff_);
    }

    uint32_t GetAccessFlags() const
    {
        return accessFlags_;
    }

    std::optional<File::EntityId> GetCodeId();

    std::optional<SourceLang> GetSourceLang();

    template <class Callback>
    void EnumerateRuntimeAnnotations(Callback cb);

    template <typename Callback>
    void EnumerateTypesInProto(Callback cb, bool skipThis = false);

    Type GetReturnType() const;

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

    std::optional<size_t> GetProfileSize();

    std::optional<File::EntityId> GetParamAnnotationId();

    size_t GetSize()
    {
        if (size_ == 0) {
            GetProfileSize();
        }

        return size_;
    }

    const File &GetPandaFile() const
    {
        return pandaFile_;
    }

    File::EntityId GetMethodId() const
    {
        return methodId_;
    }

    uint32_t GetAnnotationsNumber();
    uint32_t GetRuntimeAnnotationsNumber();
    uint32_t GetTypeAnnotationsNumber();
    uint32_t GetRuntimeTypeAnnotationsNumber();

    template <std::size_t SIZE>
    void NumAnnonationProcess(uint32_t &result, File::EntityId &annotationId,
                              std::array<const char *, SIZE> elemNameTable, uint32_t fieldId);
    uint32_t GetNumericalAnnotation(uint32_t fieldId);

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

    const File &pandaFile_;
    File::EntityId methodId_;

    bool isExternal_;

    uint16_t classIdx_;
    uint16_t protoIdx_;
    uint32_t classOff_;
    uint32_t protoOff_;
    uint32_t nameOff_;
    uint32_t accessFlags_;

    Span<const uint8_t> taggedValuesSp_ {nullptr, nullptr};
    Span<const uint8_t> sourceLangSp_ {nullptr, nullptr};
    Span<const uint8_t> runtimeAnnotationsSp_ {nullptr, nullptr};
    Span<const uint8_t> runtimeParamAnnotationSp_ {nullptr, nullptr};
    Span<const uint8_t> debugSp_ {nullptr, nullptr};
    Span<const uint8_t> annotationsSp_ {nullptr, nullptr};
    Span<const uint8_t> paramAnnotationSp_ {nullptr, nullptr};
    Span<const uint8_t> typeAnnotationSp_ {nullptr, nullptr};
    Span<const uint8_t> runtimeTypeAnnotationSp_ {nullptr, nullptr};
    Span<const uint8_t> profileInfoSp_ {nullptr, nullptr};

    size_t size_;
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_METHOD_DATA_ACCESSOR_H_
