//===--- AlignedAllocation.h - Aligned Allocation ---------------*- C++ -*-===//
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
///
/// \file
/// Defines a function that returns the minimum OS versions supporting
/// C++17's aligned allocation functions.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_ALIGNEDALLOCATION_H
#define LANGUAGE_CORE_BASIC_ALIGNEDALLOCATION_H

#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/VersionTuple.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {

inline toolchain::VersionTuple alignedAllocMinVersion(toolchain::Triple::OSType OS) {
  switch (OS) {
  default:
    break;
  case toolchain::Triple::Darwin:
  case toolchain::Triple::MacOSX: // Earliest supporting version is 10.13.
    return toolchain::VersionTuple(10U, 13U);
  case toolchain::Triple::IOS:
  case toolchain::Triple::TvOS: // Earliest supporting version is 11.0.0.
    return toolchain::VersionTuple(11U);
  case toolchain::Triple::WatchOS: // Earliest supporting version is 4.0.0.
    return toolchain::VersionTuple(4U);
  case toolchain::Triple::ZOS:
    return toolchain::VersionTuple(); // All z/OS versions have no support.
  }

  toolchain_unreachable("Unexpected OS");
}

} // end namespace language::Core

#endif // LANGUAGE_CORE_BASIC_ALIGNEDALLOCATION_H
