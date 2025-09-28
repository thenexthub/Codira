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

// constexpr W operator*() const noexcept(is_nothrow_copy_constructible_v<W>);

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "types.h"

__host__ __device__ constexpr bool test()
{
  // taken from fake_bijection
  constexpr int random_indices[] = {4, 1, 2, 0, 3};
  {
    cuda::shuffle_iterator iter{fake_bijection{}};
    using value_type = cuda::std::iter_value_t<decltype(iter)>;

    for (int i = 0; i < 5; ++i, ++iter)
    {
      assert(*iter == static_cast<value_type>(random_indices[i]));
    }

    static_assert(noexcept(*iter));
    static_assert(cuda::std::is_same_v<decltype(*iter), value_type>);
  }

  {
    cuda::shuffle_iterator iter{fake_bijection<true, false>{}};
    using value_type = cuda::std::iter_value_t<decltype(iter)>;

    for (int i = 0; i < 5; ++i, ++iter)
    {
      assert(*iter == static_cast<value_type>(random_indices[i]));
    }

    static_assert(!noexcept(*iter));
    static_assert(cuda::std::is_same_v<decltype(*iter), value_type>);
  }

  {
    const cuda::shuffle_iterator iter{fake_bijection{}, 3};
    using value_type = cuda::std::iter_value_t<decltype(iter)>;
    assert(*iter == static_cast<value_type>(random_indices[3]));

    static_assert(noexcept(*iter));
    static_assert(cuda::std::is_same_v<decltype(*iter), value_type>);
  }

  {
    const cuda::shuffle_iterator iter{fake_bijection<true, false>{}, 3};
    using value_type = cuda::std::iter_value_t<decltype(iter)>;
    assert(*iter == static_cast<value_type>(random_indices[3]));

    static_assert(!noexcept(*iter));
    static_assert(cuda::std::is_same_v<decltype(*iter), value_type>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
