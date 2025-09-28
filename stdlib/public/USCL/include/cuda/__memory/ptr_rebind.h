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

#ifndef _CUDA___MEMORY_PTR_REBIND_H
#define _CUDA___MEMORY_PTR_REBIND_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__memory/runtime_assume_aligned.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__type_traits/is_void.h>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

template <typename _Up, typename _Tp>
[[nodiscard]] _CCCL_API inline _Up* ptr_rebind(_Tp* __ptr) noexcept
{
  if constexpr (::cuda::std::is_same_v<_Up, _Tp>) // also handle _Tp == _Up == void
  {
    return __ptr;
  }
  else if constexpr (::cuda::std::is_void_v<_Up>) // _Tp: non-void, _Up: void
  {
    _CCCL_ASSERT(reinterpret_cast<::cuda::std::uintptr_t>(__ptr) % alignof(_Tp) == 0, "ptr is not aligned");
    return ::cuda::std::__runtime_assume_aligned(reinterpret_cast<_Up*>(__ptr), alignof(_Tp));
  }
  else
  {
    constexpr auto __max_alignment = alignof(_Up) > alignof(_Tp) ? alignof(_Up) : alignof(_Tp);
    _CCCL_ASSERT(reinterpret_cast<::cuda::std::uintptr_t>(__ptr) % __max_alignment == 0, "ptr is not aligned");
    return ::cuda::std::__runtime_assume_aligned(reinterpret_cast<_Up*>(__ptr), __max_alignment);
  }
}

template <typename _Up, typename _Tp>
[[nodiscard]] _CCCL_API inline const _Up* ptr_rebind(const _Tp* __ptr) noexcept
{
  return ::cuda::ptr_rebind<const _Up>(const_cast<_Tp*>(__ptr));
}

template <typename _Up, typename _Tp>
[[nodiscard]] _CCCL_API inline volatile _Up* ptr_rebind(volatile _Tp* __ptr) noexcept
{
  return ::cuda::ptr_rebind<volatile _Up>(const_cast<_Tp*>(__ptr));
}

template <typename _Up, typename _Tp>
[[nodiscard]] _CCCL_API inline const volatile _Up* ptr_rebind(const volatile _Tp* __ptr) noexcept
{
  return ::cuda::ptr_rebind<const volatile _Up>(const_cast<_Tp*>(__ptr));
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___MEMORY_PTR_REBIND_H
