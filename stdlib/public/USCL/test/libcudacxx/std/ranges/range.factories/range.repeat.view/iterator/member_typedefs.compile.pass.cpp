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

// Test iterator category and iterator concepts.

// using index-type = conditional_t<same_as<Bound, unreachable_sentinel_t>, ptrdiff_t, Bound>;
// using iterator_concept = random_access_iterator_tag;
// using iterator_category = random_access_iterator_tag;
// using value_type = T;
// using difference_type = see below:
// If is-signed-integer-like<index-type> is true, the member typedef-name difference_type denotes
// index-type. Otherwise, it denotes IOTA-DIFF-T(index-type).

#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/cstdint>
#include <uscl/std/ranges>
#include <uscl/std/type_traits>

__host__ __device__ constexpr bool test()
{
  // unbound
  {
    using Iter = cuda::std::ranges::iterator_t<cuda::std::ranges::repeat_view<int>>;
    static_assert(cuda::std::same_as<Iter::iterator_concept, cuda::std::random_access_iterator_tag>);
    static_assert(cuda::std::same_as<Iter::iterator_category, cuda::std::random_access_iterator_tag>);
    static_assert(cuda::std::same_as<Iter::value_type, int>);
    static_assert(cuda::std::same_as<Iter::difference_type, ptrdiff_t>);
    static_assert(cuda::std::is_signed_v<Iter::difference_type>);
  }

  // bound
  {
    {
      using Iter = cuda::std::ranges::iterator_t<const cuda::std::ranges::repeat_view<int, cuda::std::int8_t>>;
      static_assert(cuda::std::same_as<Iter::iterator_concept, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::iterator_category, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::value_type, int>);
      static_assert(cuda::std::is_signed_v<Iter::difference_type>);
      static_assert(sizeof(Iter::difference_type) == sizeof(cuda::std::int8_t));
    }

    {
      using Iter = cuda::std::ranges::iterator_t<const cuda::std::ranges::repeat_view<int, cuda::std::uint8_t>>;
      static_assert(cuda::std::same_as<Iter::iterator_concept, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::iterator_category, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::value_type, int>);
      static_assert(cuda::std::is_signed_v<Iter::difference_type>);
      static_assert(sizeof(Iter::difference_type) > sizeof(cuda::std::uint8_t));
    }

    {
      using Iter = cuda::std::ranges::iterator_t<const cuda::std::ranges::repeat_view<int, cuda::std::int16_t>>;
      static_assert(cuda::std::same_as<Iter::iterator_concept, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::iterator_category, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::value_type, int>);
      static_assert(cuda::std::is_signed_v<Iter::difference_type>);
      static_assert(sizeof(Iter::difference_type) == sizeof(cuda::std::int16_t));
    }

    {
      using Iter = cuda::std::ranges::iterator_t<const cuda::std::ranges::repeat_view<int, cuda::std::uint16_t>>;
      static_assert(cuda::std::same_as<Iter::iterator_concept, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::iterator_category, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::value_type, int>);
      static_assert(cuda::std::is_signed_v<Iter::difference_type>);
      static_assert(sizeof(Iter::difference_type) > sizeof(cuda::std::uint16_t));
    }

    {
      using Iter = cuda::std::ranges::iterator_t<const cuda::std::ranges::repeat_view<int, cuda::std::int32_t>>;
      static_assert(cuda::std::same_as<Iter::iterator_concept, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::iterator_category, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::value_type, int>);
      static_assert(cuda::std::is_signed_v<Iter::difference_type>);
      static_assert(sizeof(Iter::difference_type) == sizeof(cuda::std::int32_t));
    }

    {
      using Iter = cuda::std::ranges::iterator_t<const cuda::std::ranges::repeat_view<int, cuda::std::uint32_t>>;
      static_assert(cuda::std::same_as<Iter::iterator_concept, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::iterator_category, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::value_type, int>);
      static_assert(cuda::std::is_signed_v<Iter::difference_type>);
      static_assert(sizeof(Iter::difference_type) > sizeof(cuda::std::uint32_t));
    }

    {
      using Iter = cuda::std::ranges::iterator_t<const cuda::std::ranges::repeat_view<int, cuda::std::int64_t>>;
      static_assert(cuda::std::same_as<Iter::iterator_concept, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::iterator_category, cuda::std::random_access_iterator_tag>);
      static_assert(cuda::std::same_as<Iter::value_type, int>);
      static_assert(cuda::std::is_signed_v<Iter::difference_type>);
      static_assert(sizeof(Iter::difference_type) == sizeof(cuda::std::int64_t));
    }
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
