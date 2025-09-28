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

#ifndef CUDAX_TEST_CONTAINER_VECTOR_TEST_RESOURCES_H
#define CUDAX_TEST_CONTAINER_VECTOR_TEST_RESOURCES_H

#include <uscl/memory_resource>
#include <uscl/std/type_traits>
#include <uscl/stream_ref>

#include <uscl/experimental/memory_resource.cuh>

#include <cstdint>
#include <unordered_map>

#include <testing.cuh>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

struct other_property
{};

// make the cudax resources have that property for tests
inline void get_property(const cuda::experimental::device_memory_resource&, other_property) {}
inline void get_property(const cuda::experimental::legacy_pinned_memory_resource&, other_property) {}
#if _CCCL_CUDACC_AT_LEAST(12, 6)
inline void get_property(const cuda::experimental::pinned_memory_resource&, other_property) {}
#endif

//! @brief Simple wrapper around a memory resource to ensure that it compares differently and we can test those code
//! paths
template <class... Properties>
struct memory_resource_wrapper
{
  // Not a resource_ref, because it can't be used to create any_resource (yet)
  // https://github.com/NVIDIA/cccl/issues/4166
  cudax::any_resource<Properties...> resource_;

  void* allocate_sync(std::size_t size, std::size_t alignment)
  {
    return resource_.allocate_sync(size, alignment);
  }
  void deallocate_sync(void* ptr, std::size_t size, std::size_t alignment)
  {
    resource_.deallocate_sync(ptr, size, alignment);
  }
  void* allocate(cuda::stream_ref stream, std::size_t size, std::size_t alignment)
  {
    return resource_.allocate(stream, size, alignment);
  }
  void deallocate(cuda::stream_ref stream, void* ptr, std::size_t size, std::size_t alignment)
  {
    resource_.deallocate(stream, ptr, size, alignment);
  }

  bool operator==(const memory_resource_wrapper&) const
  {
    return true;
  }
  bool operator!=(const memory_resource_wrapper&) const
  {
    return false;
  }

  _CCCL_TEMPLATE(class Property)
  _CCCL_REQUIRES(cuda::std::__is_included_in_v<Property, Properties...>)
  friend void get_property(const memory_resource_wrapper&, Property) noexcept {}

  friend void get_property(const memory_resource_wrapper&, other_property) noexcept {}
};

#endif // CUDAX_TEST_CONTAINER_VECTOR_TEST_RESOURCES_H
