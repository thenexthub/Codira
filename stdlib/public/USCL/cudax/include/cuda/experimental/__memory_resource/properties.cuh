/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, July 24, 2024.
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

#ifndef _CUDAX__MEMORY_RESOURCE_PROPERTIES_CUH
#define _CUDAX__MEMORY_RESOURCE_PROPERTIES_CUH

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__memory_resource/get_property.h>
#include <uscl/__memory_resource/properties.h>
#include <uscl/std/__type_traits/decay.h>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{

using ::cuda::mr::device_accessible;
using ::cuda::mr::host_accessible;

//! @brief A type representing a list of memory resource properties
//! @tparam _Properties The properties to be included in the list
//! It has a member template `rebind` that allows constructing a type by combining
//! a template and type arguments with the properties from this list. The properties
//! are appended after the type arguments in the resulting type.
template <class... _Properties>
struct properties_list
{
  //! @brief A type alias for a type template instantiated with the properties
  //! from this list appended to the type arguments.
  template <template <class...> class _Fn, class... _ExtraArgs>
  using rebind = _Fn<_ExtraArgs..., _Properties...>;

  template <class _QueryProperty>
  static constexpr bool has_property([[maybe_unused]] _QueryProperty)
  {
    return ::cuda::std::__type_set_contains_v<::cuda::std::__make_type_set<_Properties...>, _QueryProperty>;
  }
};

template <class _T>
inline constexpr bool __is_queries_list = false;

template <class... _T>
inline constexpr bool __is_queries_list<properties_list<_T...>> = true;

template <typename _Tp>
_CCCL_CONCEPT __has_default_queries =
  _CCCL_REQUIRES_EXPR((_Tp))(requires(__is_queries_list<typename ::cuda::std::decay_t<_Tp>::default_queries>));

template <typename _Resource, bool _HasDefaultQueries = __has_default_queries<_Resource>>
struct __copy_default_queries;

template <typename _Resource>
struct __copy_default_queries<_Resource, true>
{
  using default_queries = typename _Resource::default_queries;
};

template <typename _Resource>
struct __copy_default_queries<_Resource, false>
{};

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif //_CUDAX__MEMORY_RESOURCE_PROPERTIES_CUH
