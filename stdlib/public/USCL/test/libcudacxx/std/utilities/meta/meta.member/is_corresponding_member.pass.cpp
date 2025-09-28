/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Sunday, April 14, 2024.
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

#include <uscl/std/cassert>
#include <uscl/std/type_traits>

struct A
{
  int m1;
  unsigned m2;
  float m3;
  double m4;

  __host__ __device__ void fn() {}
};

struct B
{
  int m1;
  unsigned m2;
  float m3;
  double m4;

  __host__ __device__ void fn() {}
};

struct NonStandard
    : A
    , B
{
  TEST_NVRTC_VIRTUAL_DEFAULT_DTOR_ANNOTATION virtual ~NonStandard() = default;

  int m;
};

__host__ __device__ constexpr bool test()
{
#if defined(_CCCL_BUILTIN_IS_CORRESPONDING_MEMBER)
  // 1. Test struct A members to be corresponding with itself
  assert(cuda::std::is_corresponding_member(&A::m1, &A::m1));
  assert(cuda::std::is_corresponding_member(&A::m2, &A::m2));
  assert(cuda::std::is_corresponding_member(&A::m3, &A::m3));
  assert(cuda::std::is_corresponding_member(&A::m4, &A::m4));

  // 2. Test struct A members to be corresponding with struct B members
  assert(cuda::std::is_corresponding_member(&A::m1, &B::m1));
  assert(cuda::std::is_corresponding_member(&A::m2, &B::m2));
  assert(cuda::std::is_corresponding_member(&A::m3, &B::m3));
  assert(cuda::std::is_corresponding_member(&A::m4, &B::m4));

  // 3. Test struct A members not to be corresponding with each other
  assert(!cuda::std::is_corresponding_member(&A::m1, &A::m2));
  assert(!cuda::std::is_corresponding_member(&A::m1, &A::m3));
  assert(!cuda::std::is_corresponding_member(&A::m1, &A::m4));
  assert(!cuda::std::is_corresponding_member(&A::m2, &A::m1));
  assert(!cuda::std::is_corresponding_member(&A::m2, &A::m3));
  assert(!cuda::std::is_corresponding_member(&A::m2, &A::m4));
  assert(!cuda::std::is_corresponding_member(&A::m3, &A::m1));
  assert(!cuda::std::is_corresponding_member(&A::m3, &A::m2));
  assert(!cuda::std::is_corresponding_member(&A::m3, &A::m4));
  assert(!cuda::std::is_corresponding_member(&A::m4, &A::m1));
  assert(!cuda::std::is_corresponding_member(&A::m4, &A::m2));
  assert(!cuda::std::is_corresponding_member(&A::m4, &A::m3));

  // 4. Member functions should not be corresponding
  assert(!cuda::std::is_corresponding_member(&A::fn, &A::fn));

  // 5. If nullptr is passed, it should not be corresponding
  assert(!cuda::std::is_corresponding_member(static_cast<int A::*>(nullptr), static_cast<int A::*>(nullptr)));
  assert(!cuda::std::is_corresponding_member(&A::m1, static_cast<int A::*>(nullptr)));
  assert(!cuda::std::is_corresponding_member(static_cast<int A::*>(nullptr), &A::m1));

  // 6. Non-standard layout types always return false
  assert(!cuda::std::is_corresponding_member(&NonStandard::m, &NonStandard::m));
#endif // _CCCL_BUILTIN_IS_CORRESPONDING_MEMBER

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
