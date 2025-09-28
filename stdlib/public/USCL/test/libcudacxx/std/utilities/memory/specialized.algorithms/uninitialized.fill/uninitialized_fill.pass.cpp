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

// template <class ForwardIterator, class T>
//   void
//   uninitialized_fill(ForwardIterator first, ForwardIterator last,
//                      const T& x);

#include <uscl/std/cassert>
#include <uscl/std/memory>

#include "test_macros.h"

TEST_GLOBAL_VARIABLE int Nasty_count = 0;
struct Nasty
{
  __host__ __device__ Nasty()
      : i_(Nasty_count++)
  {}
  __host__ __device__ Nasty* operator&() const
  {
    return nullptr;
  }
  int i_;
};

#if TEST_HAS_EXCEPTIONS()
static int B_count      = 0;
static int B_population = 0;
struct B
{
  int data_;
  explicit B()
      : data_(1)
  {
    ++B_population;
  }
  B(const B& b)
  {
    ++B_count;
    if (B_count == 3)
    {
      TEST_THROW(1);
    }
    data_ = b.data_;
    ++B_population;
  }
  ~B()
  {
    data_ = 0;
    --B_population;
  }
};

void test_exceptions()
{
  const int N              = 5;
  char pool[sizeof(B) * N] = {0};
  B* bp                    = (B*) pool;
  assert(B_population == 0);
  try
  {
    cuda::std::uninitialized_fill(bp, bp + N, B());
    assert(false);
  }
  catch (...)
  {
    assert(B_population == 0);
  }
  B_count = 0;
  cuda::std::uninitialized_fill(bp, bp + 2, B());
  for (int i = 0; i < 2; ++i)
  {
    assert(bp[i].data_ == 1);
  }
  assert(B_population == 2);
}
#endif // TEST_HAS_EXCEPTIONS()

int main(int, char**)
{
  {
    const int N                  = 5;
    char pool[N * sizeof(Nasty)] = {0};
    Nasty* bp                    = (Nasty*) pool;

    Nasty_count = 23;
    cuda::std::uninitialized_fill(bp, bp + N, Nasty());
    for (int i = 0; i < N; ++i)
    {
      assert(bp[i].i_ == 23);
    }
  }
#if TEST_HAS_EXCEPTIONS()
  NV_IF_TARGET(NV_IS_HOST, (test_exceptions();))
#endif // TEST_HAS_EXCEPTIONS()

  return 0;
}
