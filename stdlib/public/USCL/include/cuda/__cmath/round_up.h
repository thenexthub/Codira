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

#ifndef _CUDA___CMATH_ROUND_UP_H
#define _CUDA___CMATH_ROUND_UP_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__cmath/ceil_div.h>
#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__type_traits/common_type.h>
#include <uscl/std/__type_traits/is_enum.h>
#include <uscl/std/__type_traits/is_integral.h>
#include <uscl/std/__type_traits/is_signed.h>
#include <uscl/std/__type_traits/make_unsigned.h>
#include <uscl/std/__utility/to_underlying.h>
#include <uscl/std/limits>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

//! @brief Round the number \p __a to the next multiple of \p __b
//! @param __a The input number
//! @param __b The multiplicand
//! @pre \p __a must be non-negative
//! @pre \p __b must be positive
_CCCL_TEMPLATE(class _Tp, class _Up)
_CCCL_REQUIRES(::cuda::std::is_integral_v<_Tp> _CCCL_AND ::cuda::std::is_integral_v<_Up>)
[[nodiscard]] _CCCL_API constexpr ::cuda::std::common_type_t<_Tp, _Up> round_up(const _Tp __a, const _Up __b) noexcept
{
  _CCCL_ASSERT(__b > _Up{0}, "cuda::round_up: 'b' must be positive");
  if constexpr (::cuda::std::is_signed_v<_Tp>)
  {
    _CCCL_ASSERT(__a >= _Tp{0}, "cuda::round_up: 'a' must be non negative");
  }
  using _Common = ::cuda::std::common_type_t<_Tp, _Up>;
  using _Prom   = decltype(_Tp{} / _Up{});
  auto __c      = ::cuda::ceil_div(static_cast<_Prom>(__a), static_cast<_Prom>(__b));
  _CCCL_ASSERT(static_cast<_Common>(__c) <= ::cuda::std::numeric_limits<_Common>::max() / static_cast<_Common>(__b),
               "cuda::round_up: result overflow");
  return static_cast<_Common>(static_cast<_Prom>(__c) * static_cast<_Prom>(__b));
}

//! @brief Round the number \p __a to the next multiple of \p __b
//! @param __a The input number
//! @param __b The multiplicand
//! @pre \p __a must be non-negative
//! @pre \p __b must be positive
_CCCL_TEMPLATE(class _Tp, class _Up)
_CCCL_REQUIRES(::cuda::std::is_integral_v<_Tp> _CCCL_AND ::cuda::std::is_enum_v<_Up>)
[[nodiscard]] _CCCL_API constexpr ::cuda::std::common_type_t<_Tp, ::cuda::std::underlying_type_t<_Up>>
round_up(const _Tp __a, const _Up __b) noexcept
{
  return ::cuda::round_up(__a, ::cuda::std::to_underlying(__b));
}

//! @brief Round the number \p __a to the next multiple of \p __b
//! @param __a The input number
//! @param __b The multiplicand
//! @pre \p __a must be non-negative
//! @pre \p __b must be positive
_CCCL_TEMPLATE(class _Tp, class _Up)
_CCCL_REQUIRES(::cuda::std::is_enum_v<_Tp> _CCCL_AND ::cuda::std::is_integral_v<_Up>)
[[nodiscard]] _CCCL_API constexpr ::cuda::std::common_type_t<::cuda::std::underlying_type_t<_Tp>, _Up>
round_up(const _Tp __a, const _Up __b) noexcept
{
  return ::cuda::round_up(::cuda::std::to_underlying(__a), __b);
}

//! @brief Round the number \p __a to the next multiple of \p __b
//! @param __a The input number
//! @param __b The multiplicand
//! @pre \p __a must be non-negative
//! @pre \p __b must be positive
_CCCL_TEMPLATE(class _Tp, class _Up)
_CCCL_REQUIRES(::cuda::std::is_enum_v<_Tp> _CCCL_AND ::cuda::std::is_enum_v<_Up>)
[[nodiscard]]
_CCCL_API constexpr ::cuda::std::common_type_t<::cuda::std::underlying_type_t<_Tp>, ::cuda::std::underlying_type_t<_Up>>
round_up(const _Tp __a, const _Up __b) noexcept
{
  return ::cuda::round_up(::cuda::std::to_underlying(__a), ::cuda::std::to_underlying(__b));
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___CMATH_ROUND_UP_H
