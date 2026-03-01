/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_TYPED_POINTER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_TYPED_POINTER_H_

#include "libarkbase/macros.h"

namespace ark::ets::interop::js::ets_proxy {

template <typename U, typename R>
struct TypedPointer {
    TypedPointer() : TypedPointer(static_cast<U *>(nullptr)) {}

    explicit TypedPointer(U *ptr)
    {
        static_assert(alignof(U) >= (1U << 1U));
        SetTagged(ptr, true);
        ASSERT(!IsResolved());
    }
    explicit TypedPointer(R *ptr)
    {
        static_assert(alignof(R) >= (1U << 1U));
        SetTagged(ptr, false);
        ASSERT(IsResolved());
    }

    bool IsResolved() const
    {
        return !GetTag();
    }

    U *GetUnresolved() const
    {
        ASSERT(!IsResolved());
        return reinterpret_cast<U *>(GetUPtr());
    }

    R *GetResolved() const
    {
        ASSERT(IsResolved());
        return reinterpret_cast<R *>(ptr_);
    }

    template <typename T>
    void Set(T *ptr)
    {
        *this = TypedPointer(ptr);
    }

    DEFAULT_COPY_SEMANTIC(TypedPointer);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(TypedPointer);
    ~TypedPointer() = default;

private:
    uintptr_t static constexpr MASK = ~static_cast<uintptr_t>(1);

    bool GetTag() const
    {
        return static_cast<bool>(ptr_ & ~MASK);
    }

    uintptr_t GetUPtr() const
    {
        return ptr_ & MASK;
    }

    template <typename T>
    void SetTagged(T *ptr, bool tag)
    {
        ptr_ = reinterpret_cast<uintptr_t>(ptr);
        ASSERT(!GetTag());
        ptr_ |= static_cast<uintptr_t>(tag);
    }

    uintptr_t ptr_ {};
};

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_TYPED_POINTER_H_
