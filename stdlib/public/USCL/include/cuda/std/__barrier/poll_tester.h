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

#ifndef __CUDA_STD___BARRIER_POLL_TESTER_H
#define __CUDA_STD___BARRIER_POLL_TESTER_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__utility/move.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <class _Barrier>
class __barrier_poll_tester_phase
{
  _Barrier const* __this;
  typename _Barrier::arrival_token __phase;

public:
  _CCCL_API inline __barrier_poll_tester_phase(_Barrier const* __this_, typename _Barrier::arrival_token&& __phase_)
      : __this(__this_)
      , __phase(::cuda::std::move(__phase_))
  {}

  [[nodiscard]] _CCCL_API inline bool operator()() const
  {
    return __this->__try_wait(__phase);
  }
};

template <class _Barrier>
class __barrier_poll_tester_parity
{
  _Barrier const* __this;
  bool __parity;

public:
  _CCCL_API inline __barrier_poll_tester_parity(_Barrier const* __this_, bool __parity_)
      : __this(__this_)
      , __parity(__parity_)
  {}

  [[nodiscard]] _CCCL_API inline bool operator()() const
  {
    return __this->__try_wait_parity(__parity);
  }
};

template <class _Barrier>
[[nodiscard]] _CCCL_API inline bool __call_try_wait(const _Barrier& __b, typename _Barrier::arrival_token&& __phase)
{
  return __b.__try_wait(::cuda::std::move(__phase));
}

template <class _Barrier>
[[nodiscard]] _CCCL_API inline bool __call_try_wait_parity(const _Barrier& __b, bool __parity)
{
  return __b.__try_wait_parity(__parity);
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // __CUDA_STD___BARRIER_POLL_TESTER_H
