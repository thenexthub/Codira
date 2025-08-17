//===--- HIPUtility.h - Common HIP Tool Chain Utilities ---------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_HIPUTILITY_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_HIPUTILITY_H

#include "language/Core/Driver/Tool.h"

namespace language::Core {
namespace driver {
namespace tools {
namespace HIP {

// Construct command for creating HIP fatbin.
void constructHIPFatbinCommand(Compilation &C, const JobAction &JA,
                               StringRef OutputFileName,
                               const InputInfoList &Inputs,
                               const toolchain::opt::ArgList &TCArgs, const Tool &T);

// Construct command for creating Object from HIP fatbin.
void constructGenerateObjFileFromHIPFatBinary(
    Compilation &C, const InputInfo &Output, const InputInfoList &Inputs,
    const toolchain::opt::ArgList &Args, const JobAction &JA, const Tool &T);

} // namespace HIP
} // namespace tools
} // namespace driver
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_HIPUTILITY_H
