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
#ifndef PANDA_RUNTIME_FIELD_H_
#define PANDA_RUNTIME_FIELD_H_

#include <cstdint>
#include <atomic>

#include "intrinsics_enum.h"
#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "libarkfile/modifiers.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/class_helper.h"
#include "runtime/include/value-inl.h"
#include "libarkbase/macros.h"

namespace ark {

class Class;

class ClassLinkerErrorHandler;

class Field {
public:
    using UniqId = uint64_t;

    Field(Class *klass, panda_file::File::EntityId fileId, uint32_t accessFlags, panda_file::Type type)
        : classWord_(static_cast<ClassHelper::ClassWordSize>(ToObjPtrType(klass))), fileId_(fileId)
    {
        accessFlags_ = accessFlags | (static_cast<uint32_t>(type.GetEncoding()) << ACC_TYPE_SHIFT);
    }

    Class *GetClass() const
    {
        return reinterpret_cast<Class *>(classWord_);
    }

    void SetClass(Class *cls)
    {
        classWord_ = static_cast<ClassHelper::ClassWordSize>(ToObjPtrType(cls));
    }

    static constexpr uint32_t GetClassOffset()
    {
        return MEMBER_OFFSET(Field, classWord_);
    }

    PANDA_PUBLIC_API const panda_file::File *GetPandaFile() const;

    panda_file::File::EntityId GetFileId() const
    {
        return fileId_;
    }

    uint32_t GetAccessFlags() const
    {
        return accessFlags_;
    }

    uint32_t GetOffset() const
    {
        return offset_;
    }

    void SetOffset(uint32_t offset)
    {
        offset_ = offset;
    }

    static constexpr uint32_t GetOffsetOffset()
    {
        return MEMBER_OFFSET(Field, offset_);
    }

    PANDA_PUBLIC_API Class *ResolveTypeClass(ClassLinkerErrorHandler *errorHandler = nullptr) const;

    panda_file::Type GetType() const
    {
        return panda_file::Type(GetTypeId());
    }

    panda_file::Type::TypeId GetTypeId() const
    {
        return static_cast<panda_file::Type::TypeId>((accessFlags_ & ACC_TYPE) >> ACC_TYPE_SHIFT);
    }

    static constexpr uint32_t GetAccessFlagsOffset()
    {
        return MEMBER_OFFSET(Field, accessFlags_);
    }

    PANDA_PUBLIC_API panda_file::File::StringData GetName() const;

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

    bool IsStatic() const
    {
        return (accessFlags_ & ACC_STATIC) != 0;
    }

    bool IsVolatile() const
    {
        return (accessFlags_ & ACC_VOLATILE) != 0;
    }

    bool IsFinal() const
    {
        return (accessFlags_ & ACC_FINAL) != 0;
    }

    bool IsReadonly() const
    {
        return (accessFlags_ & ACC_READONLY) != 0;
    }

    static inline UniqId CalcUniqId(const panda_file::File *file, panda_file::File::EntityId fileId)
    {
        constexpr uint64_t HALF = 32ULL;
        uint64_t uid = file->GetFilenameHash();
        uid <<= HALF;
        uid |= fileId.GetOffset();
        return uid;
    }

    UniqId GetUniqId() const
    {
        return CalcUniqId(GetPandaFile(), fileId_);
    }

    ~Field() = default;

    NO_COPY_SEMANTIC(Field);
    NO_MOVE_SEMANTIC(Field);

private:
    ClassHelper::ClassWordSize classWord_;
    panda_file::File::EntityId fileId_;
    uint32_t accessFlags_;
    uint32_t offset_ {0};
};

}  // namespace ark

#endif  // PANDA_RUNTIME_FIELD_H_
