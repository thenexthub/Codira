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

// constexpr explicit iterator(W value);

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "types.h"

__host__ __device__ constexpr bool test()
{
  { // CTAD
    const int val = 42;
    cuda::counting_iterator iter{val};
    assert(*iter == 42);
  }

  { // CTAD
    cuda::counting_iterator iter{42};
    assert(*iter == 42);
  }

  {
    const int val = 42;
    cuda::counting_iterator<int> iter{val};
    assert(*iter == 42);
  }

  {
    cuda::counting_iterator<int> iter{42};
    assert(*iter == 42);
  }

  {
    const Int42<ValueCtor> val{42};
    cuda::counting_iterator<Int42<ValueCtor>> iter{val};
    assert(*iter == Int42<ValueCtor>{42});
  }

  {
    cuda::counting_iterator<Int42<ValueCtor>> iter{Int42<ValueCtor>{42}};
    assert(*iter == Int42<ValueCtor>{42});
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
