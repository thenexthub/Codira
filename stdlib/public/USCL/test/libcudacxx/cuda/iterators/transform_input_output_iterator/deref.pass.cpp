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

// constexpr auto operator*() const noexcept(is_nothrow_copy_constructible_v<W>);

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "types.h"

template <class InputFn, class OutputFn>
__host__ __device__ constexpr void test()
{
  int buffer[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  InputFn input_func{};
  OutputFn output_func{};

  {
    cuda::transform_input_output_iterator iter{buffer, input_func, output_func};
    for (int i = 0; i < 8; ++i, ++iter)
    {
      assert(*iter == input_func(buffer[i]));
      assert((*iter = i) == input_func(output_func(i)));
      assert(buffer[i] == output_func(i));
    }
    static_assert(noexcept(*iter));
    static_assert(noexcept(static_cast<int>(*iter)) == !cuda::std::is_same_v<InputFn, TimesTwoMayThrow>);
    static_assert(noexcept(*iter = 2) == !cuda::std::is_same_v<OutputFn, PlusOneMayThrow>);
    static_assert(!cuda::std::is_same_v<decltype(*iter), int&>);
    static_assert(cuda::std::is_convertible_v<decltype(*iter), int>);
  }

  {
    const cuda::transform_input_output_iterator iter{buffer + 2, input_func, output_func};
    assert(*iter == input_func(buffer[2]));
    *iter = 2;
    assert(buffer[2] == output_func(2));
    static_assert(noexcept(*iter));
    static_assert(noexcept(static_cast<int>(*iter)) == !cuda::std::is_same_v<InputFn, TimesTwoMayThrow>);
    static_assert(noexcept(*iter = 2) == !cuda::std::is_same_v<OutputFn, PlusOneMayThrow>);
    static_assert(!cuda::std::is_same_v<decltype(*iter), int&>);
    static_assert(cuda::std::is_convertible_v<decltype(*iter), int>);
  }
}

__host__ __device__ constexpr bool test()
{
  test<TimesTwo, PlusOne>();
  test<TimesTwo, PlusOneMutable>();
  test<TimesTwo, PlusOneMayThrow>();
  test<TimesTwoMayThrow, PlusOne>();
  NV_IF_ELSE_TARGET(NV_IS_HOST, (test<TimesTwo, PlusOneHost>();), (test<TimesTwo, PlusOneDevice>();))

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
