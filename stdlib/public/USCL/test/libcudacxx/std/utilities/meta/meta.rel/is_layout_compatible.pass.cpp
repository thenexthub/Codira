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

#include <uscl/std/cassert>
#include <uscl/std/type_traits>

struct A
{
  int m1;
  unsigned m2;
  float m3;
  double m4;

  __host__ __device__ void fn() {}
};

struct B
{
  int m1;
  unsigned m2;
  float m3;
  double m4;

  __host__ __device__ void fn() {}
};

struct NonStandard
    : A
    , B
{
  TEST_NVRTC_VIRTUAL_DEFAULT_DTOR_ANNOTATION virtual ~NonStandard() = default;

  int m;
};

enum E1 : int
{
};
enum E2 : unsigned
{
};

enum class EC1 : int
{
};
enum class EC2 : unsigned
{
};

template <class T, class U>
__host__ __device__ constexpr void test_is_layout_compatible(bool expected)
{
#if defined(_CCCL_BUILTIN_IS_LAYOUT_COMPATIBLE)
  assert((cuda::std::is_layout_compatible<T, U>::value == expected));
  assert((cuda::std::is_layout_compatible<T, const U>::value == expected));
  assert((cuda::std::is_layout_compatible<T, volatile U>::value == expected));
  assert((cuda::std::is_layout_compatible<T, const volatile U>::value == expected));
  assert((cuda::std::is_layout_compatible<const T, U>::value == expected));
  assert((cuda::std::is_layout_compatible<const T, const U>::value == expected));
  assert((cuda::std::is_layout_compatible<const T, volatile U>::value == expected));
  assert((cuda::std::is_layout_compatible<const T, const volatile U>::value == expected));
  assert((cuda::std::is_layout_compatible<volatile T, U>::value == expected));
  assert((cuda::std::is_layout_compatible<volatile T, const U>::value == expected));
  assert((cuda::std::is_layout_compatible<volatile T, volatile U>::value == expected));
  assert((cuda::std::is_layout_compatible<volatile T, const volatile U>::value == expected));
  assert((cuda::std::is_layout_compatible<const volatile T, U>::value == expected));
  assert((cuda::std::is_layout_compatible<const volatile T, const U>::value == expected));
  assert((cuda::std::is_layout_compatible<const volatile T, volatile U>::value == expected));
  assert((cuda::std::is_layout_compatible<const volatile T, const volatile U>::value == expected));

  assert((cuda::std::is_layout_compatible_v<T, U> == expected));
  assert((cuda::std::is_layout_compatible_v<T, const U> == expected));
  assert((cuda::std::is_layout_compatible_v<T, volatile U> == expected));
  assert((cuda::std::is_layout_compatible_v<T, const volatile U> == expected));
  assert((cuda::std::is_layout_compatible_v<const T, U> == expected));
  assert((cuda::std::is_layout_compatible_v<const T, const U> == expected));
  assert((cuda::std::is_layout_compatible_v<const T, volatile U> == expected));
  assert((cuda::std::is_layout_compatible_v<const T, const volatile U> == expected));
  assert((cuda::std::is_layout_compatible_v<volatile T, U> == expected));
  assert((cuda::std::is_layout_compatible_v<volatile T, const U> == expected));
  assert((cuda::std::is_layout_compatible_v<volatile T, volatile U> == expected));
  assert((cuda::std::is_layout_compatible_v<volatile T, const volatile U> == expected));
  assert((cuda::std::is_layout_compatible_v<const volatile T, U> == expected));
  assert((cuda::std::is_layout_compatible_v<const volatile T, const U> == expected));
  assert((cuda::std::is_layout_compatible_v<const volatile T, volatile U> == expected));
  assert((cuda::std::is_layout_compatible_v<const volatile T, const volatile U> == expected));
#endif // _CCCL_BUILTIN_IS_LAYOUT_COMPATIBLE
}

__host__ __device__ constexpr bool test()
{
  // 1. Signed integer type and its unsigned counterpart are not layout-compatible
  test_is_layout_compatible<int, int>(true);
  test_is_layout_compatible<unsigned, unsigned>(true);
  test_is_layout_compatible<int, unsigned>(false);
  test_is_layout_compatible<unsigned, int>(false);

  // 2. char is layout-compatible with neither signed char nor unsigned char
  test_is_layout_compatible<char, signed char>(false);
  test_is_layout_compatible<char, unsigned char>(false);
  test_is_layout_compatible<signed char, unsigned char>(false);
  test_is_layout_compatible<signed char, char>(false);

  // 3. Similar types are not layout-compatible if they are not the same type
  test_is_layout_compatible<const int* const*, int**>(false);

  // 4. An enumeration type and its underlying type are not layout-compatible
  test_is_layout_compatible<E1, int>(false);
  test_is_layout_compatible<E2, unsigned>(false);
  test_is_layout_compatible<EC1, int>(false);
  test_is_layout_compatible<EC2, unsigned>(false);

  // 5. Enums with same underlying type are layout-compatible
  test_is_layout_compatible<E1, EC1>(true);
  test_is_layout_compatible<E2, EC2>(true);

  // 6. Structs with the same layout are layout-compatible
  test_is_layout_compatible<A, A>(true);
  test_is_layout_compatible<A, B>(true);
  test_is_layout_compatible<B, A>(true);
  test_is_layout_compatible<B, B>(true);

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
