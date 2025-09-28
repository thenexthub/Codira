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

#include "common.cuh"

C2H_TEST("Fill", "[data_manipulation]")
{
  cudax::stream _stream{cuda::device_ref{0}};
  SECTION("Host resource")
  {
    cudax::legacy_pinned_memory_resource host_resource;
    cudax::uninitialized_buffer<int, cuda::mr::device_accessible> buffer(host_resource, buffer_size);

    cudax::fill_bytes(_stream, buffer, fill_byte);

    check_result_and_erase(_stream, cuda::std::span(buffer));
  }

  SECTION("Device resource")
  {
    cudax::device_memory_resource device_resource{cuda::device_ref{0}};
    cudax::uninitialized_buffer<int, cuda::mr::device_accessible> buffer(device_resource, buffer_size);
    cudax::fill_bytes(_stream, buffer, fill_byte);

    std::vector<int> host_vector(42);
    CUDART(cudaMemcpyAsync(
      host_vector.data(), buffer.data(), buffer.size() * sizeof(int), cudaMemcpyDefault, _stream.get()));

    check_result_and_erase(_stream, host_vector);
  }
  SECTION("Launch transform")
  {
    cudax::legacy_pinned_memory_resource host_resource;
    cudax::weird_buffer buffer(host_resource, buffer_size);

    cudax::fill_bytes(_stream, buffer, fill_byte);
    check_result_and_erase(_stream, cuda::std::span(buffer.data, buffer.size));
  }
}

C2H_TEST("Mdspan Fill", "[data_manipulation]")
{
  cudax::stream stream{cuda::device_ref{0}};
  {
    cuda::std::dextents<size_t, 3> dynamic_extents{1, 2, 3};
    auto buffer = make_buffer_for_mdspan(dynamic_extents, 0);
    cuda::std::mdspan<int, decltype(dynamic_extents)> dynamic_mdspan(buffer.data(), dynamic_extents);

    cudax::fill_bytes(stream, dynamic_mdspan, fill_byte);
    check_result_and_erase(stream, cuda::std::span(buffer.data(), buffer.size()));
  }
  {
    cuda::std::extents<size_t, 2, cuda::std::dynamic_extent, 4> mixed_extents{1};
    auto buffer = make_buffer_for_mdspan(mixed_extents, 0);
    cuda::std::mdspan<int, decltype(mixed_extents)> mixed_mdspan(buffer.data(), mixed_extents);

    cudax::fill_bytes(stream, cuda::std::move(mixed_mdspan), fill_byte);
    check_result_and_erase(stream, cuda::std::span(buffer.data(), buffer.size()));
  }
  {
    cudax::legacy_pinned_memory_resource host_resource;
    using static_extents = cuda::std::extents<size_t, 2, 3, 4>;
    auto size            = cuda::std::layout_left::mapping<static_extents>().required_span_size();
    cudax::weird_buffer<cuda::std::mdspan<int, static_extents>> buffer(host_resource, size);

    cudax::fill_bytes(stream, buffer, fill_byte);
    check_result_and_erase(stream, cuda::std::span(buffer.data, buffer.size));
  }
}

C2H_TEST("Non exhaustive mdspan fill_bytes", "[data_manipulation]")
{
  cudax::stream stream{cuda::device_ref{0}};
  {
    auto fake_strided_mdspan = create_fake_strided_mdspan();

    try
    {
      cudax::fill_bytes(stream, fake_strided_mdspan, fill_byte);
    }
    catch (const ::std::invalid_argument& e)
    {
      CHECK(e.what() == ::std::string("fill_bytes supports only exhaustive mdspans"));
    }
  }
}
