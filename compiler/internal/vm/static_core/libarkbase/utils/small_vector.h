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

#ifndef PANDA_SMALL_VECTOR_H
#define PANDA_SMALL_VECTOR_H

#include "libarkbase/utils/arch.h"
#include <algorithm>
#include <array>
#include <vector>

namespace ark {

/// Helper class that provides main Panda's allocator interface: AdapterType, Adapter(), GetInstance().
class StdAllocatorStub {
public:
    template <typename T>
    using AdapterType = std::allocator<T>;

    auto Adapter()
    {
        // We can't std::allocator<void> because it is removed in C++20
        return std::allocator<uint8_t>();
    }

    static StdAllocatorStub *GetInstance()
    {
        alignas(StdAllocatorStub *) static StdAllocatorStub instance;
        return &instance;
    }
};

template <typename Allocator, typename T, bool USE_ALLOCATOR>
class AllocatorConfig {
public:
    using Adapter = typename Allocator::template AdapterType<T>;
};

template <typename Allocator, typename T>
class AllocatorConfig<Allocator, T, false> {
public:
    using Adapter = Allocator;
};

/**
 * SmallVector stores `N` elements statically inside its static buffer. Static buffer shares memory with a std::vector
 * that will be created once number of elements exceed size of the static buffer - `N`.
 *
 * @tparam T Type of elements to store
 * @tparam N Number of elements to be stored statically
 * @tparam Allocator Allocator that will be used to allocate memory for the dynamic storage
 * @tparam use_allocator indicates type of Allocator:
 * false - Allocator is adapter(e.g.AllocatorAdapter) for memory allocate/deallocate/construct/destry...
 * true - Allocator is allocator(e.g.StdAllocatorStub) that implements Adapter() returning a adapter instance
 */
template <typename T, size_t N, typename Allocator = std::allocator<T>, bool USE_ALLOCATOR = false>
class SmallVector {
    // Least bit of the pointer should not be used in a memory addressing, because we pack `allocated` flag there
    static_assert(alignof(Allocator *) > 1);
    // When N is zero, then consider using std::vector directly
    static_assert(N != 0);

    struct StaticBuffer {
        uint32_t size {0};
        std::array<T, N> data;
    };

    using VectorType = std::vector<T, typename AllocatorConfig<Allocator, T, USE_ALLOCATOR>::Adapter>;

public:
    // NOLINTBEGIN(readability-identifier-naming)
    using value_type = typename VectorType::value_type;
    using reference = typename VectorType::reference;
    using const_reference = typename VectorType::const_reference;
    using pointer = typename VectorType::pointer;
    using const_pointer = typename VectorType::const_pointer;
    using difference_type = typename VectorType::difference_type;
    // NOLINTEND(readability-identifier-naming)
    template <typename IteratorType, bool REVERSE>
    class Iterator {
        IteratorType *Add(difference_type v)
        {
            if constexpr (REVERSE) {  // NOLINT
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return pointer_ -= v;
            } else {  // NOLINT
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return pointer_ += v;
            }
        }
        IteratorType *Sub(difference_type v)
        {
            if constexpr (REVERSE) {  // NOLINT
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return pointer_ + v;
            } else {  // NOLINT
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return pointer_ - v;
            }
        }

    public:
        // NOLINTBEGIN(readability-identifier-naming)
        using iterator_category = std::random_access_iterator_tag;
        using value_type = IteratorType;
        using difference_type = int32_t;
        using pointer = value_type *;
        using reference = value_type &;
        // NOLINTEND(readability-identifier-naming)

        explicit Iterator(IteratorType *paramPointer) : pointer_(paramPointer) {}

        IteratorType *operator->()
        {
            return pointer_;
        }
        IteratorType &operator*()
        {
            return *pointer_;
        }
        Iterator &operator++()
        {
            pointer_ = Add(1);
            return *this;
        }
        Iterator operator++(int)  // NOLINT(cert-dcl21-cpp)
        {
            Iterator it(pointer_);
            pointer_ = Add(1);
            return it;
        }
        Iterator &operator--()
        {
            pointer_ = Sub(1);
            return *this;
        }
        Iterator operator--(int)  // NOLINT(cert-dcl21-cpp)
        {
            Iterator it(pointer_);
            pointer_ = Sub(1);
            return it;
        }
        Iterator &operator+=(difference_type n)
        {
            pointer_ = Add(n);
            return *this;
        }
        Iterator &operator-=(difference_type n)
        {
            pointer_ = Sub(n);
            return *this;
        }
        Iterator operator+(int32_t n) const
        {
            Iterator it(*this);
            it.pointer_ = it.Add(n);
            return it;
        }
        Iterator operator-(int32_t n) const
        {
            Iterator it(*this);
            it.pointer_ = it.Sub(n);
            return it;
        }
        difference_type operator-(const Iterator &rhs) const
        {
            if constexpr (REVERSE) {  // NOLINT
                return rhs.pointer_ - pointer_;
            }
            return pointer_ - rhs.pointer_;
        }
        bool operator==(const Iterator &rhs) const
        {
            return pointer_ == rhs.pointer_;
        }
        bool operator!=(const Iterator &rhs) const
        {
            return !(*this == rhs);
        }

        ~Iterator() = default;

        DEFAULT_COPY_SEMANTIC(Iterator);
        DEFAULT_NOEXCEPT_MOVE_SEMANTIC(Iterator);

    private:
        IteratorType *pointer_;
    };

    // NOLINTBEGIN(readability-identifier-naming)
    using iterator = Iterator<T, false>;
    using const_iterator = Iterator<const T, false>;
    using reverse_iterator = Iterator<T, true>;
    using const_reverse_iterator = Iterator<const T, true>;
    // NOLINTEND(readability-identifier-naming)

    SmallVector()
    {
        static_assert(!USE_ALLOCATOR);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        buffer.size = 0;
    }

    SmallVector(std::initializer_list<T> list)
    {
        static_assert(!USE_ALLOCATOR);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        buffer.size = 0;

        for (auto it = list.begin(); it != list.end(); ++it) {
            push_back(*it);
        }
    }

    explicit SmallVector(Allocator *allocator) : allocator_(AddStaticFlag(allocator))
    {
        static_assert(USE_ALLOCATOR);
        ASSERT(allocator != nullptr);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        buffer.size = 0;
    }

    SmallVector(const SmallVector &other) : allocator_(other.allocator_)
    {
        if (other.IsStatic()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            buffer.size = other.buffer.size;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            for (uint32_t i = 0; i < buffer.size; ++i) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&buffer.data[i]) T(other.buffer.data[i]);
            }
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&vector) VectorType(other.vector);
        }
    }

    SmallVector(SmallVector &&other) noexcept : allocator_(other.allocator_)
    {
        if (other.IsStatic()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            buffer.size = other.buffer.size;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            for (uint32_t i = 0; i < buffer.size; ++i) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&buffer.data[i]) T(std::move(other.buffer.data[i]));
            }
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&vector) VectorType(std::move(other.vector));
        }
        other.ResetToStatic();
    }

    virtual ~SmallVector()
    {
        Destroy();
    }

    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment, cert-oop54-cpp)
    SmallVector &operator=(const SmallVector &other)
    {
        if (&other == this) {
            return *this;
        }

        Destroy();
        allocator_ = other.allocator_;
        if (other.IsStatic()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            buffer.size = other.buffer.size;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            for (uint32_t i = 0; i < buffer.size; ++i) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&buffer.data[i]) T(other.buffer.data[i]);
            }
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&vector) VectorType(other.vector);
        }

        return *this;
    }

    SmallVector &operator=(SmallVector &&other) noexcept
    {
        Destroy();
        allocator_ = other.allocator_;
        if (other.IsStatic()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            buffer.size = other.buffer.size;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            for (uint32_t i = 0; i < buffer.size; ++i) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&buffer.data[i]) T(std::move(other.buffer.data[i]));
            }
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&vector) VectorType(std::move(other.vector));
        }
        other.ResetToStatic();
        return *this;
    }

    bool operator==(const SmallVector &other) const
    {
        if (this == &other) {
            return true;
        }

        if (size() != other.size()) {
            return false;
        }

        auto it1 = begin();
        auto it2 = other.begin();
        for (; it1 != end(); ++it1, ++it2) {
            if (*it1 != *it2) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const SmallVector &other) const
    {
        return !operator==(other);
    }

    // NOLINTBEGIN(readability-identifier-naming,cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const_iterator begin() const
    {
        return const_iterator(data());
    }
    iterator begin()
    {
        return iterator(data());
    }
    const_iterator cbegin() const
    {
        return const_iterator(data());
    }
    const_iterator end() const
    {
        return begin() + size();
    }
    iterator end()
    {
        return begin() + size();
    }
    const_iterator cend() const
    {
        return begin() + size();
    }

    auto rbegin() const
    {
        return const_reverse_iterator(data() + size() - 1);
    }
    auto rbegin()
    {
        return reverse_iterator(data() + size() - 1);
    }
    auto crbegin() const
    {
        return const_reverse_iterator(data() + size() - 1);
    }
    auto rend() const
    {
        return const_reverse_iterator(data() - 1);
    }
    auto rend()
    {
        return reverse_iterator(data() - 1);
    }
    auto crend() const
    {
        return const_reverse_iterator(data() - 1);
    }

    bool empty() const
    {
        return size() == 0;
    }

    const_pointer data() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer.data.data() : vector.data();
    }

    pointer data()
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer.data.data() : vector.data();
    }

    size_t size() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer.size : vector.size();
    }

    size_t capacity() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer.data.size() : vector.capacity();
    }

    void push_back(const value_type &value)
    {
        if (!EnsureStaticSpace(1)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            vector.push_back(value);
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&buffer.data[buffer.size++]) T(value);
        }
    }

    void push_back(value_type &&value)
    {
        if (!EnsureStaticSpace(1)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            vector.push_back(std::move(value));
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&buffer.data[buffer.size++]) T(std::move(value));
        }
    }

    template <typename... _Args>
    reference emplace_back(_Args &&...values)
    {
        if (!EnsureStaticSpace(1)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            return vector.emplace_back(std::move(values)...);
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return *(new (&buffer.data[buffer.size++]) T(std::move(values)...));
    }

    void reserve(size_t size)
    {
        if (size > capacity()) {
            if (IsStatic()) {
                ASSERT(size > N);
                MoveToVector(size);
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                vector.reserve(size);
            }
        }
    }

    void resize(size_t size)
    {
        if (size <= this->size()) {
            if (IsStatic()) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                std::for_each(buffer.data.begin() + size, buffer.data.begin() + buffer.size, [](T &v) { v.~T(); });
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                buffer.size = size;
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                vector.resize(size);
            }
        } else {
            if (EnsureStaticSpace(size - this->size())) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                std::for_each(buffer.data.begin() + buffer.size, buffer.data.begin() + size, [](T &v) { new (&v) T; });
#pragma GCC diagnostic pop
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                buffer.size = size;
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                vector.resize(size);
            }
        }
    }

    void resize(size_t size, const value_type &val)
    {
        if (size <= this->size()) {
            if (IsStatic()) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                std::for_each(buffer.data.begin() + size, buffer.data.begin() + buffer.size, [](T &v) { v.~T(); });
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                buffer.size = size;
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                vector.resize(size, val);
            }
        } else {
            if (EnsureStaticSpace(size - this->size())) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                std::for_each(buffer.data.begin() + buffer.size, buffer.data.begin() + size,
                              [&val](T &v) { new (&v) T(val); });
#pragma GCC diagnostic pop
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                buffer.size = size;
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                vector.resize(size, val);
            }
        }
    }

    void clear()
    {
        if (IsStatic()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            std::for_each(buffer.data.begin(), buffer.data.begin() + buffer.size, [](T &v) { v.~T(); });
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            buffer.size = 0;
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            vector.clear();
        }
    }

    iterator erase(iterator it)
    {
        ASSERT(size() > 0);
        ASSERT(it != end());
        if (!IsStatic()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            auto vectorIt = vector.erase(vector.begin() + (it - begin()));
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            return iterator(data() + (vectorIt - vector.begin()));
        }
        auto newIt = it;
        auto copyIt = it + 1;
        auto endIt = end();
        while (copyIt != endIt) {
            *it = *copyIt;
            it++;
            copyIt++;
        }
        it->~T();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        buffer.size--;
        return newIt;
    }

    reference front()
    {
        ASSERT(size() > 0);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer.data[0] : vector.front();
    }

    reference back()
    {
        ASSERT(size() > 0);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer.data[buffer.size - 1] : vector.back();
    }
    // NOLINTEND(readability-identifier-naming,cppcoreguidelines-pro-bounds-pointer-arithmetic)

    reference operator[](size_t i)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer.data[i] : vector[i];
    }
    const_reference operator[](size_t i) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer.data[i] : vector[i];
    }

    bool IsStatic() const
    {
        return (bit_cast<uintptr_t>(allocator_) & 1U) != 0;
    }

private:
    bool EnsureStaticSpace(size_t size)
    {
        if (!IsStatic()) {
            return false;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        size_t sizeRequired = buffer.size + size;
        if (sizeRequired > N) {
            MoveToVector(sizeRequired);
            return false;
        }
        return true;
    }

    void MoveStaticBufferData(VectorType &tmpVector)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        for (uint32_t i = 0; i < buffer.size; ++i) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            tmpVector.emplace_back(std::move(buffer.data[i]));
        }
    }

    void MoveToVector(size_t reservedSize)
    {
        ASSERT(IsStatic());
        allocator_ = reinterpret_cast<Allocator *>(bit_cast<uintptr_t>(allocator_) & ~1LLU);
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (USE_ALLOCATOR) {
            ASSERT(allocator_ != nullptr);
            VectorType tmpVector(allocator_->Adapter());
            tmpVector.reserve(reservedSize);
            MoveStaticBufferData(tmpVector);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&vector) VectorType(std::move(tmpVector));
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            VectorType tmpVector;
            tmpVector.reserve(reservedSize);
            MoveStaticBufferData(tmpVector);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&vector) VectorType(std::move(tmpVector));
        }
    }

    void Destroy()
    {
        if (IsStatic()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            std::for_each(buffer.data.begin(), buffer.data.begin() + buffer.size, [](T &v) { v.~T(); });
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            vector.~VectorType();
        }
    }

    void ResetToStatic()
    {
        allocator_ = AddStaticFlag(allocator_);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        buffer.size = 0;
    }

    static Allocator *AddStaticFlag(Allocator *p)
    {
        return reinterpret_cast<Allocator *>((bit_cast<uintptr_t>(p) | 1U));
    }

private:
    union {
        StaticBuffer buffer;
        VectorType vector;
    };
    Allocator *allocator_ {AddStaticFlag(nullptr)};
};

}  // namespace ark

#endif  // PANDA_SMALL_VECTOR_H
