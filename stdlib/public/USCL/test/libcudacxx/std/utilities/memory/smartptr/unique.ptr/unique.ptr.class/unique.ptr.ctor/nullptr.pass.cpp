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

// constexpr unique_ptr(nullptr_t);  // constexpr since C++23

#include <uscl/std/__memory_>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"

#if !TEST_COMPILER(NVRTC) // no dynamic initialization
_CCCL_CONSTINIT cuda::std::unique_ptr<int> global_static_unique_ptr_single(nullptr);
_CCCL_CONSTINIT cuda::std::unique_ptr<int[]> global_static_unique_ptr_runtime(nullptr);
#endif // TEST_COMPILER(NVRTC)

struct NonDefaultDeleter
{
  NonDefaultDeleter() = delete;
  __host__ __device__ void operator()(void*) const {}
};

template <class VT>
__host__ __device__ TEST_CONSTEXPR_CXX23 void test_basic()
{
  {
    using U1 = cuda::std::unique_ptr<VT>;
    using U2 = cuda::std::unique_ptr<VT, Deleter<VT>>;
    static_assert(cuda::std::is_nothrow_constructible<U1, decltype(nullptr)>::value, "");
    static_assert(cuda::std::is_nothrow_constructible<U2, decltype(nullptr)>::value, "");
  }
  {
    cuda::std::unique_ptr<VT> p(nullptr);
    assert(p.get() == 0);
  }
  {
    cuda::std::unique_ptr<VT, NCDeleter<VT>> p(nullptr);
    assert(p.get() == 0);
    assert(p.get_deleter().state() == 0);
  }
  {
    cuda::std::unique_ptr<VT, DefaultCtorDeleter<VT>> p(nullptr);
    assert(p.get() == 0);
    assert(p.get_deleter().state() == 0);
  }
}

template <class VT>
__host__ __device__ TEST_CONSTEXPR_CXX23 void test_sfinae()
{
  { // the constructor does not participate in overload resolution when
    // the deleter is a pointer type
    using U = cuda::std::unique_ptr<VT, void (*)(void*)>;
    static_assert(!cuda::std::is_constructible<U, decltype(nullptr)>::value, "");
  }
  { // the constructor does not participate in overload resolution when
    // the deleter is not default constructible
    using Del = CDeleter<VT>;
    using U1  = cuda::std::unique_ptr<VT, NonDefaultDeleter>;
    using U2  = cuda::std::unique_ptr<VT, Del&>;
    using U3  = cuda::std::unique_ptr<VT, Del const&>;
    static_assert(!cuda::std::is_constructible<U1, decltype(nullptr)>::value, "");
    static_assert(!cuda::std::is_constructible<U2, decltype(nullptr)>::value, "");
    static_assert(!cuda::std::is_constructible<U3, decltype(nullptr)>::value, "");
  }
}

#if !_CCCL_CUDA_COMPILATION()
DEFINE_AND_RUN_IS_INCOMPLETE_TEST({
  {
    doIncompleteTypeTest(0, nullptr);
  }
  checkNumIncompleteTypeAlive(0);
  {
    doIncompleteTypeTest<IncompleteType, NCDeleter<IncompleteType>>(0, nullptr);
  }
  checkNumIncompleteTypeAlive(0);
  {
    doIncompleteTypeTest<IncompleteType[]>(0, nullptr);
  }
  checkNumIncompleteTypeAlive(0);
  {
    doIncompleteTypeTest<IncompleteType[], NCDeleter<IncompleteType[]>>(0, nullptr);
  }
  checkNumIncompleteTypeAlive(0);
})
#endif // !_CCCL_CUDA_COMPILATION()

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  {
    test_basic<int>();
    test_sfinae<int>();
  }
  {
    test_basic<int[]>();
    test_sfinae<int[]>();
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
