/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, July 24, 2024.
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

#include <uscl/memory_resource>
#include <uscl/std/array>
#include <uscl/std/cassert>
#include <uscl/std/initializer_list>
#include <uscl/std/tuple>
#include <uscl/std/type_traits>
#include <uscl/std/utility>

#include <uscl/experimental/container.cuh>

#include "helper.h"
#include "types.h"

#if _CCCL_CUDACC_AT_LEAST(12, 6)
using test_types = c2h::type_list<cuda::std::tuple<cuda::mr::host_accessible>,
                                  cuda::std::tuple<cuda::mr::device_accessible>,
                                  cuda::std::tuple<cuda::mr::host_accessible, cuda::mr::device_accessible>>;
#else
using test_types = c2h::type_list<cuda::std::tuple<cuda::mr::device_accessible>>;
#endif

C2H_CCCLRT_TEST("cudax::async_buffer properties", "[container][async_buffer]", test_types)
{
  using TestT                  = c2h::get<0, TestType>;
  using Buffer                 = typename extract_properties<TestT>::async_buffer;
  using iterator               = typename extract_properties<TestT>::iterator;
  using const_iterator         = typename extract_properties<TestT>::const_iterator;
  using reverse_iterator       = cuda::std::reverse_iterator<iterator>;
  using const_reverse_iterator = cuda::std::reverse_iterator<const_iterator>;

  // Check the type aliases
  static_assert(cuda::std::is_same_v<int, typename Buffer::value_type>, "");
  static_assert(cuda::std::is_same_v<cuda::std::size_t, typename Buffer::size_type>, "");
  static_assert(cuda::std::is_same_v<cuda::std::ptrdiff_t, typename Buffer::difference_type>, "");
  static_assert(cuda::std::is_same_v<int*, typename Buffer::pointer>, "");
  static_assert(cuda::std::is_same_v<const int*, typename Buffer::const_pointer>, "");
  static_assert(cuda::std::is_same_v<int&, typename Buffer::reference>, "");
  static_assert(cuda::std::is_same_v<const int&, typename Buffer::const_reference>, "");
  static_assert(cuda::std::is_same_v<iterator, typename Buffer::iterator>, "");
  static_assert(cuda::std::is_same_v<const_iterator, typename Buffer::const_iterator>, "");
  static_assert(cuda::std::is_same_v<cuda::std::reverse_iterator<iterator>, typename Buffer::reverse_iterator>, "");
  static_assert(
    cuda::std::is_same_v<cuda::std::reverse_iterator<const_iterator>, typename Buffer::const_reverse_iterator>, "");
  static_assert(cuda::std::ranges::contiguous_range<Buffer>);
}
