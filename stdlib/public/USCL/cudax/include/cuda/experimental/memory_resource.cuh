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

#ifndef __CUDAX_MEMORY_RESOURCE___
#define __CUDAX_MEMORY_RESOURCE___

// If the memory resource header was included without the experimental flag,
// tell the user to define the experimental flag.
#ifndef LIBCUDACXX_ENABLE_EXPERIMENTAL_MEMORY_RESOURCE
#  ifdef _CUDA_MEMORY_RESOURCE
#    error "To use the experimental memory resource, define LIBCUDACXX_ENABLE_EXPERIMENTAL_MEMORY_RESOURCE"
#  else // ^^^ _CUDA_MEMORY_RESOURCE ^^^ / vvv !_CUDA_MEMORY_RESOURCE vvv
#    warning "To use the experimental memory resource, define LIBCUDACXX_ENABLE_EXPERIMENTAL_MEMORY_RESOURCE"
#    define LIBCUDACXX_ENABLE_EXPERIMENTAL_MEMORY_RESOURCE
#  endif // _CUDA_MEMORY_RESOURCE
#endif // !LIBCUDACXX_ENABLE_EXPERIMENTAL_MEMORY_RESOURCE

#include <uscl/experimental/__memory_resource/any_resource.cuh>
#include <uscl/experimental/__memory_resource/device_memory_pool.cuh>
#include <uscl/experimental/__memory_resource/device_memory_resource.cuh>
#include <uscl/experimental/__memory_resource/legacy_managed_memory_resource.cuh>
#include <uscl/experimental/__memory_resource/legacy_pinned_memory_resource.cuh>
#include <uscl/experimental/__memory_resource/pinned_memory_pool.cuh>
#include <uscl/experimental/__memory_resource/pinned_memory_resource.cuh>
#include <uscl/experimental/__memory_resource/properties.cuh>
#include <uscl/experimental/__memory_resource/resource.cuh>
#include <uscl/experimental/__memory_resource/shared_resource.cuh>

#endif // __CUDAX_MEMORY_RESOURCE___
