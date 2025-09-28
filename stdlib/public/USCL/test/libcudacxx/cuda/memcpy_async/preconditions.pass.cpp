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

#define _LIBCUDACXX_MEMCPY_ASYNC_PRE_TESTING

#include <uscl/__memcpy_async/check_preconditions.h>
#include <uscl/std/cstddef>

__host__ __device__ void test()
{
  using T = int;

  constexpr cuda::std::size_t align_scale = 2;
  constexpr cuda::std::size_t align       = align_scale * alignof(T);
  constexpr cuda::std::size_t n           = 16;
  constexpr cuda::std::size_t size        = n * sizeof(T);

  // test typed overloads
  {
    alignas(align) T a[n * 2]{};
    alignas(align) const T b[n * 2]{};

    const auto a_missaligned = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(a) + alignof(T) / 2);
    const auto b_missaligned = reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(b) + alignof(T) / 2);

    // 1. test ordinary size type
    {
      assert(cuda::__memcpy_async_check_pre(a, b, size));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, b, size));
      assert(!cuda::__memcpy_async_check_pre(a, b_missaligned, size));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, b_missaligned, size));
    }

    // 2. test overaligned cuda::aligned_size_t
    {
      cuda::aligned_size_t<align> size_aligned(size);
      assert(cuda::__memcpy_async_check_pre(a, b, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, b, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a, b_missaligned, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, b_missaligned, size_aligned));
    }

    // 3. test cuda::aligned_size_t aligned to alignof(T)
    {
      cuda::aligned_size_t<align / align_scale> size_aligned(size);
      assert(cuda::__memcpy_async_check_pre(a, b, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, b, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a, b_missaligned, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, b_missaligned, size_aligned));
    }

    // 4. test underaligned cuda::aligned_size_t
    {
      cuda::aligned_size_t<align / (2 * align_scale)> size_aligned(size);
      assert(cuda::__memcpy_async_check_pre(a, b, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, b, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a, b_missaligned, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, b_missaligned, size_aligned));
    }

    // 5. test overlap
    {
      assert(!cuda::__memcpy_async_check_pre(a, a, size));
      assert(!cuda::__memcpy_async_check_pre(a, a_missaligned, size));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, a, size));
      assert(cuda::__memcpy_async_check_pre(a, a + n, size));
      assert(cuda::__memcpy_async_check_pre(a + n, a, size));
      assert(!cuda::__memcpy_async_check_pre(a, a + n - 1, size));
      assert(!cuda::__memcpy_async_check_pre(a + n - 1, a, size));
    }
  }

  // test void overloads
  {
    alignas(align) T a_buff[n * 2]{};
    alignas(align) const T b_buff[n * 2]{};

    void* a       = a_buff;
    const void* b = b_buff;

    const auto a_missaligned = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(a) + alignof(T) / 2);
    const auto b_missaligned = reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(b) + alignof(T) / 2);

    // 1. test ordinary size type
    {
      assert(cuda::__memcpy_async_check_pre(a, b, size));
      assert(cuda::__memcpy_async_check_pre(a_missaligned, b, size));
      assert(cuda::__memcpy_async_check_pre(a, b_missaligned, size));
      assert(cuda::__memcpy_async_check_pre(a_missaligned, b_missaligned, size));
    }

    // 2. test overaligned cuda::aligned_size_t
    {
      cuda::aligned_size_t<align> size_aligned(size);
      assert(cuda::__memcpy_async_check_pre(a, b, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, b, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a, b_missaligned, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, b_missaligned, size_aligned));
    }

    // 3. test cuda::aligned_size_t aligned to alignof(T)
    {
      cuda::aligned_size_t<align / align_scale> size_aligned(size);
      assert(cuda::__memcpy_async_check_pre(a, b, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, b, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a, b_missaligned, size_aligned));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, b_missaligned, size_aligned));
    }

    // 4. test underaligned cuda::aligned_size_t
    {
      cuda::aligned_size_t<align / (2 * align_scale)> size_aligned(size);
      assert(cuda::__memcpy_async_check_pre(a, b, size_aligned));
      assert(cuda::__memcpy_async_check_pre(a_missaligned, b, size_aligned));
      assert(cuda::__memcpy_async_check_pre(a, b_missaligned, size_aligned));
      assert(cuda::__memcpy_async_check_pre(a_missaligned, b_missaligned, size_aligned));
    }

    // 5. test overlap
    {
      assert(!cuda::__memcpy_async_check_pre(a, a, size));
      assert(!cuda::__memcpy_async_check_pre(a, a_missaligned, size));
      assert(!cuda::__memcpy_async_check_pre(a_missaligned, a, size));
      assert(cuda::__memcpy_async_check_pre(a, (const void*) (a_buff + n), size));
      assert(cuda::__memcpy_async_check_pre((void*) (a_buff + n), a, size));
      assert(!cuda::__memcpy_async_check_pre(a, (const void*) (a_buff + n - 1), size));
      assert(!cuda::__memcpy_async_check_pre((void*) (a_buff + n - 1), a, size));
    }
  }
}

int main(int, char**)
{
  test();
  return 0;
}
