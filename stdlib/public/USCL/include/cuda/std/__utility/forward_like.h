/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, November 8, 2023.
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

#ifndef _CUDA_STD___UTILITY_FORWARD_LIKE_H
#define _CUDA_STD___UTILITY_FORWARD_LIKE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/conditional.h>
#include <uscl/std/__type_traits/copy_cvref.h>
#include <uscl/std/__type_traits/remove_reference.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

#if _CCCL_HAS_BUILTIN_STD_FORWARD_LIKE()

// The compiler treats ::std::forward_like as a builtin function so it does not need to be
// instantiated and will be compiled away even at -O0.
using ::std::forward_like;

#else // ^^^ _CCCL_HAS_BUILTIN_STD_FORWARD_LIKE() ^^^ / vvv !_CCCL_HAS_BUILTIN_STD_FORWARD_LIKE() vvv

template <class _Ap, class _Bp>
using _ForwardLike = __copy_cvref_t<_Ap&&, remove_reference_t<_Bp>>;

template <class _Tp, class _Up>
[[nodiscard]] _CCCL_INTRINSIC _CCCL_API constexpr auto forward_like(_Up&& __ux) noexcept -> _ForwardLike<_Tp, _Up>
{
  return static_cast<_ForwardLike<_Tp, _Up>>(__ux);
}

#endif // _CCCL_HAS_BUILTIN_STD_FORWARD_LIKE()

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___UTILITY_FORWARD_LIKE_H
