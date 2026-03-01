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
#ifndef RUNTIME_INCLUDE_TAIHE_OPTIONAL_HPP_
#define RUNTIME_INCLUDE_TAIHE_OPTIONAL_HPP_
// NOLINTBEGIN

#include <taihe/optional.abi.h>
#include <taihe/common.hpp>

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

namespace taihe {
template <typename cpp_owner_t>
struct optional_view;

template <typename cpp_owner_t>
struct optional;

template <typename cpp_owner_t>
struct optional_view {
    optional_view(cpp_owner_t *handle) noexcept : m_handle(handle) {}  // main constructor

    optional_view() noexcept : optional_view(nullptr) {}

    optional_view(std::nullopt_t) : optional_view(nullptr) {}

    explicit operator bool() const
    {
        return m_handle;
    }

    bool has_value() const
    {
        return m_handle;
    }

    cpp_owner_t const *operator->() const
    {
        return m_handle;
    }

    cpp_owner_t const &operator*() const
    {
        return *m_handle;
    }

    cpp_owner_t const &value() const
    {
        return *m_handle;
    }

    cpp_owner_t value_or(cpp_owner_t &&default_value) const
    {
        if (m_handle) {
            return *m_handle;
        } else {
            return std::move(default_value);
        }
    }

    friend bool operator==(optional_view const &lhs, optional_view const &rhs)
    {
        return (!lhs && !rhs) || (lhs && rhs && *lhs == *rhs);
    }

    friend bool operator!=(optional_view const &lhs, optional_view const &rhs)
    {
        return !(lhs == rhs);
    }

protected:
    cpp_owner_t *m_handle;
};

template <typename cpp_owner_t>
struct optional : public optional_view<cpp_owner_t> {
    explicit optional(cpp_owner_t *handle) noexcept : optional_view<cpp_owner_t>(handle) {}  // main constructor

    optional() noexcept : optional(nullptr) {}

    optional(std::nullopt_t) : optional(nullptr) {}

    template <typename... Args>
    explicit optional(std::in_place_t, Args &&...args) : optional(new cpp_owner_t(std::forward<Args>(args)...))
    {
    }

    template <typename... Args>
    static optional make(Args &&...args)
    {
        return optional(std::in_place, std::forward<Args>(args)...);
    }

    template <typename... Args>
    cpp_owner_t &emplace(Args &&...args)
    {
        if (this->m_handle) {
            delete this->m_handle;
        }
        this->m_handle = new cpp_owner_t(std::forward<Args>(args)...);
        return *this->m_handle;
    }

    void reset()
    {
        if (this->m_handle) {
            delete this->m_handle;
            this->m_handle = nullptr;
        }
    }

    optional(optional_view<cpp_owner_t> const &other) : optional(other ? new cpp_owner_t(*other) : nullptr) {}

    optional(optional<cpp_owner_t> const &other) : optional(other ? new cpp_owner_t(*other) : nullptr) {}

    optional(optional<cpp_owner_t> &&other) : optional(std::exchange(other.m_handle, nullptr)) {}

    optional &operator=(optional other)
    {
        std::swap(this->m_handle, other.m_handle);
        return *this;
    }

    ~optional()
    {
        if (this->m_handle) {
            delete this->m_handle;
        }
    }
};

template <typename cpp_owner_t>
struct as_abi<optional_view<cpp_owner_t>> {
    using type = struct TOptional;
};

template <typename cpp_owner_t>
struct as_abi<optional<cpp_owner_t>> {
    using type = struct TOptional;
};

template <typename cpp_owner_t>
struct as_param<optional<cpp_owner_t>> {
    using type = optional_view<cpp_owner_t>;
};
}  // namespace taihe

template <typename cpp_owner_t>
struct std::hash<taihe::optional<cpp_owner_t>> {
    std::size_t operator()(taihe::optional_view<cpp_owner_t> val) const
    {
        return val ? std::hash<cpp_owner_t>()(*val) + 0x9e3779b9 : 0;
    }
};
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_OPTIONAL_HPP_