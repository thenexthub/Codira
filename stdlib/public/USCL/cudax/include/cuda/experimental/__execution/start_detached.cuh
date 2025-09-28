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

#ifndef __CUDAX_EXECUTION_START_DETACHED
#define __CUDAX_EXECUTION_START_DETACHED

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__utility/immovable.h>
#include <uscl/std/__exception/terminate.h>

#include <uscl/experimental/__detail/utility.cuh>
#include <uscl/experimental/__execution/apply_sender.cuh>
#include <uscl/experimental/__execution/cpos.cuh>
#include <uscl/experimental/__execution/env.cuh>
#include <uscl/experimental/__execution/utility.cuh>

#include <uscl/experimental/__execution/prologue.cuh>

namespace cuda::experimental::execution
{
struct start_detached_t
{
private:
  struct _CCCL_TYPE_VISIBILITY_DEFAULT __opstate_base_t
  {};

  struct _CCCL_TYPE_VISIBILITY_DEFAULT __rcvr_t
  {
    using receiver_concept = receiver_t;

    __opstate_base_t* __opstate_;
    void (*__destroy)(__opstate_base_t*) noexcept;

    template <class... _As>
    constexpr void set_value(_As&&...) noexcept
    {
      __destroy(__opstate_);
    }

    template <class _Error>
    constexpr void set_error(_Error&&) noexcept
    {
      ::cuda::std::terminate();
    }

    constexpr void set_stopped() noexcept
    {
      __destroy(__opstate_);
    }
  };

  template <class _Sndr>
  struct _CCCL_TYPE_VISIBILITY_DEFAULT __opstate_t : __opstate_base_t
  {
    using operation_state_concept = operation_state_t;
    connect_result_t<_Sndr, __rcvr_t> __opstate_;

    static void __destroy(__opstate_base_t* __ptr) noexcept
    {
      delete static_cast<__opstate_t*>(__ptr);
    }

    _CCCL_API constexpr explicit __opstate_t(_Sndr&& __sndr)
        : __opstate_(execution::connect(static_cast<_Sndr&&>(__sndr), __rcvr_t{this, &__destroy}))
    {}

    _CCCL_IMMOVABLE(__opstate_t);

    _CCCL_API constexpr void start() noexcept
    {
      execution::start(__opstate_);
    }
  };

public:
  template <class _Sndr>
  _CCCL_API static auto apply_sender(_Sndr __sndr)
  {
    execution::start(*new __opstate_t<_Sndr>{static_cast<_Sndr&&>(__sndr)});
  }

  /// run detached.
  template <class _Sndr>
  _CCCL_NODEBUG_API void operator()(_Sndr __sndr) const
  {
    using __dom_t _CCCL_NODEBUG_ALIAS = __late_domain_of_t<_Sndr, env<>, __early_domain_of_t<_Sndr>>;
    execution::apply_sender(__dom_t{}, *this, static_cast<_Sndr&&>(__sndr));
  }
};

_CCCL_GLOBAL_CONSTANT start_detached_t start_detached{};
} // namespace cuda::experimental::execution

#include <uscl/experimental/__execution/epilogue.cuh>

#endif // __CUDAX_EXECUTION_START_DETACHED
