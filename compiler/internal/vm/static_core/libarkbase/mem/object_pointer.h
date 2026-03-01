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

#ifndef PANDA_LIBPANDABASE_MEM_OBJECT_POINTER_H
#define PANDA_LIBPANDABASE_MEM_OBJECT_POINTER_H

#include "mem.h"

namespace ark {

/**
 * @class ObjectPointer<class Object>
 *
 * @brief This class is wrapper for object types.
 *
 * Wraps pointer Object * into class ObjectPointer<Object> and provides interfaces to work with it as a pointer.
 * This is needed to use object pointers of size 32 bits in 64-bit architectures.
 */
template <class Object>
class ObjectPointer final {
public:
    ObjectPointer() = default;
    // NOLINTNEXTLINE(google-explicit-constructor)
    ObjectPointer(Object *object) : object_(ToObjPtrType(object))
    {
        ASSERT(IsAddressInObjectsHeapOrNull(ToUintPtr(object)));
    }
    // NOLINTNEXTLINE(google-explicit-constructor)
    ObjectPointer(std::nullptr_t aNullptr) noexcept : object_(ToObjPtrType(aNullptr)) {}

    DEFAULT_COPY_SEMANTIC(ObjectPointer);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(ObjectPointer);

    ObjectPointer &operator=(std::nullptr_t aNullptr)
    {
        object_ = ToObjPtrType(aNullptr);
        return *this;
    }

    ObjectPointer &operator=(const Object *object)
    {
        ASSERT(IsAddressInObjectsHeapOrNull(ToUintPtr(object)));
        object_ = ToObjPtrType(object);
        return *this;
    }

    ALWAYS_INLINE Object *operator->()
    {
        ASSERT(object_ != 0);
        return ToObjectPtr(object_);
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    ALWAYS_INLINE operator Object *() const
    {
        return ToObjectPtr(object_);
    }

    ALWAYS_INLINE Object &operator*()
    {
        ASSERT(object_ != 0);
        return *ToObjectPtr(object_);
    }

    ALWAYS_INLINE bool operator==(const ObjectPointer &other) const noexcept
    {
        return object_ == other.object_;
    }

    ALWAYS_INLINE bool operator!=(const ObjectPointer &other) const noexcept
    {
        return object_ != other.object_;
    }

    ALWAYS_INLINE bool operator==(Object *other) const noexcept
    {
        return ToObjectPtr(object_) == other;
    }

    ALWAYS_INLINE bool operator!=(Object *other) const noexcept
    {
        return ToObjectPtr(object_) != other;
    }

    ALWAYS_INLINE bool operator==(std::nullptr_t) const noexcept
    {
        return ToObjectPtr(object_) == nullptr;
    }

    ALWAYS_INLINE bool operator!=(std::nullptr_t) const noexcept
    {
        return ToObjectPtr(object_) != nullptr;
    }

    ALWAYS_INLINE Object &operator[](size_t index)
    {
        return ToObjectPtr(object_)[index];
    }

    ALWAYS_INLINE const Object &operator[](size_t index) const
    {
        return ToObjectPtr(object_)[index];
    }

    template <class U>
    ALWAYS_INLINE U ReinterpretCast() const noexcept
    {
        return reinterpret_cast<U>(ToObjectPtr(object_));
    }

    ALWAYS_INLINE ObjectPointerType &GetPointer()
    {
        return object_;
    }

    ~ObjectPointer() = default;

private:
    ObjectPointerType object_ {};

    ALWAYS_INLINE static Object *ToObjectPtr(const ObjectPointerType pointer) noexcept
    {
        return ToNativePtr<Object>(static_cast<uintptr_t>(pointer));
    }
};

// size of ObjectPointer<T> must be equal size of ObjectPointerType
static_assert(sizeof(ObjectPointer<bool>) == sizeof(ObjectPointerType));

}  // namespace ark

#endif  // PANDA_LIBPANDABASE_MEM_OBJECT_POINTER_H
