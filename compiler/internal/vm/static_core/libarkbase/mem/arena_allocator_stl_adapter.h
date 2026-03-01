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

#ifndef PANDA_RUNTIME_MEM_STL_ADAPTER_H
#define PANDA_RUNTIME_MEM_STL_ADAPTER_H

#include "arena_allocator.h"

namespace ark {

// Adapter for use of ArenaAllocator in STL containers.
template <typename T, bool USE_OOM_HANDLER>
class ArenaAllocatorAdapter;

template <bool USE_OOM_HANDLER>
class ArenaAllocatorAdapter<void, USE_OOM_HANDLER> {
public:
    // Naming is not by code style because we need to have allocator traits compatibility. Don't change it.
    using value_type = void;             // NOLINT(readability-identifier-naming)
    using pointer = void *;              // NOLINT(readability-identifier-naming)
    using const_pointer = const void *;  // NOLINT(readability-identifier-naming)

    template <typename U>
    struct Rebind {
        // NOLINTNEXTLINE(readability-identifier-naming)
        using other = ArenaAllocatorAdapter<U, USE_OOM_HANDLER>;
    };

    template <typename U>
    using rebind = Rebind<U>;  // NOLINT(readability-identifier-naming)

    explicit ArenaAllocatorAdapter(ArenaAllocatorT<USE_OOM_HANDLER> *allocator) : allocator_(allocator) {}
    template <typename U>
    // NOLINTNEXTLINE(google-explicit-constructor)
    ArenaAllocatorAdapter(const ArenaAllocatorAdapter<U, USE_OOM_HANDLER> &other) : allocator_(other.allocator_)
    {
    }
    ArenaAllocatorAdapter(const ArenaAllocatorAdapter &) = default;
    ArenaAllocatorAdapter &operator=(const ArenaAllocatorAdapter &) = default;
    ArenaAllocatorAdapter(ArenaAllocatorAdapter &&) = default;
    ArenaAllocatorAdapter &operator=(ArenaAllocatorAdapter &&) = default;
    ~ArenaAllocatorAdapter() = default;

private:
    ArenaAllocatorT<USE_OOM_HANDLER> *allocator_;

    template <typename U, bool USE_OOM_HANDLE>
    friend class ArenaAllocatorAdapter;
};

template <typename T, bool USE_OOM_HANDLER = false>
class ArenaAllocatorAdapter {
public:
    // Naming is not by code style because we need to have allocator traits compatibility. Don't change it.
    using value_type = T;               // NOLINT(readability-identifier-naming)
    using pointer = T *;                // NOLINT(readability-identifier-naming)
    using reference = T &;              // NOLINT(readability-identifier-naming)
    using const_pointer = const T *;    // NOLINT(readability-identifier-naming)
    using const_reference = const T &;  // NOLINT(readability-identifier-naming)
    using size_type = size_t;           // NOLINT(readability-identifier-naming)
    using difference_type = ptrdiff_t;  // NOLINT(readability-identifier-naming)

    template <typename U>
    struct Rebind {
        // NOLINTNEXTLINE(readability-identifier-naming)
        using other = ArenaAllocatorAdapter<U, USE_OOM_HANDLER>;
    };

    template <typename U>
    using rebind = Rebind<U>;  // NOLINT(readability-identifier-naming)

    explicit ArenaAllocatorAdapter(ArenaAllocatorT<USE_OOM_HANDLER> *allocator) : allocator_(allocator) {}
    template <typename U>
    // NOLINTNEXTLINE(google-explicit-constructor)
    ArenaAllocatorAdapter(const ArenaAllocatorAdapter<U, USE_OOM_HANDLER> &other) : allocator_(other.allocator_)
    {
    }
    ArenaAllocatorAdapter(const ArenaAllocatorAdapter &) = default;
    ArenaAllocatorAdapter &operator=(const ArenaAllocatorAdapter &) = default;
    ArenaAllocatorAdapter(ArenaAllocatorAdapter &&other) noexcept
    {
        ASSERT(other.allocator_ != nullptr);
        allocator_ = other.allocator_;
        other.allocator_ = nullptr;
    }
    ArenaAllocatorAdapter &operator=(ArenaAllocatorAdapter &&other) noexcept
    {
        ASSERT(other.allocator_ != nullptr);
        allocator_ = other.allocator_;
        other.allocator_ = nullptr;
        return *this;
    }
    ~ArenaAllocatorAdapter() = default;

    // NOLINTNEXTLINE(readability-identifier-naming)
    size_type max_size() const
    {
        return static_cast<size_type>(-1) / sizeof(T);
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    pointer address(reference x) const
    {
        return &x;
    }
    // NOLINTNEXTLINE(readability-identifier-naming)
    const_reference address(const_reference x) const
    {
        return &x;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    pointer allocate(size_type n,
                     [[maybe_unused]] typename ArenaAllocatorAdapter<void, USE_OOM_HANDLER>::pointer ptr = nullptr)
    {
        ASSERT(n <= max_size());
        ASSERT(allocator_ != nullptr);
        return allocator_->template AllocArray<T>(n);
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    void deallocate([[maybe_unused]] pointer p, [[maybe_unused]] size_type n) {}

    template <typename U, typename... Args>
    void construct(U *p, Args &&...args)  // NOLINT(readability-identifier-naming)
    {
        ASSERT(p != nullptr);
        ::new (static_cast<void *>(p)) U(std::forward<Args>(args)...);
    }
    template <typename U>
    void destroy(U *p)  // NOLINT(readability-identifier-naming)
    {
        p->~U();
    }

private:
    ArenaAllocatorT<USE_OOM_HANDLER> *allocator_ {nullptr};

    template <typename U, bool USE_OOM_HANDLE>
    friend class ArenaAllocatorAdapter;

    template <typename U, bool USE_OOM_HANDLE>
    // NOLINTNEXTLINE(readability-redundant-declaration)
    friend inline bool operator==(const ArenaAllocatorAdapter<U, USE_OOM_HANDLE> &lhs,
                                  const ArenaAllocatorAdapter<U, USE_OOM_HANDLE> &rhs);
};

template <typename T, bool USE_OOM_HANDLE>
inline bool operator==(const ArenaAllocatorAdapter<T, USE_OOM_HANDLE> &lhs,
                       const ArenaAllocatorAdapter<T, USE_OOM_HANDLE> &rhs)
{
    return lhs.allocator_ == rhs.allocator_;
}

template <typename T, bool USE_OOM_HANDLE>
inline bool operator!=(const ArenaAllocatorAdapter<T, USE_OOM_HANDLE> &lhs,
                       const ArenaAllocatorAdapter<T, USE_OOM_HANDLE> &rhs)
{
    return !(lhs == rhs);
}

template <bool USE_OOM_HANDLER>
inline ArenaAllocatorAdapter<void, USE_OOM_HANDLER> ArenaAllocatorT<USE_OOM_HANDLER>::Adapter()
{
    return ArenaAllocatorAdapter<void, USE_OOM_HANDLER>(this);
}

}  // namespace ark

#endif  // PANDA_RUNTIME_MEM_STL_ADAPTER_H
