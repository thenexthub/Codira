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

// FIXME(EricWF): This test contains tests for constructing a unique_ptr from NULL.
// The behavior demonstrated in this test is not meant to be standard; It simply
// tests the current status quo in libc++.

#include <uscl/std/__memory_>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"

template <class VT>
__host__ __device__ TEST_CONSTEXPR_CXX23 void test_pointer_ctor()
{
  {
    cuda::std::unique_ptr<VT> p(0);
    assert(p.get() == 0);
  }
  {
    cuda::std::unique_ptr<VT, Deleter<VT>> p(0);
    assert(p.get() == 0);
    assert(p.get_deleter().state() == 0);
  }
}

template <class VT>
__host__ __device__ TEST_CONSTEXPR_CXX23 void test_pointer_deleter_ctor()
{
  {
    cuda::std::default_delete<VT> d;
    cuda::std::unique_ptr<VT> p(0, d);
    assert(p.get() == 0);
  }
  {
    cuda::std::unique_ptr<VT, Deleter<VT>> p(0, Deleter<VT>(5));
    assert(p.get() == 0);
    assert(p.get_deleter().state() == 5);
  }
  {
    NCDeleter<VT> d(5);
    cuda::std::unique_ptr<VT, NCDeleter<VT>&> p(0, d);
    assert(p.get() == 0);
    assert(p.get_deleter().state() == 5);
  }
  {
    NCConstDeleter<VT> d(5);
    cuda::std::unique_ptr<VT, NCConstDeleter<VT> const&> p(0, d);
    assert(p.get() == 0);
    assert(p.get_deleter().state() == 5);
  }
}

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  {
    // test_pointer_ctor<int>();
    test_pointer_deleter_ctor<int>();
  }
  {
    test_pointer_ctor<int[]>();
    test_pointer_deleter_ctor<int[]>();
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
