/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
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

#ifndef __COMMON_UTILITY_H__
#define __COMMON_UTILITY_H__

#include <cuda_runtime_api.h>
// cuda_runtime_api needs to come first

#include <uscl/__runtime/ensure_current_context.h>
#include <uscl/atomic>
#include <uscl/std/__cuda/api_wrapper.h>
#include <uscl/std/utility>
#include <uscl/stream_ref>

#include <new> // IWYU pragma: keep (needed for placement new)

#include "testing.cuh"

namespace
{
namespace test
{

struct _malloc_pinned
{
private:
  void* pv = nullptr;

public:
  explicit _malloc_pinned(std::size_t size)
  {
    cuda::__ensure_current_context guard(cuda::device_ref{0});
    _CCCL_TRY_CUDA_API(::cudaMallocHost, "failed to allocate pinned memory", &pv, size);
  }

  ~_malloc_pinned()
  {
    cuda::__ensure_current_context guard(cuda::device_ref{0});
    [[maybe_unused]] auto status = ::cudaFreeHost(pv);
  }

  template <class T>
  T* get_as() const noexcept
  {
    return static_cast<T*>(pv);
  }
};

template <class T>
struct pinned
{
private:
  _malloc_pinned _mem;

public:
  explicit pinned(T t)
      : _mem(sizeof(T))
  {
    ::new (_mem.get_as<void>()) T(std::move(t));
  }

  ~pinned()
  {
    get()->~T();
  }

  T* get() noexcept
  {
    return _mem.get_as<T>();
  }
  const T* get() const noexcept
  {
    return _mem.get_as<T>();
  }

  T& operator*() noexcept
  {
    return *get();
  }
  const T& operator*() const noexcept
  {
    return *get();
  }
};

template <int N>
struct assign_n
{
  __device__ constexpr void operator()(int* pi) const noexcept
  {
    *pi = N;
  }
};

template <int N>
struct verify_n
{
  __device__ void operator()(int* pi) const noexcept
  {
    // TODO: fix clang CUDA require macro
    // CCCLRT_REQUIRE(*pi == N);
    ccclrt_require_impl(*pi == N, "*pi == N", __FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
};

using assign_42 = assign_n<42>;
using verify_42 = verify_n<42>;

struct atomic_add_one
{
  __device__ void operator()(int* pi) const noexcept
  {
    cuda::atomic_ref atomic_pi(*pi);
    atomic_pi.fetch_add(1);
  }
};

struct atomic_sub_one
{
  __device__ void operator()(int* pi) const noexcept
  {
    cuda::atomic_ref atomic_pi(*pi);
    atomic_pi.fetch_sub(1);
  }
};

struct spin_until_80
{
  __device__ void operator()(int* pi) const noexcept
  {
    cuda::atomic_ref atomic_pi(*pi);
    while (atomic_pi.load() != 80)
      ;
  }
};

struct empty_kernel
{
  __device__ void operator()() const noexcept {}
};

template <class Fn, class... Args>
static __global__ void kernel_launcher(Fn fn, Args... args)
{
  fn(args...);
}

template <class Fn, class... Args>
void launch_kernel_single_thread(cuda::stream_ref stream, Fn fn, Args... args)
{
  cuda::__ensure_current_context guard(stream);
  kernel_launcher<<<1, 1, 0, stream.get()>>>(fn, args...);
  CUDART(cudaGetLastError());
}

} // namespace test
} // namespace
#endif // __COMMON_UTILITY_H__
