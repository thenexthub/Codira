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

// test get_deleter()

#include <uscl/std/__memory_>
#include <uscl/std/cassert>
#include <uscl/std/type_traits>

#include "test_macros.h"

struct Deleter
{
  __host__ __device__ TEST_CONSTEXPR_CXX23 Deleter() {}

  __host__ __device__ TEST_CONSTEXPR_CXX23 void operator()(void*) const {}

  __host__ __device__ TEST_CONSTEXPR_CXX23 int test()
  {
    return 5;
  }
  __host__ __device__ TEST_CONSTEXPR_CXX23 int test() const
  {
    return 6;
  }
};

template <bool IsArray>
__host__ __device__ TEST_CONSTEXPR_CXX23 void test_basic()
{
  typedef typename cuda::std::conditional<IsArray, int[], int>::type VT;
  {
    cuda::std::unique_ptr<int, Deleter> p;
    assert(p.get_deleter().test() == 5);
  }
  {
    const cuda::std::unique_ptr<VT, Deleter> p;
    assert(p.get_deleter().test() == 6);
  }
  {
    typedef cuda::std::unique_ptr<VT, const Deleter&> UPtr;
    const Deleter d;
    UPtr p(nullptr, d);
    const UPtr& cp = p;
    static_assert(cuda::std::is_same_v<decltype(p.get_deleter()), const Deleter&>);
    static_assert(cuda::std::is_same_v<decltype(cp.get_deleter()), const Deleter&>);
    assert(p.get_deleter().test() == 6);
    assert(cp.get_deleter().test() == 6);
  }
  {
    typedef cuda::std::unique_ptr<VT, Deleter&> UPtr;
    Deleter d;
    UPtr p(nullptr, d);
    const UPtr& cp = p;
    static_assert(cuda::std::is_same_v<decltype(p.get_deleter()), Deleter&>);
    static_assert(cuda::std::is_same_v<decltype(cp.get_deleter()), Deleter&>);
    assert(p.get_deleter().test() == 5);
    assert(cp.get_deleter().test() == 5);
  }
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
