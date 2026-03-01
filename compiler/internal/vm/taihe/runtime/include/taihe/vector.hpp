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
#ifndef RUNTIME_INCLUDE_TAIHE_VECTOR_HPP_
#define RUNTIME_INCLUDE_TAIHE_VECTOR_HPP_
// NOLINTBEGIN

#include <taihe/vector.abi.h>
#include <taihe/common.hpp>

#include <algorithm>
#include <utility>

#define VEC_GROWTH_FACTOR 2
#define VEC_DEFAULT_CAPACITY 0

namespace taihe {
template <typename T>
struct vector_view;

template <typename T>
struct vector;

template <typename T>
struct vector_view {
public:
    using value_type = T;
    using size_type = std::size_t;
    using reference = T &;
    using pointer = T *;

    void reserve(std::size_t cap) const
    {
        if (cap < m_handle->len) {
            return;
        }
        m_handle->cap = cap;
        m_handle->buffer = reinterpret_cast<T *>(realloc(m_handle->buffer, sizeof(T) * cap));
    }

    std::size_t size() const noexcept
    {
        return m_handle->len;
    }

    bool empty() const noexcept
    {
        return m_handle->len == 0;
    }

    std::size_t capacity() const noexcept
    {
        return m_handle->cap;
    }

    template <typename... Args>
    T &emplace_back(Args &&...args) const
    {
        std::size_t required_cap = m_handle->len + 1;
        if (required_cap > m_handle->cap) {
            this->reserve(std::max(required_cap, m_handle->cap * VEC_GROWTH_FACTOR));
        }
        T &item = m_handle->buffer[m_handle->len];
        new (&item) T {std::forward<Args>(args)...};
        ++m_handle->len;
        return item;
    }

    T &push_back(T &&value) const
    {
        return emplace_back(std::move(value));
    }

    T &push_back(T const &value) const
    {
        return emplace_back(value);
    }

    T &operator[](std::size_t index) const
    {
        return m_handle->buffer[index];
    }

    void pop_back() const
    {
        if (m_handle->len == 0) {
            return;
        }
        std::destroy_at(&m_handle->buffer[m_handle->len]);
        --m_handle->len;
    }

    void clear() const noexcept
    {
        for (std::size_t i = 0; i < m_handle->len; i++) {
            std::destroy_at(&m_handle->buffer[i]);
        }
        m_handle->len = 0;
    }

    using iterator = T *;
    using const_iterator = T const *;

    iterator begin() const
    {
        return m_handle->buffer;
    }

    iterator end() const
    {
        return m_handle->buffer + m_handle->len;
    }

    const_iterator cbegin() const
    {
        return begin();
    }

    const_iterator cend() const
    {
        return end();
    }

protected:
    struct data_t {
        TRefCount count;
        std::size_t cap;
        T *buffer;
        std::size_t len;
    } *m_handle;

    explicit vector_view(data_t *handle) : m_handle(handle) {}

    friend struct vector<T>;

    friend struct std::hash<vector<T>>;

    friend bool operator==(vector_view lhs, vector_view rhs)
    {
        return lhs.m_handle == rhs.m_handle;
    }
};

template <typename T>
struct vector : vector_view<T> {
    using typename vector_view<T>::data_t;
    using vector_view<T>::m_handle;

    explicit vector(std::size_t cap = VEC_DEFAULT_CAPACITY) : vector(new data_t)
    {
        tref_init(&m_handle->count, 1);
        m_handle->cap = cap;
        m_handle->buffer = reinterpret_cast<T *>(malloc(sizeof(T) * cap));
        m_handle->len = 0;
    }

    vector(vector<T> &&other) noexcept : vector(other.m_handle)
    {
        other.m_handle = nullptr;
    }

    vector(vector<T> const &other) : vector(other.m_handle)
    {
        if (m_handle) {
            tref_inc(&m_handle->count);
        }
    }

    vector(vector_view<T> const &other) : vector(other.m_handle)
    {
        if (m_handle) {
            tref_inc(&m_handle->count);
        }
    }

    vector &operator=(vector other)
    {
        std::swap(this->m_handle, other.m_handle);
        return *this;
    }

    ~vector()
    {
        if (m_handle && tref_dec(&m_handle->count)) {
            this->clear();
            free(m_handle->buffer);
            delete m_handle;
        }
    }

private:
    explicit vector(data_t *handle) : vector_view<T>(handle) {}
};

template <typename T>
struct as_abi<vector<T>> {
    using type = TVector;
};

template <typename T>
struct as_abi<vector_view<T>> {
    using type = TVector;
};

template <typename T>
struct as_param<vector<T>> {
    using type = vector_view<T>;
};
}  // namespace taihe

template <typename T>
struct std::hash<taihe::vector<T>> {
    std::size_t operator()(taihe::vector_view<T> val) const noexcept
    {
        return reinterpret_cast<std::size_t>(val.m_handle);
    }
};

#undef VEC_GROWTH_FACTOR
#undef VEC_DEFAULT_CAPACITY
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_VECTOR_HPP_