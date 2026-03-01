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

#ifndef LIBPANDABASE_UTILS_RING_BUFFER_H
#define LIBPANDABASE_UTILS_RING_BUFFER_H

#include <array>
#include <limits>

#include "macros.h"

namespace panda {
template <class T, size_t N>
class RingBuffer;

template <class T, size_t N, bool is_const = false>
class RingBufferIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::conditional_t<is_const, const T, T>;
    using difference_type = ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    constexpr RingBufferIterator() noexcept = default;
    constexpr RingBufferIterator(const RingBufferIterator &other) noexcept = default;
    constexpr RingBufferIterator(RingBufferIterator &&other) noexcept = default;

    constexpr RingBufferIterator &operator=(const RingBufferIterator &other) = default;
    constexpr RingBufferIterator &operator=(RingBufferIterator &&other) = default;

    ~RingBufferIterator() = default;

    constexpr RingBufferIterator &operator++() noexcept
    {
        increment_index();
        return *this;
    }

    constexpr RingBufferIterator operator++(int) noexcept  // NOLINT(cert-dcl21-cpp)
    {
        auto tmp = RingBufferIterator(*this);
        increment_index();
        return tmp;
    }

    constexpr RingBufferIterator &operator--() noexcept
    {
        decrement_index();
        return *this;
    }

    constexpr RingBufferIterator operator--(int) noexcept  // NOLINT(cert-dcl21-cpp)
    {
        auto tmp = RingBufferIterator(*this);
        decrement_index();
        return tmp;
    }

    constexpr bool operator==(const RingBufferIterator &other) const noexcept
    {
        return index_ == other.index_;
    }

    constexpr bool operator!=(const RingBufferIterator &other) const noexcept
    {
        return index_ != other.index_;
    }

    constexpr reference operator*() const noexcept
    {
        return buffer_->operator[](index_);
    }

    constexpr pointer operator->() const noexcept
    {
        return static_cast<pointer>(&buffer_->operator[](index_));
    }

private:
    using array_ptr = std::conditional_t<is_const, const std::array<T, N + 1> *, std::array<T, N + 1> *>;

    constexpr RingBufferIterator(array_ptr buffer, size_t index) : buffer_(buffer), index_(index) {}

    constexpr void increment_index() noexcept
    {
        index_ = (index_ == N) ? 0U : index_ + 1;
    }

    constexpr void decrement_index() noexcept
    {
        index_ = (index_ == 0U) ? N : index_ - 1;
    }

    friend class RingBuffer<T, N>;

    array_ptr buffer_ = nullptr;
    size_t index_ = 0U;
};

/**
 * Static circular buffer in STL-style
 * @tparam T type of values in buffer
 * @tparam N maximum count of elements in buffer
 */
template <class T, size_t N>
class RingBuffer {
public:
    static_assert(N > 0U, "0 is invalid size for ring buffer");

    using value_type = T;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = size_t;

    using iterator = RingBufferIterator<value_type, N>;
    using const_iterator = RingBufferIterator<value_type, N, true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr RingBuffer() = default;

    constexpr RingBuffer(const RingBuffer &other) = default;
    constexpr RingBuffer(RingBuffer &&other) = default;
    constexpr RingBuffer &operator=(const RingBuffer &other) = default;
    constexpr RingBuffer &operator=(RingBuffer &&other) = default;

    ~RingBuffer() = default;

    /**
     * Appends the given element value to the end of the ring buffer
     * @param value the value of the element to append
     */
    constexpr void push_back(const value_type &value)  // NOLINT(readability-identifier-naming)
    {
        increment_tail();
        buffer_[tail_] = value;
    }

    /**
     * Moves the given element value to the end of the ring buffer
     * @param value the value of the element to append
     */
    constexpr void push_back(value_type &&value)  // NOLINT(readability-identifier-naming)
    {
        emplace_back(std::move(value));
    }

    /**
     * Appends a new element to the end of the ring buffer
     *
     * @tparam Args arguments types for elemnt construction
     * @param args arguments to forward to the constructor of the element
     * @return a reference to the inserted element
     */
    template <class... Args>
    constexpr reference emplace_back(Args... args)  // NOLINT(readability-identifier-naming)
    {
        increment_tail();
        buffer_[tail_] = value_type(std::forward<Args>(args)...);
        return buffer_[tail_];
    }

    /**
     * Appends the given element value to the begin of the ring buffer
     * @param value the value of the element to append
     */
    constexpr void push_front(const value_type &value)  // NOLINT(readability-identifier-naming)
    {
        decrement_head();
        buffer_[head_] = value;
    }

    /**
     * Moves the given element value to the begin of the ring buffer
     * @param value the value of the element to append
     */
    constexpr void push_front(value_type &&value)  // NOLINT(readability-identifier-naming)
    {
        emplace_front(std::move(value));
    }

    /**
     * Appends a new element to the begin of the ring buffer
     *
     * @tparam Args arguments types for elemnt construction
     * @param args arguments to forward to the constructor of the element
     * @return a reference to the inserted element
     */
    template <class... Args>
    constexpr reference emplace_front(Args... args)  // NOLINT(readability-identifier-naming)
    {
        decrement_head();
        buffer_[head_] = value_type(std::forward<Args>(args)...);
        return buffer_[head_];
    }

    /**
     * Removes the last element of the ring buffer
     */
    constexpr void pop_back()  // NOLINT(readability-identifier-naming)
    {
        if constexpr (std::is_class_v<T>) {  // NOLINT(readability-braces-around-statements)
            buffer_[tail_].~value_type();
        }
        decrement_buffer_index(tail_);
        --current_size_;
    }

    /**
     * Removes the first element of the ring buffer
     */
    constexpr void pop_front()  // NOLINT(readability-identifier-naming)
    {
        if constexpr (std::is_class_v<T>) {  // NOLINT(readability-braces-around-statements)
            buffer_[head_].~value_type();
        }
        increment_buffer_index(head_);
        --current_size_;
    }

    /**
     * @return iterator to the first element
     */
    constexpr iterator begin() noexcept  // NOLINT(readability-identifier-naming)
    {
        return iterator(&buffer_, head_);
    }

    /**
     * @return a reverse iterator to the first element of the revesred ring buffer
     */
    constexpr reverse_iterator rbegin() noexcept  // NOLINT(readability-identifier-naming)
    {
        return std::make_reverse_iterator(end());
    }

    /**
     * @return const context iterator to the first element
     */
    constexpr const_iterator begin() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return const_iterator(&buffer_, head_);
    }

    /**
     * @return a const context reverse iterator to the first element of the reversed ring buffer
     */
    constexpr const_reverse_iterator rbegin() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return std::make_reverse_iterator(end());
    }

    /**
     * @return const iterator to the first element
     */
    constexpr const_iterator cbegin() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return begin();
    }

    /**
     * @return a const reverse iterator to the first element of the reversed ring buffer
     */
    constexpr const_reverse_iterator crbegin() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return rbegin();
    }

    /**
     * @return iterator to the element following the last element
     */
    constexpr iterator end() noexcept  // NOLINT(readability-identifier-naming)
    {
        auto tmp = tail_;
        increment_buffer_index(tmp);
        return iterator(&buffer_, tmp);
    }

    /**
     * @return a reverse iterator to the element following the last element of the reversed ring buffer
     */
    constexpr reverse_iterator rend() noexcept  // NOLINT(readability-identifier-naming)
    {
        return std::make_reverse_iterator(begin());
    }

    /**
     * @return const context iterator to the element following the last element
     */
    constexpr const_iterator end() const noexcept  // NOLINT(readability-identifier-naming)
    {
        auto tmp = tail_;
        increment_buffer_index(tmp);
        return const_iterator(&buffer_, tmp);
    }

    /**
     * @return a const context iterator to the element following the last element of the reversed ring buffer
     */
    constexpr const_reverse_iterator rend() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return std::make_reverse_iterator(begin());
    }

    /**
     * @return const iterator to the element following the last element
     */
    constexpr const_iterator cend() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return end();
    }

    /**
     * @return const iterator to the element following the last element of the reversed ring buffer
     */
    constexpr const_reverse_iterator crend() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return rend();
    }

    /**
     * @return reference to first element in ring buffer
     */
    constexpr reference front() noexcept  // NOLINT(readability-identifier-naming)
    {
        return buffer_[head_];
    }

    /**
     * @return const reference to first element in ring buffer
     */
    constexpr const_reference front() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return buffer_[head_];
    }

    /**
     * @return reference to last element in ring buffer
     */
    constexpr reference back() noexcept  // NOLINT(readability-identifier-naming)
    {
        return buffer_[tail_];
    }

    /**
     * @return const reference to last element in ring buffer
     */
    constexpr const_reference back() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return buffer_[tail_];
    }

    constexpr reference operator[](size_type index) noexcept
    {
        return buffer_[(head_ + index) % buffer_.size()];
    }

    constexpr const_reference operator[](size_type index) const noexcept
    {
        return buffer_[(head_ + index) % buffer_.size()];
    }

    /**
     * @return true if buffer is empty and false otherwise
     */
    [[nodiscard]] constexpr bool empty() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return size() == 0;
    }

    /**
     * @return true if buffer is full (all buffer space is used)
     */
    [[nodiscard]] constexpr bool full() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return size() == capacity();
    }

    /**
     * @return current ring buffer size
     */
    constexpr size_type size() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return current_size_;
    }

    /**
     * @return maximum ring buffer size
     */
    constexpr size_type capacity() const noexcept  // NOLINT(readability-identifier-naming)
    {
        return N;
    }

    /**
     * Erases all elements from the ring buffer. After this call, size() returns zero.
     */
    constexpr void clear() noexcept  // NOLINT(readability-identifier-naming)
    {
        if constexpr (std::is_class_v<T>) {  // NOLINT(readability-braces-around-statements)
            for (auto &element : *this) {
                element.~value_type();
            }
        }
        head_ = 0;
        tail_ = capacity();
        current_size_ = 0;
    }

private:
    constexpr void increment_tail() noexcept
    {
        increment_buffer_index(tail_);
        if (full()) {
            increment_buffer_index(head_);
        } else {
            ++current_size_;
        }
    }

    constexpr void decrement_head() noexcept
    {
        decrement_buffer_index(head_);
        if (full()) {
            decrement_buffer_index(tail_);
        } else {
            ++current_size_;
        }
    }

    constexpr void increment_buffer_index(size_type &index) const noexcept
    {
        index = (index + 1) % buffer_.size();
    }

    constexpr void decrement_buffer_index(size_type &index) const noexcept
    {
        index = index == 0U ? N : index - 1;
    }

    std::array<T, N + 1> buffer_ = {};
    size_type head_ = 0U;
    size_type tail_ = N;
    size_type current_size_ = 0U;
};
}  // namespace panda

#endif  // LIBPANDABASE_UTILS_RING_BUFFER_H
