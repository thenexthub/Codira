/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Sunday, April 14, 2024.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 *
 */

#ifndef _CUDA___UTILITY_BASIC_ANY_VIRTCALL_H
#define _CUDA___UTILITY_BASIC_ANY_VIRTCALL_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__utility/__basic_any/access.h>
#include <uscl/__utility/__basic_any/basic_any_from.h>
#include <uscl/__utility/__basic_any/basic_any_fwd.h>
#include <uscl/__utility/__basic_any/interfaces.h>
#include <uscl/__utility/__basic_any/virtual_functions.h>
#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__type_traits/is_callable.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_DIAG_PUSH
_CCCL_DIAG_SUPPRESS_CLANG("-Wunused-local-typedef")

_CCCL_BEGIN_NAMESPACE_CUDA

//!
//! __virtuals_map
//!

//! The virtuals map is an extra convenience for interface authors. To make a
//! virtual function call, the user must provide the member function pointer
//! corresponding to the virtual, as in:
//!
//! \code{.cpp}
//! template <class...>
//! struct ifoo {
//!   void meow(auto... args) {
//!     // dispatch to the &ifoo<>::meow virtual function
//!     // NB: the `<>` after `ifoo` is significant!
//!     __virtcall<&ifoo<>::meow>(this, args...);
//!     //              ^^
//!   }
//!  ...
//! };
//! \endcode
//!
//! When taking the address of the member, it is very easy to forget the `<>`
//! after the interface name, which would result in a compilation error --
//! except for the virtuals map, which substitutes the correct member function
//! pointer for the user so they don't have to think about it.
template <auto _Mbr, auto _BoundMbr>
struct __virtuals_map_element
{
  // map ifoo<>::meow to itself
  _CCCL_API auto operator()(__ctag<_Mbr>) const -> __virtual_fn<_Mbr>;

  // map ifoo<_Super>::meow to ifoo<>::meow
  _CCCL_API auto operator()(__ctag<_BoundMbr>) const -> __virtual_fn<_Mbr>;
};

template <class, class>
struct __virtuals_map;

template <class _Interface, class... _Mbrs, class _BoundInterface, auto... _BoundMbrs>
struct __virtuals_map<__overrides_list<_Interface, _Mbrs...>, __overrides_for<_BoundInterface, _BoundMbrs...>>
    : __virtuals_map_element<_Mbrs::value, _BoundMbrs>...
{
  using __virtuals_map_element<_Mbrs::value, _BoundMbrs>::operator()...;
};

template <class _Interface, class _Super>
using __virtuals_map_for _CCCL_NODEBUG_ALIAS =
  __virtuals_map<__overrides_for_t<_Interface>, __overrides_for_t<__rebind_interface<_Interface, _Super>>>;

template <auto _Mbr, class _Interface, class _Super>
extern ::cuda::std::__call_result_t<__virtuals_map_for<_Interface, _Super>, __ctag<_Mbr>> __virtual_fn_for_v;

// This alias indirects through the above variable template to cache the result
// of the virtuals map lookup.
template <auto _Mbr, class _Interface, class _Super>
using __virtual_fn_for _CCCL_NODEBUG_ALIAS = decltype(__virtual_fn_for_v<_Mbr, _Interface, _Super>);

//!
//! __virtcall
//!

// If the interface is __ireference<MyInterface const>, then calls to non-const
// member functions are not allowed.
template <auto, class... _Interface>
inline constexpr bool __valid_virtcall = sizeof...(_Interface) == 1;

template <auto _Mbr, class _Interface>
inline constexpr bool __valid_virtcall<_Mbr, __ireference<_Interface const>> = __virtual_fn<_Mbr>::__const_fn;

template <auto _Mbr, class _Interface, class _Super, class _Self, class... _Args>
_CCCL_API auto __virtcall(_Self* __self, _Args&&... __args) //
  noexcept(__virtual_fn<_Mbr>::__nothrow_fn) //
  -> typename __virtual_fn<_Mbr>::__result_t
{
  auto* __vptr = __basic_any_access::__get_vptr(*__self)->__query_interface(_Interface());
  auto* __obj  = __basic_any_access::__get_optr(*__self);
  // map the member function pointer to the correct one if necessary
  using __virtual_fn_t = __virtual_fn_for<_Mbr, _Interface, _Super>;
  return __vptr->__virtual_fn_t::__fn_(__obj, static_cast<_Args&&>(__args)...);
}

_CCCL_TEMPLATE(auto _Mbr, template <class...> class _Interface, class _Super, class... _Args)
_CCCL_REQUIRES(__valid_virtcall<_Mbr, _Super>)
_CCCL_NODEBUG_API auto __virtcall(_Interface<_Super>* __self, _Args&&... __args) //
  noexcept(__virtual_fn<_Mbr>::__nothrow_fn) //
  -> typename __virtual_fn<_Mbr>::__result_t
{
  return ::cuda::__virtcall<_Mbr, _Interface<>, _Super>(
    ::cuda::__basic_any_from(__self), static_cast<_Args&&>(__args)...);
}

_CCCL_TEMPLATE(auto _Mbr, template <class...> class _Interface, class _Super, class... _Args)
_CCCL_REQUIRES(__valid_virtcall<_Mbr, _Super>)
_CCCL_NODEBUG_API auto __virtcall(_Interface<_Super> const* __self, _Args&&... __args) //
  noexcept(__virtual_fn<_Mbr>::__nothrow_fn) //
  -> typename __virtual_fn<_Mbr>::__result_t
{
  return ::cuda::__virtcall<_Mbr, _Interface<>, _Super>(
    ::cuda::__basic_any_from(__self), static_cast<_Args&&>(__args)...);
}

_CCCL_TEMPLATE(auto _Mbr, template <class...> class _Interface, class... _Super, class... _Args)
_CCCL_REQUIRES((!__valid_virtcall<_Mbr, _Super...>) )
_CCCL_NODEBUG_API auto __virtcall(_Interface<_Super...> const*, _Args&&...) //
  noexcept(__virtual_fn<_Mbr>::__nothrow_fn) //
  -> typename __virtual_fn<_Mbr>::__result_t
{
  constexpr bool __const_correct_virtcall = __valid_virtcall<_Mbr, _Super...> || sizeof...(_Super) == 0;
  // If this static assert fires, then you have called a non-const member
  // function on a `__basic_any<I const&>`. This would violate const-correctness.
  static_assert(__const_correct_virtcall, "This function call is not const correct.");
  // This overload can also be selected when called from the thunks of
  // unspecialized interfaces. Those thunks should never be called, but they
  // must exist to satisfy the compiler.
  _CCCL_UNREACHABLE();
}

_CCCL_END_NAMESPACE_CUDA

_CCCL_DIAG_POP

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___UTILITY_BASIC_ANY_VIRTCALL_H
