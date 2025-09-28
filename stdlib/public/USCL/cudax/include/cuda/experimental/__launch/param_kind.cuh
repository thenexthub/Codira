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

#ifndef _CUDAX__LAUNCH_PARAM_KIND_CUH
#define _CUDAX__LAUNCH_PARAM_KIND_CUH

#include <uscl/__cccl_config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/maybe_const.h>

#include <uscl/experimental/__detail/utility.cuh>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{
namespace __detail
{
enum class __param_kind : unsigned
{
  _in    = 1,
  _out   = 2,
  _inout = 3
};

[[nodiscard]] _CCCL_HOST_DEVICE inline constexpr __param_kind operator&(__param_kind __a, __param_kind __b) noexcept
{
  return __param_kind(unsigned(__a) & unsigned(__b));
}

template <typename _Ty, __param_kind _Kind>
struct [[nodiscard]] __box
{
  ::cuda::std::__maybe_const<_Kind == __param_kind::_in, _Ty>& __val;
};

struct __in_t
{
  template <class _Ty>
  __box<_Ty, __param_kind::_in> operator()(const _Ty& __v) const noexcept
  {
    return {__v};
  }
};

struct __out_t
{
  template <class _Ty>
  __box<_Ty, __param_kind::_out> operator()(_Ty& __v) const noexcept
  {
    return {__v};
  }
};

struct __inout_t
{
  template <class _Ty>
  __box<_Ty, __param_kind::_inout> operator()(_Ty& __v) const noexcept
  {
    return {__v};
  }
};

} // namespace __detail

_CCCL_GLOBAL_CONSTANT __detail::__in_t in{};
_CCCL_GLOBAL_CONSTANT __detail::__out_t out{};
_CCCL_GLOBAL_CONSTANT __detail::__inout_t inout{};

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDAX__LAUNCH_PARAM_KIND_CUH
