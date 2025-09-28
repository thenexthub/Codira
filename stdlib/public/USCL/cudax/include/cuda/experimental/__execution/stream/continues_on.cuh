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

#ifndef __CUDAX_EXECUTION_STREAM_CONTINUES_ON
#define __CUDAX_EXECUTION_STREAM_CONTINUES_ON

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__stream/get_stream.h>
#include <uscl/__type_traits/is_specialization_of.h>
#include <uscl/__utility/immovable.h>
#include <uscl/std/__utility/forward_like.h>

#include <uscl/experimental/__detail/utility.cuh>
#include <uscl/experimental/__execution/completion_signatures.cuh>
#include <uscl/experimental/__execution/continues_on.cuh>
#include <uscl/experimental/__execution/cpos.cuh>
#include <uscl/experimental/__execution/rcvr_ref.cuh>
#include <uscl/experimental/__execution/stream/adaptor.cuh>
#include <uscl/experimental/__execution/stream/domain.cuh>
#include <uscl/experimental/__execution/visit.cuh>
#include <uscl/experimental/__launch/launch.cuh>
#include <uscl/experimental/__stream/stream_ref.cuh>

#include <cuda_runtime_api.h>

#include <uscl/experimental/__execution/prologue.cuh>

namespace cuda::experimental::execution
{
namespace __stream
{
struct __continues_on_t
{
  // Transition from the GPU to the CPU domain
  template <class _Rcvr>
  struct _CCCL_TYPE_VISIBILITY_DEFAULT __rcvr_t
  {
    using receiver_concept = receiver_t;

    template <class... _Values>
    _CCCL_API constexpr void set_value(_Values&&...) noexcept
    {
      // no-op
    }

    _CCCL_API constexpr void set_error(::cuda::std::__ignore_t) noexcept
    {
      // no-op
    }

    _CCCL_API constexpr void set_stopped() noexcept
    {
      // no-op
    }

    [[nodiscard]] _CCCL_API constexpr auto get_env() const noexcept -> __fwd_env_t<env_of_t<_Rcvr>>
    {
      return __fwd_env(execution::get_env(__rcvr_));
    }

    _Rcvr& __rcvr_;
  };

  // This opstate will be stored in host memory.
  template <class _Sndr, class _Rcvr>
  struct _CCCL_TYPE_VISIBILITY_DEFAULT __opstate_t
  {
    using operation_state_concept = operation_state_t;
    using __env_t                 = __fwd_env_t<env_of_t<_Rcvr>>;

    _CCCL_API constexpr explicit __opstate_t(_Sndr&& __sndr, _Rcvr __rcvr)
        : __rcvr_(static_cast<_Rcvr&&>(__rcvr))
        , __stream_(__get_stream(__sndr, execution::get_env(__rcvr_)))
        , __opstate_(execution::connect(static_cast<_Sndr&&>(__sndr), __rcvr_t<_Rcvr>{__rcvr_}))
    {}

    _CCCL_IMMOVABLE(__opstate_t);

    _CCCL_HOST_API void start() noexcept
    {
      execution::start(__opstate_);
      if (auto __status = ::cudaStreamSynchronize(__stream_.get()); __status != ::cudaSuccess)
      {
        execution::set_error(static_cast<_Rcvr&&>(__rcvr_), cudaError_t(__status));
      }
      else
      {
        // __opstate_ is an instance of __stream::__opstate_t, and it has a __set_results
        // member function that will pass the results to the receiver on the host.
        __opstate_.__set_results(__rcvr_);
      }
    }

    _Rcvr __rcvr_;
    stream_ref __stream_;
    connect_result_t<_Sndr, __rcvr_t<_Rcvr>> __opstate_;
  };

  struct _CCCL_TYPE_VISIBILITY_DEFAULT __thunk_t
  {};

  template <class _Sndr>
  struct _CCCL_TYPE_VISIBILITY_DEFAULT __sndr_t
  {
    using sender_concept = sender_t;

    template <class _Self, class... _Env>
    [[nodiscard]] _CCCL_API static _CCCL_CONSTEVAL auto get_completion_signatures()
    {
      return execution::get_child_completion_signatures<_Self, _Sndr, _Env...>();
    }

    template <class _Rcvr>
    [[nodiscard]] _CCCL_API constexpr auto connect(_Rcvr __rcvr) && -> __opstate_t<_Sndr, _Rcvr>
    {
      return __opstate_t<_Sndr, _Rcvr>{static_cast<_Sndr&&>(__sndr_), static_cast<_Rcvr&&>(__rcvr)};
    }

    template <class _Rcvr>
    [[nodiscard]] _CCCL_API constexpr auto connect(_Rcvr __rcvr) const& -> __opstate_t<const _Sndr&, _Rcvr>
    {
      return __opstate_t<const _Sndr&, _Rcvr>{__sndr_, static_cast<_Rcvr&&>(__rcvr)};
    }

    [[nodiscard]] _CCCL_API constexpr auto get_env() const noexcept -> env_of_t<_Sndr>
    {
      return execution::get_env(__sndr_);
    }

    _CCCL_NO_UNIQUE_ADDRESS __thunk_t __tag_;
    ::cuda::std::__ignore_t __ignore_;
    _Sndr __sndr_;
  };

  template <class _Sndr>
  [[nodiscard]] _CCCL_API auto operator()(_Sndr&& __sndr, ::cuda::std::__ignore_t) const
  {
    auto& [__tag, __sched, __child] = __sndr;
    using __child_t                 = ::cuda::std::__copy_cvref_t<_Sndr, decltype(__child)>;

    // If the child sender has not already been adapted to be a stream sender,
    // we adapt it now.
    if constexpr (!::cuda::__is_specialization_of_v<decltype(__child), __stream::__sndr_t>)
    {
      auto __adapted_sndr    = __stream::__adapt(static_cast<__child_t&&>(__child));
      using __adapted_sndr_t = decltype(__adapted_sndr);
      return execution::schedule_from(
        __sched, __sndr_t<__adapted_sndr_t>{{}, {}, static_cast<__adapted_sndr_t&&>(__adapted_sndr)});
    }
    else
    {
      return execution::schedule_from(__sched, __sndr_t<decltype(__child)>{{}, {}, static_cast<__child_t&&>(__child)});
    }
  }
};
} // namespace __stream

template <>
struct stream_domain::__apply_t<continues_on_t> : __stream::__continues_on_t
{};

template <class _Sndr>
inline constexpr size_t structured_binding_size<__stream::__continues_on_t::__sndr_t<_Sndr>> = 3;
} // namespace cuda::experimental::execution

#include <uscl/experimental/__execution/epilogue.cuh>

#endif // __CUDAX_EXECUTION_STREAM_CONTINUES_ON
