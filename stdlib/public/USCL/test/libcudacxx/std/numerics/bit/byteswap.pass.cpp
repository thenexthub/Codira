/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
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

#include <uscl/std/bit>
#include <uscl/std/cassert>
#include <uscl/std/cstddef>
#include <uscl/std/cstdint>
#include <uscl/std/utility>

#include "test_macros.h"

template <class T, class = void>
struct has_byteswap : cuda::std::false_type
{};

template <class T>
struct has_byteswap<T, cuda::std::void_t<decltype(cuda::std::byteswap(cuda::std::declval<T>()))>> : cuda::std::true_type
{};

enum class Byte : cuda::std::uint8_t
{
};

static_assert(!has_byteswap<void*>::value, "");
static_assert(!has_byteswap<float>::value, "");
static_assert(!has_byteswap<char[2]>::value, "");
static_assert(!has_byteswap<Byte>::value, "");

template <class T>
struct MakeUnsigned
{
  using type = cuda::std::make_unsigned_t<T>;
};

template <>
struct MakeUnsigned<bool>
{
  using type = bool;
};

template <class T>
__host__ __device__ constexpr void test_num(T in, T expected)
{
  using U = typename MakeUnsigned<T>::type;

  assert(static_cast<U>(cuda::std::byteswap(in)) == static_cast<U>(expected));
  static_assert(cuda::std::is_same_v<decltype(cuda::std::byteswap(in)), decltype(in)>);
  static_assert(noexcept(cuda::std::byteswap(in)));
}

template <class T>
struct TestData
{
  T in;
  T expected;
};

template <class T>
__host__ __device__ constexpr TestData<T> get_test_data()
{
  switch (sizeof(T))
  {
    case 2:
      return {static_cast<T>(0x1234), static_cast<T>(0x3412)};
    case 4:
      return {static_cast<T>(0x60AF8503), static_cast<T>(0x0385AF60)};
    case 8:
      return {static_cast<T>(0xABCDFE9477936406), static_cast<T>(0x0664937794FECDAB)};
    default:
      assert(false);
      cuda::std::unreachable();
  }
}

template <class T>
__host__ __device__ constexpr void test_implementation_defined_size()
{
  const auto test_data = get_test_data<T>();
  test_num<T>(test_data.in, test_data.expected);
}

__host__ __device__ constexpr bool test()
{
  test_num<cuda::std::uint8_t>(0xAB, 0xAB);
  test_num<cuda::std::uint16_t>(0xCDEF, 0xEFCD);
  test_num<cuda::std::uint32_t>(0x01234567, 0x67452301);
  test_num<cuda::std::uint64_t>(0x0123456789ABCDEF, 0xEFCDAB8967452301);

  test_num<cuda::std::int8_t>(static_cast<cuda::std::int8_t>(0xAB), static_cast<cuda::std::int8_t>(0xAB));
  test_num<cuda::std::int16_t>(static_cast<cuda::std::int16_t>(0xCDEF), static_cast<cuda::std::int16_t>(0xEFCD));
  test_num<cuda::std::int32_t>(0x01234567, 0x67452301);
  // requires static_cast to silence integer conversion resulted in a change of sign warning
  test_num<cuda::std::int64_t>(
    static_cast<cuda::std::int64_t>(0x0123456789ABCDEF), static_cast<cuda::std::int64_t>(0xEFCDAB8967452301));

#if _CCCL_HAS_INT128()
  const auto in       = static_cast<__uint128_t>(0x0123456789ABCDEF) << 64 | 0x13579BDF02468ACE;
  const auto expected = static_cast<__uint128_t>(0xCE8A4602DF9B5713) << 64 | 0xEFCDAB8967452301;
  test_num<__uint128_t>(in, expected);
  test_num<__int128_t>(in, expected);
#endif // _CCCL_HAS_INT128()

  test_num<bool>(true, true);
  test_num<bool>(false, false);
  test_num<char>(static_cast<char>(0xCD), static_cast<char>(0xCD));
  test_num<unsigned char>(0xEF, 0xEF);
  test_num<signed char>(0x45, 0x45);
#if TEST_STD_VER >= 2020
  test_num<char8_t>(0xAB, 0xAB);
#endif // TEST_STD_VER >= 2020
  test_num<char16_t>(0xABCD, 0xCDAB);
  test_num<char32_t>(0xABCDEF01, 0x01EFCDAB);
  test_implementation_defined_size<wchar_t>();

  test_implementation_defined_size<short>();
  test_implementation_defined_size<unsigned short>();
  test_implementation_defined_size<int>();
  test_implementation_defined_size<unsigned int>();
  test_implementation_defined_size<long>();
  test_implementation_defined_size<unsigned long>();
  test_implementation_defined_size<long long>();
  test_implementation_defined_size<unsigned long long>();
  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
