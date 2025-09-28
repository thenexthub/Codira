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

// test op[](size_t)

#include <uscl/std/__memory_>
#include <uscl/std/cassert>
#include <uscl/std/type_traits>

// TODO: Move TEST_IS_CONSTANT_EVALUATED_CXX23() into it's own header
#include "test_macros.h"

#if TEST_CUDA_COMPILER(NVCC) || TEST_COMPILER(NVRTC)
TEST_NV_DIAG_SUPPRESS(3060) // call to __builtin_is_constant_evaluated appearing in a non-constexpr function
#endif // TEST_CUDA_COMPILER(NVCC) || TEST_COMPILER(NVRTC)
TEST_DIAG_SUPPRESS_GCC("-Wtautological-compare")
TEST_DIAG_SUPPRESS_CLANG("-Wtautological-compare")

TEST_GLOBAL_VARIABLE int A_next_ = 0;
class A
{
  int state_;

public:
  __host__ __device__ TEST_CONSTEXPR_CXX23 A()
      : state_(0)
  {
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      state_ = ++A_next_;
    }
  }

  __host__ __device__ TEST_CONSTEXPR_CXX23 int get() const
  {
    return state_;
  }

  __host__ __device__ friend TEST_CONSTEXPR_CXX23 bool operator==(const A& x, int y)
  {
    return x.state_ == y;
  }

  __host__ __device__ TEST_CONSTEXPR_CXX23 A& operator=(int i)
  {
    state_ = i;
    return *this;
  }
};

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  cuda::std::unique_ptr<A[]> p(new A[3]);
  if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
  {
    assert(p[0] == 1);
    assert(p[1] == 2);
    assert(p[2] == 3);
  }
  p[0] = 3;
  p[1] = 2;
  p[2] = 1;
  assert(p[0] == 3);
  assert(p[1] == 2);
  assert(p[2] == 1);

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
