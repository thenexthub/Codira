/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Friday, December 29, 2023.
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

#ifndef _CUDA_STD___MEMORY_TEMPORARY_BUFFER_H
#define _CUDA_STD___MEMORY_TEMPORARY_BUFFER_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__iterator/iterator.h>
#include <uscl/std/__iterator/iterator_traits.h>
#include <uscl/std/__memory/addressof.h>
#include <uscl/std/__new_>
#include <uscl/std/__type_traits/alignment_of.h>
#include <uscl/std/__utility/move.h>
#include <uscl/std/__utility/pair.h>
#include <uscl/std/climits>
#include <uscl/std/cstddef>
#include <uscl/std/limits>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <class _Tp>
[[nodiscard]] _CCCL_NO_CFI _CCCL_API inline pair<_Tp*, ptrdiff_t> get_temporary_buffer(ptrdiff_t __n) noexcept
{
  pair<_Tp*, ptrdiff_t> __r(0, 0);
  const ptrdiff_t __m = (~ptrdiff_t(0) ^ ptrdiff_t(ptrdiff_t(1) << (sizeof(ptrdiff_t) * CHAR_BIT - 1))) / sizeof(_Tp);
  if (__n > __m)
  {
    __n = __m;
  }
  while (__n > 0)
  {
#if _LIBCUDACXX_HAS_ALIGNED_ALLOCATION()
    if (__is_overaligned_for_new(alignof(_Tp)))
    {
      ::cuda::std::align_val_t __al = ::cuda::std::align_val_t(::cuda::std::alignment_of<_Tp>::value);
      __r.first                     = static_cast<_Tp*>(::operator new(__n * sizeof(_Tp), __al));
    }
    else
    {
      __r.first = static_cast<_Tp*>(::operator new(__n * sizeof(_Tp)));
    }
#else // ^^^ _LIBCUDACXX_HAS_ALIGNED_ALLOCATION() ^^^ / vvv !_LIBCUDACXX_HAS_ALIGNED_ALLOCATION() vvv
    if (__is_overaligned_for_new(alignof(_Tp)))
    {
      // Since aligned operator new is unavailable, return an empty
      // buffer rather than one with invalid alignment.
      return __r;
    }

    __r.first = static_cast<_Tp*>(::operator new(__n * sizeof(_Tp)));
#endif // !_LIBCUDACXX_HAS_ALIGNED_ALLOCATION()

    if (__r.first)
    {
      __r.second = __n;
      break;
    }
    __n /= 2;
  }
  return __r;
}

template <class _Tp>
_CCCL_API inline void return_temporary_buffer(_Tp* __p) noexcept
{
  ::cuda::std::__cccl_deallocate_unsized((void*) __p, alignof(_Tp));
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___MEMORY_TEMPORARY_BUFFER_H
