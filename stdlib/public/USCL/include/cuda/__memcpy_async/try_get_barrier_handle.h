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

#ifndef _CUDA___MEMCPY_ASYNC_TRY_GET_BARRIER_HANDLE_H
#define _CUDA___MEMCPY_ASYNC_TRY_GET_BARRIER_HANDLE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__barrier/barrier_block_scope.h>
#include <uscl/__barrier/barrier_native_handle.h>
#include <uscl/std/__atomic/scopes.h>
#include <uscl/std/__barrier/barrier.h>
#include <uscl/std/__barrier/empty_completion.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/cstdint>

#include <nv/target>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

//! @brief __try_get_barrier_handle returns barrier handle of block-scoped barriers and a nullptr otherwise.
template <thread_scope _Sco, typename _CompF>
_CCCL_API inline ::cuda::std::uint64_t* __try_get_barrier_handle(barrier<_Sco, _CompF>& __barrier)
{
  return nullptr;
}

template <>
_CCCL_API inline ::cuda::std::uint64_t*
__try_get_barrier_handle<::cuda::thread_scope_block, ::cuda::std::__empty_completion>(
  [[maybe_unused]] barrier<thread_scope_block>& __barrier)
{
  NV_DISPATCH_TARGET(
    NV_IS_DEVICE, (return ::cuda::device::barrier_native_handle(__barrier);), NV_ANY_TARGET, (return nullptr;));
  _CCCL_UNREACHABLE();
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___MEMCPY_ASYNC_TRY_GET_BARRIER_HANDLE_H
