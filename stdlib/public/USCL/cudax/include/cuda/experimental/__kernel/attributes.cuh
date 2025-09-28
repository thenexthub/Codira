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

#ifndef _CUDAX___KERNEL_ATTRIBUTES_CUH
#define _CUDAX___KERNEL_ATTRIBUTES_CUH

#include <uscl/__cccl_config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__device/arch_traits.h>
#include <uscl/__device/device_ref.h>
#include <uscl/__driver/driver_api.h>
#include <uscl/std/cstddef>

#include <uscl/experimental/__kernel/kernel_ref.cuh>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{

namespace __detail
{

template <::CUfunction_attribute _Attr, typename _Type>
struct __kernel_attr_impl
{
  using type = _Type;

  [[nodiscard]] constexpr operator ::CUfunction_attribute() const noexcept
  {
    return _Attr;
  }

  template <class _Signature>
  [[nodiscard]] type operator()(kernel_ref<_Signature> __kernel, device_ref __dev) const
  {
    return static_cast<type>(
      _CUDA_DRIVER::__kernelGetAttribute(_Attr, __kernel.get(), _CUDA_DRIVER::__deviceGet(__dev.get())));
  }
};

template <::CUfunction_attribute _Attr>
struct __kernel_attr : __kernel_attr_impl<_Attr, int>
{};

template <>
struct __kernel_attr<::CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES> //
    : __kernel_attr_impl<::CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES, ::cuda::std::size_t>
{};
template <>
struct __kernel_attr<::CU_FUNC_ATTRIBUTE_CONST_SIZE_BYTES> //
    : __kernel_attr_impl<::CU_FUNC_ATTRIBUTE_CONST_SIZE_BYTES, ::cuda::std::size_t>
{};
template <>
struct __kernel_attr<::CU_FUNC_ATTRIBUTE_LOCAL_SIZE_BYTES> //
    : __kernel_attr_impl<::CU_FUNC_ATTRIBUTE_LOCAL_SIZE_BYTES, ::cuda::std::size_t>
{};
template <>
struct __kernel_attr<::CU_FUNC_ATTRIBUTE_PTX_VERSION> //
    : __kernel_attr_impl<::CU_FUNC_ATTRIBUTE_PTX_VERSION, ::cuda::arch::id>
{};
template <>
struct __kernel_attr<::CU_FUNC_ATTRIBUTE_BINARY_VERSION> //
    : __kernel_attr_impl<::CU_FUNC_ATTRIBUTE_BINARY_VERSION, ::cuda::arch::id>
{};
template <>
struct __kernel_attr<::CU_FUNC_ATTRIBUTE_CACHE_MODE_CA> //
    : __kernel_attr_impl<::CU_FUNC_ATTRIBUTE_CACHE_MODE_CA, bool>
{};
template <>
struct __kernel_attr<::CU_FUNC_ATTRIBUTE_CLUSTER_SIZE_MUST_BE_SET> //
    : __kernel_attr_impl<::CU_FUNC_ATTRIBUTE_CLUSTER_SIZE_MUST_BE_SET, bool>
{};

} // namespace __detail

namespace kernel_attributes
{
// Maximum number of threads per block
using max_threads_per_block_t = __detail::__kernel_attr<::CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK>;
static constexpr max_threads_per_block_t max_threads_per_block{};

// The size in bytes of statically-allocated shared memory required by this kernel
using shared_size_bytes_t = __detail::__kernel_attr<::CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES>;
static constexpr shared_size_bytes_t shared_memory_size{};

// The size in bytes of user-allocated constant memory required by this kernel
using const_size_bytes_t = __detail::__kernel_attr<::CU_FUNC_ATTRIBUTE_CONST_SIZE_BYTES>;
static constexpr const_size_bytes_t const_memory_size{};

// The size in bytes of local memory used by each thread of this kernel
using local_size_bytes_t = __detail::__kernel_attr<::CU_FUNC_ATTRIBUTE_LOCAL_SIZE_BYTES>;
static constexpr local_size_bytes_t local_memory_size{};

// The number of registers used by each thread of this kernel
using num_regs_t = __detail::__kernel_attr<::CU_FUNC_ATTRIBUTE_NUM_REGS>;
static constexpr num_regs_t num_regs{};

// The virtual architecture for which the kernel was compiled
using virtual_arch_t = __detail::__kernel_attr<::CU_FUNC_ATTRIBUTE_PTX_VERSION>;
static constexpr virtual_arch_t virtual_arch{};

// The binary (real) architecture for which the kernel was compiled
using binary_arch_t = __detail::__kernel_attr<::CU_FUNC_ATTRIBUTE_BINARY_VERSION>;
static constexpr binary_arch_t binary_arch{};

// The attribute to indicate whether the function has been compiled with user specified option "-Xptxas --dlcm=ca" set
using cache_mode_ca_t = __detail::__kernel_attr<::CU_FUNC_ATTRIBUTE_CACHE_MODE_CA>;
static constexpr cache_mode_ca_t cache_mode_ca{};

// Is the kernel required to be launched with a cluster size?
using requires_cluster_dims_t = __detail::__kernel_attr<::CU_FUNC_ATTRIBUTE_CLUSTER_SIZE_MUST_BE_SET>;
static constexpr requires_cluster_dims_t requires_cluster_dims{};
} // namespace kernel_attributes
} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDAX___KERNEL_ATTRIBUTES_CUH
