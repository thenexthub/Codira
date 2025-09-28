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

#ifndef _CUDA___MEMCPY_ASYNC_IS_LOCAL_SMEM_BARRIER_H
#define _CUDA___MEMCPY_ASYNC_IS_LOCAL_SMEM_BARRIER_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__barrier/barrier.h>
#include <uscl/__memory/address_space.h>
#include <uscl/std/__atomic/scopes.h>
#include <uscl/std/__barrier/empty_completion.h>
#include <uscl/std/__type_traits/is_same.h>

#include <nv/target>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

//! @brief __is_local_smem_barrier returns true if barrier is (1) block-scoped and (2) located in shared memory.
template <thread_scope _Sco,
          typename _CompF,
          bool _Is_mbarrier = (_Sco == thread_scope_block)
                           && ::cuda::std::is_same_v<_CompF, ::cuda::std::__empty_completion>>
_CCCL_API inline bool __is_local_smem_barrier(barrier<_Sco, _CompF>& __barrier)
{
  NV_IF_ELSE_TARGET(
    NV_IS_DEVICE,
    (return _Is_mbarrier && ::cuda::device::is_object_from(__barrier, ::cuda::device::address_space::shared);),
    (return false;));
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___MEMCPY_ASYNC_IS_LOCAL_SMEM_BARRIER_H
