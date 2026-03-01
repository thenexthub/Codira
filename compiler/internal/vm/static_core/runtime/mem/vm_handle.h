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
#ifndef RUNTIME_MEM_OBJECT_VMHANDLE_H
#define RUNTIME_MEM_OBJECT_VMHANDLE_H

#include "libarkbase/macros.h"
#include "runtime/handle_base.h"
#include "runtime/include/managed_thread.h"
#include "runtime/handle_scope.h"

namespace ark {

using TaggedType = coretypes::TaggedType;
using TaggedValue = coretypes::TaggedValue;

template <typename T>
class LocalObjectHandle;

// VMHandle should be used in language-agnostic part of runtime
template <typename T>
class VMHandle : public HandleBase {
public:
    inline explicit VMHandle() : HandleBase(reinterpret_cast<uintptr_t>(nullptr)) {}

    explicit VMHandle(ManagedThread *thread, ObjectHeader *object)
    {
        if (object != nullptr) {
            ASSERT(thread != nullptr);
            auto scope = thread->GetTopScope<ObjectHeader *>();
            ASSERT(scope != nullptr);
            address_ = scope->NewHandle(object);
        } else {
            address_ = reinterpret_cast<uintptr_t>(nullptr);
        }
    }

    template <typename P, typename = std::enable_if_t<std::is_convertible_v<P *, T *>>>
    inline explicit VMHandle(const VMHandle<P> &other) : HandleBase(other.GetAddress())
    {
    }

    template <typename P, typename = std::enable_if_t<std::is_convertible_v<P *, T *>>>
    inline explicit VMHandle(const LocalObjectHandle<P> &other);

    inline explicit VMHandle(mem::GlobalObjectStorage *globalStorage, mem::Reference *reference);

    ~VMHandle() = default;

    DEFAULT_COPY_SEMANTIC(VMHandle);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(VMHandle);

    T *GetPtr() const
    {
        if (address_ == reinterpret_cast<uintptr_t>(nullptr)) {
            return nullptr;
        }
        return *(reinterpret_cast<T **>(GetAddress()));
    }

    explicit operator T *()
    {
        return GetPtr();
    }

    T *operator->()
    {
        T *ptr = GetPtr();
        ASSERT(ptr != nullptr);
        return ptr;
    }

    const T *operator->() const
    {
        T *ptr = GetPtr();
        ASSERT(ptr != nullptr);
        return ptr;
    }
};

template <typename T>
class VMMutableHandle : public VMHandle<T> {
public:
    VMMutableHandle(ManagedThread *thread, ObjectHeader *object)
    {
        scope_ = thread->GetTopScope<ObjectHeader *>();
        ASSERT(scope_ != nullptr);
        if (object != nullptr) {
            this->address_ = scope_->NewHandle(object);
        } else {
            this->address_ = reinterpret_cast<uintptr_t>(nullptr);
        }
    }
    ~VMMutableHandle() = default;

    void Update(ObjectHeader *object)
    {
        ASSERT(object != nullptr);
        if (this->address_ == reinterpret_cast<uintptr_t>(nullptr)) {
            this->address_ = scope_->NewHandle(object);
        } else {
            *reinterpret_cast<ObjectHeader **>(this->address_) = object;
        }
    }

    NO_COPY_SEMANTIC(VMMutableHandle);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(VMMutableHandle);

private:
    HandleScope<ObjectHeader *> *scope_ {nullptr};
};
}  // namespace ark

#endif  // RUNTIME_MEM_OBJECT_VMHANDLE_H
