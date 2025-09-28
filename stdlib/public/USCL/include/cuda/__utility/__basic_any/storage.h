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

#ifndef _CUDA___UTILITY_BASIC_ANY_STORAGE_H
#define _CUDA___UTILITY_BASIC_ANY_STORAGE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__utility/__basic_any/basic_any_fwd.h>
#include <uscl/std/__algorithm/max.h>
#include <uscl/std/__tuple_dir/ignore.h>
#include <uscl/std/__type_traits/is_nothrow_move_constructible.h>
#include <uscl/std/__type_traits/type_identity.h>
#include <uscl/std/__utility/swap.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

[[nodiscard]] _CCCL_API inline constexpr auto __buffer_size(size_t __size) -> size_t
{
  //! round up to the nearest multiple of `__word`, which is the size of a
  //! void*.
  return ((__size ? (::cuda::std::max) (__size, sizeof(void*)) : __default_small_object_size) + __word - 1) / __word
       * __word;
}

[[nodiscard]] _CCCL_API inline constexpr auto __buffer_align(size_t __align) -> size_t
{
  //! need to be able to store a void* in the buffer.
  return __align ? (::cuda::std::max) (__align, alignof(void*)) : __default_small_object_align;
}

template <class _Tp>
[[nodiscard]] _CCCL_API inline constexpr auto __is_small(size_t __size, size_t __align) noexcept -> bool
{
  return (sizeof(_Tp) <= __size) && (__align % alignof(_Tp) == 0) && ::cuda::std::is_nothrow_move_constructible_v<_Tp>;
}

_CCCL_API inline void __swap_ptr_ptr(void* __lhs, void* __rhs) noexcept
{
  ::cuda::std::swap(*static_cast<void**>(__lhs), *static_cast<void**>(__rhs));
}

template <class _Tp,
          class _Up,
          class _Vp = decltype(true ? ::cuda::std::type_identity_t<_Tp*>() : ::cuda::std::type_identity_t<_Up*>())>
[[nodiscard]] _CCCL_NODEBUG_API auto __ptr_eq(_Tp* __lhs, _Up* __rhs) noexcept -> bool
{
  return static_cast<_Vp>(__lhs) == static_cast<_Vp>(__rhs);
}

[[nodiscard]]
_CCCL_NODEBUG_API constexpr auto __ptr_eq(::cuda::std::__ignore_t, ::cuda::std::__ignore_t) noexcept -> bool
{
  return false;
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___UTILITY_BASIC_ANY_STORAGE_H
