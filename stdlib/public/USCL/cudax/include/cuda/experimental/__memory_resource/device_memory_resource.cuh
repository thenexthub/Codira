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

#ifndef _CUDAX__MEMORY_RESOURCE_CUDA_DEVICE_MEMORY_RESOURCE_CUH
#define _CUDAX__MEMORY_RESOURCE_CUDA_DEVICE_MEMORY_RESOURCE_CUH

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#if _CCCL_CUDA_COMPILER(CLANG)
#  include <cuda_runtime.h>
#  include <cuda_runtime_api.h>
#endif // _CCCL_CUDA_COMPILER(CLANG)

#include <uscl/__memory_resource/get_property.h>
#include <uscl/__memory_resource/properties.h>
#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__cuda/api_wrapper.h>

#include <uscl/experimental/__memory_resource/device_memory_pool.cuh>
#include <uscl/experimental/__memory_resource/memory_resource_base.cuh>

#include <uscl/std/__cccl/prologue.h>

//! @file
//! The \c device_memory_pool class provides an asynchronous memory resource that allocates device memory in stream
//! order.
namespace cuda::experimental
{

//! @rst
//! .. _cudax-memory-resource-async:
//!
//! Stream ordered memory resource
//! ------------------------------
//!
//! ``device_memory_resource`` uses `cudaMallocFromPoolAsync / cudaFreeAsync
//! <https://docs.nvidia.com/cuda/cuda-runtime-api/group__CUDART__MEMORY__POOLS.html>`__ for allocation/deallocation. A
//! ``device_memory_resource`` is a thin wrapper around a \c cudaMemPool_t.
//!
//! .. warning::
//!
//!    ``device_memory_resource`` does not own the pool and it is the responsibility of the user to ensure that the
//!    lifetime of the pool exceeds the lifetime of the ``device_memory_resource``.
//!
//! @endrst
class device_memory_resource : public __memory_resource_base
{
private:
  //! @brief  Returns the default ``cudaMemPool_t`` from the specified device.
  //! @throws cuda_error if retrieving the default ``cudaMemPool_t`` fails.
  //! @returns The default memory pool of the specified device.
  [[nodiscard]] static ::cudaMemPool_t __get_default_device_mem_pool(const int __device_id)
  {
    ::cuda::experimental::__verify_device_supports_stream_ordered_allocations(__device_id);

    ::cudaMemPool_t __pool;
    _CCCL_TRY_CUDA_API(
      ::cudaDeviceGetDefaultMemPool, "Failed to call cudaDeviceGetDefaultMemPool", &__pool, __device_id);
    return __pool;
  }

public:
  //! @brief Constructs a device_memory_resource using the default \c cudaMemPool_t of a given device.
  //! @throws cuda_error if retrieving the default \c cudaMemPool_t fails.
  explicit device_memory_resource(::cuda::device_ref __device)
      : __memory_resource_base(__get_default_device_mem_pool(__device.get()))
  {}

  device_memory_resource(int)                    = delete;
  device_memory_resource(::cuda::std::nullptr_t) = delete;

  //! @brief  Constructs the device_memory_resource from a \c cudaMemPool_t.
  //! @param __pool The \c cudaMemPool_t used to allocate memory.
  explicit device_memory_resource(::cudaMemPool_t __pool) noexcept
      : __memory_resource_base(__pool)
  {}

  //! @brief  Constructs the device_memory_resource from a \c device_memory_pool by calling get().
  //! @param __pool The \c device_memory_pool used to allocate memory.
  explicit device_memory_resource(device_memory_pool& __pool) noexcept
      : __memory_resource_base(__pool.get())
  {}

  //! @brief Enables the \c device_accessible property for \c device_memory_resource.
  //! @relates device_memory_resource
  friend constexpr void get_property(device_memory_resource const&, device_accessible) noexcept {}

  using default_queries = properties_list<device_accessible>;
};
static_assert(::cuda::mr::synchronous_resource_with<device_memory_resource, device_accessible>, "");
} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif //_CUDAX__MEMORY_RESOURCE_CUDA_DEVICE_MEMORY_RESOURCE_CUH
