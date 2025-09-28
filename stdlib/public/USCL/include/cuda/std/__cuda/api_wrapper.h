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

#ifndef _CUDA__STD__CUDA_API_WRAPPER_H
#define _CUDA__STD__CUDA_API_WRAPPER_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__exception/cuda_error.h>

#define _CCCL_TRY_CUDA_API(_NAME, _MSG, ...)                        \
  do                                                                \
  {                                                                 \
    const ::cudaError_t __status = _NAME(__VA_ARGS__);              \
    switch (__status)                                               \
    {                                                               \
      case ::cudaSuccess:                                           \
        break;                                                      \
      default:                                                      \
        /* CUDA error state is cleared inside __throw_cuda_error */ \
        ::cuda::__throw_cuda_error(__status, _MSG, #_NAME);         \
    }                                                               \
  } while (0)

#define _CCCL_ASSERT_CUDA_API(_NAME, _MSG, ...)                         \
  do                                                                    \
  {                                                                     \
    [[maybe_unused]] const ::cudaError_t __status = _NAME(__VA_ARGS__); \
    ::cudaGetLastError(); /* clear CUDA error state */                  \
    _CCCL_ASSERT(__status == cudaSuccess, _MSG);                        \
  } while (0)

#define _CCCL_LOG_CUDA_API(_NAME, _MSG, ...)                                       \
  [&]() {                                                                          \
    const ::cudaError_t __status = _NAME(__VA_ARGS__);                             \
    if (__status != ::cudaSuccess)                                                 \
    {                                                                              \
      ::cuda::__detail::__msg_storage __msg_buffer;                                \
      ::cuda::__detail::__format_cuda_error(__msg_buffer, __status, _MSG, #_NAME); \
      ::fprintf(stderr, "%s\n", __msg_buffer.__buffer);                            \
      ::fflush(stderr);                                                            \
    }                                                                              \
    ::cudaGetLastError(); /* clear CUDA error state */                             \
    return __status;                                                               \
  }()

#endif //_CUDA__STD__CUDA_API_WRAPPER_H
