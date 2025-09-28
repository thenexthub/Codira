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

// Test swap

#include <uscl/std/__memory_>
#include <uscl/std/cassert>

#include "deleter_types.h"
#include "test_macros.h"

#if TEST_CUDA_COMPILER(NVCC) || TEST_COMPILER(NVRTC)
TEST_NV_DIAG_SUPPRESS(3060) // call to __builtin_is_constant_evaluated appearing in a non-constexpr function
#endif // TEST_CUDA_COMPILER(NVCC) || TEST_COMPILER(NVRTC)
TEST_DIAG_SUPPRESS_GCC("-Wtautological-compare")
TEST_DIAG_SUPPRESS_CLANG("-Wtautological-compare")

TEST_GLOBAL_VARIABLE int A_count = 0;

struct A
{
  int state_;
  __host__ __device__ TEST_CONSTEXPR_CXX23 A()
      : state_(0)
  {
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      ++A_count;
    }
  }
  __host__ __device__ TEST_CONSTEXPR_CXX23 explicit A(int i)
      : state_(i)
  {
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      ++A_count;
    }
  }
  __host__ __device__ TEST_CONSTEXPR_CXX23 A(const A& a)
      : state_(a.state_)
  {
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      ++A_count;
    }
  }
  __host__ __device__ TEST_CONSTEXPR_CXX23 A& operator=(const A& a)
  {
    state_ = a.state_;
    return *this;
  }
  __host__ __device__ TEST_CONSTEXPR_CXX23 ~A()
  {
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      --A_count;
    }
  }

  __host__ __device__ friend TEST_CONSTEXPR_CXX23 bool operator==(const A& x, const A& y)
  {
    return x.state_ == y.state_;
  }
};

template <class T>
struct NonSwappableDeleter
{
  __host__ __device__ TEST_CONSTEXPR_CXX23 explicit NonSwappableDeleter(int) {}
  __host__ __device__ TEST_CONSTEXPR_CXX23 NonSwappableDeleter& operator=(NonSwappableDeleter const&)
  {
    return *this;
  }
  __host__ __device__ TEST_CONSTEXPR_CXX23 void operator()(T*) const {}

private:
  __host__ __device__ NonSwappableDeleter(NonSwappableDeleter const&);
};

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  {
    A* p1 = new A(1);
    cuda::std::unique_ptr<A, Deleter<A>> s1(p1, Deleter<A>(1));
    A* p2 = new A(2);
    cuda::std::unique_ptr<A, Deleter<A>> s2(p2, Deleter<A>(2));
    assert(s1.get() == p1);
    assert(*s1 == A(1));
    assert(s1.get_deleter().state() == 1);
    assert(s2.get() == p2);
    assert(*s2 == A(2));
    assert(s2.get_deleter().state() == 2);
    swap(s1, s2);
    assert(s1.get() == p2);
    assert(*s1 == A(2));
    assert(s1.get_deleter().state() == 2);
    assert(s2.get() == p1);
    assert(*s2 == A(1));
    assert(s2.get_deleter().state() == 1);
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert(A_count == 2);
    }
  }
  if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
  {
    assert(A_count == 0);
  }
  {
    A* p1 = new A[3];
    cuda::std::unique_ptr<A[], Deleter<A[]>> s1(p1, Deleter<A[]>(1));
    A* p2 = new A[3];
    cuda::std::unique_ptr<A[], Deleter<A[]>> s2(p2, Deleter<A[]>(2));
    assert(s1.get() == p1);
    assert(s1.get_deleter().state() == 1);
    assert(s2.get() == p2);
    assert(s2.get_deleter().state() == 2);
    swap(s1, s2);
    assert(s1.get() == p2);
    assert(s1.get_deleter().state() == 2);
    assert(s2.get() == p1);
    assert(s2.get_deleter().state() == 1);
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert(A_count == 6);
    }
  }
  if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
  {
    assert(A_count == 0);
  }
  {
    // test that unique_ptr's specialized swap is disabled when the deleter
    // is non-swappable. Instead we should pick up the generic swap(T, T)
    // and perform 3 move constructions.
    typedef NonSwappableDeleter<int> D;
    D d(42);
    int x = 42;
    int y = 43;
    cuda::std::unique_ptr<int, D&> p(&x, d);
    cuda::std::unique_ptr<int, D&> p2(&y, d);
    cuda::std::swap(p, p2);
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
