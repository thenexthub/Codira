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
// This code triggers https://gcc.gnu.org/bugzilla/show_bug.cgi?id=104568
// UNSUPPORTED: msvc
// UNSUPPORTED: nvrtc
// UNSUPPORTED: nvhpc

// Test the fix for https://llvm.org/PR54100

#include <uscl/std/__memory_>
#include <uscl/std/cassert>

#include "test_macros.h"

struct A
{
  int m[0];
};
static_assert(sizeof(A) == 0, ""); // an extension supported by GCC and Clang

int main(int, char**)
{
  {
    cuda::std::unique_ptr<A> p = cuda::std::unique_ptr<A>(new A);
    assert(p != nullptr);
  }
  {
    cuda::std::unique_ptr<A[]> p = cuda::std::unique_ptr<A[]>(new A[1]);
    assert(p != nullptr);
  }
  {
    cuda::std::unique_ptr<A> p = cuda::std::make_unique<A>();
    assert(p != nullptr);
  }
  {
    cuda::std::unique_ptr<A[]> p = cuda::std::make_unique<A[]>(1);
    assert(p != nullptr);
  }

  return 0;
}
