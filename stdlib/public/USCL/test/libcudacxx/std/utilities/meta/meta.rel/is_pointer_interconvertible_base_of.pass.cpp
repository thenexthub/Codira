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

struct BaseA
{};

struct BaseB
{};

struct _CCCL_DECLSPEC_EMPTY_BASES A : BaseA
{};

struct _CCCL_DECLSPEC_EMPTY_BASES B : BaseB
{};

struct _CCCL_DECLSPEC_EMPTY_BASES C
    : A
    , B
{
  float m;
};

struct _CCCL_DECLSPEC_EMPTY_BASES NonStandard
    : BaseA
    , BaseB
{
  TEST_NVRTC_VIRTUAL_DEFAULT_DTOR_ANNOTATION virtual ~NonStandard() = default;

  int m;
};

template <class T, class U>
__host__ __device__ constexpr void test_is_pointer_interconvertible_base_of(bool expected)
{
#if defined(_CCCL_BUILTIN_IS_POINTER_INTERCONVERTIBLE_BASE_OF)
  assert((cuda::std::is_pointer_interconvertible_base_of<T, U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<T, const U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<T, volatile U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<T, const volatile U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<const T, U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<const T, const U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<const T, volatile U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<const T, const volatile U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<volatile T, U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<volatile T, const U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<volatile T, volatile U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<volatile T, const volatile U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<const volatile T, U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<const volatile T, const U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<const volatile T, volatile U>::value == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of<const volatile T, const volatile U>::value == expected));

  assert((cuda::std::is_pointer_interconvertible_base_of_v<T, U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<T, const U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<T, volatile U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<T, const volatile U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<const T, U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<const T, const U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<const T, volatile U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<const T, const volatile U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<volatile T, U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<volatile T, const U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<volatile T, volatile U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<volatile T, const volatile U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<const volatile T, U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<const volatile T, const U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<const volatile T, volatile U> == expected));
  assert((cuda::std::is_pointer_interconvertible_base_of_v<const volatile T, const volatile U> == expected));
#endif // _CCCL_BUILTIN_IS_POINTER_INTERCONVERTIBLE_BASE_OF
}

__host__ __device__ constexpr bool test()
{
  // 1. Structs have pointer-interconvertible
  test_is_pointer_interconvertible_base_of<BaseA, BaseA>(true);
  test_is_pointer_interconvertible_base_of<BaseB, BaseB>(true);
  test_is_pointer_interconvertible_base_of<A, A>(true);
  test_is_pointer_interconvertible_base_of<B, B>(true);
  test_is_pointer_interconvertible_base_of<C, C>(true);

  // 2. Test derived classes to be pointer-interconvertible with base classes
  test_is_pointer_interconvertible_base_of<BaseA, A>(true);
  test_is_pointer_interconvertible_base_of<BaseB, B>(true);
  test_is_pointer_interconvertible_base_of<BaseA, C>(true);
  test_is_pointer_interconvertible_base_of<BaseB, C>(true);
  test_is_pointer_interconvertible_base_of<A, C>(true);
  test_is_pointer_interconvertible_base_of<B, C>(true);

  // 3. Test combinations returning false
  test_is_pointer_interconvertible_base_of<A, B>(false);
  test_is_pointer_interconvertible_base_of<B, A>(false);
  test_is_pointer_interconvertible_base_of<C, NonStandard>(false);
  test_is_pointer_interconvertible_base_of<NonStandard, C>(false);
  test_is_pointer_interconvertible_base_of<int, int>(false);
  test_is_pointer_interconvertible_base_of<int, A>(false);

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
