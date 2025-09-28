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

// IMPORTANT: This file intentionally lacks a header guard.

#if !defined(_CUDAX_ASYNC_PROLOGUE_INCLUDED)
#  error epilogue.cuh included without a prior inclusion of prologue.cuh
#endif

#undef _CUDAX_ASYNC_PROLOGUE_INCLUDED

#if _CCCL_CUDA_COMPILER(NVHPC)
_CCCL_END_NV_DIAG_SUPPRESS()
#endif // _CCCL_CUDA_COMPILER(NVHPC)

_CCCL_DIAG_POP
#include <uscl/std/__cccl/epilogue.h>
