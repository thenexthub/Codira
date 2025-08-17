//===--- Cuda.h - Utilities for compiling CUDA code  ------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_CUDA_H
#define LANGUAGE_CORE_BASIC_CUDA_H

#include "language/Core/Basic/OffloadArch.h"

namespace toolchain {
class StringRef;
class Twine;
class VersionTuple;
} // namespace toolchain

namespace language::Core {

enum class CudaVersion {
  UNKNOWN,
  CUDA_70,
  CUDA_75,
  CUDA_80,
  CUDA_90,
  CUDA_91,
  CUDA_92,
  CUDA_100,
  CUDA_101,
  CUDA_102,
  CUDA_110,
  CUDA_111,
  CUDA_112,
  CUDA_113,
  CUDA_114,
  CUDA_115,
  CUDA_116,
  CUDA_117,
  CUDA_118,
  CUDA_120,
  CUDA_121,
  CUDA_122,
  CUDA_123,
  CUDA_124,
  CUDA_125,
  CUDA_126,
  CUDA_128,
  CUDA_129,
  FULLY_SUPPORTED = CUDA_128,
  PARTIALLY_SUPPORTED =
      CUDA_129, // Partially supported. Proceed with a warning.
  NEW = 10000,  // Too new. Issue a warning, but allow using it.
};
const char *CudaVersionToString(CudaVersion V);
// Input is "Major.Minor"
CudaVersion CudaStringToVersion(const toolchain::Twine &S);

enum class CUDAFunctionTarget {
  Device,
  Global,
  Host,
  HostDevice,
  InvalidTarget
};

/// Get the earliest CudaVersion that supports the given OffloadArch.
CudaVersion MinVersionForOffloadArch(OffloadArch A);

/// Get the latest CudaVersion that supports the given OffloadArch.
CudaVersion MaxVersionForOffloadArch(OffloadArch A);

//  Various SDK-dependent features that affect CUDA compilation
enum class CudaFeature {
  // CUDA-9.2+ uses a new API for launching kernels.
  CUDA_USES_NEW_LAUNCH,
  // CUDA-10.1+ needs explicit end of GPU binary registration.
  CUDA_USES_FATBIN_REGISTER_END,
};

CudaVersion ToCudaVersion(toolchain::VersionTuple);
bool CudaFeatureEnabled(toolchain::VersionTuple, CudaFeature);
bool CudaFeatureEnabled(CudaVersion, CudaFeature);

} // namespace language::Core

#endif
