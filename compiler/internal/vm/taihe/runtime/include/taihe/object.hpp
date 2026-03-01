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
#ifndef RUNTIME_INCLUDE_TAIHE_OBJECT_HPP_
#define RUNTIME_INCLUDE_TAIHE_OBJECT_HPP_
// NOLINTBEGIN

#include <taihe/object.abi.h>
#include <taihe/common.hpp>

#include <cstddef>
#include <type_traits>

// Raw Data Handler //

namespace taihe {
struct data_view;
struct data_holder;

struct data_view {
    DataBlockHead *data_ptr;

    explicit data_view(DataBlockHead *other_data_ptr) : data_ptr(other_data_ptr) {}
};

struct data_holder : public data_view {
    explicit data_holder(DataBlockHead *other_data_ptr) : data_view(other_data_ptr) {}

    data_holder &operator=(data_holder other)
    {
        std::swap(this->data_ptr, other.data_ptr);
        return *this;
    }

    ~data_holder()
    {
        tobj_drop(this->data_ptr);
    }

    data_holder(data_view const &other) : data_holder(tobj_dup(other.data_ptr)) {}

    data_holder(data_holder const &other) : data_holder(tobj_dup(other.data_ptr)) {}

    data_holder(data_holder &&other) : data_holder(other.data_ptr)
    {
        other.data_ptr = nullptr;
    }
};

inline bool operator==(data_view lhs, data_view rhs)
{
    return lhs.data_ptr->rtti_ptr->same_fptr(lhs.data_ptr, rhs.data_ptr);
}
}  // namespace taihe

template <>
struct std::hash<taihe::data_holder> {
    std::size_t operator()(taihe::data_view val) const
    {
        return val.data_ptr->rtti_ptr->hash_fptr(val.data_ptr);
    }
};

// Specific Impl Type Object Handler //

namespace taihe {
template <typename Impl>
struct data_block : DataBlockHead {
    Impl impl;

    template <typename... Args>
    data_block(TypeInfo const *rtti_ptr, Args &&...args) : impl(std::forward<Args>(args)...)
    {
        tobj_init(this, rtti_ptr);
    }
};

template <typename Impl, typename Enabled = void>
struct hash_impl_t {
    std::size_t operator()(data_view val) const
    {
        return reinterpret_cast<std::size_t>(val.data_ptr);
    }
};

template <typename Impl, typename Enabled = void>
struct same_impl_t {
    bool operator()(data_view lhs, data_view rhs) const
    {
        return lhs.data_ptr == rhs.data_ptr;
    }
};

template <typename Impl>
constexpr inline hash_impl_t<Impl> hash_impl;

template <typename Impl>
constexpr inline same_impl_t<Impl> same_impl;

template <typename Impl>
inline Impl *cast_data_ptr(struct DataBlockHead *data_ptr)
{
    return &static_cast<data_block<Impl> *>(data_ptr)->impl;
}

template <typename Impl, typename... Args>
inline DataBlockHead *make_data_ptr(TypeInfo const *rtti_ptr, Args &&...args)
{
    return new data_block<Impl>(rtti_ptr, std::forward<Args>(args)...);
}

template <typename Impl>
inline void free_data_ptr(struct DataBlockHead *data_ptr)
{
    delete static_cast<data_block<Impl> *>(data_ptr);
}

template <typename Impl>
inline std::size_t hash_data_ptr(struct DataBlockHead *val_data_ptr)
{
    return hash_impl<Impl>(data_view(val_data_ptr));
}

template <typename Impl>
inline bool same_data_ptr(struct DataBlockHead *lhs_data_ptr, struct DataBlockHead *rhs_data_ptr)
{
    return same_impl<Impl>(data_view(lhs_data_ptr), data_view(rhs_data_ptr));
}

template <typename Impl, typename... InterfaceTypes>
struct impl_view;

template <typename Impl, typename... InterfaceTypes>
struct impl_holder;

template <typename Impl, typename... InterfaceTypes>
struct impl_view {
    DataBlockHead *data_ptr;

    explicit impl_view(DataBlockHead *other_data_ptr) : data_ptr(other_data_ptr) {}

    template <typename InterfaceView, std::enable_if_t<!InterfaceView::is_holder, int> = 0>
    operator InterfaceView() const &
    {
        return InterfaceView({
            this->template get_vtbl_ptr<InterfaceView>(),
            this->data_ptr,
        });
    }

    template <typename InterfaceHolder, std::enable_if_t<InterfaceHolder::is_holder, int> = 0>
    operator InterfaceHolder() const &
    {
        return InterfaceHolder({
            this->template get_vtbl_ptr<InterfaceHolder>(),
            tobj_dup(this->data_ptr),
        });
    }

    operator data_view() const &
    {
        return data_view(this->data_ptr);
    }

    operator data_holder() const &
    {
        return data_holder(tobj_dup(this->data_ptr));
    }

public:
    Impl *operator->() const
    {
        return cast_data_ptr<Impl>(this->data_ptr);
    }

    Impl &operator*() const
    {
        return *cast_data_ptr<Impl>(this->data_ptr);
    }

    template <typename... Args>
    decltype(auto) operator()(Args &&...args) const
    {
        return cast_data_ptr<Impl>(this->data_ptr)->operator()(std::forward<Args>(args)...);
    }

public:
    static inline void const *query_interface_id(void const *id)
    {
        void const *vtbl_ptr = nullptr;
        (
            [&] {
                using InterfaceType = InterfaceTypes;
                for (std::size_t j = 0; j < sizeof(InterfaceType::template idmap_impl<Impl>) / sizeof(IdMapItem); j++) {
                    if (id == InterfaceType::template idmap_impl<Impl>[j].id) {
                        vtbl_ptr = InterfaceType::template idmap_impl<Impl>[j].vtbl_ptr;
                    }
                }
            }(),
            ...);
        return vtbl_ptr;
    }

    static constexpr struct typeinfo_t {
        free_func_t *free_fptr = &free_data_ptr<Impl>;
        hash_func_t *hash_fptr = &hash_data_ptr<Impl>;
        same_func_t *same_fptr = &same_data_ptr<Impl>;
        qiid_func_t *qiid_fptr = &query_interface_id;
        uint64_t len = 0;
        struct IdMapItem idmap[((sizeof(InterfaceTypes::template idmap_impl<Impl>) / sizeof(IdMapItem)) + ...)] = {};
    } rtti = [] {
        struct typeinfo_t info;
        (
            [&] {
                using InterfaceType = InterfaceTypes;
                for (std::size_t j = 0; j < sizeof(InterfaceType::template idmap_impl<Impl>) / sizeof(IdMapItem);
                     j++, info.len++) {
                    info.idmap[info.len] = InterfaceType::template idmap_impl<Impl>[j];
                }
            }(),
            ...);
        return info;
    }();

    template <typename InterfaceDest,
              std::enable_if_t<
                  (std::is_convertible_v<typename InterfaceTypes::view_type, typename InterfaceDest::view_type> || ...),
                  int> = 0>
    static inline typename InterfaceDest::vtable_type const *get_vtbl_ptr()
    {
        typename InterfaceDest::vtable_type const *vtbl_ptr = nullptr;
        (
            [&] {
                using InterfaceType = InterfaceTypes;
                if constexpr (std::is_convertible_v<typename InterfaceType::view_type,
                                                    typename InterfaceDest::view_type>) {
                    vtbl_ptr = typename InterfaceDest::view_type(typename InterfaceType::view_type({
                                                                     &InterfaceType::template vtbl_impl<Impl>,
                                                                     nullptr,
                                                                 }))
                                   .m_handle.vtbl_ptr;
                }
            }(),
            ...);
        return vtbl_ptr;
    }
};

template <typename Impl, typename... InterfaceTypes>
struct impl_holder : public impl_view<Impl, InterfaceTypes...> {
    using impl_view<Impl, InterfaceTypes...>::rtti;

    explicit impl_holder(DataBlockHead *other_data_ptr) : impl_view<Impl, InterfaceTypes...>(other_data_ptr) {}

    template <typename... Args>
    static impl_holder make(Args &&...args)
    {
        DataBlockHead *data_ptr =
            make_data_ptr<Impl>(reinterpret_cast<TypeInfo const *>(&rtti), std::forward<Args>(args)...);
        return impl_holder(data_ptr);
    }

    impl_holder &operator=(impl_holder other)
    {
        std::swap(this->data_ptr, other.data_ptr);
        return *this;
    }

    ~impl_holder()
    {
        tobj_drop(this->data_ptr);
    }

    impl_holder(impl_view<Impl, InterfaceTypes...> const &other) : impl_holder(tobj_dup(other.data_ptr)) {}

    impl_holder(impl_holder<Impl, InterfaceTypes...> const &other) : impl_holder(tobj_dup(other.data_ptr)) {}

    impl_holder(impl_holder<Impl, InterfaceTypes...> &&other) : impl_holder(std::exchange(other.data_ptr, nullptr)) {}

    template <typename InterfaceView, std::enable_if_t<!InterfaceView::is_holder, int> = 0>
    operator InterfaceView() const &
    {
        return InterfaceView({
            this->template get_vtbl_ptr<InterfaceView>(),
            this->data_ptr,
        });
    }

    template <typename InterfaceHolder, std::enable_if_t<InterfaceHolder::is_holder, int> = 0>
    operator InterfaceHolder() const &
    {
        return InterfaceHolder({
            this->template get_vtbl_ptr<InterfaceHolder>(),
            tobj_dup(this->data_ptr),
        });
    }

    template <typename InterfaceHolder, std::enable_if_t<InterfaceHolder::is_holder, int> = 0>
    operator InterfaceHolder() &&
    {
        return InterfaceHolder({
            this->template get_vtbl_ptr<InterfaceHolder>(),
            std::exchange(this->data_ptr, nullptr),
        });
    }

    operator data_view() const &
    {
        return data_view(this->data_ptr);
    }

    operator data_holder() const &
    {
        return data_holder(tobj_dup(this->data_ptr));
    }

    operator data_holder() &&
    {
        return data_holder(std::exchange(this->data_ptr, nullptr));
    }
};

template <typename Impl, typename... InterfaceTypes, typename... Args>
inline auto make_holder(Args &&...args)
{
    return impl_holder<Impl, InterfaceTypes...>::make(std::forward<Args>(args)...);
}

template <typename Impl, typename... InterfaceTypes>
inline bool operator==(impl_view<Impl, InterfaceTypes...> lhs, impl_view<Impl, InterfaceTypes...> rhs)
{
    return data_view(lhs) == data_view(rhs);
}
}  // namespace taihe

template <typename Impl, typename... InterfaceTypes>
struct std::hash<taihe::impl_holder<Impl, InterfaceTypes...>> {
    std::size_t operator()(taihe::data_view val) const noexcept
    {
        return std::hash<taihe::data_holder>()(val);
    }
};
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_OBJECT_HPP_