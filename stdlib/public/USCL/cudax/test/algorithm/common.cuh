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

#ifndef __ALGORITHM_COMMON__
#define __ALGORITHM_COMMON__

#include <uscl/memory_resource>
#include <uscl/std/mdspan>

#include <uscl/experimental/algorithm.cuh>
#include <uscl/experimental/container.cuh>
#include <uscl/experimental/memory_resource.cuh>

#include <testing.cuh>
#include <utility.cuh>

inline constexpr uint8_t fill_byte    = 1;
inline constexpr uint32_t buffer_size = 42;

inline int get_expected_value(uint8_t pattern_byte)
{
  int result;
  memset(&result, pattern_byte, sizeof(int));
  return result;
}

template <typename Result>
void check_result_and_erase(cudax::stream_ref stream, Result&& result, uint8_t pattern_byte = fill_byte)
{
  int expected = get_expected_value(pattern_byte);

  stream.sync();
  for (int& i : result)
  {
    CUDAX_REQUIRE(i == expected);
    i = 0;
  }
}

template <typename Layout = cuda::std::layout_right, typename Extents>
auto make_buffer_for_mdspan(Extents extents, char value = 0)
{
  cudax::legacy_pinned_memory_resource host_resource;
  auto mapping = typename Layout::template mapping<decltype(extents)>{extents};

  cudax::uninitialized_buffer<int, cuda::mr::host_accessible> buffer(host_resource, mapping.required_span_size());

  memset(buffer.data(), value, buffer.size_bytes());

  return buffer;
}

inline auto create_fake_strided_mdspan()
{
  cuda::std::dextents<size_t, 3> dynamic_extents{1, 2, 3};
  cuda::std::array<size_t, 3> strides{12, 4, 1};
#if _CCCL_CUDACC_BELOW(12, 6)
  auto map = cuda::std::layout_stride::mapping{dynamic_extents, strides};
#else
  cuda::std::layout_stride::mapping map{dynamic_extents, strides};
#endif
  return cuda::std::mdspan<int, decltype(dynamic_extents), cuda::std::layout_stride>(nullptr, map);
};

namespace cuda::experimental
{

// Need a type that goes through all launch_transform steps, but is not a contiguous_range
template <typename RelocatableValue = cuda::std::span<int>>
struct weird_buffer
{
  legacy_pinned_memory_resource& resource;
  int* data;
  std::size_t size;

  weird_buffer(legacy_pinned_memory_resource& res, std::size_t s)
      : resource(res)
      , data((int*) res.allocate_sync(s * sizeof(int)))
      , size(s)
  {
    memset(data, 0, size);
  }

  ~weird_buffer()
  {
    resource.deallocate_sync(data, size);
  }

  weird_buffer(const weird_buffer&) = delete;
  weird_buffer(weird_buffer&&)      = delete;

  struct transform_result
  {
    int* data;
    std::size_t size;

    RelocatableValue transformed_argument()
    {
      return *this;
    };

    operator cuda::std::span<int>()
    {
      return {data, size};
    }

    template <typename Extents>
    operator cuda::std::mdspan<int, Extents>()
    {
      return cuda::std::mdspan<int, Extents>{data};
    }
  };

  [[nodiscard]] friend transform_result transform_device_argument(cuda::stream_ref, const weird_buffer& self) noexcept
  {
    return {self.data, self.size};
  }
};
} // namespace cuda::experimental

#endif // __ALGORITHM_COMMON__
