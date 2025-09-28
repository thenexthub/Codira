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

#ifndef __CCCL_CUDA_CAPABILITIES
#define __CCCL_CUDA_CAPABILITIES

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__cccl/cuda_toolkit.h>

#include <nv/target>

#ifdef _CCCL_DOXYGEN_INVOKED // Only parse this during doxygen passes:
//! When this macro is defined, Programmatic Dependent Launch (PDL) is disabled across CCCL
#  define CCCL_DISABLE_PDL
#endif // _CCCL_DOXYGEN_INVOKED

#ifdef CCCL_DISABLE_PDL
#  define _CCCL_HAS_PDL() 0
#else // CCCL_DISABLE_PDL
#  define _CCCL_HAS_PDL() 1
#endif // CCCL_DISABLE_PDL

#if _CCCL_HAS_PDL()
// Waits for the previous kernel to complete (when it reaches its final membar). Should be put before the first global
// memory access in a kernel.
#  define _CCCL_PDL_GRID_DEPENDENCY_SYNC() NV_IF_TARGET(NV_PROVIDES_SM_90, cudaGridDependencySynchronize();)
// Allows the subsequent kernel in the same stream to launch. Can be put anywhere in a kernel.
// Heuristic(ahendriksen): put it after the last load.
#  define _CCCL_PDL_TRIGGER_NEXT_LAUNCH() NV_IF_TARGET(NV_PROVIDES_SM_90, cudaTriggerProgrammaticLaunchCompletion();)
#else // _CCCL_HAS_PDL()
#  define _CCCL_PDL_GRID_DEPENDENCY_SYNC()
#  define _CCCL_PDL_TRIGGER_NEXT_LAUNCH()
#endif // _CCCL_HAS_PDL()

#endif // __CCCL_CUDA_CAPABILITIES
