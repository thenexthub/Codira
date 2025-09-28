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
#include <uscl/std/ctime>
#include <uscl/std/type_traits>
#include <uscl/std/utility>

// Undefine macros that conflict with the tested symbols

#if defined(clock)
#  undef clock
#endif // clock

#if defined(difftime)
#  undef difftime
#endif // difftime

#if defined(time)
#  undef time
#endif // time

#if defined(timespec_get)
#  undef timespec_get
#endif // timespec_get

#ifndef TIME_UTC
#  error TIME_UTC not defined
#endif

static_assert(TIME_UTC != 0);

__host__ __device__ bool test()
{
  // struct timespec

  {
    cuda::std::timespec t{};
    assert(t.tv_sec == 0);
    assert(t.tv_nsec == 0);
  }

  // clock_t clock()

  {
    static_assert(cuda::std::is_same_v<cuda::std::clock_t, decltype(cuda::std::clock())>);
    cuda::std::ignore = cuda::std::clock();
  }

  // double difftime(time_t end, time_t start)

  {
    static_assert(
      cuda::std::is_same_v<double, decltype(cuda::std::difftime(cuda::std::time_t{}, cuda::std::time_t{}))>);
    assert(cuda::std::difftime(cuda::std::time_t{0}, cuda::std::time_t{0}) == 0.0);
    assert(cuda::std::difftime(cuda::std::time_t{1}, cuda::std::time_t{0}) == 1.0);
    assert(cuda::std::difftime(cuda::std::time_t{0}, cuda::std::time_t{1}) == -1.0);
  }

  // time_t time(time_t* __v)

  {
    static_assert(
      cuda::std::is_same_v<cuda::std::time_t, decltype(cuda::std::time(cuda::std::declval<cuda::std::time_t*>()))>);
    cuda::std::time_t t{};
    assert(cuda::std::time(&t) == t);
  }

  // int timespec_get(timespec* __ts, int __base)

  {
    static_assert(
      cuda::std::is_same_v<int, decltype(cuda::std::timespec_get(cuda::std::declval<cuda::std::timespec*>(), int{}))>);
    cuda::std::timespec t{};
    assert(cuda::std::timespec_get(&t, 0) == 0);
    assert(cuda::std::timespec_get(&t, TIME_UTC) == TIME_UTC);
  }

  return true;
}

int main(int, char**)
{
  test();
  return 0;
}
