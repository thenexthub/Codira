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
// <memory>

// unique_ptr

// Test unique_ptr::pointer type

#include <uscl/std/__memory_>
#include <uscl/std/type_traits>

#include "test_macros.h"

struct Deleter
{
  struct pointer
  {};
};

#if !TEST_COMPILER(GCC) && !TEST_COMPILER(MSVC)
struct D2
{
private:
  typedef void pointer;
};
#endif // !TEST_COMPILER(GCC) && !TEST_COMPILER(MSVC)

#if !TEST_COMPILER(NVRTC) // A class static data member with non-const type is considered a host variable
struct D3
{
  static long pointer;
};
#endif // !TEST_COMPILER(NVRTC)

template <bool IsArray>
__host__ __device__ TEST_CONSTEXPR_CXX23 void test_basic()
{
  typedef typename cuda::std::conditional<IsArray, int[], int>::type VT;
  {
    typedef cuda::std::unique_ptr<VT> P;
    static_assert((cuda::std::is_same<typename P::pointer, int*>::value), "");
  }
  {
    typedef cuda::std::unique_ptr<VT, Deleter> P;
    static_assert((cuda::std::is_same<typename P::pointer, Deleter::pointer>::value), "");
  }
#if !TEST_COMPILER(GCC) && !TEST_COMPILER(MSVC)
  {
    typedef cuda::std::unique_ptr<VT, D2> P;
    static_assert(cuda::std::is_same<typename P::pointer, int*>::value, "");
  }
#endif // !TEST_COMPILER(GCC) && !TEST_COMPILER(MSVC)
#if !TEST_COMPILER(NVRTC)
  {
    typedef cuda::std::unique_ptr<VT, D3> P;
    static_assert(cuda::std::is_same<typename P::pointer, int*>::value, "");
  }
#endif // !TEST_COMPILER(NVRTC)
}

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  test_basic</*IsArray*/ false>();
  test_basic<true>();

  return true;
}

int main(int, char**)
{
  test();
#if TEST_STD_VER >= 2023
  static_assert(test());
#endif // TEST_STD_VER >= 2023

  return 0;
}
