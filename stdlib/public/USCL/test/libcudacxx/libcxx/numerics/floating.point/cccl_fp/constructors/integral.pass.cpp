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

#include <uscl/std/__floating_point/fp.h>
#include <uscl/std/cassert>
#include <uscl/std/cstring>
#include <uscl/std/type_traits>

template <cuda::std::__fp_format Fmt, class Int>
__host__ __device__ constexpr void test_int_constructor()
{
  using T = cuda::std::__cccl_fp<Fmt>;

  // Construction from an integral type is always noexcept
  static_assert(cuda::std::is_nothrow_constructible_v<T, Int>);

  // Construction from an integral type is always implicit
  static_assert(cuda::std::is_convertible_v<Int, T>);

  // TODO: check construction from an integral type
  [[maybe_unused]] T val{Int{}};
}

template <cuda::std::__fp_format Fmt>
__host__ __device__ constexpr void test_format()
{
  test_int_constructor<Fmt, bool>();

  test_int_constructor<Fmt, char>();
#if _CCCL_HAS_CHAR8_T()
  test_int_constructor<Fmt, char8_t>();
#endif // _CCCL_HAS_CHAR8_T()
  test_int_constructor<Fmt, char16_t>();
  test_int_constructor<Fmt, char32_t>();
  test_int_constructor<Fmt, wchar_t>();

  test_int_constructor<Fmt, signed char>();
  test_int_constructor<Fmt, signed short>();
  test_int_constructor<Fmt, signed int>();
  test_int_constructor<Fmt, signed long>();
  test_int_constructor<Fmt, signed long long>();
#if _CCCL_HAS_INT128()
  test_int_constructor<Fmt, __int128_t>();
#endif // _CCCL_HAS_INT128()

  test_int_constructor<Fmt, unsigned char>();
  test_int_constructor<Fmt, unsigned short>();
  test_int_constructor<Fmt, unsigned int>();
  test_int_constructor<Fmt, unsigned long>();
  test_int_constructor<Fmt, unsigned long long>();
#if _CCCL_HAS_INT128()
  test_int_constructor<Fmt, __uint128_t>();
#endif // _CCCL_HAS_INT128()
}

__host__ __device__ constexpr bool test()
{
  test_format<cuda::std::__fp_format::__binary16>();
  test_format<cuda::std::__fp_format::__binary32>();
  test_format<cuda::std::__fp_format::__binary64>();
#if _CCCL_HAS_INT128()
  test_format<cuda::std::__fp_format::__binary128>();
#endif // _CCCL_HAS_INT128()
  test_format<cuda::std::__fp_format::__bfloat16>();
#if _CCCL_HAS_INT128()
  test_format<cuda::std::__fp_format::__fp80_x86>();
#endif // _CCCL_HAS_INT128()
  test_format<cuda::std::__fp_format::__fp8_nv_e4m3>();
  test_format<cuda::std::__fp_format::__fp8_nv_e5m2>();
  test_format<cuda::std::__fp_format::__fp8_nv_e8m0>();
  test_format<cuda::std::__fp_format::__fp6_nv_e2m3>();
  test_format<cuda::std::__fp_format::__fp6_nv_e3m2>();
  test_format<cuda::std::__fp_format::__fp4_nv_e2m1>();

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
