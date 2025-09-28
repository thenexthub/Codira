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

// Test unique_ptr converting move ctor

#include <uscl/std/__memory_>
#include <uscl/std/cassert>

#include "deleter_types.h"
#include "test_macros.h"
#include "unique_ptr_test_helper.h"

template <int ID = 0>
struct GenericDeleter
{
  __host__ __device__ void operator()(void*) const {}
};

template <int ID = 0>
struct GenericConvertingDeleter
{
  template <int OID>
  __host__ __device__ GenericConvertingDeleter(GenericConvertingDeleter<OID>)
  {}
  __host__ __device__ void operator()(void*) const {}
};

__host__ __device__ TEST_CONSTEXPR_CXX23 void test_sfinae()
{
  { // Disallow copying
    using U1 = cuda::std::unique_ptr<A[], GenericConvertingDeleter<0>>;
    using U2 = cuda::std::unique_ptr<A[], GenericConvertingDeleter<1>>;
    static_assert(cuda::std::is_constructible<U1, U2&&>::value, "");
    static_assert(!cuda::std::is_constructible<U1, U2&>::value, "");
    static_assert(!cuda::std::is_constructible<U1, const U2&>::value, "");
    static_assert(!cuda::std::is_constructible<U1, const U2&&>::value, "");
  }
  { // Disallow illegal qualified conversions
    using U1 = cuda::std::unique_ptr<const A[]>;
    using U2 = cuda::std::unique_ptr<A[]>;
    static_assert(cuda::std::is_constructible<U1, U2&&>::value, "");
    static_assert(!cuda::std::is_constructible<U2, U1&&>::value, "");
  }
  { // Disallow base-to-derived conversions.
    using UA = cuda::std::unique_ptr<A[]>;
    using UB = cuda::std::unique_ptr<B[]>;
    static_assert(!cuda::std::is_constructible<UA, UB&&>::value, "");
  }
  { // Disallow base-to-derived conversions.
    using UA = cuda::std::unique_ptr<A[], GenericConvertingDeleter<0>>;
    using UB = cuda::std::unique_ptr<B[], GenericConvertingDeleter<1>>;
    static_assert(!cuda::std::is_constructible<UA, UB&&>::value, "");
  }
  { // Disallow invalid deleter initialization
    using U1 = cuda::std::unique_ptr<A[], GenericDeleter<0>>;
    using U2 = cuda::std::unique_ptr<A[], GenericDeleter<1>>;
    static_assert(!cuda::std::is_constructible<U1, U2&&>::value, "");
  }
  { // Disallow reference deleters with different qualifiers
    using U1 = cuda::std::unique_ptr<A[], Deleter<A[]>&>;
    using U2 = cuda::std::unique_ptr<A[], const Deleter<A[]>&>;
    static_assert(!cuda::std::is_constructible<U1, U2&&>::value, "");
    static_assert(!cuda::std::is_constructible<U2, U1&&>::value, "");
  }
  {
    using U1 = cuda::std::unique_ptr<A[]>;
    using U2 = cuda::std::unique_ptr<A>;
    static_assert(!cuda::std::is_constructible<U1, U2&&>::value, "");
    static_assert(!cuda::std::is_constructible<U2, U1&&>::value, "");
  }
}

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  test_sfinae();

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
