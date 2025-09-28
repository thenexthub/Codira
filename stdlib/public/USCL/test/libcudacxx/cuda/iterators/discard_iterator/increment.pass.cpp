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

// constexpr discard_iterator& operator++();
// constexpr discard_iterator operator++(int);

#include <uscl/iterator>

#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  const int index = 2;
  cuda::discard_iterator iter(index);

  assert(iter++ == cuda::discard_iterator(index + 0));
  assert(++iter == cuda::discard_iterator(index + 2));

  static_assert(cuda::std::is_same_v<decltype(iter++), cuda::discard_iterator>);
  static_assert(cuda::std::is_same_v<decltype(++iter), cuda::discard_iterator&>);

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
