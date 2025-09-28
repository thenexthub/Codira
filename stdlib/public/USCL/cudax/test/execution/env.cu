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

#include <uscl/std/execution>
#include <uscl/std/type_traits>

#include <uscl/experimental/container.cuh>
#include <uscl/experimental/execution.cuh>
#include <uscl/experimental/memory_resource.cuh>

#include <testing.cuh>

namespace cudax = cuda::experimental;
using env_t     = cudax::env_t<cuda::mr::device_accessible>;

struct test_resource
{
  void* allocate_sync(size_t, size_t)
  {
    return nullptr;
  }
  void* allocate(cuda::stream_ref, size_t, size_t)
  {
    return nullptr;
  }
  void deallocate_sync(void*, size_t, size_t) {}
  void deallocate(cuda::stream_ref, void*, size_t, size_t) {}

  constexpr bool operator==(const test_resource&) const noexcept
  {
    return true;
  }

  constexpr bool operator!=(const test_resource&) const noexcept
  {
    return false;
  }

  friend void get_property(const test_resource&, cuda::mr::device_accessible) noexcept {}
};

C2H_TEST("env_t is queryable for all properties we want", "[execution][env]")
{
  STATIC_REQUIRE(cuda::std::execution::__queryable_with<env_t, cuda::get_stream_t>);
  STATIC_REQUIRE(cuda::std::execution::__queryable_with<env_t, cuda::mr::get_memory_resource_t>);
  STATIC_REQUIRE(cuda::std::execution::__queryable_with<env_t, cudax::execution::get_execution_policy_t>);
}

C2H_TEST("env_t is default constructible", "[execution][env]")
{
  env_t env{cudax::device_memory_resource{cuda::device_ref{0}}};
  CHECK(env.query(cuda::get_stream) == ::cuda::__detail::__invalid_stream);
  CHECK(env.query(cudax::execution::get_execution_policy) == cudax::execution::any_execution_policy{});
  CHECK(env.query(cuda::mr::get_memory_resource) == cudax::device_memory_resource{cuda::device_ref{0}});
}

C2H_TEST("env_t is constructible from an any_resource", "[execution][env]")
{
  const cudax::any_resource<cuda::mr::device_accessible> mr{test_resource{}};

  SECTION("Passing an any_resource")
  {
    env_t env{mr};
    CHECK(env.query(cuda::get_stream) == ::cuda::__detail::__invalid_stream);
    CHECK(env.query(cudax::execution::get_execution_policy) == cudax::execution::any_execution_policy{});
    CHECK(env.query(cuda::mr::get_memory_resource) == mr);
  }

  SECTION("Passing an any_resource and a stream")
  {
    cudax::stream stream{cuda::device_ref{0}};
    env_t env{mr, stream};
    CHECK(env.query(cuda::get_stream) == stream);
    CHECK(env.query(cudax::execution::get_execution_policy) == cudax::execution::any_execution_policy{});
    CHECK(env.query(cuda::mr::get_memory_resource) == mr);
  }

  SECTION("Passing an any_resource, a stream and a policy")
  {
    cudax::stream stream{cuda::device_ref{0}};
    env_t env{mr, stream, cuda::std::execution::par_unseq};
    CHECK(env.query(cuda::get_stream) == stream);
    CHECK((env.query(cudax::execution::get_execution_policy) == cuda::std::execution::par_unseq));
    CHECK(env.query(cuda::mr::get_memory_resource) == mr);
  }
}

C2H_TEST("env_t is constructible from an any_resource passed as an rvalue", "[execution][env]")
{
  SECTION("Passing an any_resource")
  {
    env_t env{cudax::any_resource<cuda::mr::device_accessible>{test_resource{}}};
    CHECK(env.query(cuda::get_stream) == ::cuda::__detail::__invalid_stream);
    CHECK(env.query(cudax::execution::get_execution_policy) == cudax::execution::any_execution_policy{});
    CHECK(env.query(cuda::mr::get_memory_resource)
          == cudax::any_resource<cuda::mr::device_accessible>{test_resource{}});
  }

  SECTION("Passing an any_resource and a stream")
  {
    cudax::stream stream{cuda::device_ref{0}};
    env_t env{cudax::any_resource<cuda::mr::device_accessible>{test_resource{}}, stream};
    CHECK(env.query(cuda::get_stream) == stream);
    CHECK(env.query(cudax::execution::get_execution_policy) == cudax::execution::any_execution_policy{});
    CHECK(env.query(cuda::mr::get_memory_resource)
          == cudax::any_resource<cuda::mr::device_accessible>{test_resource{}});
  }

  SECTION("Passing an any_resource, a stream and a policy")
  {
    cudax::stream stream{cuda::device_ref{0}};
    env_t env{
      cudax::any_resource<cuda::mr::device_accessible>{test_resource{}}, stream, cuda::std::execution::par_unseq};
    CHECK(env.query(cuda::get_stream) == stream);
    CHECK(env.query(cudax::execution::get_execution_policy) == cuda::std::execution::par_unseq);
    CHECK(env.query(cuda::mr::get_memory_resource)
          == cudax::any_resource<cuda::mr::device_accessible>{test_resource{}});
  }
}

C2H_TEST("env_t is constructible from a resource", "[execution][env]")
{
  test_resource mr{};

  SECTION("Passing an any_resource")
  {
    env_t env{mr};
    CHECK(env.query(cuda::get_stream) == ::cuda::__detail::__invalid_stream);
    CHECK(env.query(cudax::execution::get_execution_policy) == cudax::execution::any_execution_policy{});
    CHECK(env.query(cuda::mr::get_memory_resource) == mr);
  }

  SECTION("Passing an any_resource and a stream")
  {
    cudax::stream stream{cuda::device_ref{0}};
    env_t env{mr, stream};
    CHECK(env.query(cuda::get_stream) == stream);
    CHECK(env.query(cudax::execution::get_execution_policy) == cudax::execution::any_execution_policy{});
    CHECK(env.query(cuda::mr::get_memory_resource) == mr);
  }

  SECTION("Passing an any_resource, a stream and a policy")
  {
    cudax::stream stream{cuda::device_ref{0}};
    env_t env{mr, stream, cuda::std::execution::par_unseq};
    CHECK(env.query(cuda::get_stream) == stream);
    CHECK(env.query(cudax::execution::get_execution_policy) == cuda::std::execution::par_unseq);
    CHECK(env.query(cuda::mr::get_memory_resource) == mr);
  }
}

C2H_TEST("env_t is constructible from a resource passed as an rvalue", "[execution][env]")
{
  SECTION("Passing an any_resource")
  {
    env_t env{test_resource{}};
    CHECK(env.query(cuda::get_stream) == ::cuda::__detail::__invalid_stream);
    CHECK(env.query(cudax::execution::get_execution_policy) == cudax::execution::any_execution_policy{});
    CHECK(env.query(cuda::mr::get_memory_resource) == test_resource{});
  }

  SECTION("Passing an any_resource and a stream")
  {
    cudax::stream stream{cuda::device_ref{0}};
    env_t env{test_resource{}, stream};
    CHECK(env.query(cuda::get_stream) == stream);
    CHECK(env.query(cudax::execution::get_execution_policy) == cudax::execution::any_execution_policy{});
    CHECK(env.query(cuda::mr::get_memory_resource) == test_resource{});
  }

  SECTION("Passing an any_resource, a stream and a policy")
  {
    cudax::stream stream{cuda::device_ref{0}};
    env_t env{test_resource{}, stream, cuda::std::execution::par_unseq};
    CHECK(env.query(cuda::get_stream) == stream);
    CHECK(env.query(cudax::execution::get_execution_policy) == cuda::std::execution::par_unseq);
    CHECK(env.query(cuda::mr::get_memory_resource) == test_resource{});
  }
}

struct some_env_t
{
  test_resource res_{};
  cudax::stream stream_{cuda::device_ref{0}};
  cudax::execution::any_execution_policy policy_ = cuda::std::execution::par_unseq;

  const test_resource& query(cuda::mr::get_memory_resource_t) const noexcept
  {
    return res_;
  }

  cudax::stream_ref query(cuda::get_stream_t) const noexcept
  {
    return stream_;
  }

  cudax::execution::any_execution_policy query(cudax::execution::get_execution_policy_t) const noexcept
  {
    return policy_;
  }
};
C2H_TEST("env_t is constructible from a suitable env", "[execution][env]")
{
  some_env_t other_env{};
  env_t env{other_env};
  CHECK(env.query(cuda::get_stream) == other_env.stream_);
  CHECK(env.query(cudax::execution::get_execution_policy) == other_env.policy_);
  CHECK(env.query(cuda::mr::get_memory_resource) == other_env.res_);
}

template <bool WithResource, bool WithStream, bool WithPolicy>
struct bad_env_t
{
  test_resource res_{};
  cudax::stream stream_{cuda::device_ref{0}};
  cudax::execution::any_execution_policy policy_ = cuda::std::execution::par_unseq;

  template <bool Enable = WithResource, cuda::std::enable_if_t<Enable, int> = 0>
  const test_resource& query(cuda::mr::get_memory_resource_t) const noexcept
  {
    return res_;
  }

  template <bool Enable = WithStream, cuda::std::enable_if_t<Enable, int> = 0>
  cudax::stream_ref query(cuda::get_stream_t) const noexcept
  {
    return stream_;
  }

  template <bool Enable = WithPolicy, cuda::std::enable_if_t<Enable, int> = 0>
  cudax::execution::any_execution_policy query(cudax::execution::get_execution_policy_t) const noexcept
  {
    return policy_;
  }
};
C2H_TEST("env_t is not constructible from a env missing queries", "[execution][env]")
{
  STATIC_REQUIRE(cuda::std::is_constructible_v<env_t, bad_env_t<true, true, true>>);
  STATIC_REQUIRE(!cuda::std::is_constructible_v<env_t, bad_env_t<false, true, true>>);
  STATIC_REQUIRE(!cuda::std::is_constructible_v<env_t, bad_env_t<true, false, true>>);
  STATIC_REQUIRE(!cuda::std::is_constructible_v<env_t, bad_env_t<true, true, false>>);
}

C2H_TEST("Can use query to construct various objects", "[execution][env]")
{
  SECTION("Can create an any_resource")
  {
    env_t env{test_resource{}};
    cudax::any_synchronous_resource<cuda::mr::device_accessible> resource = env.query(cuda::mr::get_memory_resource);
    CHECK(resource == test_resource{});
  }

  SECTION("Can create an uninitialized_async_buffer")
  {
    cudax::stream stream_{cuda::device_ref{0}};
    env_t env{test_resource{}, stream_};
    cudax::uninitialized_async_buffer<int, cuda::mr::device_accessible> buf{
      env.query(cuda::mr::get_memory_resource), env.query(cuda::get_stream), 0ull};
    CHECK(buf.memory_resource() == test_resource{});
    CHECK(buf.stream() == stream_);
  }
}
