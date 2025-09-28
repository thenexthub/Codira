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

#include <uscl/experimental/container.cuh>

#include "helper.h"
#include "test_resources.h"
#include "types.h"

#if _CCCL_CUDACC_AT_LEAST(12, 6)
using test_types = c2h::type_list<cuda::std::tuple<cuda::mr::host_accessible>,
                                  cuda::std::tuple<cuda::mr::device_accessible>,
                                  cuda::std::tuple<cuda::mr::host_accessible, cuda::mr::device_accessible>>;
#else
using test_types = c2h::type_list<cuda::std::tuple<cuda::mr::device_accessible>>;
#endif

C2H_CCCLRT_TEST("cudax::async_buffer conversion", "[container][async_buffer]", test_types)
{
  using TestT    = c2h::get<0, TestType>;
  using Env      = typename extract_properties<TestT>::env;
  using Resource = typename extract_properties<TestT>::resource;
  using Buffer   = typename extract_properties<TestT>::async_buffer;
  using T        = typename Buffer::value_type;

  cudax::stream stream{cuda::device_ref{0}};
  Resource resource{};
  Env env{resource, stream};

  // Convert from a async_buffer that has more properties than the current one
  using MatchingBuffer   = typename extract_properties<TestT>::matching_vector;
  using MatchingResource = typename extract_properties<TestT>::matching_resource;
  Env matching_env{MatchingResource{resource}, stream};

  SECTION("cudax::async_buffer construction with matching async_buffer")
  {
    { // can be copy constructed from empty input
      const MatchingBuffer input{matching_env, 0, cudax::no_init};
      Buffer buf(input);
      CUDAX_CHECK(buf.empty());
      CUDAX_CHECK(input.empty());
    }

    { // can be copy constructed from non-empty input
      const MatchingBuffer input{matching_env, {T(1), T(42), T(1337), T(0), T(12), T(-1)}};
      Buffer buf(input);
      CUDAX_CHECK(!buf.empty());
      CUDAX_CHECK(equal_range(buf));
      CUDAX_CHECK(equal_range(input));
    }

    { // can be move constructed with empty input
      MatchingBuffer input{matching_env, 0, cudax::no_init};
      Buffer buf(cuda::std::move(input));
      CUDAX_CHECK(buf.empty());
      CUDAX_CHECK(input.empty());
    }

    { // can be move constructed from non-empty input
      MatchingBuffer input{matching_env, {T(1), T(42), T(1337), T(0), T(12), T(-1)}};

      // ensure that we steal the data
      const auto* allocation = input.data();
      Buffer buf(cuda::std::move(input));
      CUDAX_CHECK(buf.size() == 6);
      CUDAX_CHECK(buf.data() == allocation);
      CUDAX_CHECK(input.size() == 0);
      CUDAX_CHECK(input.data() == nullptr);
      CUDAX_CHECK(equal_range(buf));
    }
  }
}
