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
#ifndef RUNTIME_MEM_OBJECT_LOCAL_OBJECT_HANDLE_H
#define RUNTIME_MEM_OBJECT_LOCAL_OBJECT_HANDLE_H

#include "libarkbase/macros.h"
#include "runtime/include/managed_thread.h"

namespace ark {

template <typename T>
class LocalObjectHandle {
public:
    NO_COPY_SEMANTIC(LocalObjectHandle);
    NO_MOVE_SEMANTIC(LocalObjectHandle);

    explicit LocalObjectHandle(ManagedThread *thread, ObjectHeader *object) : root_(object), thread_(thread)
    {
        thread_->PushLocalObject(&root_);
    }

    // Disabled when T is ObjectHeader, delayed instantiation
    template <typename P = T, typename = std::enable_if_t<!std::is_same_v<P, ObjectHeader>>>
    explicit LocalObjectHandle(ManagedThread *thread, T *object)
        : LocalObjectHandle(thread, reinterpret_cast<ObjectHeader *>(object))
    {
    }

    ~LocalObjectHandle()
    {
        thread_->PopLocalObject();
    }

    T *GetPtr() const
    {
        return reinterpret_cast<T *>(root_);
    }

    explicit operator T *()
    {
        return GetPtr();
    }

    T *operator->()
    {
        return GetPtr();
    }

    inline uintptr_t GetAddress() const
    {
        return ToUintPtr(&root_);
    }

private:
    ObjectHeader *root_ {};
    ManagedThread *thread_ {};
};

}  // namespace ark

#endif  // RUNTIME_MEM_OBJECT_LOCAL_OBJECT_HANDLE_H
