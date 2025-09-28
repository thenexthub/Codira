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

#include <uscl/std/__memory_>
#if defined(_LIBCUDACXX_HAS_STRING)
#  include <cuda/std/string>
#endif // _LIBCUDACXX_HAS_STRING
#include <uscl/std/cassert>

#include "test_macros.h"

//    The only way to create an unique_ptr<T[]> is to default construct them.

class foo
{
public:
  __host__ __device__ TEST_CONSTEXPR_CXX23 foo()
      : val_(3)
  {}
  __host__ __device__ TEST_CONSTEXPR_CXX23 int get() const
  {
    return val_;
  }

private:
  int val_;
};

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  {
    auto p1 = cuda::std::make_unique<int[]>(5);
    for (int i = 0; i < 5; ++i)
    {
      assert(p1[i] == 0);
    }
  }

#if defined(_LIBCUDACXX_HAS_STRING)
  {
    auto p2 = cuda::std::make_unique<cuda::std::string[]>(5);
    for (int i = 0; i < 5; ++i)
    {
      assert(p2[i].size() == 0);
    }
  }
#endif // _LIBCUDACXX_HAS_STRING

  {
    auto p3 = cuda::std::make_unique<foo[]>(7);
    for (int i = 0; i < 7; ++i)
    {
      assert(p3[i].get() == 3);
    }
  }

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
