//===--- LoongArch.h - LoongArch-specific Tool Helpers ----------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_ARCH_LOONGARCH_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_ARCH_LOONGARCH_H

#include "language/Core/Driver/Driver.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Option/Option.h"

namespace language::Core {
namespace driver {
namespace tools {
namespace loongarch {
void getLoongArchTargetFeatures(const Driver &D, const toolchain::Triple &Triple,
                                const toolchain::opt::ArgList &Args,
                                std::vector<toolchain::StringRef> &Features);

StringRef getLoongArchABI(const Driver &D, const toolchain::opt::ArgList &Args,
                          const toolchain::Triple &Triple);

std::string postProcessTargetCPUString(const std::string &CPU,
                                       const toolchain::Triple &Triple);

std::string getLoongArchTargetCPU(const toolchain::opt::ArgList &Args,
                                  const toolchain::Triple &Triple);
} // end namespace loongarch
} // end namespace tools
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_ARCH_LOONGARCH_H
