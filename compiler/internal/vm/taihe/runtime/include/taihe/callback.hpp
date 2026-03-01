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
#ifndef RUNTIME_INCLUDE_TAIHE_CALLBACK_HPP_
#define RUNTIME_INCLUDE_TAIHE_CALLBACK_HPP_
// NOLINTBEGIN

#include <taihe/callback.abi.h>
#include <taihe/common.hpp>
#include <taihe/object.hpp>

#include <type_traits>

namespace taihe {
template <typename Signature>
struct callback_view;

template <typename Signature>
struct callback;

template <typename Return, typename... Params>
struct callback_view<Return(Params...)> {
    static constexpr bool is_holder = false;

    struct abi_type;

    using vtable_type = as_abi_t<Return>(abi_type, as_abi_t<Params>...);
    using view_type = callback_view<Return(Params...)>;
    using holder_type = callback<Return(Params...)>;

    struct abi_type {
        vtable_type *vtbl_ptr;
        DataBlockHead *data_ptr;
    } m_handle;

    explicit callback_view(abi_type handle) : m_handle(handle) {}

    operator data_view() const &
    {
        return data_view(this->m_handle.data_ptr);
    }

    operator data_holder() const &
    {
        return data_holder(tobj_dup(this->m_handle.data_ptr));
    }

public:
    bool is_error() const &
    {
        return m_handle.vtbl_ptr == nullptr;
    }

    Return operator()(Params... params) const &
    {
        if constexpr (std::is_void_v<Return>) {
            return m_handle.vtbl_ptr(m_handle, into_abi<Params>(params)...);
        } else {
            return from_abi<Return>(m_handle.vtbl_ptr(m_handle, into_abi<Params>(params)...));
        }
    }

public:
    template <typename Impl>
    static as_abi_t<Return> vtbl_impl(abi_type handle, as_abi_t<Params>... params)
    {
        if constexpr (std::is_void_v<Return>) {
            return cast_data_ptr<Impl>(handle.data_ptr)->operator()(from_abi<Params>(params)...);
        } else {
            return into_abi<Return>(cast_data_ptr<Impl>(handle.data_ptr)->operator()(from_abi<Params>(params)...));
        }
    };

    template <typename Impl>
    static constexpr struct IdMapItem idmap_impl[0] = {};
};

template <typename Return, typename... Params>
struct callback<Return(Params...)> : callback_view<Return(Params...)> {
    static constexpr bool is_holder = true;

    using typename callback_view<Return(Params...)>::abi_type;

    explicit callback(abi_type handle) : callback_view<Return(Params...)>(handle) {}

    ~callback()
    {
        tobj_drop(this->m_handle.data_ptr);
    }

    callback &operator=(callback other)
    {
        std::swap(this->m_handle, other.m_handle);
        return *this;
    }

    callback(callback<Return(Params...)> &&other)
        : callback({
              other.m_handle.vtbl_ptr,
              std::exchange(other.m_handle.data_ptr, nullptr),
          })
    {
    }

    callback(callback<Return(Params...)> const &other)
        : callback({
              other.m_handle.vtbl_ptr,
              tobj_dup(other.m_handle.data_ptr),
          })
    {
    }

    callback(callback_view<Return(Params...)> const &other)
        : callback({
              other.m_handle.vtbl_ptr,
              tobj_dup(other.m_handle.data_ptr),
          })
    {
    }

    operator data_view() const &
    {
        return data_view(this->m_handle.data_ptr);
    }

    operator data_holder() const &
    {
        return data_holder(tobj_dup(this->m_handle.data_ptr));
    }

    operator data_holder() &&
    {
        return data_holder(std::exchange(this->m_handle.data_ptr, nullptr));
    }
};

template <typename Return, typename... Params>
struct as_abi<callback_view<Return(Params...)>> {
    using type = TCallback;
};

template <typename Return, typename... Params>
struct as_abi<callback<Return(Params...)>> {
    using type = TCallback;
};

template <typename Return, typename... Params>
struct as_param<callback<Return(Params...)>> {
    using type = callback_view<Return(Params...)>;
};

template <typename Return, typename... Params>
inline bool operator==(callback_view<Return(Params...)> lhs, callback_view<Return(Params...)> rhs)
{
    return data_view(lhs) == data_view(rhs);
}
}  // namespace taihe

template <typename Return, typename... Params>
struct std::hash<taihe::callback<Return(Params...)>> {
    std::size_t operator()(taihe::callback_view<Return(Params...)> val) const noexcept
    {
        return std::hash<taihe::data_holder>()(val);
    }
};
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_CALLBACK_HPP_