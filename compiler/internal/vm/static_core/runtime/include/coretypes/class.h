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
#ifndef PANDA_RUNTIME_CORETYPES_CLASS_H_
#define PANDA_RUNTIME_CORETYPES_CLASS_H_

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "libarkbase/utils/utf.h"
#include "runtime/include/class.h"
#include "runtime/include/object_header.h"

namespace ark::coretypes {

class Class : public ObjectHeader {
public:
    Class(const uint8_t *descriptor, uint32_t vtableSize, uint32_t imtSize, uint32_t klassSize)
        : ObjectHeader(), klass_(descriptor, panda_file::SourceLang::PANDA_ASSEMBLY, vtableSize, imtSize, klassSize)
    {
    }

    // We shouldn't init header_ here - because it has been memset(0) in object allocation,
    // otherwise it may cause data race while visiting object's class concurrently in gc.
    void InitClass(const uint8_t *descriptor, uint32_t vtableSize, uint32_t imtSize, uint32_t klassSize)
    {
        new (&klass_) ark::Class(descriptor, panda_file::SourceLang::PANDA_ASSEMBLY, vtableSize, imtSize, klassSize);
    }

    ark::Class *GetRuntimeClass()
    {
        return &klass_;
    }

    const ark::Class *GetRuntimeClass() const
    {
        return &klass_;
    }

    template <class T>
    T GetFieldPrimitive(const Field &field) const
    {
        return klass_.GetFieldPrimitive<T>(field);
    }

    template <class T>
    void SetFieldPrimitive(const Field &field, T value)
    {
        klass_.SetFieldPrimitive(field, value);
    }

    template <bool NEED_READ_BARRIER = true>
    ObjectHeader *GetFieldObject(const Field &field) const
    {
        return klass_.GetFieldObject<NEED_READ_BARRIER>(field);
    }

    template <bool NEED_WRITE_BARRIER = true>
    void SetFieldObject(const Field &field, ObjectHeader *value)
    {
        klass_.SetFieldObject<NEED_WRITE_BARRIER>(field, value);
    }

    static size_t GetSize(uint32_t klassSize)
    {
        return GetRuntimeClassOffset() + klassSize;
    }

    static constexpr size_t GetRuntimeClassOffset()
    {
        return MEMBER_OFFSET(Class, klass_);
    }

    static Class *FromRuntimeClass(ark::Class *klass)
    {
        return reinterpret_cast<Class *>(reinterpret_cast<uintptr_t>(klass) - GetRuntimeClassOffset());
    }

private:
    ark::Class klass_;
};

// Klass field has variable size so it must be last
static_assert(Class::GetRuntimeClassOffset() + sizeof(ark::Class) == sizeof(Class));

}  // namespace ark::coretypes

#endif  // PANDA_RUNTIME_CORETYPES_CLASS_H_
