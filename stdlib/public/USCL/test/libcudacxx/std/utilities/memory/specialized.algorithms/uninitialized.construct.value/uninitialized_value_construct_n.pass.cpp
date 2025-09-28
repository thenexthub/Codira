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

// template <class ForwardIt>
// void uninitialized_value_construct(ForwardIt, ForwardIt);

#include <uscl/std/cassert>
#include <uscl/std/cstdlib>
#include <uscl/std/memory>

#include "test_iterators.h"
#include "test_macros.h"

TEST_GLOBAL_VARIABLE int Counted_count       = 0;
TEST_GLOBAL_VARIABLE int Counted_constructed = 0;
struct Counted
{
  __host__ __device__ static void reset()
  {
    Counted_count = Counted_constructed = 0;
  }
  __host__ __device__ explicit Counted()
  {
    ++Counted_count;
    ++Counted_constructed;
  }
  __host__ __device__ Counted(Counted const&)
  {
    assert(false);
  }
  __host__ __device__ ~Counted()
  {
    assert(Counted_count > 0);
    --Counted_count;
  }
  __host__ __device__ friend void operator&(Counted) = delete;
};

#if TEST_HAS_EXCEPTIONS()
static int ThrowsCounted_count       = 0;
static int ThrowsCounted_constructed = 0;
static int ThrowsCounted_throw_after = 0;
struct ThrowsCounted
{
  static void reset()
  {
    ThrowsCounted_count = ThrowsCounted_constructed = ThrowsCounted_throw_after = 0;
  }
  explicit ThrowsCounted()
  {
    ++ThrowsCounted_constructed;
    if (ThrowsCounted_throw_after > 0 && --ThrowsCounted_throw_after == 0)
    {
      TEST_THROW(1);
    }
    ++ThrowsCounted_count;
  }
  ThrowsCounted(ThrowsCounted const&)
  {
    assert(false);
  }
  ~ThrowsCounted()
  {
    assert(ThrowsCounted_count > 0);
    --ThrowsCounted_count;
  }
  friend void operator&(ThrowsCounted) = delete;
};

void test_ctor_throws()
{
  using It                                                    = forward_iterator<ThrowsCounted*>;
  const int N                                                 = 5;
  alignas(ThrowsCounted) char pool[sizeof(ThrowsCounted) * N] = {};
  ThrowsCounted* p                                            = (ThrowsCounted*) pool;
  try
  {
    ThrowsCounted_throw_after = 4;
    cuda::std::uninitialized_value_construct_n(It(p), N);
    assert(false);
  }
  catch (...)
  {}
  assert(ThrowsCounted_count == 0);
  assert(ThrowsCounted_constructed == 4); // forth construction throws
}
#endif // TEST_HAS_EXCEPTIONS()

__host__ __device__ void test_counted()
{
  using It                                        = forward_iterator<Counted*>;
  const int N                                     = 5;
  alignas(Counted) char pool[sizeof(Counted) * N] = {};
  Counted* p                                      = (Counted*) pool;
  It e                                            = cuda::std::uninitialized_value_construct_n(It(p), 1);
  assert(e == It(p + 1));
  assert(Counted_count == 1);
  assert(Counted_constructed == 1);
  e = cuda::std::uninitialized_value_construct_n(It(p + 1), 4);
  assert(e == It(p + N));
  assert(Counted_count == 5);
  assert(Counted_constructed == 5);
  cuda::std::__destroy(p, p + N);
  assert(Counted_count == 0);
}

__host__ __device__ void test_value_initialized()
{
  using It    = forward_iterator<int*>;
  const int N = 5;
  int pool[N] = {-1, -1, -1, -1, -1};
  int* p      = pool;
  It e        = cuda::std::uninitialized_value_construct_n(It(p), 1);
  assert(e == It(p + 1));
  assert(pool[0] == 0);
  assert(pool[1] == -1);
  e = cuda::std::uninitialized_value_construct_n(It(p + 1), 4);
  assert(e == It(p + N));
  assert(pool[1] == 0);
  assert(pool[2] == 0);
  assert(pool[3] == 0);
  assert(pool[4] == 0);
}

int main(int, char**)
{
  test_counted();
  test_value_initialized();

#if TEST_HAS_EXCEPTIONS()
  NV_IF_TARGET(NV_IS_HOST, (test_ctor_throws();))
#endif // TEST_HAS_EXCEPTIONS()

  return 0;
}
