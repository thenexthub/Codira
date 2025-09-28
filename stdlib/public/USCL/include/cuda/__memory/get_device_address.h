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

#ifndef _CUDA___MEMORY_GET_DEVICE_ADDRESS_H
#define _CUDA___MEMORY_GET_DEVICE_ADDRESS_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#if _CCCL_HAS_CTK()

#  include <cuda/std/__cuda/api_wrapper.h>
#  include <cuda/std/__memory/addressof.h>

#  include <nv/target>

#  include <cuda/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

//! @brief Returns the device address of the passed \c __device_object
//! @param __device_object the object residing in device memory
//! @return Valid pointer to the device object
template <class _Tp>
[[nodiscard]] _CCCL_API inline _Tp* get_device_address(_Tp& __device_object)
{
  NV_IF_ELSE_TARGET(
    NV_IS_DEVICE,
    (return ::cuda::std::addressof(__device_object);),
    (void* __device_ptr = nullptr; _CCCL_TRY_CUDA_API(
       ::cudaGetSymbolAddress,
       "failed to call cudaGetSymbolAddress in cuda::get_device_address",
       &__device_ptr,
       __device_object);
     return static_cast<_Tp*>(__device_ptr);))
}

_CCCL_END_NAMESPACE_CUDA

#  include <cuda/std/__cccl/epilogue.h>

#endif // _CCCL_HAS_CTK()

#endif // _CUDA___MEMORY_GET_DEVICE_ADDRESS_H
