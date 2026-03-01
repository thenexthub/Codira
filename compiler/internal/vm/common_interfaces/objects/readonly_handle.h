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

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-macro-usage,
//             cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//             readability-else-after-return, readability-duplicate-include,
//             misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//             google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//             modernize-use-auto, llvm-namespace-comment,
//             cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//             readability-implicit-bool-conversion)

#ifndef COMMON_INTERFACES_OBJECTS_READONLY_HANDLE_H
#define COMMON_INTERFACES_OBJECTS_READONLY_HANDLE_H

#include "objects/base_object.h"
namespace common {
template <typename T>
class ReadOnlyHandle {
public:
    inline explicit ReadOnlyHandle(uintptr_t slot) : address_(slot)
    {
        DCHECK_CC(slot != 0);
        T::Cast(*reinterpret_cast<BaseObject **>(slot));
    }
    inline ReadOnlyHandle() : address_(reinterpret_cast<uintptr_t>(nullptr)) {}
    ~ReadOnlyHandle() = default;
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC_CC(ReadOnlyHandle);
    DEFAULT_COPY_SEMANTIC_CC(ReadOnlyHandle);

    uintptr_t GetAddress() const
    {
        return address_;
    }

    template <typename S>
    explicit ReadOnlyHandle(const ReadOnlyHandle<S> &handle) : address_(handle.GetAddress())
    {
        T::Cast(handle.GetBaseObject());
    }

    template <typename S>
    static ReadOnlyHandle<T> Cast(const ReadOnlyHandle<S> &handle)
    {
        T::Cast(handle.GetBaseObject());
        return ReadOnlyHandle<T>(handle.GetAddress());
    }

    BaseObject* GetBaseObject() const
    {
        if (GetAddress() == 0U) {
            return nullptr;
        }
        // temporarily add ReadBarrier for JSHandle
        return *reinterpret_cast<BaseObject **>(GetAddress());
    }

    T *operator*() const
    {
        return T::Cast(GetBaseObject());
    }

    T *operator->() const
    {
        return T::Cast(GetBaseObject());
    }

    bool operator==(const ReadOnlyHandle<T> &other) const
    {
        return GetBaseObject() == other.GetBaseObject();
    }

    bool operator!=(const ReadOnlyHandle<T> &other) const
    {
        return GetBaseObject() != other.GetBaseObject();
    }

    bool IsEmpty() const
    {
        return GetAddress() == 0U;
    }

    template <typename R>
    R *GetObject() const
    {
        return reinterpret_cast<R *>(GetBaseObject());
    }
private:
    uintptr_t address_;
};
}

#endif //COMMON_INTERFACES_OBJECTS_READONLY_HANDLE_H
// NOLINTEND(readability-identifier-naming, cppcoreguidelines-macro-usage,
//           cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//           readability-else-after-return, readability-duplicate-include,
//           misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//           google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//           modernize-use-auto, llvm-namespace-comment,
//           cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//           readability-implicit-bool-conversion)

