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

// The deleter is not called if get() == 0

#include <uscl/std/__memory_>
#include <uscl/std/cassert>

#include "test_macros.h"

class Deleter
{
  int state_;

  __host__ __device__ Deleter(Deleter&);
  __host__ __device__ Deleter& operator=(Deleter&);

public:
  __host__ __device__ TEST_CONSTEXPR_CXX23 Deleter()
      : state_(0)
  {}

  __host__ __device__ TEST_CONSTEXPR_CXX23 int state() const
  {
    return state_;
  }

  __host__ __device__ TEST_CONSTEXPR_CXX23 void operator()(void*)
  {
    ++state_;
  }
};

template <class T>
__host__ __device__ TEST_CONSTEXPR_CXX23 void test_basic()
{
  Deleter d;
  assert(d.state() == 0);
  {
    cuda::std::unique_ptr<T, Deleter&> p(nullptr, d);
    assert(p.get() == nullptr);
    assert(&p.get_deleter() == &d);
  }
  assert(d.state() == 0);
}

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  test_basic<int>();
  test_basic<int[]>();

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
