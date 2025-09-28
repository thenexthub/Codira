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

//=============================================================================
// TESTING cuda::std::unique_ptr::unique_ptr(pointer)
//
// Concerns:
//   1 The pointer constructor works for any default constructible deleter types.
//   2 The pointer constructor accepts pointers to derived types.
//   2 The stored type 'T' is allowed to be incomplete.
//
// Plan
//  1 Construct unique_ptr<T, D>'s with a pointer to 'T' and various deleter
//   types (C-1)
//  2 Construct unique_ptr<T, D>'s with a pointer to 'D' and various deleter
//    types where 'D' is derived from 'T'. (C-1,2)
//  3 Construct a unique_ptr<T, D> with a pointer to 'T' and various deleter
//    types where 'T' is an incomplete type (C-1,3)

// Test unique_ptr(pointer) ctor

#include <uscl/std/__memory_>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"

// unique_ptr(pointer) ctor should only require default Deleter ctor

template <bool IsArray>
__host__ __device__ TEST_CONSTEXPR_CXX23 void test_pointer()
{
  typedef typename cuda::std::conditional<!IsArray, A, A[]>::type ValueT;
  const int expect_alive = IsArray ? 5 : 1;
  {
    using U1 = cuda::std::unique_ptr<ValueT>;
    using U2 = cuda::std::unique_ptr<ValueT, Deleter<ValueT>>;

    // Test for noexcept
    static_assert(cuda::std::is_nothrow_constructible<U1, A*>::value, "");
    static_assert(cuda::std::is_nothrow_constructible<U2, A*>::value, "");

    // Test for explicit
    static_assert(!cuda::std::is_convertible<A*, U1>::value, "");
    static_assert(!cuda::std::is_convertible<A*, U2>::value, "");
  }
  {
    A* p = newValue<ValueT>(expect_alive);
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert(A_count == expect_alive);
    }

    cuda::std::unique_ptr<ValueT> s(p);
    assert(s.get() == p);
  }
  if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
  {
    assert(A_count == 0);
  }
  {
    A* p = newValue<ValueT>(expect_alive);
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert(A_count == expect_alive);
    }

    cuda::std::unique_ptr<ValueT, NCDeleter<ValueT>> s(p);
    assert(s.get() == p);
    assert(s.get_deleter().state() == 0);
  }
  if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
  {
    assert(A_count == 0);
  }
  {
    A* p = newValue<ValueT>(expect_alive);
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert(A_count == expect_alive);
    }

    cuda::std::unique_ptr<ValueT, DefaultCtorDeleter<ValueT>> s(p);
    assert(s.get() == p);
    assert(s.get_deleter().state() == 0);
  }
  if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
  {
    assert(A_count == 0);
  }
}

__host__ __device__ TEST_CONSTEXPR_CXX23 void test_derived()
{
  {
    B* p = new B;
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert(A_count == 1);
      assert(B_count == 1);
    }
    cuda::std::unique_ptr<A> s(p);
    assert(s.get() == p);
  }
  if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
  {
    assert(A_count == 0);
    assert(B_count == 0);
  }
  {
    B* p = new B;
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert(A_count == 1);
      assert(B_count == 1);
    }
    cuda::std::unique_ptr<A, NCDeleter<A>> s(p);
    assert(s.get() == p);
    assert(s.get_deleter().state() == 0);
  }
  if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
  {
    assert(A_count == 0);
    assert(B_count == 0);
  }
}

struct NonDefaultDeleter
{
  __host__ __device__ NonDefaultDeleter() = delete;
  __host__ __device__ void operator()(void*) const {}
};

struct GenericDeleter
{
  __host__ __device__ void operator()(void*) const;
};

template <class T>
__host__ __device__ void TEST_CONSTEXPR_CXX23 test_sfinae()
{
  { // the constructor does not participate in overload resolution when
    // the deleter is a pointer type
    using U = cuda::std::unique_ptr<T, void (*)(void*)>;
    static_assert(!cuda::std::is_constructible<U, T*>::value, "");
  }
  { // the constructor does not participate in overload resolution when
    // the deleter is not default constructible
    using Del = CDeleter<T>;
    using U1  = cuda::std::unique_ptr<T, NonDefaultDeleter>;
    using U2  = cuda::std::unique_ptr<T, Del&>;
    using U3  = cuda::std::unique_ptr<T, Del const&>;
    static_assert(!cuda::std::is_constructible<U1, T*>::value, "");
    static_assert(!cuda::std::is_constructible<U2, T*>::value, "");
    static_assert(!cuda::std::is_constructible<U3, T*>::value, "");
  }
}

__host__ __device__ static TEST_CONSTEXPR_CXX23 void test_sfinae_runtime()
{
  { // the constructor does not participate in overload resolution when
    // a base <-> derived conversion would occur.
    using UA  = cuda::std::unique_ptr<A[]>;
    using UAD = cuda::std::unique_ptr<A[], GenericDeleter>;
    using UAC = cuda::std::unique_ptr<const A[]>;
    using UB  = cuda::std::unique_ptr<B[]>;
    using UBD = cuda::std::unique_ptr<B[], GenericDeleter>;
    using UBC = cuda::std::unique_ptr<const B[]>;

    static_assert(!cuda::std::is_constructible<UA, B*>::value, "");
    static_assert(!cuda::std::is_constructible<UB, A*>::value, "");
    static_assert(!cuda::std::is_constructible<UAD, B*>::value, "");
    static_assert(!cuda::std::is_constructible<UBD, A*>::value, "");
    static_assert(!cuda::std::is_constructible<UAC, const B*>::value, "");
    static_assert(!cuda::std::is_constructible<UBC, const A*>::value, "");
  }
}

#if !_CCCL_CUDA_COMPILATION()
DEFINE_AND_RUN_IS_INCOMPLETE_TEST({
  {
    doIncompleteTypeTest(1, getNewIncomplete());
  }
  checkNumIncompleteTypeAlive(0);
  {
    doIncompleteTypeTest<IncompleteType, NCDeleter<IncompleteType>>(1, getNewIncomplete());
  }
  checkNumIncompleteTypeAlive(0);
})
#endif // !_CCCL_CUDA_COMPILATION()

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  {
    test_pointer</*IsArray*/ false>();
    test_derived();
    test_sfinae<int>();
  }
  {
    test_pointer</*IsArray*/ true>();
    test_sfinae<int[]>();
    test_sfinae_runtime();
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
