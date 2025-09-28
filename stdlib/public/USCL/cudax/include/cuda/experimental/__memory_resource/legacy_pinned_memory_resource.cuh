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

#ifndef _CUDA__MEMORY_RESOURCE_LEGACY_PINNED_MEMORY_RESOURCE_H
#define _CUDA__MEMORY_RESOURCE_LEGACY_PINNED_MEMORY_RESOURCE_H

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

#include <uscl/__memory_resource/properties.h>
#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__cuda/api_wrapper.h>
#include <uscl/std/detail/libcxx/include/stdexcept>

#include <uscl/experimental/__memory_resource/memory_resource_base.cuh>
#include <uscl/experimental/__memory_resource/pinned_memory_pool.cuh>

#include <uscl/std/__cccl/prologue.h>

//! @file
//! The \c legacy_pinned_memory_resource class provides a memory resource that allocates pinned memory.
namespace cuda::experimental
{

//! @brief legacy_pinned_memory_resource uses `cudaMallocHost` / `cudaFreeHost` for allocation / deallocation.
//! @note This memory resource will be deprecated in the future. For CUDA 12.6 and above, use
//! `cuda::experimental::pinned_memory_resource` instead, which is the long-term replacement.
class legacy_pinned_memory_resource
{
public:
  constexpr legacy_pinned_memory_resource() noexcept {}

  //! @brief Allocate host memory of size at least \p __bytes.
  //! @param __bytes The size in bytes of the allocation.
  //! @param __alignment The requested alignment of the allocation.
  //! @throw std::invalid_argument in case of invalid alignment or \c cuda::cuda_error of the returned error code.
  //! @return Pointer to the newly allocated memory
  [[nodiscard]] void* allocate_sync(const size_t __bytes,
                                    const size_t __alignment = ::cuda::mr::default_cuda_malloc_host_alignment) const
  {
    // We need to ensure that the provided alignment matches the minimal provided alignment
    if (!__is_valid_alignment(__alignment))
    {
      ::cuda::std::__throw_invalid_argument(
        "Invalid alignment passed to "
        "legacy_pinned_memory_resource::allocate_sync.");
    }

    void* __ptr{nullptr};
    _CCCL_TRY_CUDA_API(::cudaMallocHost, "Failed to allocate memory with cudaMallocHost.", &__ptr, __bytes);
    return __ptr;
  }

  //! @brief Deallocate memory pointed to by \p __ptr.
  //! @param __ptr Pointer to be deallocated. Must have been allocated through a call to `allocate_sync`.
  //! @param __bytes The number of bytes that was passed to the allocation call that returned \p __ptr.
  //! @param __alignment The alignment that was passed to the allocation call that returned \p __ptr.
  void deallocate_sync(
    void* __ptr, const size_t, const size_t __alignment = ::cuda::mr::default_cuda_malloc_host_alignment) const noexcept
  {
    // We need to ensure that the provided alignment matches the minimal provided alignment
    _CCCL_ASSERT(__is_valid_alignment(__alignment),
                 "Invalid alignment passed to legacy_pinned_memory_resource::deallocate_sync.");
    _CCCL_ASSERT_CUDA_API(::cudaFreeHost, "legacy_pinned_memory_resource::deallocate_sync failed", __ptr);
    (void) __alignment;
  }

  //! @brief Equality comparison with another \c legacy_pinned_memory_resource.
  //! @param __other The other \c legacy_pinned_memory_resource.
  //! @return Whether both \c legacy_pinned_memory_resource were constructed with the same flags.
  [[nodiscard]] constexpr bool operator==(legacy_pinned_memory_resource const&) const noexcept
  {
    return true;
  }
#if _CCCL_STD_VER <= 2017
  //! @brief Equality comparison with another \c legacy_pinned_memory_resource.
  //! @param __other The other \c legacy_pinned_memory_resource.
  //! @return Whether both \c legacy_pinned_memory_resource were constructed with different flags.
  [[nodiscard]] constexpr bool operator!=(legacy_pinned_memory_resource const&) const noexcept
  {
    return false;
  }
#endif // _CCCL_STD_VER <= 2017

  //! @brief Enables the \c device_accessible property
  friend constexpr void get_property(legacy_pinned_memory_resource const&, device_accessible) noexcept {}
  //! @brief Enables the \c host_accessible property
  friend constexpr void get_property(legacy_pinned_memory_resource const&, host_accessible) noexcept {}

  //! @brief Checks whether the passed in alignment is valid
  static constexpr bool __is_valid_alignment(const size_t __alignment) noexcept
  {
    return __alignment <= ::cuda::mr::default_cuda_malloc_host_alignment
        && (::cuda::mr::default_cuda_malloc_host_alignment % __alignment == 0);
  }

  using default_queries = properties_list<device_accessible, host_accessible>;
};

static_assert(::cuda::mr::synchronous_resource_with<legacy_pinned_memory_resource, device_accessible>, "");
static_assert(::cuda::mr::synchronous_resource_with<legacy_pinned_memory_resource, host_accessible>, "");

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif //_CUDA__MEMORY_RESOURCE_LEGACY_PINNED_MEMORY_RESOURCE_H
