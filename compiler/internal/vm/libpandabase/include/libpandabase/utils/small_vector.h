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

#ifndef LIBPANDABASE_UTILS_SMALL_VECTOR_H
#define LIBPANDABASE_UTILS_SMALL_VECTOR_H

#include "utils/arch.h"
#include <algorithm>
#include <array>
#include <vector>
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace panda {

/**
 * Helper class that provides main Panda's allocator interface: AdapterType, Adapter(), GetInstance().
 */
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

template <typename Allocator, typename T, bool use_allocator>
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
template <typename T, size_t N, typename Allocator = std::allocator<T>, bool use_allocator = false>
class SmallVector {
    // Least bit of the pointer should not be used in a memory addressing, because we pack `allocated` flag there
    static_assert(alignof(Allocator *) > 1);
    // When N is zero, then consider using std::vector directly
    static_assert(N != 0);

    struct StaticBuffer {
        uint32_t size {0};
        std::array<T, N> data;
    };

    using VectorType = std::vector<T, typename AllocatorConfig<Allocator, T, use_allocator>::Adapter>;

public:
    using value_type = typename VectorType::value_type;
    using reference = typename VectorType::reference;
    using const_reference = typename VectorType::const_reference;
    using pointer = typename VectorType::pointer;
    using const_pointer = typename VectorType::const_pointer;
    using difference_type = typename VectorType::difference_type;

    template <typename IteratorType, bool reverse>
    class Iterator
        : public std::iterator<std::random_access_iterator_tag, IteratorType, int32_t, IteratorType *, IteratorType &> {
        IteratorType *Add(difference_type v)
        {
            if constexpr (reverse) {  // NOLINT
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return pointer_ -= v;
            } else {  // NOLINT
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return pointer_ += v;
            }
        }
        IteratorType *Sub(difference_type v)
        {
            if constexpr (reverse) {  // NOLINT
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return pointer_ + v;
            } else {  // NOLINT
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return pointer_ - v;
            }
        }

    public:
        explicit Iterator(IteratorType *param_pointer) : pointer_(param_pointer) {}

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
            if constexpr (reverse) {  // NOLINT
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

    using iterator = Iterator<T, false>;
    using const_iterator = Iterator<const T, false>;
    using reverse_iterator = Iterator<T, true>;
    using const_reverse_iterator = Iterator<const T, true>;

    SmallVector()
    {
        static_assert(!use_allocator);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        buffer_.size = 0;
    }

    SmallVector(std::initializer_list<T> list)
    {
        static_assert(!use_allocator);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        buffer_.size = 0;

        for (auto it = list.begin(); it != list.end(); ++it) {
            push_back(*it);
        }
    }

    explicit SmallVector(Allocator *allocator) : allocator_(AddStaticFlag(allocator))
    {
        static_assert(use_allocator);
        ASSERT(allocator != nullptr);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        buffer_.size = 0;
    }

    SmallVector(const SmallVector &other) : allocator_(other.allocator_)
    {
        if (other.IsStatic()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            buffer_.size = other.buffer_.size;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            for (uint32_t i = 0; i < buffer_.size; ++i) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&buffer_.data[i]) T(other.buffer_.data[i]);
            }
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&vector_) VectorType(other.vector_);
        }
    }

    SmallVector(SmallVector &&other) noexcept : allocator_(other.allocator_)
    {
        if (other.IsStatic()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            buffer_.size = other.buffer_.size;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            for (uint32_t i = 0; i < buffer_.size; ++i) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&buffer_.data[i]) T(std::move(other.buffer_.data[i]));
            }
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&vector_) VectorType(std::move(other.vector_));
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
            buffer_.size = other.buffer_.size;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            for (uint32_t i = 0; i < buffer_.size; ++i) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&buffer_.data[i]) T(other.buffer_.data[i]);
            }
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&vector_) VectorType(other.vector_);
        }

        return *this;
    }

    SmallVector &operator=(SmallVector &&other) noexcept
    {
        Destroy();
        allocator_ = other.allocator_;
        if (other.IsStatic()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            buffer_.size = other.buffer_.size;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            for (uint32_t i = 0; i < buffer_.size; ++i) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&buffer_.data[i]) T(std::move(other.buffer_.data[i]));
            }
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&vector_) VectorType(std::move(other.vector_));
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
        return IsStatic() ? buffer_.data.data() : vector_.data();
    }

    pointer data()
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer_.data.data() : vector_.data();
    }

    size_t size() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer_.size : vector_.size();
    }

    size_t capacity() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer_.data.size() : vector_.capacity();
    }

    void push_back(const value_type &value)
    {
        if (!EnsureStaticSpace(1)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            vector_.push_back(value);
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&buffer_.data[buffer_.size++]) T(value);
        }
    }

    void push_back(value_type &&value)
    {
        if (!EnsureStaticSpace(1)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            vector_.push_back(std::move(value));
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&buffer_.data[buffer_.size++]) T(std::move(value));
        }
    }

    template <typename... _Args>
    reference emplace_back(_Args &&... values)
    {
        if (!EnsureStaticSpace(1)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            return vector_.emplace_back(std::move(values)...);
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return *(new (&buffer_.data[buffer_.size++]) T(std::move(values)...));
    }

    void reserve(size_t size)
    {
        if (size > capacity()) {
            if (IsStatic()) {
                ASSERT(size > N);
                MoveToVector(size);
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                vector_.reserve(size);
            }
        }
    }

    void resize(size_t size)
    {
        if (size <= this->size()) {
            if (IsStatic()) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                std::for_each(buffer_.data.begin() + size, buffer_.data.begin() + buffer_.size, [](T &v) { v.~T(); });
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                buffer_.size = size;
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                vector_.resize(size);
            }
        } else {
            if (EnsureStaticSpace(size - this->size())) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                std::for_each(buffer_.data.begin() + buffer_.size, buffer_.data.begin() + size,
                              [](T &v) { new (&v) T; });
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                buffer_.size = size;
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                vector_.resize(size);
            }
        }
    }

    void resize(size_t size, const value_type &val)
    {
        if (size <= this->size()) {
            if (IsStatic()) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                std::for_each(buffer_.data.begin() + size, buffer_.data.begin() + buffer_.size, [](T &v) { v.~T(); });
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                buffer_.size = size;
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                vector_.resize(size, val);
            }
        } else {
            if (EnsureStaticSpace(size - this->size())) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                std::for_each(buffer_.data.begin() + buffer_.size, buffer_.data.begin() + size,
                              [&val](T &v) { new (&v) T(val); });
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                buffer_.size = size;
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                vector_.resize(size, val);
            }
        }
    }

    void clear()
    {
        if (IsStatic()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            std::for_each(buffer_.data.begin(), buffer_.data.begin() + buffer_.size, [](T &v) { v.~T(); });
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            buffer_.size = 0;
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            vector_.clear();
        }
    }

    reference back()
    {
        ASSERT(size() > 0);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer_.data[buffer_.size - 1] : vector_.back();
    }

    reference operator[](size_t i)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer_.data[i] : vector_[i];
    }
    const_reference operator[](size_t i) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return IsStatic() ? buffer_.data[i] : vector_[i];
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
        size_t size_required = buffer_.size + size;
        if (size_required > N) {
            MoveToVector(size_required);
            return false;
        }
        return true;
    }

    void MoveStaticBufferData(VectorType &tmp_vector)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        for (uint32_t i = 0; i < buffer_.size; ++i) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            tmp_vector.emplace_back(std::move(buffer_.data[i]));
        }
    }

    void MoveToVector(size_t reserved_size)
    {
        ASSERT(IsStatic());
        allocator_ = reinterpret_cast<Allocator *>(bit_cast<uintptr_t>(allocator_) & ~1LLU);
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (use_allocator) {
            ASSERT(allocator_ != nullptr);
            VectorType tmp_vector(allocator_->Adapter());
            tmp_vector.reserve(reserved_size);
            MoveStaticBufferData(tmp_vector);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&vector_) VectorType(std::move(tmp_vector));
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            VectorType tmp_vector;
            tmp_vector.reserve(reserved_size);
            MoveStaticBufferData(tmp_vector);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            new (&vector_) VectorType(std::move(tmp_vector));
        }
    }

    void Destroy()
    {
        if (IsStatic()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            std::for_each(buffer_.data.begin(), buffer_.data.begin() + buffer_.size, [](T &v) { v.~T(); });
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            vector_.~VectorType();
        }
    }

    void ResetToStatic()
    {
        allocator_ = AddStaticFlag(allocator_);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        buffer_.size = 0;
    }

    static Allocator *AddStaticFlag(Allocator *p)
    {
        return reinterpret_cast<Allocator *>((bit_cast<uintptr_t>(p) | 1U));
    }

private:
    union {
        StaticBuffer buffer_;
        VectorType vector_;
    };
    Allocator *allocator_ {AddStaticFlag(nullptr)};
};

}  // namespace panda

#endif  // LIBPANDABASE_UTILS_SMALL_VECTOR_H
