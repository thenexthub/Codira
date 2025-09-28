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

#include <uscl/devices>
#include <uscl/memory_resource>
#include <uscl/std/__algorithm_>
#include <uscl/std/array>
#include <uscl/std/cassert>
#include <uscl/std/initializer_list>
#include <uscl/std/tuple>
#include <uscl/std/type_traits>

#include <uscl/experimental/container.cuh>

#include <stdexcept>

#include "helper.h"
#include "types.h"

#if _CCCL_CUDACC_AT_LEAST(12, 6)
using test_types = c2h::type_list<cuda::std::tuple<cuda::mr::host_accessible>,
                                  cuda::std::tuple<cuda::mr::device_accessible>,
                                  cuda::std::tuple<cuda::mr::host_accessible, cuda::mr::device_accessible>>;
#else
using test_types = c2h::type_list<cuda::std::tuple<cuda::mr::device_accessible>>;
#endif

C2H_CCCLRT_TEST("cudax::async_buffer access and stream", "[container][async_buffer]", test_types)
{
  using TestT           = c2h::get<0, TestType>;
  using Env             = typename extract_properties<TestT>::env;
  using Resource        = typename extract_properties<TestT>::resource;
  using Buffer          = typename extract_properties<TestT>::async_buffer;
  using T               = typename Buffer::value_type;
  using reference       = typename Buffer::reference;
  using const_reference = typename Buffer::const_reference;
  using pointer         = typename Buffer::pointer;
  using const_pointer   = typename Buffer::const_pointer;

  cudax::stream stream{cuda::device_ref{0}};
  Env env{Resource{}, stream};

  SECTION("cudax::async_buffer::get_unsynchronized")
  {
    static_assert(cuda::std::is_same_v<decltype(cuda::std::declval<Buffer&>().get_unsynchronized(1ull)), reference>);
    static_assert(
      cuda::std::is_same_v<decltype(cuda::std::declval<const Buffer&>().get_unsynchronized(1ull)), const_reference>);

    {
      Buffer buf{env, {T(1), T(42), T(1337), T(0)}};
      buf.stream().sync();
      auto& res = buf.get_unsynchronized(2);
      CUDAX_CHECK(compare_value<Buffer>(res, T(1337)));
      CUDAX_CHECK(static_cast<size_t>(cuda::std::addressof(res) - buf.data()) == 2);
      assign_value<Buffer>(res, T(4));

      auto& const_res = cuda::std::as_const(buf).get_unsynchronized(2);
      CUDAX_CHECK(compare_value<Buffer>(const_res, T(4)));
      CUDAX_CHECK(static_cast<size_t>(cuda::std::addressof(const_res) - buf.data()) == 2);
    }
  }

  SECTION("cudax::async_buffer::data")
  {
    static_assert(cuda::std::is_same_v<decltype(cuda::std::declval<Buffer&>().data()), pointer>);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::declval<const Buffer&>().data()), const_pointer>);

    { // Works without allocation
      Buffer buf{env};
      buf.stream().sync();
      CUDAX_CHECK(buf.data() == nullptr);
      CUDAX_CHECK(cuda::std::as_const(buf).data() == nullptr);
    }

    { // Works with allocation
      Buffer buf{env, {T(1), T(42), T(1337), T(0)}};
      buf.stream().sync();
      CUDAX_CHECK(buf.data() != nullptr);
      CUDAX_CHECK(cuda::std::as_const(buf).data() != nullptr);
      CUDAX_CHECK(cuda::std::as_const(buf).data() == buf.data());
    }
  }

  SECTION("cudax::async_buffer::stream")
  {
    Buffer buf{env, {T(1), T(42), T(1337), T(0)}};
    CUDAX_CHECK(buf.stream() == stream);

    {
      cudax::stream other_stream{cuda::device_ref{0}};
      buf.set_stream(other_stream);
      CUDAX_CHECK(buf.stream() == other_stream);
      buf.set_stream(stream);
    }

    CUDAX_CHECK(buf.stream() == stream);
    buf.destroy(stream);
  }
}
