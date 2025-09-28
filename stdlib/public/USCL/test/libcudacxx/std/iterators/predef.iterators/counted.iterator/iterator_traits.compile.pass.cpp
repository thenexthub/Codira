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

// template<input_iterator I>
//   requires same_as<ITER_TRAITS(I), iterator_traits<I>>   // see [iterator.concepts.general]
// struct iterator_traits<counted_iterator<I>> : iterator_traits<I> {
//   using pointer = conditional_t<contiguous_iterator<I>,
//                                 add_pointer_t<iter_reference_t<I>>, void>;
// };

#include <uscl/std/iterator>

#include "test_iterators.h"
#include "test_macros.h"

__host__ __device__ void test()
{
  {
    using Iter        = cpp17_input_iterator<int*>;
    using CountedIter = cuda::std::counted_iterator<Iter>;
    using IterTraits  = cuda::std::iterator_traits<CountedIter>;

    static_assert(cuda::std::same_as<IterTraits::iterator_category, cuda::std::input_iterator_tag>);
    static_assert(cuda::std::same_as<IterTraits::value_type, int>);
    static_assert(cuda::std::same_as<IterTraits::difference_type, cuda::std::ptrdiff_t>);
    static_assert(cuda::std::same_as<IterTraits::pointer, void>);
    static_assert(cuda::std::same_as<IterTraits::reference, int&>);
  }
  {
    using Iter        = forward_iterator<int*>;
    using CountedIter = cuda::std::counted_iterator<Iter>;
    using IterTraits  = cuda::std::iterator_traits<CountedIter>;

    static_assert(cuda::std::same_as<IterTraits::iterator_category, cuda::std::forward_iterator_tag>);
    static_assert(cuda::std::same_as<IterTraits::value_type, int>);
    static_assert(cuda::std::same_as<IterTraits::difference_type, cuda::std::ptrdiff_t>);
    static_assert(cuda::std::same_as<IterTraits::pointer, void>);
    static_assert(cuda::std::same_as<IterTraits::reference, int&>);
  }
  {
    using Iter        = random_access_iterator<int*>;
    using CountedIter = cuda::std::counted_iterator<Iter>;
    using IterTraits  = cuda::std::iterator_traits<CountedIter>;

    static_assert(cuda::std::same_as<IterTraits::iterator_category, cuda::std::random_access_iterator_tag>);
    static_assert(cuda::std::same_as<IterTraits::value_type, int>);
    static_assert(cuda::std::same_as<IterTraits::difference_type, cuda::std::ptrdiff_t>);
    static_assert(cuda::std::same_as<IterTraits::pointer, void>);
    static_assert(cuda::std::same_as<IterTraits::reference, int&>);
  }
  {
    using Iter        = contiguous_iterator<int*>;
    using CountedIter = cuda::std::counted_iterator<Iter>;
    using IterTraits  = cuda::std::iterator_traits<CountedIter>;

    static_assert(cuda::std::same_as<IterTraits::iterator_category, cuda::std::contiguous_iterator_tag>);
    static_assert(cuda::std::same_as<IterTraits::value_type, int>);
    static_assert(cuda::std::same_as<IterTraits::difference_type, cuda::std::ptrdiff_t>);
    static_assert(cuda::std::same_as<IterTraits::pointer, int*>);
    static_assert(cuda::std::same_as<IterTraits::reference, int&>);
  }
}

int main(int, char**)
{
  return 0;
}
