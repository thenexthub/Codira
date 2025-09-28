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

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  {
    cuda::std::unique_ptr<int> p1 = cuda::std::make_unique<int>(1);
    assert(*p1 == 1);
    p1 = cuda::std::make_unique<int>();
    assert(*p1 == 0);
  }

#if defined(_LIBCUDACXX_HAS_STRING)
  {
    cuda::std::unique_ptr<cuda::std::string> p2 = cuda::std::make_unique<cuda::std::string>("Meow!");
    assert(*p2 == "Meow!");
    p2 = cuda::std::make_unique<cuda::std::string>();
    assert(*p2 == "");
    p2 = cuda::std::make_unique<cuda::std::string>(6, 'z');
    assert(*p2 == "zzzzzz");
  }
#endif // _LIBCUDACXX_HAS_STRING

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
