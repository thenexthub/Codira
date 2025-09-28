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

#ifndef _CUDA_STD___EXCEPTION_TERMINATE_H
#define _CUDA_STD___EXCEPTION_TERMINATE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/cstdlib> // ::exit

#include <uscl/std/__cccl/prologue.h>

_CCCL_DIAG_PUSH
_CCCL_DIAG_SUPPRESS_MSVC(4702) // unreachable code

_CCCL_BEGIN_NAMESPACE_CUDA_STD_NOVERSION // purposefully not using versioning namespace

[[noreturn]] _CCCL_API inline void __cccl_terminate() noexcept
{
  NV_IF_ELSE_TARGET(NV_IS_HOST, (::exit(-1);), (__trap();))
  _CCCL_UNREACHABLE();
}

#if 0 // Expose once atomic is universally available

using terminate_handler = void (*)();

#  ifdef __CUDA_ARCH__
__device__
#  endif // __CUDA_ARCH__
  static _CCCL_CONSTINIT ::cuda::std::atomic<terminate_handler>
    __cccl_terminate_handler{&__cccl_terminate};

_CCCL_API inline  terminate_handler set_terminate(terminate_handler __func) noexcept
{
  return __cccl_terminate_handler.exchange(__func);
}
_CCCL_API inline  terminate_handler get_terminate() noexcept
{
  return __cccl_terminate_handler.load(__func);
}

#endif

[[noreturn]] _CCCL_API inline void terminate() noexcept
{
  __cccl_terminate();
  _CCCL_UNREACHABLE();
}

_CCCL_END_NAMESPACE_CUDA_STD_NOVERSION

_CCCL_DIAG_POP

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___EXCEPTION_TERMINATE_H
