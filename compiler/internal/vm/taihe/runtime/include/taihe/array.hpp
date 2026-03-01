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
#ifndef RUNTIME_INCLUDE_TAIHE_ARRAY_HPP_
#define RUNTIME_INCLUDE_TAIHE_ARRAY_HPP_
// NOLINTBEGIN

#include <taihe/array.abi.h>
#include <taihe/common.hpp>

#include <array>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace taihe {
template <typename cpp_owner_t>
struct array_view;

template <typename cpp_owner_t>
struct array;

template <typename cpp_owner_t>
struct array_view {
    using value_type = cpp_owner_t;
    using size_type = std::size_t;
    using reference = value_type &;
    using const_reference = value_type const &;
    using pointer = value_type *;
    using const_pointer = value_type const *;
    using iterator = value_type *;
    using const_iterator = value_type const *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    array_view(pointer data, size_type size) noexcept : m_size(size), m_data(data) {}  // main constructor

    array_view() noexcept : m_size(0), m_data(nullptr) {}

    template <typename C, size_type N>
    array_view(C (&value)[N]) noexcept : array_view(value, N)
    {
    }

    template <typename C>
    array_view(std::vector<C> &value) noexcept : array_view(value.data(), static_cast<size_type>(value.size()))
    {
    }

    template <typename C>
    array_view(std::vector<C> const &value) noexcept : array_view(value.data(), static_cast<size_type>(value.size()))
    {
    }

    template <typename C, std::size_t N>
    array_view(std::array<C, N> &value) noexcept : array_view(value.data(), static_cast<size_type>(value.size()))
    {
    }

    template <typename C, std::size_t N>
    array_view(std::array<C, N> const &value) noexcept : array_view(value.data(), static_cast<size_type>(value.size()))
    {
    }

    template <typename C>
    array_view(array_view<C> const &other) noexcept : array_view(other.data(), other.size())
    {
    }

    template <typename C>
    array_view(array<C> const &other) noexcept : array_view(other.data(), other.size())
    {
    }

    reference operator[](size_type const pos) const noexcept
    {
        TH_ASSERT(pos < size(), "Pos should be less than array's size");
        return m_data[pos];
    }

    reference at(size_type const pos) const
    {
        if (size() <= pos) {
            TH_THROW(std::out_of_range, "Invalid array subscript");
        }
        return m_data[pos];
    }

    reference front() const noexcept
    {
        TH_ASSERT(m_size > 0, "Array's size should be greater than 0");
        return *m_data;
    }

    reference back() const noexcept
    {
        TH_ASSERT(m_size > 0, "Array's size should be greater than 0");
        return m_data[m_size - 1];
    }

    pointer data() const noexcept
    {
        return m_data;
    }

    bool empty() const noexcept
    {
        return m_size == 0;
    }

    size_type size() const noexcept
    {
        return m_size;
    }

    iterator begin() const noexcept
    {
        return m_data;
    }

    iterator end() const noexcept
    {
        return m_data + m_size;
    }

    reverse_iterator rbegin() const noexcept
    {
        return m_data + m_size;
    }

    reverse_iterator rend() const noexcept
    {
        return m_data;
    }

    const_iterator cbegin() const noexcept
    {
        return m_data;
    }

    const_iterator cend() const noexcept
    {
        return m_data + m_size;
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return m_data + m_size;
    }

    const_reverse_iterator crend() const noexcept
    {
        return m_data;
    }

    friend bool operator==(array_view left, array_view right) noexcept
    {
        return std::equal(left.begin(), left.end(), right.begin(), right.end());
    }

    friend bool operator!=(array_view left, array_view right) noexcept
    {
        return !(left == right);
    }

    friend bool operator<(array_view left, array_view right) noexcept
    {
        return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
    }

    friend bool operator>(array_view left, array_view right) noexcept
    {
        return right < left;
    }

    friend bool operator<=(array_view left, array_view right) noexcept
    {
        return !(right < left);
    }

    friend bool operator>=(array_view left, array_view right) noexcept
    {
        return !(left < right);
    }

protected:
    std::size_t m_size;
    cpp_owner_t *m_data;
};

struct copy_data_t {};

constexpr inline copy_data_t copy_data;

struct move_data_t {};

constexpr inline move_data_t move_data;

template <typename cpp_owner_t>
struct array : public array_view<cpp_owner_t> {
    using typename array_view<cpp_owner_t>::pointer;
    using typename array_view<cpp_owner_t>::size_type;

    explicit array(pointer data, size_type size) noexcept : array_view<cpp_owner_t>(data, size) {}  // main constructor

    template <typename InputIt>
    array(copy_data_t, InputIt begin, size_type size) noexcept
        : array_view<cpp_owner_t>(reinterpret_cast<cpp_owner_t *>(malloc(size * sizeof(cpp_owner_t))), size)
    {
        std::uninitialized_copy_n(begin, size, this->m_data);
    }

    template <typename InputIt>
    array(move_data_t, InputIt begin, size_type size) noexcept
        : array_view<cpp_owner_t>(reinterpret_cast<cpp_owner_t *>(malloc(size * sizeof(cpp_owner_t))), size)
    {
        std::uninitialized_move_n(begin, size, this->m_data);
    }

    explicit array(size_type size) : array(reinterpret_cast<cpp_owner_t *>(malloc(size * sizeof(cpp_owner_t))), size)
    {
        std::uninitialized_default_construct_n(this->m_data, size);
    }

    explicit array(size_type size, cpp_owner_t const &value)
        : array(reinterpret_cast<cpp_owner_t *>(malloc(size * sizeof(cpp_owner_t))), size)
    {
        std::uninitialized_fill_n(this->m_data, size, value);
    }

    static array make(size_type size)
    {
        return array(size);
    }

    static array make(size_type size, cpp_owner_t const &value)
    {
        return array(size, value);
    }

    array(std::initializer_list<cpp_owner_t> value) noexcept : array(copy_data, value.begin(), value.size()) {}

    array(array_view<cpp_owner_t> const &other) : array(copy_data, other.data(), other.size()) {}

    array(array<cpp_owner_t> const &other) : array(copy_data, other.data(), other.size()) {}

    array(array<cpp_owner_t> &&other) : array(std::exchange(other.m_data, nullptr), std::exchange(other.m_size, 0)) {}

    array &operator=(array other)
    {
        std::swap(this->m_size, other.m_size);
        std::swap(this->m_data, other.m_data);
        return *this;
    }

    ~array()
    {
        if (this->m_data) {
            std::destroy_n(this->m_data, this->m_size);
            free(this->m_data);
            this->m_size = 0;
            this->m_data = nullptr;
        }
    }
};

template <typename cpp_owner_t>
struct as_abi<array_view<cpp_owner_t>> {
    using type = TArray;
};

template <typename cpp_owner_t>
struct as_abi<array<cpp_owner_t>> {
    using type = TArray;
};

template <typename cpp_owner_t>
struct as_param<array<cpp_owner_t>> {
    using type = array_view<cpp_owner_t>;
};
}  // namespace taihe

template <typename cpp_owner_t>
struct std::hash<taihe::array<cpp_owner_t>> {
    std::size_t operator()(taihe::array_view<cpp_owner_t> val) const
    {
        std::size_t seed = 0;
        static constexpr std::size_t GOLDEN_RATIO_CONSTANT = 0x9e3779b9;
        static constexpr std::size_t LEFT_SHIFT_BITS = 6;
        static constexpr std::size_t RIGHT_SHIFT_BITS = 2;
        for (std::size_t i = 0; i < val.size(); i++) {
            seed ^= (seed << LEFT_SHIFT_BITS) + (seed >> RIGHT_SHIFT_BITS) + GOLDEN_RATIO_CONSTANT +
                    std::hash<cpp_owner_t>()(val[i]);
        }
        return seed;
    }
};
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_ARRAY_HPP_