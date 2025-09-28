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

// test get

#include <uscl/std/__memory_>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"

template <bool IsArray>
__host__ __device__ TEST_CONSTEXPR_CXX23 void test_basic()
{
  typedef typename cuda::std::conditional<IsArray, int[], int>::type VT;
  typedef const VT CVT;
  {
    typedef cuda::std::unique_ptr<VT> U;
    int* p = newValue<VT>(1);
    U s(p);
    U const& sc = s;
    static_assert(cuda::std::is_same_v<decltype(s.get()), int*>);
    static_assert(cuda::std::is_same_v<decltype(sc.get()), int*>);
    assert(s.get() == p);
    assert(sc.get() == s.get());
  }
  {
    typedef cuda::std::unique_ptr<CVT> U;
    const int* p = newValue<VT>(1);
    U s(p);
    U const& sc = s;
    static_assert(cuda::std::is_same_v<decltype(s.get()), const int*>);
    static_assert(cuda::std::is_same_v<decltype(sc.get()), const int*>);
    assert(s.get() == p);
    assert(sc.get() == s.get());
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
