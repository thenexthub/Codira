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

#ifndef _CUDA_STD___COMPLEX_TUPLE_H
#define _CUDA_STD___COMPLEX_TUPLE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__complex/complex.h>
#include <uscl/std/__fwd/get.h>
#include <uscl/std/__tuple_dir/tuple_element.h>
#include <uscl/std/__tuple_dir/tuple_size.h>
#include <uscl/std/__type_traits/integral_constant.h>
#include <uscl/std/__utility/move.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <class _Tp>
struct tuple_size<complex<_Tp>> : ::cuda::std::integral_constant<size_t, 2>
{};

template <size_t _Index, class _Tp>
  struct tuple_element<_Index, complex<_Tp>> : ::cuda::std::enable_if < _Index<2, _Tp>
{};

template <class _Tp>
struct __get_complex_impl
{
  template <size_t _Index>
  [[nodiscard]] static _CCCL_API constexpr _Tp& get(complex<_Tp>& __z) noexcept
  {
    return (_Index == 0) ? __z.__re_ : __z.__im_;
  }

  template <size_t _Index>
  [[nodiscard]] static _CCCL_API constexpr _Tp&& get(complex<_Tp>&& __z) noexcept
  {
    return ::cuda::std::move((_Index == 0) ? __z.__re_ : __z.__im_);
  }

  template <size_t _Index>
  [[nodiscard]] static _CCCL_API constexpr const _Tp& get(const complex<_Tp>& __z) noexcept
  {
    return (_Index == 0) ? __z.__re_ : __z.__im_;
  }

  template <size_t _Index>
  [[nodiscard]] static _CCCL_API constexpr const _Tp&& get(const complex<_Tp>&& __z) noexcept
  {
    return ::cuda::std::move((_Index == 0) ? __z.__re_ : __z.__im_);
  }
};

template <size_t _Index, class _Tp>
[[nodiscard]] _CCCL_API constexpr _Tp& get(complex<_Tp>& __z) noexcept
{
  static_assert(_Index < 2, "Index value is out of range");

  return __get_complex_impl<_Tp>::template get<_Index>(__z);
}

template <size_t _Index, class _Tp>
[[nodiscard]] _CCCL_API constexpr _Tp&& get(complex<_Tp>&& __z) noexcept
{
  static_assert(_Index < 2, "Index value is out of range");

  return __get_complex_impl<_Tp>::template get<_Index>(::cuda::std::move(__z));
}

template <size_t _Index, class _Tp>
[[nodiscard]] _CCCL_API constexpr const _Tp& get(const complex<_Tp>& __z) noexcept
{
  static_assert(_Index < 2, "Index value is out of range");

  return __get_complex_impl<_Tp>::template get<_Index>(__z);
}

template <size_t _Index, class _Tp>
[[nodiscard]] _CCCL_API constexpr const _Tp&& get(const complex<_Tp>&& __z) noexcept
{
  static_assert(_Index < 2, "Index value is out of range");

  return __get_complex_impl<_Tp>::template get<_Index>(::cuda::std::move(__z));
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___COMPLEX_TUPLE_H
