/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Friday, December 29, 2023.
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

#ifndef _CUDA___MEMCPY_ASYNC_CP_ASYNC_BULK_SHARED_GLOBAL_H_
#define _CUDA___MEMCPY_ASYNC_CP_ASYNC_BULK_SHARED_GLOBAL_H_

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#if _CCCL_CUDA_COMPILATION()
#  if __cccl_ptx_isa >= 800

#    include <cuda/__ptx/instructions/cp_async_bulk.h>
#    include <cuda/__ptx/ptx_dot_variants.h>
#    include <cuda/__ptx/ptx_helper_functions.h>
#    include <cuda/std/cstdint>

#    include <nv/target>

#    include <cuda/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

extern "C" _CCCL_DEVICE void __cuda_ptx_cp_async_bulk_shared_global_is_not_supported_before_SM_90__();
template <typename _Group>
inline _CCCL_DEVICE void __cp_async_bulk_shared_global(
  const _Group& __g, char* __dest, const char* __src, ::cuda::std::size_t __size, ::cuda::std::uint64_t* __bar_handle)
{
  // https://docs.nvidia.com/cuda/parallel-thread-execution/index.html#data-movement-and-conversion-instructions-cp-async-bulk
  NV_IF_ELSE_TARGET(NV_PROVIDES_SM_90,
                    (if (__g.thread_rank() == 0) {
                      ::cuda::ptx::cp_async_bulk(
                        ::cuda::ptx::space_cluster, ::cuda::ptx::space_global, __dest, __src, __size, __bar_handle);
                    }),
                    (::cuda::__cuda_ptx_cp_async_bulk_shared_global_is_not_supported_before_SM_90__();));
}

_CCCL_END_NAMESPACE_CUDA

#    include <cuda/std/__cccl/epilogue.h>

#  endif // __cccl_ptx_isa >= 800
#endif // _CCCL_CUDA_COMPILATION()

#endif // _CUDA___MEMCPY_ASYNC_CP_ASYNC_BULK_SHARED_GLOBAL_H_
