/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef RUNTIME_MEM_ALLOCATOR_ADAPTER_H
#define RUNTIME_MEM_ALLOCATOR_ADAPTER_H

#include "runtime/include/mem/allocator.h"

namespace ark::mem {

template <typename T, AllocScope ALLOC_SCOPE_T>
class AllocatorAdapter;

template <AllocScope ALLOC_SCOPE_T>
class AllocatorAdapter<void, ALLOC_SCOPE_T> {
public:
    // Naming is not by code style because we need to have allocator traits compatibility. Don't change it.
    using value_type = void;             // NOLINT(readability-identifier-naming)
    using pointer = void *;              // NOLINT(readability-identifier-naming)
    using const_pointer = const void *;  // NOLINT(readability-identifier-naming)

    using propagate_on_container_move_assignment = std::true_type;  // NOLINT(readability-identifier-naming)

    template <typename U>
    struct Rebind {
        // NOLINTNEXTLINE(readability-identifier-naming)
        using other = AllocatorAdapter<U, ALLOC_SCOPE_T>;
    };

    template <typename U>
    using rebind = Rebind<U>;  // NOLINT(readability-identifier-naming)

    explicit AllocatorAdapter(Allocator *allocator = InternalAllocator<>::GetInternalAllocatorFromRuntime())
        : allocator_(allocator)
    {
        ASSERT(allocator_ != nullptr);
    }
    template <typename U>
    // NOLINTNEXTLINE(google-explicit-constructor)
    AllocatorAdapter(const AllocatorAdapter<U, ALLOC_SCOPE_T> &other) : allocator_(other.allocator_)
    {
    }
    AllocatorAdapter(const AllocatorAdapter &) noexcept = default;
    AllocatorAdapter &operator=(const AllocatorAdapter &) noexcept = default;

    AllocatorAdapter(AllocatorAdapter &&other) noexcept = default;
    AllocatorAdapter &operator=(AllocatorAdapter &&other) noexcept = default;

    ~AllocatorAdapter() = default;

private:
    Allocator *allocator_;

    template <typename U, AllocScope TYPE_T>
    friend class AllocatorAdapter;
};

template <typename T, AllocScope ALLOC_SCOPE_T = AllocScope::GLOBAL>
class AllocatorAdapter {
public:
    // Naming is not by code style because we need to have allocator traits compatibility. Don't change it.
    using value_type = T;               // NOLINT(readability-identifier-naming)
    using pointer = T *;                // NOLINT(readability-identifier-naming)
    using reference = T &;              // NOLINT(readability-identifier-naming)
    using const_pointer = const T *;    // NOLINT(readability-identifier-naming)
    using const_reference = const T &;  // NOLINT(readability-identifier-naming)
    using size_type = size_t;           // NOLINT(readability-identifier-naming)
    using difference_type = ptrdiff_t;  // NOLINT(readability-identifier-naming)

    using propagate_on_container_move_assignment = std::true_type;  // NOLINT(readability-identifier-naming)

    template <typename U>
    struct Rebind {
        // NOLINTNEXTLINE(readability-identifier-naming)
        using other = AllocatorAdapter<U, ALLOC_SCOPE_T>;
    };

    template <typename U>
    using rebind = Rebind<U>;  // NOLINT(readability-identifier-naming)

    explicit AllocatorAdapter(Allocator *allocator = InternalAllocator<>::GetInternalAllocatorFromRuntime())
        : allocator_(allocator)
    {
    }
    template <typename U>
    // NOLINTNEXTLINE(google-explicit-constructor)
    AllocatorAdapter(const AllocatorAdapter<U, ALLOC_SCOPE_T> &other) : allocator_(other.allocator_)
    {
    }
    AllocatorAdapter(const AllocatorAdapter &) noexcept = default;
    AllocatorAdapter &operator=(const AllocatorAdapter &) noexcept = default;

    AllocatorAdapter(AllocatorAdapter &&other) noexcept = default;
    AllocatorAdapter &operator=(AllocatorAdapter &&other) noexcept = default;

    ~AllocatorAdapter() = default;

    // NOLINTNEXTLINE(readability-identifier-naming)
    pointer allocate(size_type size, [[maybe_unused]] const void *hint = nullptr)
    {
        // NOLINTNEXTLINE(bugprone-suspicious-semicolon, readability-braces-around-statements)
        if constexpr (ALLOC_SCOPE_T == AllocScope::GLOBAL) {
            return allocator_->AllocArray<T>(size);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            return allocator_->AllocArrayLocal<T>(size);
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    void deallocate(pointer ptr, [[maybe_unused]] size_type size)
    {
        allocator_->Free(ptr);
    }

    template <typename U, typename... Args>
    void construct(U *ptr, Args &&...args)  // NOLINT(readability-identifier-naming)
    {
        ::new (static_cast<void *>(ptr)) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U *ptr)  // NOLINT(readability-identifier-naming)
    {
        ptr->~U();
    }

    template <typename U>
    bool operator==(const AllocatorAdapter<U> &other) const
    {
        return this->allocator_ == other.allocator_;
    }

    template <typename U>
    bool operator!=(const AllocatorAdapter<U> &other) const
    {
        return this->allocator_ != other.allocator_;
    }

private:
    Allocator *allocator_;

    template <typename U, AllocScope TYPE_T>
    friend class AllocatorAdapter;
};

template <AllocScope ALLOC_SCOPE_T>
inline AllocatorAdapter<void, ALLOC_SCOPE_T> Allocator::Adapter()
{
    return AllocatorAdapter<void, ALLOC_SCOPE_T>(this);
}

}  // namespace ark::mem

#endif  // RUNTIME_MEM_ALLOCATOR_ADAPTER_H
