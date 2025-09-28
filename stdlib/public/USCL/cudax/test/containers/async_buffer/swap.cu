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
#include <uscl/std/__algorithm_>
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

C2H_CCCLRT_TEST("cudax::async_buffer swap", "[container][async_buffer]", test_types)
{
  using TestT     = c2h::get<0, TestType>;
  using Env       = typename extract_properties<TestT>::env;
  using Resource  = typename extract_properties<TestT>::resource;
  using Buffer    = typename extract_properties<TestT>::async_buffer;
  using T         = typename Buffer::value_type;
  using size_type = typename Buffer::size_type;

  cudax::stream stream{cuda::device_ref{0}};
  Env env{Resource{}, stream};
  STATIC_REQUIRE(
    cuda::std::is_same_v<decltype(cuda::std::declval<Buffer&>().swap(cuda::std::declval<Buffer&>())), void>);
  STATIC_REQUIRE(
    cuda::std::is_same_v<decltype(swap(cuda::std::declval<Buffer&>(), cuda::std::declval<Buffer&>())), void>);
  STATIC_REQUIRE(noexcept(cuda::std::declval<Buffer&>().swap(cuda::std::declval<Buffer&>())));
  STATIC_REQUIRE(noexcept(swap(cuda::std::declval<Buffer&>(), cuda::std::declval<Buffer&>())));

  // Note we do not care about the elements just the sizes
  Buffer vec_small{env, 5, cudax::no_init};

  SECTION("Can swap async_buffer")
  {
    Buffer vec_large{env, 42, cudax::no_init};

    CUDAX_CHECK(vec_large.size() == 42);
    CUDAX_CHECK(vec_small.size() == 5);
    CUDAX_CHECK(vec_large.size() == 42);
    CUDAX_CHECK(vec_small.size() == 5);

    vec_large.swap(vec_small);
    CUDAX_CHECK(vec_small.size() == 42);
    CUDAX_CHECK(vec_large.size() == 5);
    CUDAX_CHECK(vec_small.size() == 42);
    CUDAX_CHECK(vec_large.size() == 5);

    swap(vec_large, vec_small);
    CUDAX_CHECK(vec_large.size() == 42);
    CUDAX_CHECK(vec_small.size() == 5);
    CUDAX_CHECK(vec_large.size() == 42);
    CUDAX_CHECK(vec_small.size() == 5);
  }

  SECTION("Can swap async_buffer without allocation")
  {
    Buffer vec_no_allocation{env, 0, cudax::no_init};

    CUDAX_CHECK(vec_no_allocation.size() == 0);
    CUDAX_CHECK(vec_small.size() == 5);
    CUDAX_CHECK(vec_no_allocation.size() == 0);
    CUDAX_CHECK(vec_small.size() == 5);

    vec_no_allocation.swap(vec_small);
    CUDAX_CHECK(vec_small.size() == 0);
    CUDAX_CHECK(vec_no_allocation.size() == 5);
    CUDAX_CHECK(vec_small.size() == 0);
    CUDAX_CHECK(vec_no_allocation.size() == 5);

    swap(vec_no_allocation, vec_small);
    CUDAX_CHECK(vec_no_allocation.size() == 0);
    CUDAX_CHECK(vec_small.size() == 5);
    CUDAX_CHECK(vec_no_allocation.size() == 0);
    CUDAX_CHECK(vec_small.size() == 5);
  }
}
