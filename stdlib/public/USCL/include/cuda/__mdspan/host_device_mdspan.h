/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Sunday, April 14, 2024.
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

#ifndef _CUDA___MDSPAN_HOST_DEVICE_MDSPAN_H
#define _CUDA___MDSPAN_HOST_DEVICE_MDSPAN_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__mdspan/host_device_accessor.h>
#include <uscl/std/mdspan>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

template <typename _ElementType,
          typename _Extents,
          typename _LayoutPolicy   = ::cuda::std::layout_right,
          typename _AccessorPolicy = ::cuda::std::default_accessor<_ElementType>>
using host_mdspan = ::cuda::std::mdspan<_ElementType, _Extents, _LayoutPolicy, host_accessor<_AccessorPolicy>>;

template <typename _ElementType,
          typename _Extents,
          typename _LayoutPolicy   = ::cuda::std::layout_right,
          typename _AccessorPolicy = ::cuda::std::default_accessor<_ElementType>>
using device_mdspan = ::cuda::std::mdspan<_ElementType, _Extents, _LayoutPolicy, device_accessor<_AccessorPolicy>>;

template <typename _ElementType,
          typename _Extents,
          typename _LayoutPolicy   = ::cuda::std::layout_right,
          typename _AccessorPolicy = ::cuda::std::default_accessor<_ElementType>>
using managed_mdspan = ::cuda::std::mdspan<_ElementType, _Extents, _LayoutPolicy, managed_accessor<_AccessorPolicy>>;

/***********************************************************************************************************************
 * Accessibility Traits
 **********************************************************************************************************************/

template <typename _Tp, typename _Ep, typename _Lp, typename _Ap>
inline constexpr bool is_host_accessible_v<::cuda::std::mdspan<_Tp, _Ep, _Lp, _Ap>> = is_host_accessible_v<_Ap>;

template <typename _Tp, typename _Ep, typename _Lp, typename _Ap>
inline constexpr bool is_device_accessible_v<::cuda::std::mdspan<_Tp, _Ep, _Lp, _Ap>> = is_device_accessible_v<_Ap>;

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___MDSPAN_HOST_DEVICE_MDSPAN_H
