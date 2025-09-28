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

#ifndef _CUDA___MEMCPY_ASYNC_DISPATCH_MEMCPY_ASYNC_H_
#define _CUDA___MEMCPY_ASYNC_DISPATCH_MEMCPY_ASYNC_H_

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__memcpy_async/completion_mechanism.h>
#include <uscl/__memcpy_async/cp_async_bulk_shared_global.h>
#include <uscl/__memcpy_async/cp_async_fallback.h>
#include <uscl/__memcpy_async/cp_async_shared_global.h>
#include <uscl/__memory/address_space.h>
#include <uscl/std/cstddef>
#include <uscl/std/cstdint>
#include <uscl/std/cstring>

#include <nv/target>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

/***********************************************************************
 * cuda::memcpy_async dispatch
 *
 * The dispatch mechanism takes all the arguments and dispatches to the
 * fastest asynchronous copy mechanism available.
 *
 * It returns a __completion_mechanism that indicates which completion mechanism
 * was used by the copy mechanism. This value can be used by the sync object to
 * further synchronize if necessary.
 *
 ***********************************************************************/

template <::cuda::std::size_t _Align, typename _Group>
[[nodiscard]] _CCCL_DEVICE inline __completion_mechanism __dispatch_memcpy_async_any_to_any(
  _Group const& __group,
  char* __dest_char,
  char const* __src_char,
  ::cuda::std::size_t __size,
  ::cuda::std::uint32_t __allowed_completions,
  ::cuda::std::uint64_t* __bar_handle)
{
  ::cuda::__cp_async_fallback_mechanism<_Align>(__group, __dest_char, __src_char, __size);
  return __completion_mechanism::__sync;
}

template <::cuda::std::size_t _Align, typename _Group>
[[nodiscard]] _CCCL_DEVICE inline __completion_mechanism __dispatch_memcpy_async_global_to_shared(
  _Group const& __group,
  char* __dest_char,
  char const* __src_char,
  ::cuda::std::size_t __size,
  ::cuda::std::uint32_t __allowed_completions,
  ::cuda::std::uint64_t* __bar_handle)
{
#if __cccl_ptx_isa >= 800
  NV_IF_TARGET(
    NV_PROVIDES_SM_90,
    ([[maybe_unused]] const bool __can_use_complete_tx =
       __allowed_completions & uint32_t(__completion_mechanism::__mbarrier_complete_tx);
     _CCCL_ASSERT(__can_use_complete_tx == (nullptr != __bar_handle),
                  "Pass non-null bar_handle if and only if can_use_complete_tx.");
     if constexpr (_Align >= 16) {
       if (__can_use_complete_tx
           && ::cuda::device::is_address_from(__bar_handle, ::cuda::device::address_space::shared))
       {
         ::cuda::__cp_async_bulk_shared_global(__group, __dest_char, __src_char, __size, __bar_handle);
         return __completion_mechanism::__mbarrier_complete_tx;
       }
     }
     // Fallthrough to SM 80..
     ));
#endif // __cccl_ptx_isa >= 800

  NV_IF_TARGET(
    NV_PROVIDES_SM_80,
    (if constexpr (_Align >= 4) {
      const bool __can_use_async_group = __allowed_completions & uint32_t(__completion_mechanism::__async_group);
      if (__can_use_async_group)
      {
        ::cuda::__cp_async_shared_global_mechanism<_Align>(__group, __dest_char, __src_char, __size);
        return __completion_mechanism::__async_group;
      }
    }
     // Fallthrough..
     ));

  ::cuda::__cp_async_fallback_mechanism<_Align>(__group, __dest_char, __src_char, __size);
  return __completion_mechanism::__sync;
}

// __dispatch_memcpy_async is the internal entry point for dispatching to the correct memcpy_async implementation.
template <::cuda::std::size_t _Align, typename _Group>
[[nodiscard]] _CCCL_API inline __completion_mechanism __dispatch_memcpy_async(
  _Group const& __group,
  char* __dest_char,
  char const* __src_char,
  ::cuda::std::size_t __size,
  ::cuda::std::uint32_t __allowed_completions,
  ::cuda::std::uint64_t* __bar_handle)
{
  NV_IF_ELSE_TARGET(
    NV_IS_DEVICE,
    (
      // Dispatch based on direction of the copy: global to shared, shared to
      // global, etc.

      // CUDA compilers <= 12.2 may not propagate assumptions about the state space
      // of pointers correctly. Therefore, we
      // 1) put the code for each copy direction in a separate function, and
      // 2) make sure none of the code paths can reach each other by "falling through".
      //
      // See nvbug 4074679 and also PR #478.
      if (::cuda::device::is_address_from(__src_char, ::cuda::device::address_space::global)
          && ::cuda::device::is_address_from(__dest_char, ::cuda::device::address_space::shared)) {
        return ::cuda::__dispatch_memcpy_async_global_to_shared<_Align>(
          __group, __dest_char, __src_char, __size, __allowed_completions, __bar_handle);
      } else {
        return ::cuda::__dispatch_memcpy_async_any_to_any<_Align>(
          __group, __dest_char, __src_char, __size, __allowed_completions, __bar_handle);
      }),
    (
      // Host code path:
      if (__group.thread_rank() == 0) {
        ::cuda::std::memcpy(__dest_char, __src_char, __size);
      } return __completion_mechanism::__sync;));
}

template <::cuda::std::size_t _Align, typename _Group>
[[nodiscard]] _CCCL_API inline __completion_mechanism __dispatch_memcpy_async(
  _Group const& __group,
  char* __dest_char,
  char const* __src_char,
  ::cuda::std::size_t __size,
  ::cuda::std::uint32_t __allowed_completions)
{
  _CCCL_ASSERT(!(__allowed_completions & uint32_t(__completion_mechanism::__mbarrier_complete_tx)),
               "Cannot allow mbarrier_complete_tx completion mechanism when not passing a barrier. ");
  return ::cuda::__dispatch_memcpy_async<_Align>(
    __group, __dest_char, __src_char, __size, __allowed_completions, nullptr);
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___MEMCPY_ASYNC_DISPATCH_MEMCPY_ASYNC_H_
