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

#ifndef _CUDA_STD___EXCEPTION_CUDA_ERROR_H
#define _CUDA_STD___EXCEPTION_CUDA_ERROR_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__exception/terminate.h>
#include <uscl/std/source_location>

#if !_CCCL_COMPILER(NVRTC)
#  include <cstdio>
#  include <stdexcept>
#endif // !_CCCL_COMPILER(NVRTC)

#include <nv/target>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

#if _CCCL_HAS_CTK()
using __cuda_error_t = ::cudaError_t;
#else
using __cuda_error_t = int;
#endif

#if _CCCL_HAS_EXCEPTIONS()

namespace __detail
{

struct __msg_storage
{
  char __buffer[512]{0};
};

static char* __format_cuda_error(
  __msg_storage& __msg_buffer,
  const int __status,
  const char* __msg,
  const char* __api                  = nullptr,
  ::cuda::std::source_location __loc = ::cuda::std::source_location::current()) noexcept
{
  ::snprintf(
    __msg_buffer.__buffer,
    512,
    "%s:%d %s%s%s(%d): %s",
    __loc.file_name(),
    __loc.line(),
    __api ? __api : "",
    __api ? " " : "",
#  if _CCCL_HAS_CTK()
    ::cudaGetErrorString(::cudaError_t(__status)),
#  else // ^^^ _CCCL_HAS_CTK() ^^^ / vvv !_CCCL_HAS_CTK() vvv
    "cudaError",
#  endif // ^^^ !_CCCL_HAS_CTK() ^^^
    __status,
    __msg);
  return __msg_buffer.__buffer;
}

} // namespace __detail

/**
 * @brief Exception thrown when a CUDA error is encountered.
 */
class cuda_error : public ::std::runtime_error
{
public:
  cuda_error(const __cuda_error_t __status,
             const char* __msg,
             const char* __api                    = nullptr,
             ::cuda::std::source_location __loc   = ::cuda::std::source_location::current(),
             __detail::__msg_storage __msg_buffer = {}) noexcept
      : ::std::runtime_error(__detail::__format_cuda_error(__msg_buffer, __status, __msg, __api, __loc))
      , __status_(__status)
  {}

  [[nodiscard]] auto status() const noexcept -> __cuda_error_t
  {
    return __status_;
  }

private:
  __cuda_error_t __status_;
};

[[noreturn]] _CCCL_API inline void __throw_cuda_error(
  [[maybe_unused]] const __cuda_error_t __status,
  [[maybe_unused]] const char* __msg,
  [[maybe_unused]] const char* __api                  = nullptr,
  [[maybe_unused]] ::cuda::std::source_location __loc = ::cuda::std::source_location::current())
{
#  if _CCCL_CUDA_COMPILATION()
  NV_IF_ELSE_TARGET(NV_IS_HOST,
                    (::cudaGetLastError(); // clear CUDA error state
                     throw ::cuda::cuda_error(__status, __msg, __api, __loc);), //
                    (::cuda::std::terminate();))
#  else // ^^^ _CCCL_CUDA_COMPILATION() ^^^ / vvv !_CCCL_CUDA_COMPILATION() vvv
  throw ::cuda::cuda_error(__status, __msg, __api, __loc);
#  endif // !_CCCL_CUDA_COMPILATION()
}
#else // ^^^ _CCCL_HAS_EXCEPTIONS() ^^^ / vvv !_CCCL_HAS_EXCEPTIONS() vvv
class cuda_error
{
public:
  _CCCL_API inline cuda_error(const __cuda_error_t,
                              const char*,
                              const char*                  = nullptr,
                              ::cuda::std::source_location = ::cuda::std::source_location::current()) noexcept
  {}
};

[[noreturn]] _CCCL_API inline void __throw_cuda_error(
  const __cuda_error_t,
  const char*,
  const char*                  = nullptr,
  ::cuda::std::source_location = ::cuda::std::source_location::current())
{
  ::cuda::std::terminate();
}
#endif // !_CCCL_HAS_EXCEPTIONS()

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___EXCEPTION_CUDA_ERROR_H
