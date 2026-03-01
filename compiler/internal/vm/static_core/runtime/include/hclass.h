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
#ifndef PANDA_RUNTIME_HCLASS_H_
#define PANDA_RUNTIME_HCLASS_H_

#include "libarkbase/mem/mem.h"
#include "mem/vm_handle.h"
#include "runtime/include/class.h"

namespace ark {

namespace coretypes {
class DynClass;
}  // namespace coretypes

// Class for objects in DYNAMIC_CLASS languages like JavaScript
class HClass : public BaseClass {
public:
    static constexpr uint32_t HCLASS = DYNAMIC_CLASS << 1U;
    static constexpr uint32_t STRING = HCLASS << 1U;
    static constexpr uint32_t ARRAY = STRING << 1U;
    static constexpr uint32_t BIGINT = ARRAY << 1U;
    static constexpr uint32_t NATIVE_POINTER = BIGINT << 1U;
    static constexpr uint32_t IS_DICTIONARY_ARRAY = NATIVE_POINTER << 1U;
    static constexpr uint32_t IS_BUILTINS_CTOR = IS_DICTIONARY_ARRAY << 1U;
    static constexpr uint32_t IS_CALLABLE = IS_BUILTINS_CTOR << 1U;
    static constexpr uint32_t IS_FREE_OBJECT = IS_CALLABLE << 1U;

public:
    HClass(uint32_t flags, panda_file::SourceLang lang) : BaseClass(lang)
    {
        SetFlags(flags | BaseClass::DYNAMIC_CLASS);
    }

    inline bool IsFreeObject() const
    {
        return (GetFlags() & IS_FREE_OBJECT) != 0;
    }

    inline bool IsNativePointer() const
    {
        return (GetFlags() & NATIVE_POINTER) != 0;
    }

    inline bool IsArray() const
    {
        return (GetFlags() & ARRAY) != 0;
    }

    inline bool IsString() const
    {
        return (GetFlags() & STRING) != 0;
    }

    inline bool IsHClass() const
    {
        return (GetFlags() & HCLASS) != 0;
    }

    bool IsDictionary() const
    {
        return (BaseClass::GetFlags() & IS_DICTIONARY_ARRAY) != 0U;
    }

    bool IsBuiltinsConstructor() const
    {
        return (BaseClass::GetFlags() & IS_BUILTINS_CTOR) != 0U;
    }

    bool IsCallable() const
    {
        return (BaseClass::GetFlags() & IS_CALLABLE) != 0U;
    }

    void MarkFieldAsNative(size_t offset)
    {
        ASSERT(offset <= MaxNativeFieldOffset());
        nativeFields_ |= FieldOffsetToMask(offset);
    }

    bool IsNativeField(size_t offset) const
    {
        if (offset > MaxNativeFieldOffset()) {
            return false;
        }

        return (nativeFields_ & FieldOffsetToMask(offset)) != 0;
    }

    uint64_t GetNativeFieldMask() const
    {
        return nativeFields_;
    }

    void SetNativeFieldMask(uint64_t mask)
    {
        nativeFields_ = mask;
    }

    static constexpr uint64_t FieldOffsetToMask(size_t offset)
    {
        uint64_t index = (offset - ObjectHeader::ObjectHeaderSize()) / TaggedValue::TaggedTypeSize();
        return 1U << index;
    }

    static constexpr uint32_t GetCallableMask()
    {
        return IS_CALLABLE;
    }

    static constexpr size_t GetDataOffset()
    {
        return MEMBER_OFFSET(HClass, data_);
    }

    ~HClass() = default;

    DEFAULT_COPY_SEMANTIC(HClass);
    DEFAULT_MOVE_SEMANTIC(HClass);

protected:
    void SetFlags(uint32_t flags)
    {
        ASSERT(flags & BaseClass::DYNAMIC_CLASS);
        BaseClass::SetFlags(flags);
    }

private:
    static size_t MaxNativeFieldOffset()
    {
        size_t maxIndex = std::numeric_limits<decltype(nativeFields_)>::digits - 1;
        return ObjectHeader::ObjectHeaderSize() + maxIndex * TaggedValue::TaggedTypeSize();
    }

    friend class coretypes::DynClass;

    uint64_t nativeFields_ {0};

    // Data for language extension flags
    // NOTE(maksenov): maybe merge this with BaseClass flags
    FIELD_UNUSED uint64_t data_ {0};
};

}  // namespace ark

#endif  // PANDA_RUNTIME_HCLASS_H_
