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

#include <uscl/experimental/memory_resource.cuh>

#include <testing.cuh>

#include "test_resource.cuh"

static_assert(
  cuda::has_property<cudax::any_synchronous_resource<cudax::host_accessible, get_data>, cudax::host_accessible>);
static_assert(cuda::has_property<cudax::any_synchronous_resource<cudax::host_accessible, get_data>, get_data>);
static_assert(
  !cuda::has_property<cudax::any_synchronous_resource<cudax::host_accessible, get_data>, cudax::device_accessible>);

struct unused_property
{};

TEMPLATE_TEST_CASE_METHOD(
  test_fixture, "any_synchronous_resource", "[container][resource]", big_resource, small_resource)
{
  using TestResource    = TestType;
  constexpr bool is_big = sizeof(TestResource) > cuda::__default_small_object_size;

  SECTION("construct and destruct")
  {
    Counts expected{};
    CHECK(this->counts == expected);
    {
      cudax::any_synchronous_resource<cudax::host_accessible, get_data> mr{TestResource{42, this}};
      expected.new_count += is_big;
      ++expected.object_count;
      ++expected.move_count;
      CHECK(this->counts == expected);
      CHECK(get_property(mr, get_data{}) == 42);
      get_property(mr, cudax::host_accessible{});
    }
    expected.delete_count += is_big;
    --expected.object_count;
    CHECK(this->counts == expected);
  }

  // Reset the counters:
  this->counts = Counts();

  SECTION("copy and move")
  {
    Counts expected{};
    CHECK(this->counts == expected);
    {
      cudax::any_synchronous_resource<cudax::host_accessible, get_data> mr{TestResource{42, this}};
      expected.new_count += is_big;
      ++expected.object_count;
      ++expected.move_count;
      CHECK(this->counts == expected);

      auto mr2 = mr;
      expected.new_count += is_big;
      ++expected.copy_count;
      ++expected.object_count;
      CHECK(this->counts == expected);
      CHECK((mr == mr2));
      ++expected.equal_to_count;
      CHECK(this->counts == expected);

      auto mr3 = std::move(mr);
      expected.move_count += !is_big; // for big resources, move is a pointer swap
      CHECK(this->counts == expected);
      CHECK((mr2 == mr3));
      ++expected.equal_to_count;
      CHECK(this->counts == expected);
    }
    expected.delete_count += 2 * is_big;
    expected.object_count -= 2;
    CHECK(this->counts == expected);
  }

  // Reset the counters:
  this->counts = Counts();

  SECTION("allocate and deallocate_sync")
  {
    Counts expected{};
    CHECK(this->counts == expected);
    {
      cudax::any_synchronous_resource<cudax::host_accessible, get_data> mr{TestResource{42, this}};
      expected.new_count += is_big;
      ++expected.object_count;
      ++expected.move_count;
      CHECK(this->counts == expected);

      void* ptr = mr.allocate_sync(bytes(50), align(8));
      CHECK(ptr == this);
      ++expected.allocate_count;
      CHECK(this->counts == expected);

      mr.deallocate_sync(ptr, bytes(50), align(8));
      ++expected.deallocate_count;
      CHECK(this->counts == expected);
    }
    expected.delete_count += is_big;
    --expected.object_count;
    CHECK(this->counts == expected);
  }

  // Reset the counters:
  this->counts = Counts();

  SECTION("equality comparable")
  {
    Counts expected{};
    CHECK(this->counts == expected);
    {
      cudax::legacy_managed_memory_resource managed1{}, managed2{};
      CHECK(managed1 == managed2);
      cudax::any_synchronous_resource<cudax::device_accessible> mr{managed1};
      CHECK(mr == managed1);
    }
    CHECK(this->counts == expected);
  }

  // Reset the counters:
  this->counts = Counts();

  SECTION("conversion from any_synchronous_resource to cudax::synchronous_resource_ref")
  {
    Counts expected{};
    {
      cudax::any_synchronous_resource<cudax::host_accessible, cudax::device_accessible, get_data> mr{
        TestResource{42, this}};
      expected.new_count += is_big;
      ++expected.object_count;
      ++expected.move_count;
      CHECK(this->counts == expected);

      // conversion from any_synchronous_resource to cuda::mr::synchronous_synchronous_resource_ref:
      cudax::synchronous_resource_ref<cudax::host_accessible, cudax::device_accessible, get_data> ref = mr;

      // conversion from any_synchronous_resource to cuda::mr::synchronous_synchronous_resource_ref with narrowing:
      cudax::synchronous_resource_ref<cudax::host_accessible, get_data> ref2 = mr;
      CHECK(get_property(ref2, get_data{}) == 42);

      CHECK(this->counts == expected);
      auto* ptr = ref.allocate_sync(bytes(100), align(8));
      CHECK(ptr == this);
      ++expected.allocate_count;
      CHECK(this->counts == expected);
      ref.deallocate_sync(ptr, bytes(0), align(0));
      ++expected.deallocate_count;
      CHECK(this->counts == expected);
    }
    expected.delete_count += is_big;
    --expected.object_count;
    CHECK(this->counts == expected);
  }

  SECTION("conversion from any_synchronous_resource to cuda::mr::synchronous_resource_ref")
  {
    Counts expected{};
    {
      cudax::any_synchronous_resource<cudax::host_accessible, cudax::device_accessible, get_data> mr{
        TestResource{42, this}};
      expected.new_count += is_big;
      ++expected.object_count;
      ++expected.move_count;
      CHECK(this->counts == expected);

      // conversion from any_synchronous_resource to cuda::mr::synchronous_resource_ref:
      cuda::mr::synchronous_resource_ref<cudax::host_accessible, cudax::device_accessible, get_data> ref = mr;

      // conversion from any_synchronous_resource to cuda::mr::synchronous_resource_ref with narrowing:
      cuda::mr::synchronous_resource_ref<cudax::host_accessible, get_data> ref2 = mr;
      CHECK(get_property(ref2, get_data{}) == 42);

      CHECK(this->counts == expected);
      auto* ptr = ref.allocate_sync(bytes(100), align(8));
      CHECK(ptr == this);
      ++expected.allocate_count;
      CHECK(this->counts == expected);
      ref.deallocate_sync(ptr, bytes(0), align(0));
      ++expected.deallocate_count;
      CHECK(this->counts == expected);
    }
    expected.delete_count += is_big;
    --expected.object_count;
    CHECK(this->counts == expected);
  }

  // Reset the counters:
  this->counts = Counts();

  SECTION("conversion from synchronous_resource_ref to any_synchronous_resource")
  {
    Counts expected{};
    {
      TestResource test{42, this};
      ++expected.object_count;
      cudax::synchronous_resource_ref<cudax::host_accessible, get_data> ref{test};
      CHECK(this->counts == expected);

      cudax::any_synchronous_resource<cudax::host_accessible, get_data> mr = ref;
      expected.new_count += is_big;
      ++expected.object_count;
      ++expected.copy_count;
      CHECK(this->counts == expected);

      auto* ptr = ref.allocate_sync(bytes(100), align(8));
      CHECK(ptr == this);
      ++expected.allocate_count;
      CHECK(this->counts == expected);
      ref.deallocate_sync(ptr, bytes(0), align(0));
      ++expected.deallocate_count;
      CHECK(this->counts == expected);
    }
    expected.delete_count += is_big;
    expected.object_count -= 2;
    CHECK(this->counts == expected);
  }

  // Reset the counters:
  this->counts = Counts();

  SECTION("test slicing off of properties")
  {
    Counts expected{};
    CHECK(this->counts == expected);
    {
      cudax::any_synchronous_resource<cudax::host_accessible, cudax::device_accessible, get_data> mr{
        TestResource{42, this}};
      expected.new_count += is_big;
      ++expected.object_count;
      ++expected.move_count;
      CHECK(this->counts == expected);

      cudax::any_synchronous_resource<cudax::device_accessible, get_data> mr2 = mr;
      expected.new_count += is_big;
      ++expected.object_count;
      ++expected.copy_count;
      CHECK(this->counts == expected);

      CHECK(get_property(mr2, get_data{}) == 42);
      auto data = try_get_property(mr2, get_data{});
      static_assert(cuda::std::is_same_v<decltype(data), cuda::std::optional<int>>);
      CHECK(data.has_value());
      CHECK(data.value() == 42);

      auto host = try_get_property(mr2, cudax::host_accessible{});
      static_assert(cuda::std::is_same_v<decltype(host), bool>);
      CHECK(host);

      auto unused = try_get_property(mr2, unused_property{});
      static_assert(cuda::std::is_same_v<decltype(unused), bool>);
      CHECK(!unused);
    }
    expected.delete_count += 2 * is_big;
    expected.object_count -= 2;
    CHECK(this->counts == expected);
  }

  // Reset the counters:
  this->counts = Counts();

  SECTION("make_any_synchronous_resource")
  {
    Counts expected{};
    CHECK(this->counts == expected);
    {
      cudax::any_synchronous_resource<cudax::host_accessible, get_data> mr =
        cudax::make_any_synchronous_resource<TestResource, cudax::host_accessible, get_data>(42, this);
      expected.new_count += is_big;
      ++expected.object_count;
      CHECK(this->counts == expected);
    }
    expected.delete_count += is_big;
    --expected.object_count;
    CHECK(this->counts == expected);
  }
  // Reset the counters:
  this->counts = Counts();
}
