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
  int ma1;
  unsigned ma2;

  __host__ __device__ void fn() {}
};

struct B
{
  int mb1;
  unsigned mb2;

  __host__ __device__ void fn() {}
};

union U
{
  int mu1;
  unsigned mu2;
};

struct NonStandard
    : A
    , B
{
  TEST_NVRTC_VIRTUAL_DEFAULT_DTOR_ANNOTATION virtual ~NonStandard() = default;

  int mns1;
};

__host__ __device__ constexpr bool test()
{
#if defined(_CCCL_BUILTIN_IS_POINTER_INTERCONVERTIBLE_WITH_CLASS)
  // 1. Only the first member of a class is pointer interconvertible with the class itself
  assert(cuda::std::is_pointer_interconvertible_with_class(&A::ma1));
  assert(cuda::std::is_pointer_interconvertible_with_class(&B::mb1));

  // 2. Rest of the members of a class are not pointer interconvertible with the class itself
  assert(!cuda::std::is_pointer_interconvertible_with_class(&A::ma2));
  assert(!cuda::std::is_pointer_interconvertible_with_class(&B::mb2));

  // 3. All union members are pointer interconvertible with the union itself
  assert(cuda::std::is_pointer_interconvertible_with_class(&U::mu1));
  assert(cuda::std::is_pointer_interconvertible_with_class(&U::mu2));

  // 4. Non-standard layout class members are not pointer interconvertible with the class itself
  assert(!cuda::std::is_pointer_interconvertible_with_class(&NonStandard::mns1));

  // 5. Member functions are not pointer interconvertible with the class itself
  assert(!cuda::std::is_pointer_interconvertible_with_class(&A::fn));
  assert(!cuda::std::is_pointer_interconvertible_with_class(&B::fn));

  // 7. is_pointer_interconvertible_with_class always returns false for nullptr
  assert(!cuda::std::is_pointer_interconvertible_with_class(static_cast<int A::*>(nullptr)));
  assert(!cuda::std::is_pointer_interconvertible_with_class(static_cast<int B::*>(nullptr)));
#endif // _CCCL_BUILTIN_IS_POINTER_INTERCONVERTIBLE_WITH_CLASS

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
