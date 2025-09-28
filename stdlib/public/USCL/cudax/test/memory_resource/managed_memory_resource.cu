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

#include <uscl/std/cstdint>
#include <uscl/std/type_traits>
#include <uscl/stream_ref>

#include <uscl/experimental/memory_resource.cuh>
#include <uscl/experimental/stream.cuh>

#include <stdexcept>

#include <testing.cuh>
#include <utility.cuh>

namespace cudax = cuda::experimental;

using managed_resource = cudax::legacy_managed_memory_resource;
static_assert(!cuda::std::is_trivial<managed_resource>::value, "");
static_assert(!cuda::std::is_trivially_default_constructible<managed_resource>::value, "");
static_assert(cuda::std::is_trivially_copy_constructible<managed_resource>::value, "");
static_assert(cuda::std::is_trivially_move_constructible<managed_resource>::value, "");
static_assert(cuda::std::is_trivially_copy_assignable<managed_resource>::value, "");
static_assert(cuda::std::is_trivially_move_assignable<managed_resource>::value, "");
static_assert(cuda::std::is_trivially_destructible<managed_resource>::value, "");
static_assert(!cuda::std::is_empty<managed_resource>::value, "");

static void ensure_managed_ptr(void* ptr)
{
  CHECK(ptr != nullptr);
  cudaPointerAttributes attributes;
  cudaError_t status = cudaPointerGetAttributes(&attributes, ptr);
  CHECK(status == cudaSuccess);
  CHECK(attributes.type == cudaMemoryTypeManaged);
}

C2H_TEST("managed_memory_resource construction", "[memory_resource]")
{
  SECTION("Default construction")
  {
    STATIC_REQUIRE(cuda::std::is_default_constructible_v<managed_resource>);
  }

  SECTION("Construct with flag")
  {
    managed_resource defaulted{};
    managed_resource with_flag{cudaMemAttachHost};
    CHECK(defaulted != with_flag);
  }
}

C2H_TEST("managed_memory_resource allocation", "[memory_resource]")
{
  managed_resource res{};
  cudax::stream stream{cuda::device_ref{0}};

  { // allocate_sync / deallocate_sync
    auto* ptr = res.allocate_sync(42);
    static_assert(cuda::std::is_same<decltype(ptr), void*>::value, "");
    ensure_managed_ptr(ptr);

    res.deallocate_sync(ptr, 42);
  }

  { // allocate_sync / deallocate_sync with alignment
    auto* ptr = res.allocate_sync(42, 4);
    static_assert(cuda::std::is_same<decltype(ptr), void*>::value, "");
    ensure_managed_ptr(ptr);

    res.deallocate_sync(ptr, 42, 4);
  }

#if _CCCL_HAS_EXCEPTIONS()
  { // allocate_sync with too small alignment
    while (true)
    {
      try
      {
        [[maybe_unused]] auto* ptr = res.allocate_sync(5, 42);
      }
      catch (std::invalid_argument&)
      {
        break;
      }
      CHECK(false);
    }
  }

  { // allocate_sync with non matching alignment
    while (true)
    {
      try
      {
        [[maybe_unused]] auto* ptr = res.allocate_sync(5, 1337);
      }
      catch (std::invalid_argument&)
      {
        break;
      }
      CHECK(false);
    }
  }
#endif // _CCCL_HAS_EXCEPTIONS()
}

enum class AccessibilityType
{
  Device,
  Host,
};

template <AccessibilityType Accessibility>
struct resource
{
  void* allocate_sync(size_t, size_t)
  {
    return nullptr;
  }
  void deallocate_sync(void*, size_t, size_t) noexcept {}

  bool operator==(const resource&) const
  {
    return true;
  }
  bool operator!=(const resource& other) const
  {
    return false;
  }
};
static_assert(cuda::mr::synchronous_resource<resource<AccessibilityType::Host>>, "");
static_assert(cuda::mr::synchronous_resource<resource<AccessibilityType::Device>>, "");

template <AccessibilityType Accessibility>
struct test_resource : public resource<Accessibility>
{
  void* allocate(cuda::stream_ref, size_t, size_t)
  {
    return nullptr;
  }
  void deallocate(cuda::stream_ref, void*, size_t, size_t) {}
};
static_assert(cuda::mr::resource<test_resource<AccessibilityType::Host>>, "");
static_assert(cuda::mr::resource<test_resource<AccessibilityType::Device>>, "");

// test for cccl#2214: https://github.com/NVIDIA/cccl/issues/2214
struct derived_managed_resource : cudax::legacy_managed_memory_resource
{
  using cudax::legacy_managed_memory_resource::legacy_managed_memory_resource;
};
static_assert(cuda::mr::synchronous_resource<derived_managed_resource>, "");

C2H_TEST("managed_memory_resource comparison", "[memory_resource]")
{
  managed_resource first{};
  { // comparison against a plain managed_memory_resource
    managed_resource second{};
    CHECK((first == second));
    CHECK(!(first != second));
  }

  { // comparison against a plain managed_memory_resource with a different pool
    managed_resource second{cudaMemAttachHost};
    CHECK((first != second));
    CHECK(!(first == second));
  }

  { // comparison against a managed_memory_resource wrapped inside a synchronous_resource_ref<device_accessible>
    managed_resource second{};
    cudax::synchronous_resource_ref<cudax::device_accessible> second_ref{second};
    CHECK((first == second_ref));
    CHECK(!(first != second_ref));
    CHECK((second_ref == first));
    CHECK(!(second_ref != first));
  }

  { // comparison against a different managed_resource through synchronous_resource_ref
    resource<AccessibilityType::Host> host_resource{};
    resource<AccessibilityType::Device> device_resource{};
    CHECK(!(first == host_resource));
    CHECK((first != host_resource));
    CHECK(!(first == device_resource));
    CHECK((first != device_resource));

    CHECK(!(host_resource == first));
    CHECK((host_resource != first));
    CHECK(!(device_resource == first));
    CHECK((device_resource != first));
  }

  { // comparison against a different managed_resource through synchronous_resource_ref
    resource<AccessibilityType::Host> host_async_resource{};
    resource<AccessibilityType::Device> device_async_resource{};
    CHECK(!(first == host_async_resource));
    CHECK((first != host_async_resource));
    CHECK(!(first == device_async_resource));
    CHECK((first != device_async_resource));

    CHECK(!(host_async_resource == first));
    CHECK((host_async_resource != first));
    CHECK(!(device_async_resource == first));
    CHECK((device_async_resource != first));
  }
}
